/*
TODOs:
[x] Inform program of expected number of list elements
[x] Clean properties request API
[x] Ascii format reading
[x]   Add ascii format size calculation for list properties
[x] Ply creation API
  [x] Add multiple properties add command
  [x] Add actual writing of files
[x] Add ascii output
[x] Add big-endian output support
[x] Write data calculating function (non-destructive)
[x] Simplify read api - symmetrize it with wrtie api.
[ ] Add big-endian input support
    [ ] Try reading in lucy
[ ] Prepare different data layouts people might have.
[ ] Add regular properties optimizations
[ ] Add hints optimizations

[ ] Encoder/decorder split
[ ] Double vs. float - Add casting support
    [ ] Rebenchmark lucy
[ ] Error reporting
[ ] Revise errors
[ ] Code cleanup
  [ ] Replace duplicated code
  [ ] Decide what to do with malloc(ie. Should we alloc memory? We are using FILE* anyway..)
[ ] Fix the header names to be mply
[ ] Replace msh_array with buf.c or with pervogsen buf (https://gist.github.com/vurtun/5144bbcc2db73d51e36bf327ac19b604)
[ ] Extensive testing
[ ] Asserts!
[ ] Optimize / Simplyfy code
*/

#include <stdlib.h>
#include <stdint.h>
#define MSH_IMPLEMENTATION
#include "msh.h"

// API does not allow mixing regular and list properties


////////////////////////////////////////////////////////////////////////////////////////////////////
// This ply library only provides set of functionalities to read/write your own specific ply file.
// pf_file_t does not actually store any mesh data. You need to provide buffers to which such data
// can be written.
// This code assumes that your computer is little endian
////////////////////////////////////////////////////////////////////////////////////////////////////
// All i am doing here is jumping through hoops if data is not interleaved.
// Maybe I should assume it is, and if it is not, create separate interlevard buffers and write them...

// Like the question I should be answering is what is the best data format for the ply
// file to be writen / read easily. And then provide functions transforming to that format.
#define PLY_MAX_STR_LEN 1024

// the variables are non-descriptive here
typedef struct ply_property
{
  char name[64];

  int type;
  int list_type;

  // Should we store those or calculate on the fly
  int byte_size;
  int list_byte_size;

  int total_count;
  int total_byte_size;

  // data storage -> maybe we will put that into some write helper
  void* data;
  void* list_data;

  int offset;
  int stride;
  
  int list_offset;
  int list_stride;

  int list_count;
} ply_property_t;

typedef struct ply_element
{
  char name[64];
  int count;
  msh_array(ply_property_t) properties;
  
  int file_anchor;
  void* data;
  uint64_t data_size;
} ply_element_t;

typedef struct ply_hint
{
  char* property_name;
  int expected_size;
} ply_hint_t;

typedef struct ply_file
{
  int valid;
  int format;
  int format_version;

  ply_element_t* cur_element;
  msh_array(ply_element_t) elements;

  FILE* _fp;
  int _header_size;
  msh_array(ply_hint_t) _hints;
} ply_file_t;

////////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////////

typedef enum ply_err
{
  PLY_NO_ERRORS = 0,
  PLY_INVALID_FILE_ERR,
  PLY_INVALID_FORMAT_ERR,
  PLY_FILE_OPEN_ERR,
  PLY_FILE_NOT_OPEN_ERR,
  PLY_LINE_PARSE_ERR,
  PLY_FORMAT_CMD_ERR,
  PLY_ELEMENT_CMD_ERR,
  PLY_PROPERTY_CMD_ERR,
  PLY_ELEMENT_NOT_FOUND_ERR,
  PLY_PROPERTY_NOT_FOUND_ERR,
  PLY_REQUESTED_PROPERTY_NOT_A_LIST_ERR,
  PLY_PARSE_ERROR,
  PLY_BIG_ENDIAN_ERR,
  PLY_BINARY_FILE_READ_ERR,
  PLY_CONFLICTING_NUMBER_OF_ELEMENTS_ERR,
  PLY_UNRECOGNIZED_FORMAT_ERR,
  PLY_UNRECOGNIZED_CMD_ERR,
  PLY_UNSUPPORTED_FORMAT_ERR,
  PLY_INVALID_LIST_TYPE_ERR 
} ply_err_t;

typedef enum ply_format
{
  PLY_ASCII = 0,
  PLY_LITTLE_ENDIAN,
  PLY_BIG_ENDIAN
} ply_format_t;

typedef enum ply_type_id
{
  PLY_INVALID,
  PLY_INT8,
  PLY_UINT8,
  PLY_INT16,
  PLY_UINT16,
  PLY_INT32,
  PLY_UINT32,
  PLY_FLOAT,
  PLY_DOUBLE,
  PLY_N_TYPES
} ply_type_id_t;

// Endian swaps from tinyply by Dimitri Diakopoulos
inline uint16_t ply_file__swap_uint16(uint16_t v)
{ 
  return (v << 8) | (v >> 8); 
}

inline uint32_t ply_file__swap_uint32(uint32_t v)
{ 
  return  (v << 24) | ((v << 8) & 0x00ff0000) | 
          ((v >> 8) & 0x0000ff00) | (v >> 24); 
}

inline uint64_t ply_file__swap_uint64(uint64_t v)
{
  return (((v & 0x00000000000000ffLL) << 56) |
          ((v & 0x000000000000ff00LL) << 40) |
          ((v & 0x0000000000ff0000LL) << 24) |
          ((v & 0x00000000ff000000LL) << 8) |
          ((v & 0x000000ff00000000LL) >> 8) |
          ((v & 0x0000ff0000000000LL) >> 24) |
          ((v & 0x00ff000000000000LL) >> 40) |
          ((v & 0xff00000000000000LL) >> 56));
}

inline int16_t ply_file__swap_int16(int16_t v)
{ 
  return (v << 8) | (v >> 8); 
}

inline int32_t ply_file__swap_int32(int32_t v) 
{ 
  return  (v << 24) | ((v << 8) & 0x00ff0000) | 
          ((v >> 8) & 0x0000ff00) | (v >> 24); 
}

inline float swap_float(float v) 
{ 
  union { float f; uint32_t i; } u; 
  u.f = v; 
  u.i = ply_file__swap_uint32(u.i); 
  return u.f; 
}

inline double swap_double(double v) 
{ 
  union { double d; uint64_t i; } u; 
  u.d = v; 
  u.i = ply_file__swap_uint64(v); 
  return u.d; 
}


static char* ply_type_str[PLY_N_TYPES] = {"%s","%c","%uc","%s","%us","%d","%ud","%f","%g"};


ply_element_t*
ply_file_find_element( const ply_file_t* pf, const char* element_name )
{
  ply_element_t* el = NULL;
  for(int i = 0; i < msh_array_size(pf->elements); ++i)
  {
    if(!strcmp(element_name, pf->elements[i].name))
    {
      el = &pf->elements[i];
      break;
    }
  }
  return el;
}

ply_property_t*
ply_file_find_property( const ply_element_t* el, const char* property_name)
{
  int err_code = PLY_NO_ERRORS;

  ply_property_t* pr = NULL;
  for(int i=0; i < msh_array_size(el->properties) ; ++i)
  {
    if(!strcmp(el->properties[i].name, property_name)) { pr = &el->properties[i]; break; }
  }

  return pr;
}

int
ply_file__get_data_as_int( void* data, int type )
{
  int retval = 0;
  switch( type )
  {
    case PLY_INT8:  retval  = (int)((char*)data)[0]; break;
    case PLY_INT16: retval  = (int)((short*)data)[0]; break;
    case PLY_INT32: retval  = (int)((int*)data)[0]; break;
    case PLY_UINT8:  retval = (int)((char*)data)[0]; break;
    case PLY_UINT16: retval = (int)((short*)data)[0]; break;
    case PLY_UINT32: retval = (int)((int*)data)[0]; break;
    default: retval = 0; break;
  }
  return retval;
}


int ply_file__type_to_byte_size( ply_type_id_t type )
{
  int retval = 0;
  switch( type )
  {
    case PLY_INT8:   { retval = 1; break; }
    case PLY_INT16:  { retval = 2; break; }
    case PLY_INT32:  { retval = 4; break; }
    case PLY_UINT8:  { retval = 1; break; }
    case PLY_UINT16: { retval = 2; break; }
    case PLY_UINT32: { retval = 4; break; }
    case PLY_FLOAT:  { retval = 4; break; }
    case PLY_DOUBLE: { retval = 8; break; }
    default: { retval = 0; break; }
  }
  return retval;
}

void
ply_file__property_type_to_string( ply_type_id_t type, char** string )
{
  switch( type )
  {
    case PLY_INT8:   { *string = "char"; break; }
    case PLY_INT16:  { *string = "short"; break; }
    case PLY_INT32:  { *string = "int"; break; }
    case PLY_UINT8:  { *string = "uchar"; break; }
    case PLY_UINT16: { *string = "ushort"; break; }
    case PLY_UINT32: { *string = "uint"; break; }
    case PLY_FLOAT:  { *string = "float"; break; }
    case PLY_DOUBLE: { *string = "double"; break; }
    default: { break; }
  }
}


void
ply_file__string_to_property_type(char* type_str, int* pr_type, int* pr_size)
{
  if      (!strcmp("int8",    type_str) || !strcmp("char",   type_str)) {*pr_type=PLY_INT8;   *pr_size=1;} 
  else if (!strcmp("uint8",   type_str) || !strcmp("uchar",  type_str)) {*pr_type=PLY_UINT8;  *pr_size=1;} 
  else if (!strcmp("int16",   type_str) || !strcmp("short",  type_str)) {*pr_type=PLY_INT16;  *pr_size=2;} 
  else if (!strcmp("uint16",  type_str) || !strcmp("ushort", type_str)) {*pr_type=PLY_UINT16; *pr_size=2;} 
  else if (!strcmp("int32",   type_str) || !strcmp("int",    type_str)) {*pr_type=PLY_INT32;  *pr_size=4;} 
  else if (!strcmp("uint32",  type_str) || !strcmp("uint",   type_str)) {*pr_type=PLY_UINT32; *pr_size=4;} 
  else if (!strcmp("float32", type_str) || !strcmp("float",  type_str)) {*pr_type=PLY_FLOAT;  *pr_size=4;} 
  else if (!strcmp("float64", type_str) || !strcmp("double", type_str)) {*pr_type=PLY_DOUBLE; *pr_size=8;} 
  else { *pr_type = PLY_INVALID; *pr_size = 0;}
}


int
ply_file__parse_ply_cmd(char* line, ply_file_t* pf)
{
  char cmd_str[PLY_MAX_STR_LEN];
  if(sscanf(line, "%s", cmd_str))
  { 
    if(!strcmp(cmd_str, "ply"))
    {
      pf->valid = true; 
      return PLY_NO_ERRORS; 
    }
  }
  return PLY_INVALID_FILE_ERR;
}

int
ply_file__parse_format_cmd(char* line, ply_file_t* pf)
{
  char cmd[PLY_MAX_STR_LEN];
  char frmt_str[PLY_MAX_STR_LEN];
  char frmt_ver_str[PLY_MAX_STR_LEN];
  if(sscanf(line, "%s %s %s", &cmd[0], &frmt_str[0], &frmt_ver_str[0]) != 3) { return PLY_FORMAT_CMD_ERR; } 
  if(!strcmp("ascii", frmt_str)){ pf->format = (int)PLY_ASCII; }
  else if(!strcmp("binary_little_endian", frmt_str)){ pf->format = (int)PLY_LITTLE_ENDIAN; }
  else if(!strcmp("binary_big_endian", frmt_str)){ pf->format = (int)PLY_BIG_ENDIAN; }
  else{ return PLY_UNRECOGNIZED_FORMAT_ERR; }
  pf->format_version = atoi(frmt_ver_str);
  return PLY_NO_ERRORS;
}

int
ply_file__parse_element_cmd(char* line, ply_file_t* pf)
{
  char cmd[PLY_MAX_STR_LEN];
  ply_element_t el = {0};
  el.properties = NULL;
  if(sscanf(line, "%s %s %d", &cmd[0], &el.name[0], &el.count) != 3) { return PLY_ELEMENT_CMD_ERR; }
  msh_array_push(pf->elements, el);
  pf->cur_element = msh_array_back(pf->elements);
  return PLY_NO_ERRORS;
}

int
ply_file__parse_property_cmd(char* line, ply_file_t* pf)
{
  char cmd[PLY_MAX_STR_LEN];
  char type_str[PLY_MAX_STR_LEN];
  char list_str[PLY_MAX_STR_LEN];
  char list_type_str[PLY_MAX_STR_LEN];
  int valid_format = false;

  ply_element_t* el = pf->cur_element;
  ply_property_t pr;
  
  // Try to parse regular property format
  if(sscanf(line, "%s %s %s", &cmd[0], &type_str[0], (char*)&pr.name) == 3) 
  {
    ply_file__string_to_property_type(type_str, &pr.type, &pr.byte_size);
    pr.list_type = PLY_INVALID;
    pr.list_byte_size = 0;
    pr.list_count = 1;
    valid_format = true;
  }
  
  // Try to parse list property format
  cmd[0] = 0; type_str[0] = 0; list_str[0] = 0; list_type_str[0] = 0;
  if (sscanf(line, "%s %s %s %s %s", &cmd[0], &list_str[0], &list_type_str[0], &type_str[0], (char*)&pr.name) == 5)
  {
    if(strcmp(list_str, "list")) { return PLY_PROPERTY_CMD_ERR; }
    ply_file__string_to_property_type(type_str, &pr.type, &pr.byte_size);
    ply_file__string_to_property_type(list_type_str, &pr.list_type, &pr.list_byte_size);
    pr.list_count = -1; //UNKNOWN (TODO: Add maybe enum to indicate this?)
    valid_format = true;
  }
  
  pr.total_byte_size = 0;
  pr.total_count = 0;
  // Both failed
  if(!valid_format) { return PLY_PROPERTY_CMD_ERR; }
  
  // Either succeded
  msh_array_push(el->properties, pr);
  return PLY_NO_ERRORS;
}

int 
ply_file__parse_command(char* cmd, char* line, ply_file_t* pf)
{
  if(!strcmp(cmd, "ply"))      { return ply_file__parse_ply_cmd(line, pf); }
  if(!strcmp(cmd, "format"))   { return ply_file__parse_format_cmd(line, pf); }
  if(!strcmp(cmd, "element"))  { return ply_file__parse_element_cmd(line, pf); }
  if(!strcmp(cmd, "property")) { return ply_file__parse_property_cmd(line, pf); }
  return PLY_UNRECOGNIZED_CMD_ERR;
}

int
ply_file_parse_header(ply_file_t* pf)
{
  int line_no = 0;
  char line[PLY_MAX_STR_LEN];
  int err_code = 0;
  while(fgets(&line[0], PLY_MAX_STR_LEN, pf->_fp))
  {
    line_no++;
    char cmd[PLY_MAX_STR_LEN];
    if(sscanf(line, "%s", cmd)!=(unsigned) 1) { return PLY_LINE_PARSE_ERR; }
    if(line_no == 1 && strcmp(cmd,"ply") !=0 ) { return PLY_INVALID_FILE_ERR; }
    if(!strcmp(cmd,"end_header")) break;
    if(!strcmp(cmd,"comment")) continue;
    err_code = ply_file__parse_command(cmd, line, pf);
  }
  pf->_header_size = ftell(pf->_fp);
  return err_code;
}

int 
ply_file__calculate_elem_size_ascii(ply_file_t* pf, ply_element_t* el)
{
  char line[PLY_MAX_STR_LEN];
  while(fgets(&line[0], PLY_MAX_STR_LEN, pf->_fp))
  {
    char* cp_up  = &line[0];
    char* cp_low = &line[0];

    for(int j=0; j<msh_array_size(el->properties); ++j)
    {
      ply_property_t* pr = &el->properties[j];
      while(*cp_up!=' ' && *cp_up!='\n') {cp_up++;}
      char tmp_cp_up = *cp_up;
      *cp_up = 0; //Fake string end;

      if(pr->list_type != PLY_INVALID)
      {
        switch(pr->list_type)
        {
          case PLY_INT8:   
          case PLY_INT16:  
          case PLY_INT32:  
          case PLY_UINT8:  
          case PLY_UINT16: 
          case PLY_UINT32: pr->list_count = (int32_t)atoi(cp_low);  break;
          case PLY_FLOAT:
          case PLY_DOUBLE: pr->list_count = (int32_t)atof(cp_low);  break;
        }
        // skip
        *cp_up = tmp_cp_up;
        cp_low = cp_up;
        cp_up++;
        for(int k=0; k < pr->list_count; ++k) 
        {
          while(*cp_up!=' ' && *cp_up!='\n') {cp_up++;}
          cp_up++;
        }
      }

      // fixup pointers
      *cp_up = tmp_cp_up;
      if( *cp_up != '\n')
      {
        cp_up++;
        cp_low = cp_up;
      }
      pr->total_byte_size += pr->list_byte_size + pr->list_count * pr->byte_size;
      pr->total_count += pr->list_count;
    }
  }
 
  return PLY_NO_ERRORS;
}


int 
ply_file__calculate_elem_size_le(ply_file_t* pf, ply_element_t* el)
{
  // There exists a list property. We need to calculate required size via pass throught
  for(int i=0; i < el->count; ++i)
  {
    for(int j=0; j < msh_array_size(el->properties); ++j)
    {
      ply_property_t* pr = &el->properties[j];
      int count = 1;
      if( pr->type != PLY_INVALID ) 
      { 
        if( fread(&count, pr->list_byte_size, 1, pf->_fp ) != 1) { return PLY_PARSE_ERROR;}
        pr->total_byte_size += pr->list_byte_size;
      }
      fseek(pf->_fp, count * pr->byte_size, SEEK_CUR);
      pr->total_byte_size += count * pr->byte_size;
      pr->total_count += count;
    }
  }
  return PLY_NO_ERRORS;
}

int
ply_file_parse_contents(ply_file_t* pf)
{
  int err_code = PLY_NO_ERRORS;
  for(int i=0;i<msh_array_size(pf->elements);++i)
  {
    ply_element_t* el = &pf->elements[i];
    int n_properties = msh_array_size(el->properties);
    el->file_anchor = ftell(pf->_fp);

    // Determine if any of the properties in the element has list
    int can_precalculate_size = 1;
    for(int j=0; j<n_properties; ++j)
    {
      if( el->properties[j].list_type != PLY_INVALID ) 
      { 
        int has_hint = 0;
        for(int k=0; k<msh_array_size(pf->_hints); ++k)
        {
          if( !strcmp(el->properties[j].name, pf->_hints[k].property_name) ) 
          { 
            has_hint = 1;
            el->properties[j].list_count = pf->_hints[k].expected_size;
          };
        }
        if(!has_hint)
        {
          can_precalculate_size = 0; 
          break; 
        }
      }
    }

    if(can_precalculate_size)
    {
      // This is a faster path, as we can just calculate the size required by element in one go.
      int elem_size = 0;
      for(int j=0; j<n_properties; ++j) 
      {
        ply_property_t* pr = &el->properties[j];
        pr->total_byte_size = pr->byte_size * pr->list_count * el->count;
        elem_size += pr->byte_size * pr->list_count; 
        if(pr->list_type != PLY_INVALID )
        {
          pr->total_byte_size += pr->list_byte_size*el->count;
          elem_size += pr->list_byte_size;
        }
        pr->total_count += pr->list_count * el->count;
      }
      if(pf->format != PLY_ASCII) { fseek(pf->_fp, el->count * elem_size, SEEK_CUR); }
      else
      {
        char line[PLY_MAX_STR_LEN];
        for(int j=0; j<el->count; ++j) { fgets(&line[0], PLY_MAX_STR_LEN, pf->_fp); }
      }
    }
    else 
    {
      // There exists a list property. We need to calculate required size via pass through
      if(pf->format == PLY_ASCII )         err_code = ply_file__calculate_elem_size_ascii(pf, el);
      if(pf->format == PLY_LITTLE_ENDIAN ) err_code = ply_file__calculate_elem_size_le(pf, el);
      if(pf->format == PLY_BIG_ENDIAN )    err_code = PLY_BIG_ENDIAN_ERR;
    }
  }

  return err_code;
}

int
ply_file_get_element_count(const ply_element_t* el, int* count)
{
  int err_code = PLY_NO_ERRORS;
  *count = el->count;
  return err_code;
}

int
ply_file_get_element_size(const ply_element_t *el, uint64_t* size)
{
  int err_code = PLY_NO_ERRORS;
  
  *size = 0;
  for(int i= 0; i<msh_array_size(el->properties); ++i)
  {
    *size += el->properties[i].total_byte_size;
  }
  return err_code;
}

#define PLY_CONVERT_AND_COPY(D, C, T, conv_funct) {\
  T n = (T)conv_funct(C); \
  memcpy((void*)(D), (void*)&n, sizeof(T)); \
  (D)+=sizeof(T);}

void 
ply_file__ascii_to_value( char** dst, char* src, const int type )
{
  switch(type)
  {
    case PLY_INT8:   PLY_CONVERT_AND_COPY(*dst, src, int8_t,   atoi); break;
    case PLY_INT16:  PLY_CONVERT_AND_COPY(*dst, src, int16_t,  atoi); break;
    case PLY_INT32:  PLY_CONVERT_AND_COPY(*dst, src, int32_t,  atoi); break;
    case PLY_UINT8:  PLY_CONVERT_AND_COPY(*dst, src, uint8_t,  atoi); break;
    case PLY_UINT16: PLY_CONVERT_AND_COPY(*dst, src, uint16_t, atoi); break;
    case PLY_UINT32: PLY_CONVERT_AND_COPY(*dst, src, uint32_t, atoi); break;
    case PLY_FLOAT:  PLY_CONVERT_AND_COPY(*dst, src, float,    atof); break;
    case PLY_DOUBLE: PLY_CONVERT_AND_COPY(*dst, src, double,   atof); break;
  }
}

int
ply_file__get_element_data_ascii(ply_file_t* pf, const ply_element_t* el, void** storage)
{
  int err_code = PLY_NO_ERRORS;
  
  fseek(pf->_fp, el->file_anchor, SEEK_SET);
  char *dest = (char*)*storage;
  for(int i=0;i<el->count;++i)
  {
    char line[PLY_MAX_STR_LEN];
    fgets(line, PLY_MAX_STR_LEN, pf->_fp);
    char* cp_up  = &line[0];
    char* cp_low = &line[0];

    for(int j=0; j<msh_array_size(el->properties); ++j)
    {
      ply_property_t* pr = &el->properties[j];
      while(*cp_up!=' ' && *cp_up!='\n') {cp_up++;}
      char tmp_cp_up = *cp_up;
      *cp_up = 0; //Fake string end;
      if(pr->list_type == PLY_INVALID) // regular pr
      { 
        ply_file__ascii_to_value( &dest, cp_low, pr->type);
      }
      else //list pr
      {
        // TODO(maciej): If we have a hint, we dont need to convert the list_counter, just skip it
        ply_file__ascii_to_value( &dest, cp_low, pr->list_type);
        pr->list_count = ply_file__get_data_as_int( dest-pr->list_byte_size, pr->list_type);

        *cp_up = tmp_cp_up;
        cp_low = cp_up;
        cp_up++;
        for(int k=0; k<pr->list_count;++k) 
        {
          while(*cp_up!=' ' && *cp_up!='\n') {cp_up++;}
          tmp_cp_up = *cp_up;
          *cp_up = 0; //Fake string end;
          ply_file__ascii_to_value( &dest, cp_low, pr->type);
          *cp_up = tmp_cp_up;
          cp_low = cp_up;
          cp_up++;
        }
      }

      // fixup pointers
      *cp_up = tmp_cp_up;
      if( *cp_up != '\n')
      {
        cp_up++;
        cp_low = cp_up;
      }
    }
  }

  return err_code;
}
#undef PLY_CONVERT_AND_COPY

int
ply_file__get_element_data_le(ply_file_t* pf, const ply_element_t* el, void** storage)
{
  int err_code = PLY_NO_ERRORS;
 
  // get the size
  int size = 0;
  for(int i= 0; i<msh_array_size(el->properties); ++i)
  {
    size += el->properties[i].total_byte_size;
  }
  fseek(pf->_fp, el->file_anchor, SEEK_SET);
  if( fread(*storage, size, 1, pf->_fp ) != 1) { return PLY_PARSE_ERROR;}

  return err_code;
}

int
ply_file_get_element_data(ply_file_t* pf, const ply_element_t* el, void** storage)
{
  int err_code = PLY_NO_ERRORS;
  if(pf->format==PLY_ASCII)         err_code = ply_file__get_element_data_ascii(pf, el, storage);
  if(pf->format==PLY_LITTLE_ENDIAN) err_code = ply_file__get_element_data_le(pf, el, storage);
  if(pf->format==PLY_BIG_ENDIAN)    err_code = PLY_BIG_ENDIAN_ERR;

  return err_code;
}

int
ply_file_read(ply_file_t* pf)
{
  int error = PLY_NO_ERRORS;
  if( !pf->_fp ) { return PLY_FILE_NOT_OPEN_ERR; }

  error = ply_file_parse_header(pf);
  if( error ) { return error; }
  error = ply_file_parse_contents(pf);
  if( error ) { return error; }

  return error;
}

int
ply_file_get_properties_byte_size( ply_element_t* el, 
                              const char** properties_names, int n_properties, 
                              int* data_size, int* list_size )
{
  // find requested properties
  int n_found              = 0;
  int total_data_byte_size = 0;
  int total_list_byte_size = 0;
  for( int i=0;i<n_properties;++i)
  {
    for( int j=0;j<msh_array_size(el->properties);++j)
    {
      if( !strcmp(el->properties[j].name, properties_names[i]) )
      {
        n_found++;
        total_data_byte_size += el->properties[j].total_byte_size;
        total_list_byte_size += el->properties[j].list_byte_size;
      } 
    }
  }
  if( n_found != n_properties ) return PLY_PROPERTY_NOT_FOUND_ERR;

  total_list_byte_size *= el->count;

  *data_size = total_data_byte_size - total_list_byte_size;
  *list_size = total_list_byte_size;

  return PLY_NO_ERRORS;
}


// row size is super important
int
ply_file_get_property_from_element(ply_file_t* pf, const char* element_name, 
                                   const char** property_names, int num_properties, 
                                   int requested_type, int requested_list_type, 
                                   void** data, void** list_data, int32_t *data_count )
{
  //TODO(maciej): Errors!
  ply_element_t* el = ply_file_find_element(pf, element_name); // TODO(maciej): Make public
  if( !el ) { return PLY_ELEMENT_NOT_FOUND_ERR; }
  else
  {
    if( el->data == NULL )
    {
      ply_file_get_element_size( el, &el->data_size); // TODO(maciej): Just pass element!
      el->data = malloc(el->data_size);
      ply_file_get_element_data( pf, el, &el->data); // TODO(maciej): Just pass element!
    }
    
    // Initialize output
    *data_count = el->count;
    char* dst_data = NULL; 
    char* dst_list = NULL;
    int data_byte_size = -1;
    int list_byte_size = -1;
    ply_file_get_properties_byte_size(el, property_names, num_properties, 
                                     &data_byte_size, &list_byte_size);
    if( data != NULL ) 
    { 
      *data = malloc(data_byte_size); 
      dst_data = (char*)*data;
    }
    if( list_data != NULL )
    { 
      *list_data = malloc(list_byte_size);
      dst_list = (char*)*list_data;
    }

    // Start copying
    char* src = (char*)el->data;
    for(int i = 0; i < el->count; ++i )
    {
      int32_t dst_row_size = 0;
      int32_t dst_list_row_size = 0;
      int32_t dst_offset = 0;
      int32_t dst_list_offset = 0;
      int32_t src_row_size = 0;
      
      // Calculate the required data to read
      for(int j = 0; j < msh_array_size(el->properties); ++j)
      {
        ply_property_t* pr = &el->properties[j];
        pr->list_count = 1;
        if( pr->list_type != PLY_INVALID )
        {
          pr->list_count = ply_file__get_data_as_int(src+src_row_size, pr->list_type);
          printf("LIST_COUNT %d\n", pr->list_count);
        }

        for(int k = 0; k < num_properties; ++k)
        {
          if( !strcmp(pr->name, property_names[k]) ) 
          {
            dst_list_row_size += pr->list_byte_size;
            dst_row_size += pr->list_count * pr->byte_size;
            pr->offset = src_row_size + pr->list_byte_size;
            pr->list_offset = src_row_size;
            if( pr->list_type != PLY_INVALID ) printf("TEST %d\n", pr->list_offset);
          }
        }
        src_row_size += pr->list_byte_size + pr->list_count * pr->byte_size;
      }
      
      for(int j = 0; j < msh_array_size(el->properties); ++j)
      {
        ply_property_t* pr = &el->properties[j];
        for(int k = 0; k < num_properties; ++k)
        {
          if( !strcmp(pr->name, property_names[k]) ) 
          {
            //TODO(maciej) - replace these with casting functions
            if(dst_data) memcpy( dst_data+dst_offset, src + pr->offset, pr->list_count*pr->byte_size );
            if(dst_list) memcpy( dst_list+dst_list_offset, src + pr->list_offset, pr->list_byte_size ); 
            if(dst_list)
            { 
              printf("%d %d\n", pr->list_byte_size, ply_file__get_data_as_int(dst_list+dst_list_offset,pr->list_type) );
              printf("%d %d\n", pr->list_byte_size, ply_file__get_data_as_int(src + pr->list_offset,pr->list_type) );
              printf("%d\n", pr->list_offset);
            }
            dst_offset += pr->list_count * pr->byte_size;
            dst_list_offset += pr->list_byte_size;
          }
        }
      }
      src += src_row_size;
      if(dst_data) dst_data += dst_row_size;
      if(dst_list) dst_list += dst_list_row_size;
      printf("ROW_SIZE: %d\n", src_row_size );
    }

  }

  return PLY_NO_ERRORS;
}

// ENCODER

int  
ply_file__add_element(ply_file_t* pf, const char* element_name, const int element_count)
{
  ply_element_t el;
  strncpy( &el.name[0], element_name, 64 );
  el.count = element_count;
  el.properties = NULL;
  msh_array_push( pf->elements, el );
  return PLY_NO_ERRORS;
}


void
ply_file__fprint_data_at_offset(const ply_file_t* pf, const void* data, const int offset, const int type)
{
  switch(type)
  {
    case PLY_UINT8:  fprintf(pf->_fp,"%zd ", *(uint8_t*)(data+offset)); break;
    case PLY_UINT16: fprintf(pf->_fp,"%zd ", *(uint16_t*)(data+offset)); break;
    case PLY_UINT32: fprintf(pf->_fp,"%zd ", *(uint32_t*)(data+offset)); break;
    case PLY_INT8:   fprintf(pf->_fp,"%d ",  *(int8_t*)(data+offset)); break;
    case PLY_INT16:  fprintf(pf->_fp,"%d ",  *(int16_t*)(data+offset)); break;
    case PLY_INT32:  fprintf(pf->_fp,"%d ",  *(int32_t*)(data+offset)); break;
    case PLY_FLOAT:  fprintf(pf->_fp,"%f ",  *(float*)(data+offset)); break;
    case PLY_DOUBLE: fprintf(pf->_fp,"%f ",  *(double*)(data+offset)); break;
  }
}


int  
ply_file_add_property_to_element(ply_file_t* pf, const char* element_name, 
                  const char** property_names, int property_count,
                  const int data_type, const int list_type, void* data, void* list_data,
                  int element_count )
{
  // Check if list type is integral type
  if( list_type == PLY_FLOAT || list_type == PLY_DOUBLE )
  {
    return PLY_INVALID_LIST_TYPE_ERR;
  }

  // Find / Create element
  ply_element_t* el = ply_file_find_element(pf, element_name);
  if( !el )
  {
    ply_file__add_element(pf, element_name, element_count);
    el = msh_array_back(pf->elements);
  }

  if( el )
  {
    if( el->count != element_count ) 
    {
      return PLY_CONFLICTING_NUMBER_OF_ELEMENTS_ERR;
    }

    int init_offset = 0; // Helper variable for storing initial list offsets.
    for( int i = 0; i < property_count; ++i )
    {
      ply_property_t pr;
      strncpy(&pr.name[0], property_names[i], 64);

      pr.type = data_type;
      pr.list_type = list_type;
      pr.byte_size = ply_file__type_to_byte_size( pr.type );
      pr.list_byte_size = ply_file__type_to_byte_size( pr.list_type );
      pr.list_count = (list_data == NULL) ? 1 : 0; // TODO(If se have a hint, set this to a hint)
      pr.data = data;
      pr.list_data = list_data;
      pr.offset = pr.byte_size * i;
      pr.stride = pr.byte_size * property_count;
      pr.list_offset = pr.list_byte_size * i;
      pr.list_stride = pr.list_byte_size * property_count;
      pr.total_byte_size = (pr.list_byte_size + pr.list_count * pr.byte_size) * el->count;
     
      if( pr.list_count == 0 ) // No hint was present
      {
        pr.total_byte_size = 0;
        for( int j = 0; j < element_count; ++j )
        {
          // NOTE(list type needs to be dereferenced to correct type)
          int offset = (pr.list_offset + j * pr.list_stride);
          int list_count = ply_file__get_data_as_int(list_data+offset, list_type);
          pr.total_byte_size += pr.list_byte_size + list_count * pr.byte_size;
          if( j == 0 ) // We care only about initial offset, so first element of list counts
          {
            pr.offset = init_offset;
            init_offset += list_count * pr.byte_size;
          }
        }
      }
     
      msh_array_push( el->properties, pr );
    }
  }
  else 
  {
    return PLY_ELEMENT_NOT_FOUND_ERR;
  }
  return PLY_NO_ERRORS;
}

// NOTE(maciej): I don't really like how this works. Might need to redo it
int
ply_file__calculate_list_property_stride( const ply_property_t* pr, 
                                         msh_array(ply_property_t) el_properties )
{
  int stride = 0;
  for( int l = 0; l < msh_array_size(el_properties); ++l )
  {
    ply_property_t* qr = &el_properties[l];
    if( pr->data == qr->data )
    {
      int list_count = ply_file__get_data_as_int(qr->list_data + qr->list_offset, qr->list_type );
      stride += list_count * qr->byte_size;
      qr->list_offset += qr->list_stride;
    }
  }

  // Revert to regular
  for( int l = 0; l < msh_array_size(el_properties); ++l )
  {
    ply_property_t* qr = &el_properties[l];
    if( pr->data == qr->data )
    {
      qr->list_offset -= qr->list_stride;
    }
  }
  return ((stride > 0) ? stride : 0);
}


int
ply_file__write_header( const ply_file_t* pf )
{
  if( !pf->_fp ) { return PLY_INVALID_FILE_ERR; }
  else
  {
    char* format_string = NULL;
    switch(pf->format)
    {
      case PLY_ASCII: { format_string="ascii"; break; }
      case PLY_LITTLE_ENDIAN: { format_string = "binary_little_endian"; break; }
      case PLY_BIG_ENDIAN: { format_string = "binary_big_endian"; break;  }
      default: { return PLY_UNRECOGNIZED_FORMAT_ERR; }
    }

    fprintf(pf->_fp, "ply\nformat %s %2.1f\n", 
                      format_string, (float)pf->format_version);
    for(int i = 0; i < msh_array_size(pf->elements); ++i)
    {
      ply_element_t* el = &pf->elements[i];
      fprintf(pf->_fp, "element %s %d\n", el->name, el->count);
      for( int j = 0; j < msh_array_size(el->properties); j++)
      {
        ply_property_t* pr = &el->properties[j];
        char* pr_type_str = NULL;
        ply_file__property_type_to_string( pr->type, &pr_type_str );
        
        if( pr->list_type == PLY_INVALID )
        {
          fprintf(pf->_fp, "property %s %s\n", pr_type_str, pr->name );
        }
        else
        {
          char* pr_list_type_str = NULL;
          ply_file__property_type_to_string( pr->list_type, &pr_list_type_str );
          fprintf(pf->_fp, "property list %s %s %s\n", pr_list_type_str, 
                                                       pr_type_str, 
                                                       pr->name );
        }
      }
    }
    fprintf( pf->_fp, "end_header\n");
  }
  return PLY_NO_ERRORS;
}


int
ply_file__write_data_ascii( const ply_file_t* pf )
{
  for( int i = 0 ; i < msh_array_size(pf->elements); ++i )
  {
    ply_element_t* el = &pf->elements[i];

    for( int j = 0; j < el->count; ++j )
    {
      for( int k = 0; k < msh_array_size(el->properties); ++k )
      {
        ply_property_t* pr = &el->properties[k];
        if( pr->list_type == PLY_INVALID )
        {
          ply_file__fprint_data_at_offset( pf, pr->data, pr->offset, pr->type );
          pr->offset += pr->stride;
        }
        else
        {
          // figure out stride.
          // TODO(maciej): Check stride + hints to decide on fastest way of getting stride.
          pr->stride = ply_file__calculate_list_property_stride( pr, el->properties );

          int list_count = ply_file__get_data_as_int(pr->list_data + pr->list_offset, pr->list_type );
          ply_file__fprint_data_at_offset( pf, pr->list_data, pr->list_offset, pr->list_type );
          pr->list_offset += pr->list_stride;

          for( int l = 0; l < list_count; ++l )
          {
            int cur_offset = pr->offset + l * pr->byte_size;
            ply_file__fprint_data_at_offset( pf, pr->data, cur_offset, pr->type );
          }
          pr->offset += pr->stride;
        }
      }
      fprintf(pf->_fp, "\n");
    }
  }
  return PLY_NO_ERRORS;
}

void
ply_file__memcpy(void* dest, void* src, size_t byte_size, int8_t swap)
{
  if( !swap ) { memcpy(dest, src, byte_size); }
  else
  {
   char *src_ptr = (char *)src;
   char *dest_ptr = (char *)dest;
 
   //reverse cpy
   for (int i=0; i<byte_size; i++)
    dest_ptr[i] = src_ptr[byte_size-i-1];
  }
}

// TODO(maciej): Test if splitting into completly separate functions helps
int
ply_file__write_data_binary( const ply_file_t* pf, int swap_endiannes )
{
  for( int i = 0 ; i < msh_array_size(pf->elements); ++i )
  {
    ply_element_t* el = &pf->elements[i];
    int buffer_size = 0;
    // printf("---\n");
    for( int j = 0 ; j < msh_array_size(el->properties); ++j )
    {
      // printf("%d %s %d\n", j, el->properties[j].name, el->properties[j].total_byte_size);
      buffer_size += el->properties[j].total_byte_size;
    }

    void* dst = malloc( buffer_size );
    int dst_offset = 0;
    for( int j = 0; j < el->count; ++j )
    {
      for( int k = 0; k < msh_array_size(el->properties); ++k )
      {
        ply_property_t* pr = &el->properties[k];
        if( pr->list_type == PLY_INVALID )
        {
          ply_file__memcpy( dst+dst_offset, pr->data+pr->offset, pr->byte_size, swap_endiannes );
          pr->offset += pr->stride;
          dst_offset += pr->byte_size;
        }
        else
        {
          // figure out stride.
          // TODO(maciej): Check stride + hints to decide on fastest way of getting stride.
          pr->stride = ply_file__calculate_list_property_stride( pr, el->properties );

          int list_count = ply_file__get_data_as_int(pr->list_data + pr->list_offset, pr->list_type );
          ply_file__memcpy( dst + dst_offset, pr->list_data + pr->list_offset, pr->list_byte_size, swap_endiannes );
          pr->list_offset += pr->list_stride;
          dst_offset += pr->list_byte_size;

          // printf("%d\n", list_idx);
          
          // printf("%s LIST COUNT: %d | offest %d\n", pr->name, list_count, pr->offset/pr->byte_size );
          ply_file__memcpy( dst + dst_offset, pr->data + pr->offset, pr->byte_size * list_count, swap_endiannes );
          // for( int l = 0; l < list_count; ++l )
          // {
            // int val_a = ply_file__get_data_as_int(pr->data+pr->offset, pr->type, l );
            // int val_b = ply_file__get_data_as_int(dst+dst_offset, pr->type, l );
            // printf("%d %d\n", val_a, val_b );
          // }

          pr->offset += pr->stride;
          dst_offset += pr->byte_size * list_count;
          
          // printf("\n");
        }
      }
    }
    // printf("%d %d\n\n", dst_offset, buffer_size);
    fwrite( dst, buffer_size, 1, pf->_fp );
  }
  return PLY_NO_ERRORS;
}

int
ply_file__write_data( const ply_file_t* pf )
{
  int error = PLY_NO_ERRORS;
  if( pf->format == PLY_ASCII )         { error = ply_file__write_data_ascii(pf); }
  if( pf->format == PLY_BIG_ENDIAN )    { error = ply_file__write_data_binary(pf, 1); }
  if( pf->format == PLY_LITTLE_ENDIAN ) { error = ply_file__write_data_binary(pf, 0); }
  return error;
}

int 
ply_file_write( const ply_file_t* pf )
{
  int error = PLY_NO_ERRORS;
  error = ply_file__write_header( pf );
  if( error ) { return error; }
  error = ply_file__write_data( pf );
  return error;
}


int
ply_file_add_hint(ply_file_t* pf, ply_hint_t hint)
{
  msh_array_push(pf->_hints, hint);
  return PLY_NO_ERRORS;
}


void
ply_file_print_header(ply_file_t* pf)
{
  // Find property & element with longest name
  size_t max_pr_length = 0;

  for(int i = 0 ; i < msh_array_size(pf->elements); ++i)
  {
    for(int j = 0 ; j < msh_array_size(pf->elements[i].properties); ++j)
    {
      max_pr_length = msh_max(strlen(pf->elements[i].properties[j].name),
                              max_pr_length);
    }
  }

  char *type = NULL;
  char *list_type = NULL;
  char spaces[PLY_MAX_STR_LEN];
  for(int i = 0 ; i < PLY_MAX_STR_LEN; ++i ) { spaces[i] = ' ';}
  spaces[PLY_MAX_STR_LEN-1] = 0;

  char *format_str = NULL;
  if(pf->format == PLY_ASCII)         { format_str = "Ascii"; }
  if(pf->format == PLY_LITTLE_ENDIAN) { format_str = "Binary Little Endian"; }
  if(pf->format == PLY_BIG_ENDIAN)    { format_str = "Binary Big Endian"; }
  printf("PLY: %s %d\n", format_str, pf->format_version);
  for(int i = 0 ; i < msh_array_size(pf->elements); ++i)
  {
    printf("   '%s' count: %d\n", pf->elements[i].name,
                                    pf->elements[i].count);
    for(int j = 0 ; j < msh_array_size(pf->elements[i].properties); ++j)
    {
      size_t n_spaces = max_pr_length - strlen(pf->elements[i].properties[j].name);
      spaces[n_spaces] = 0;
      ply_file__property_type_to_string( pf->elements[i].properties[j].type, &type );
      if(pf->elements[i].properties[j].list_type == PLY_INVALID)
      {
        printf("      '%s'%s | %7s(%2d bytes)\n", pf->elements[i].properties[j].name,
                                            spaces, type, pf->elements[i].properties[j].byte_size );
      }
      else
      {
        ply_file__property_type_to_string( pf->elements[i].properties[j].list_type, &list_type );
        printf("      '%s'%s | %7s(%2d bytes) %7s(%2d bytes)\n", 
                                        pf->elements[i].properties[j].name, spaces, 
                                        list_type, pf->elements[i].properties[j].list_byte_size,
                                        type, pf->elements[i].properties[j].byte_size );
      }
      spaces[n_spaces] = ' ';
    }
  }
}


ply_file_t*
ply_file_open(const char* filename, const char* mode)
{
  ply_file_t *pf = NULL;
  if( mode[0] != 'r' && mode[0] != 'w' ) return NULL;
  FILE* fp = fopen(filename, mode);
  if( fp )
  {
    pf                 = (ply_file_t*)malloc(sizeof(ply_file_t));
    pf->valid          = 0;
    pf->format         = -1;
    pf->format_version = 0;
    pf->elements       = NULL;
    pf->cur_element    = NULL;
    pf->_hints         = NULL;
    pf->_fp            = fp;
  }
  if( mode[0] == 'w' )
  {
    pf->format_version = 1;
    pf->format = PLY_ASCII;
    if( strlen(mode) > 1 && mode[1] == 'b' ) { pf->format = PLY_LITTLE_ENDIAN; }
  } 
  return pf;
}

int
ply_file_close(ply_file_t* pf)
{
  if(pf->_fp) { fclose(pf->_fp); pf->_fp = NULL; }
  // NOTE(maciej): When we drop dynamic arrays we will need to change this
  if(pf->elements)
  {
    for( int i = 0; i < msh_array_size(pf->elements); ++i )
    {
      ply_element_t* el = &pf->elements[i];
      if(el->properties) { msh_array_free(el->properties); }
      if(el->data) { free(el->data); }
    }
    msh_array_free(pf->elements);
  }
  if(pf->_hints) msh_array_free(pf->_hints);
  free(pf);
  return 1;
}

////////////////////////////////////////////////////////////////////////////////
void write_ply_file( const char* filename )
{
  double t1 = msh_get_time(MSHT_MILLISECONDS);
  const char* positions_names[] = {"x", "y", "z"};
  const char* normals_names[]   = {"nx", "ny", "nz"};
  const char* colors_names[]    = {"red", "green", "blue", "alpha"};
  float positions[] = { 1.0f, 0.0f, 1.0f, 
                        0.0f, 0.0f, 1.0f,
                        1.0f, 0.0f, 0.0f };
  float normals[] = { 0.0f, 1.0f, 0.0f, 
                      0.0f, 1.0f, 0.0f,
                      0.0f, 1.0f, 0.0f };
  uint8_t colors[] = { 255, 125, 125, 255, 
                     0, 255, 0, 255,
                     0, 0, 255, 255 };
  const char* face_names[] = {"vertex_indices"};
  int face_ind[] = {0,1,2, 3,4,5,6, 7,8};
  unsigned char counts[] = {3,4,2};

  ply_file_t* pf = ply_file_open(filename, "w");
  ply_file_add_property_to_element(pf, "vertex", positions_names, 3, PLY_FLOAT, PLY_INVALID, positions, NULL, 3 );
  ply_file_add_property_to_element(pf, "vertex", normals_names, 3, PLY_FLOAT, PLY_INVALID, normals, NULL, 3 );
  ply_file_add_property_to_element(pf, "vertex", colors_names, 4, PLY_UINT8, PLY_INVALID, colors, NULL, 3 );
  ply_file_add_property_to_element(pf, "face", face_names, 1, PLY_INT32, PLY_UINT8, face_ind, counts, 3 );


  ply_file_write(pf);
  ply_file_close(pf);
  double t2 = msh_get_time(MSHT_MILLISECONDS);
  printf("Done in %f milliseconds\n", t2-t1);
}

void read_ply_file( const char* filename )
{
  printf("Reading file : %s\n", filename);
  ply_file_t* pf = ply_file_open(filename, "r");
  // ply_hint_t indices_size_hint = {.property_name="vertex_indices", .expected_size=3};
  // ply_file_add_hint(pf, indices_size_hint);
  if( pf )
  {
    ply_file_read(pf);
    ply_file_print_header(pf);
    const char* positions_names[] = {"x","y","z"};
    const char* vertex_indices_names[] = {"vertex_indices"};
    float* positions = NULL;
    int n_verts = -1;
    int* indices = NULL;
    int n_faces = -1;
    uint8_t* indices_counts = NULL;
    ply_file_get_property_from_element(pf, "vertex", positions_names, 3, PLY_FLOAT, PLY_INVALID, 
                                      (void**)&positions, NULL, &n_verts );
    ply_file_get_property_from_element(pf, "face", vertex_indices_names, 1, PLY_INT32, PLY_UINT8, 
                                       (void**)&indices, (void**)&indices_counts, &n_faces );
    
    printf("Vertex count: %d\n", n_verts);
    for( int i = 0 ; i < n_verts; ++i )
    {
      printf("%f %f %f\n", positions[3*i+0], positions[3*i+1], positions[3*i+2]);
    }

    printf("Face Count: %d\n", n_faces);
    for( int i = 0; i < n_faces; ++i )
    {
      int count = indices_counts[i];
      for( int j = 0 ; j < count; ++j )
      {
        printf( "%d ", *indices );
        indices+=1;
      }
      printf("\n");
    }

    // double ht1 = msh_get_time(MSHT_MICROSECONDS);
    // ply_file_parse_header(pf);
    // double ht2 = msh_get_time(MSHT_MICROSECONDS);
    // printf("Header read in %fus\n", ht2-ht1);
    // ply_file_print_header(pf);

    // double pt1 = msh_get_time(MSHT_MILLISECONDS);
    // ply_file_parse_contents(pf);
    // double pt2 = msh_get_time(MSHT_MILLISECONDS);
    // printf("Parsing took %f ms\n", pt2-pt1);

    // int vertex_count = -1;
    // int face_count = -1;
    // ply_file_get_element_count(pf, "vertex", &vertex_count);
    // ply_file_get_element_count(pf, "face", &face_count);

    // double t1 = msh_get_time(MSHT_MILLISECONDS);
    // int vertices_size = -1;
    // int faces_size = -1;
    // ply_file_get_element_size(pf, "vertex", &vertices_size);
    // ply_file_get_element_size(pf, "face", &faces_size);
    // printf("vertices_size : %d\n", vertices_size );
    // printf("faces_size : %d\n", faces_size );
    // void *vertices_data = malloc(vertices_size);
    // void *faces_data    = malloc(faces_size);
    // t1 = msh_get_time(MSHT_MILLISECONDS);
    // ply_file_get_element_data(pf, "vertex", &vertices_data);
    // double t2 = msh_get_time(MSHT_MILLISECONDS);
    // printf("data request : %fms\n", t2-t1);
    // t1 = msh_get_time(MSHT_MILLISECONDS);
    // ply_file_get_element_data(pf, "face", &faces_data);
    // t2 = msh_get_time(MSHT_MILLISECONDS);
    // printf("data request : %fms\n", t2-t1);

    // t1 = msh_get_time(MSHT_MILLISECONDS);   
    // int pos_size = -1;
    // const char* pos_prop_names[] = {"nx","ny","nz"};
    // ply_file_get_properties_size(pf, "vertex", pos_prop_names, 3, &pos_size);
    // float* positions = malloc(pos_size);
    // ply_file_get_properties_from_element(pf, "vertex", vertices_data, pos_prop_names, &positions );
    // t2 = msh_get_time(MSHT_MILLISECONDS);
    // printf("pos data copy : %fms\n", t2-t1);

    // t1 = msh_get_time(MSHT_MILLISECONDS);
    // ply_property_t* ind_prop = NULL;
    // ply_file_get_property(pf, "face", "vertex_indices", &ind_prop);
    // unsigned char* ind_count = (unsigned char*)malloc(face_count * ind_prop->list_byte_size);  
    // int* indices = (int*)malloc( ind_prop->total_count * ind_prop->byte_size);
    // ply_file_get_list_property_from_element(pf, "face", faces_data, "vertex_indices", &ind_count, &indices);
    // t2 = msh_get_time(MSHT_MILLISECONDS);
    // printf("ind data copy : %fms\n", t2-t1);

    // int offset = 0;
    // for( int i = 0; i < face_count; ++i )
    // {
    //   printf( "%d ", ind_count[i] );
    //   for( int j = 0 ; j < ind_count[i]; ++j )
    //   {
    //     printf("%d ", *(indices+offset) );
    //     offset+=1;
    //   }
    //   printf("\n");
    // }
  }
  ply_file_close(pf);
}

int main(int argc, char** argv)
{
  if(argc < 2) {printf("Please provide .ply filename\n"); return 0;} 
  char* filename = argv[1];

  write_ply_file(filename);
  read_ply_file(filename);

  return 1;
}
