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
[x] Add big-endian input support
    [x] Try reading in lucy
[x] Add hints optimizations
    [x When reading ascii, we can skip calculating layout size
    [x] READ: If we have hint we can precalculate the row sizes in data copy
    [x] WRITE: Also true
[x] Add regular properties optimizations
    [x] READ:  For regular properties we always can precalculate that
    [x] WRITE: Also true
[x] Property grouping
    [x] Switch to strides
[x] Improve data layout of the struct
[x] Allow for disregarding the face count data.
[x] Small memcpy
[-] Add options for having list data layout same as ply
[x] Rethink ply_file__memcopy... - flips vertex order when copying the face count
[x] Write benchmarks
    [x] Tinyply2.0 https://github.com/ddiakopoulos/tinyply
    [x] rply http://w3.impa.br/~diego/software/rply/
    [x] Turks ply http://people.sc.fsu.edu/~jburkardt/c_src/ply_io/ply_io.html
    [x] Bourke ply http://paulbourke.net/dataformats/ply/
    [x] Nanoply https://github.com/cnr-isti-vclab/vcglib/tree/master/wrap/nanoply
    [x] VCG Ply https://github.com/cnr-isti-vclab/vcglib/tree/master/wrap/ply
[x] Write viewer

[x] Casting
    [x] Add list property support
    [x] Make casting optional - only applied if needed.
[x] Rebenchmark lucy
[ ] Revisit writer
    [x] Allow for NULL count if hint is present
    [x] Add optimization for calculating stride if hint is present
    [x] Replace memcpy with assignment
    [ ] Test encoder with more exotic meshes - copy tinyply example.
[x] Build in c++ mode
[ ] Optimize 
  [ ] Profile lucy writing, why it is showing a big slowdown.
  [x] Add property reading in group (requested_group_size variable)
  [x] Read data sequentially without prefetching large block.
  [x] Separate functions to read all datatypes?
  [x] Why is O2 faster than O3 - autovectorization I guess.
  [-] What are the other 0.262 seconds

[x] Functions accepting descriptors like sokol
  [ ] Scrap hint system, have it as a property in descriptor
[ ] Encoder/decorder split
[ ] Error reporting
[ ] Extensive testing
[ ] Getting raw data for list property - different function.
[ ] Replace msh_array with buf.c or with pervogsen buf (https://gist.github.com/vurtun/5144bbcc2db73d51e36bf327ac19b604)
[ ] Code cleanup
  [ ] Replace duplicated code
  [ ] Replace syscalls with redefineable macros
  [ ] C++ support
[ ] Fix the header names to be mply
[ ] Enable swizzle

*/

// #include <stdlib.h>
// #include <stdint.h>
// #define MSH_IMPLEMENTATION
// #include "msh.h"
// #include <unistd.h>
// API does not allow mixing regular and list properties

#if defined(_MSC_VER)
#define PLY_INLINE __forceinline
#else
#define PLY_INLINE __attribute__((unused, always_inline)) inline
#endif

#define PLY_FILE_MAX_STR_LEN 1024
#define PLY_FILE_MAX_REQ_PROPERTIES 16
#define PLY_FILE_MAX_PROPERTIES 128


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

// TODO(maciej): Maybe simplify and remove hint?
typedef struct ply_hint
{
  char* property_name;
  int expected_size;
} ply_hint_t;

typedef struct ply_property
{
  // Should we store those or calculate on the fly
  int32_t list_count;
  int16_t byte_size;
  int64_t offset;
  int64_t stride;
  int16_t list_byte_size;
  int64_t list_offset;
  int64_t list_stride;

  ply_type_id_t type;
  ply_type_id_t list_type;

  int64_t total_count;
  int total_byte_size;

  // data storage -> maybe we will put that into some write helper
  char name[32];
  void* data;
  void* list_data;

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

typedef struct ply_file_property_desc
{
  char* element_name;
  const char** property_names;
  int32_t num_requested_properties;
  ply_type_id_t requested_type;
  ply_type_id_t requested_list_type;
  void* requested_data;
  void* requested_list_data;
  int32_t* requested_data_count;
  int32_t size_hint;
} ply_file_property_desc_t;

typedef struct ply_file
{
  int valid;
  int format;
  int format_version;
  
  msh_array(ply_element_t) elements;
  msh_array(ply_file_property_desc_t*) descriptors;
  FILE* _fp;
  int _header_size;
  int _system_format;
  msh_array(ply_hint_t) _hints;
  ply_element_t* _cur_element; //TODO(maciej): Check if necessary
} ply_file_t;


int
ply_file_get_property_from_element( ply_file_t* pf, ply_file_property_desc_t* desc );


//////1//////////////////////////////////////////////////////////////////////////////////////////////
// This ply library only provides set of functionalities to read/write your own specific ply file.
// pf_file_t does not actually store any mesh data. You need to provide buffers to which such data
// can be written.
// This code assumes that your computer is little endian
////////////////////////////////////////////////////////////////////////////////////////////////////
// All i am doing here is jumping through hoops if data is not interleaved.
// Maybe I should assume it is, and if it is not, create separate interlevard buffers and write them...

// Like the question I should be answering is what is the best data format for the ply
// file to be writen / read easily. And then provide functions transforming to that format.



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
  PLY_REQUESTED_ELEMENT_IS_MISSING,
  PLY_REQUESTED_PROPERTY_IS_MISSING,
  PLY_NO_REQUESTS,
  PLY_INVALID_LIST_TYPE_ERR 
} ply_err_t;

PLY_INLINE void 
ply_file__swap_bytes(uint8_t* buffer, int32_t type_size, int32_t count)
{  
  for( int32_t i = 0; i < count; ++i )
  {
    int32_t offset = i * type_size;  
    for( int32_t j = 0; j < type_size >> 1; ++j )
    {
      int32_t temp = buffer[offset + j];
      buffer[offset + j] = buffer[offset + type_size - 1 - j];
      buffer[offset + type_size - 1 - j] = temp;
    }
  }
}

PLY_INLINE ply_element_t*
ply_file_find_element( const ply_file_t* pf, const char* element_name )
{
  ply_element_t* el = NULL;
  for( int i = 0; i < msh_array_size(pf->elements); ++i )
  {
    if(!strcmp(element_name, pf->elements[i].name))
    {
      el = &pf->elements[i];
      break;
    }
  }
  return el;
}

PLY_INLINE ply_property_t*
ply_file_find_property( const ply_element_t* el, const char* property_name)
{
  ply_property_t* pr = NULL;
  for( int i=0; i < msh_array_size(el->properties) ; ++i )
  {
    if(!strcmp(el->properties[i].name, property_name)) { pr = &el->properties[i]; break; }
  }

  return pr;
}

PLY_INLINE int
ply_file__get_data_as_int( void* data, int type, int8_t swap_endianness )
{
  int retval = 0;
  switch( type )
  {
    case PLY_INT8:  
      retval  = ((char*)data)[0];
      break;
    case PLY_INT16: 
    {
      retval = ((int16_t*)data)[0];
      if( swap_endianness ) ply_file__swap_bytes((uint8_t*)&retval, 2, 1);
      break;
    }
    case PLY_INT32:
    {
      retval = ((int32_t*)data)[0]; 
      if( swap_endianness ) ply_file__swap_bytes((uint8_t*)&retval, 4, 1);
      break;
    }
    case PLY_UINT8:  
      retval = ((uint8_t*)data)[0];
      break;
    case PLY_UINT16: 
    {
      retval = ((uint16_t*)data)[0];
      if( swap_endianness ) ply_file__swap_bytes((uint8_t*)&retval, 2, 1);
      break;
    }
    case PLY_UINT32:
    { 
      retval = ((uint32_t*)data)[0]; 
      if( swap_endianness ) ply_file__swap_bytes((uint8_t*)&retval, 4, 1);
      break;
    }
    default: retval = 0; break;
  }
  return retval;
}


PLY_INLINE int 
ply_file__type_to_byte_size( ply_type_id_t type )
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

PLY_INLINE void
ply_file__property_type_to_string( ply_type_id_t type, char** string )
{
  switch( type )
  {
    case PLY_INT8:   { *string = (char*)"char"; break; }
    case PLY_INT16:  { *string = (char*)"short"; break; }
    case PLY_INT32:  { *string = (char*)"int"; break; }
    case PLY_UINT8:  { *string = (char*)"uchar"; break; }
    case PLY_UINT16: { *string = (char*)"ushort"; break; }
    case PLY_UINT32: { *string = (char*)"uint"; break; }
    case PLY_FLOAT:  { *string = (char*)"float"; break; }
    case PLY_DOUBLE: { *string = (char*)"double"; break; }
    default: { break; }
  }
}

PLY_INLINE void
ply_file__string_to_property_type(char* type_str, ply_type_id_t* pr_type, int16_t* pr_size)
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
  char cmd_str[PLY_FILE_MAX_STR_LEN];
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
  char cmd[PLY_FILE_MAX_STR_LEN];
  char frmt_str[PLY_FILE_MAX_STR_LEN];
  char frmt_ver_str[PLY_FILE_MAX_STR_LEN];
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
  char cmd[PLY_FILE_MAX_STR_LEN];
  ply_element_t el = {{0}};
  el.properties = NULL;
  if(sscanf(line, "%s %s %d", &cmd[0], &el.name[0], &el.count) != 3) { return PLY_ELEMENT_CMD_ERR; }
  msh_array_push(pf->elements, el);
  pf->_cur_element = msh_array_back(pf->elements);
  return PLY_NO_ERRORS;
}

int
ply_file__parse_property_cmd(char* line, ply_file_t* pf)
{
  char cmd[PLY_FILE_MAX_STR_LEN];
  char type_str[PLY_FILE_MAX_STR_LEN];
  char list_str[PLY_FILE_MAX_STR_LEN];
  char list_type_str[PLY_FILE_MAX_STR_LEN];
  int valid_format = false;

  ply_element_t* el = pf->_cur_element;
  ply_property_t pr = {0};
  
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
    pr.list_count = 0;
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
  char line[PLY_FILE_MAX_STR_LEN];
  int err_code = 0;
  while(fgets(&line[0], PLY_FILE_MAX_STR_LEN, pf->_fp))
  {
    line_no++;
    char cmd[PLY_FILE_MAX_STR_LEN];
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
  char line[PLY_FILE_MAX_STR_LEN];
  while(fgets(&line[0], PLY_FILE_MAX_STR_LEN, pf->_fp))
  {
    char* cp_up  = &line[0];
    char* cp_low = &line[0];

    for( int j=0; j<msh_array_size(el->properties); ++j )
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
          default: pr->list_count = 1; break;
        }
 
        // skip
        *cp_up = tmp_cp_up;
        cp_low = cp_up;
        cp_up++;
        for( int k=0; k < pr->list_count; ++k) 
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
ply_file__calculate_elem_size_binary(ply_file_t* pf, ply_element_t* el)
{
  for( int i = 0; i < el->count; ++i )
  {
    for( int j = 0; j < msh_array_size(el->properties); ++j )
    {
      ply_property_t* pr = &el->properties[j];
      int count = 1;
      if( pr->list_type != PLY_INVALID ) 
      { 
        int read_count = fread(&count, pr->list_byte_size, 1, pf->_fp);
        if( read_count != 1) { return PLY_PARSE_ERROR;}
        pr->total_byte_size += pr->list_byte_size;
      }
      fseek(pf->_fp, count * pr->byte_size, SEEK_CUR);
      pr->total_byte_size += count * pr->byte_size;
      pr->total_count += count;
    }
  }
  return PLY_NO_ERRORS;
}

int32_t 
ply_file__can_precalculate_sizes( const ply_file_t* pf, ply_element_t* el )
{
  int32_t can_precalculate = 1;
  for( int32_t i = 0; i < msh_array_size(el->properties); ++i )
  {
    ply_property_t* pr = &el->properties[i];
    if( pr->list_type != PLY_INVALID && pr->list_count == 0) 
    { 
      can_precalculate = 0;
      break;
    }
  }

  return can_precalculate;
}

int
ply_file_parse_contents(ply_file_t* pf)
{
  int err_code = PLY_NO_ERRORS;
  for( int i = 0; i < msh_array_size(pf->elements); ++i )
  {
    ply_element_t* el = &pf->elements[i];
    int num_properties = msh_array_size(el->properties);
    el->file_anchor = ftell(pf->_fp);

    // Determine if any of the properties in the element has list
    int can_precalculate_size = ply_file__can_precalculate_sizes( pf, el );

    if( can_precalculate_size )
    {
      // This is a faster path, as we can just calculate the size required by element in one go.
      int elem_size = 0;
      for( int j = 0; j <num_properties; ++j ) 
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
      if( pf->format != PLY_ASCII ) { fseek( pf->_fp, el->count * elem_size, SEEK_CUR ); }
      else
      {
        char line[PLY_FILE_MAX_STR_LEN];
        for( int j =0; j < el->count; ++j ) { fgets( &line[0], PLY_FILE_MAX_STR_LEN, pf->_fp ); }
      }
    }
    else 
    {
      // There exists a list property. We need to calculate required size via pass through
      if( pf->format == PLY_ASCII ) { err_code = ply_file__calculate_elem_size_ascii(pf, el); }
      else                          { err_code = ply_file__calculate_elem_size_binary(pf, el); }

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
  for( int i= 0; i < msh_array_size(el->properties); ++i )
  {
    *size += el->properties[i].total_byte_size;
  }
  return err_code;
}

// NOTE(maciej): this works better with an assignment
#define PLY_CONVERT_AND_ASSIGN(D, C, T, conv_funct) {\
  T n = (T)conv_funct(C); \
  *((T*)(D)) = n; \
  (D) += sizeof(T); }

PLY_INLINE void 
ply_file__ascii_to_value( char** dst, char* src, const int type )
{
  switch(type)
  {
    case PLY_INT8:   PLY_CONVERT_AND_ASSIGN(*dst, src, int8_t,   atoi); break;
    case PLY_INT16:  PLY_CONVERT_AND_ASSIGN(*dst, src, int16_t,  atoi); break;
    case PLY_INT32:  PLY_CONVERT_AND_ASSIGN(*dst, src, int32_t,  atoi); break;
    case PLY_UINT8:  PLY_CONVERT_AND_ASSIGN(*dst, src, uint8_t,  atoi); break;
    case PLY_UINT16: PLY_CONVERT_AND_ASSIGN(*dst, src, uint16_t, atoi); break;
    case PLY_UINT32: PLY_CONVERT_AND_ASSIGN(*dst, src, uint32_t, atoi); break;
    case PLY_FLOAT:  PLY_CONVERT_AND_ASSIGN(*dst, src, float,    atof); break;
    case PLY_DOUBLE: PLY_CONVERT_AND_ASSIGN(*dst, src, double,   atof); break;
  }
}

int32_t
ply_file__get_element_data_ascii( ply_file_t* pf, const ply_element_t* el, void** storage, int64_t storage_size )
{
  int32_t err_code = PLY_NO_ERRORS;
  int32_t num_properties = msh_array_size( el->properties );

  int32_t hinted_sizes[1024] = {0};
  for( int j = 0; j < num_properties; ++j )
  {
    ply_property_t* pr = &el->properties[j];
    hinted_sizes[j] = -1;
    if( pr->list_type != PLY_INVALID ) 
    {
      for( int k=0; k<msh_array_size(pf->_hints); ++k)
      {
        if( !strcmp(pr->name, pf->_hints[k].property_name) ) 
        { 
          hinted_sizes[j] = pf->_hints[k].expected_size;
        }
      }
    }
  }

  fseek(pf->_fp, el->file_anchor, SEEK_SET);
  char *dest = (char*)*storage;
  for( int i = 0; i < el->count; ++i )
  {
    char line[PLY_FILE_MAX_STR_LEN];
    fgets(line, PLY_FILE_MAX_STR_LEN, pf->_fp);
    char* cp_up  = &line[0];
    char* cp_low = &line[0];

    for( int j = 0; j < num_properties; ++j )
    {
      ply_property_t* pr = &el->properties[j];
      while( *cp_up!=' ' && *cp_up!='\n' ) { cp_up++; }
      char tmp_cp_up = *cp_up;
      *cp_up = 0; // fake string end;
      if(pr->list_type == PLY_INVALID) // regular property
      { 
        ply_file__ascii_to_value( &dest, cp_low, pr->type );
      }
      else // list property
      {
        ply_file__ascii_to_value( &dest, cp_low, pr->list_type);
        if( hinted_sizes[j] > 0 )
        {
          pr->list_count = hinted_sizes[j];
        }
        else
        {
          pr->list_count = ply_file__get_data_as_int( dest-pr->list_byte_size, pr->list_type, 0 );
        }
        *cp_up = tmp_cp_up;
        cp_low = cp_up;
        cp_up++;
        for( int k = 0; k < pr->list_count; ++k) 
        {
          while(*cp_up!=' ' && *cp_up!='\n') {cp_up++;}
          tmp_cp_up = *cp_up;
          *cp_up = 0; // fake string end;
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
#undef PLY_CONVERT_AND_ASSIGN

int
ply_file__get_element_data_binary( ply_file_t* pf, const ply_element_t* el, 
                                   void** storage, int64_t storage_size )
{
  int err_code = PLY_NO_ERRORS;
  fseek( pf->_fp, el->file_anchor, SEEK_SET );
  if( fread( *storage, storage_size, 1, pf->_fp ) != 1 ) { return PLY_PARSE_ERROR; }
  return err_code;
}

PLY_INLINE int
ply_file_get_element_data(ply_file_t* pf, const ply_element_t* el, void** storage, int64_t storage_size)
{
  int err_code = PLY_NO_ERRORS;
  if(pf->format == PLY_ASCII)  { err_code = ply_file__get_element_data_ascii( pf, el, storage, storage_size ); }
  else                         { err_code = ply_file__get_element_data_binary( pf, el, storage, storage_size ); }

  return err_code;
}

// NOTE(maciej): This does not really read, it is more of a parsing
int
ply_file_parse(ply_file_t* pf)
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
                              const char** properties_names, int num_properties,
                              ply_type_id_t type, ply_type_id_t list_type,
                              int* data_size, int* list_size )
{
  int n_found              = 0;
  int total_data_byte_size = 0;
  int total_list_byte_size = 0;
  int byte_size = ply_file__type_to_byte_size( type );
  int list_byte_size = ply_file__type_to_byte_size( list_type );
  for( int i = 0; i < num_properties; ++i )
  {
    for( int j = 0; j < msh_array_size( el->properties ); ++j )
    {
      ply_property_t* pr = &el->properties[j];
      if( !strcmp(pr->name, properties_names[i]) )
      {
        n_found++;
        total_data_byte_size += pr->total_count * byte_size;
        total_list_byte_size += list_byte_size;
      } 
    }
  }
  if( n_found != num_properties ) return PLY_PROPERTY_NOT_FOUND_ERR;

  total_list_byte_size *= el->count;

  *data_size = total_data_byte_size;
  *list_size = total_list_byte_size;

  return PLY_NO_ERRORS;
}

PLY_INLINE void
ply_file__data_assign( void* dst, void* src, int32_t type, int32_t count )
{
  int32_t a = count / 3;
  int32_t b = count % 3;
  
  switch( type )
  {
    case PLY_UINT8:
    {
      uint8_t* dst_ptr = (uint8_t*)dst;
      uint8_t* src_ptr = (uint8_t*)src;
      for( int32_t c = 0; c < a; c++ )
      {
        *(dst_ptr) = *(src_ptr); dst_ptr++; src_ptr++; 
        *(dst_ptr) = *(src_ptr); dst_ptr++; src_ptr++; 
        *(dst_ptr) = *(src_ptr); dst_ptr++; src_ptr++;   
      }
      for( int32_t c = 0; c < b; c++ )
      {
        *(dst_ptr) = *(src_ptr); dst_ptr++; src_ptr++; 
      }
      break;
    }
      
    case PLY_UINT16:
    {
      uint16_t* dst_ptr = (uint16_t*)dst;
      uint16_t* src_ptr = (uint16_t*)src;
      for( int32_t c = 0; c < a; c++ )
      {
        *(dst_ptr) = *(src_ptr); dst_ptr++; src_ptr++; 
        *(dst_ptr) = *(src_ptr); dst_ptr++; src_ptr++; 
        *(dst_ptr) = *(src_ptr); dst_ptr++; src_ptr++;   
      }
      for( int32_t c = 0; c < b; c++ )
      {
        *(dst_ptr) = *(src_ptr); dst_ptr++; src_ptr++; 
      }
      break;
    }
      
    case PLY_UINT32:
    {
      uint32_t* dst_ptr = (uint32_t*)dst;
      uint32_t* src_ptr = (uint32_t*)src;
      for( int32_t c = 0; c < a; c++ )
      {
        *(dst_ptr) = *(src_ptr); dst_ptr++; src_ptr++; 
        *(dst_ptr) = *(src_ptr); dst_ptr++; src_ptr++; 
        *(dst_ptr) = *(src_ptr); dst_ptr++; src_ptr++;   
      }
      for( int32_t c = 0; c < b; c++ )
      {
        *(dst_ptr) = *(src_ptr); dst_ptr++; src_ptr++; 
      }
      break;
    }
      
    case PLY_INT8:
    {
      int8_t* dst_ptr = (int8_t*)dst;
      int8_t* src_ptr = (int8_t*)src;
      for( int32_t c = 0; c < a; c++ )
      {
        *(dst_ptr) = *(src_ptr); dst_ptr++; src_ptr++; 
        *(dst_ptr) = *(src_ptr); dst_ptr++; src_ptr++; 
        *(dst_ptr) = *(src_ptr); dst_ptr++; src_ptr++;   
      }
      for( int32_t c = 0; c < b; c++ )
      {
        *(dst_ptr) = *(src_ptr); dst_ptr++; src_ptr++; 
      }
      break;
    }
      
    case PLY_INT16:
    {
      int16_t* dst_ptr = (int16_t*)dst;
      int16_t* src_ptr = (int16_t*)src;
      for( int32_t c = 0; c < a; c++ )
      {
        *(dst_ptr) = *(src_ptr); dst_ptr++; src_ptr++; 
        *(dst_ptr) = *(src_ptr); dst_ptr++; src_ptr++; 
        *(dst_ptr) = *(src_ptr); dst_ptr++; src_ptr++;   
      }
      for( int32_t c = 0; c < b; c++ )
      {
        *(dst_ptr) = *(src_ptr); dst_ptr++; src_ptr++;
      }
      break;
    }
      
    case PLY_INT32:
    {
      int32_t* dst_ptr = (int32_t*)dst;
      int32_t* src_ptr = (int32_t*)src;
      for( int32_t c = 0; c < a; c++ )
      {
        *(dst_ptr) = *(src_ptr); dst_ptr++; src_ptr++; 
        *(dst_ptr) = *(src_ptr); dst_ptr++; src_ptr++; 
        *(dst_ptr) = *(src_ptr); dst_ptr++; src_ptr++;   
      }
      for( int32_t c = 0; c < b; c++ )
      {
        *(dst_ptr) = *(src_ptr); dst_ptr++; src_ptr++;
      }
      break;
    }
      
    case PLY_FLOAT:
    {
      float* dst_ptr = (float*)dst;
      float* src_ptr = (float*)src;
      for( int32_t c = 0; c < a; c++ )
      {
        *(dst_ptr) = *(src_ptr); dst_ptr++; src_ptr++; 
        *(dst_ptr) = *(src_ptr); dst_ptr++; src_ptr++; 
        *(dst_ptr) = *(src_ptr); dst_ptr++; src_ptr++;   
      }
      for( int32_t c = 0; c < b; c++ )
      {
        *(dst_ptr) = *(src_ptr); dst_ptr++; src_ptr++;
      }
      break;
    }
      
    case PLY_DOUBLE:
    {
      double* dst_ptr = (double*)dst;
      double* src_ptr = (double*)src;
      for( int32_t c = 0; c < a; c++ )
      {
        *(dst_ptr) = *(src_ptr); dst_ptr++; src_ptr++; 
        *(dst_ptr) = *(src_ptr); dst_ptr++; src_ptr++; 
        *(dst_ptr) = *(src_ptr); dst_ptr++; src_ptr++;   
      }
      for( int32_t c = 0; c < b; c++ )
      {
        *(dst_ptr) = *(src_ptr); dst_ptr++; src_ptr++; 
      }
      break;
    }
    default:
      break;
  }
}

// NOTE(maciej): See if we can vectorize this
PLY_INLINE void
ply_file__data_assign_cast( void* dst, void* src, int32_t type_dst, int32_t type_src, int32_t count )
{
  double data;
  for( int32_t c = 0; c < count ; c++ )
  {
    switch( type_src )
    {
      // intermediate store
      case PLY_UINT8:
        data = (double)(*((uint8_t*)(src)+c));
        break;
      case PLY_UINT16:
        data = (double)(*((uint16_t*)(src)+c));
        break;
      case PLY_UINT32:
        data = (double)(*((uint32_t*)(src)+c));
        break;
      case PLY_INT8:
        data = (double)(*((int8_t*)(src)+c));
        break;
      case PLY_INT16:
        data = (double)(*((int16_t*)(src)+c));
        break;
      case PLY_INT32:
        data = (double)(*((int32_t*)(src)+c));
        break;
      case PLY_FLOAT:
        data = (double)(*((float*)(src)+c));
        break;
      case PLY_DOUBLE:
        data = (double)(*((double*)(src)+c));
        break;
      default:
        break;
    }

    // write
    switch( type_dst )
    {
      case PLY_UINT8:
        *((uint8_t*)(dst)+c) = (uint8_t)data;
        break;
      case PLY_UINT16:
        *((uint16_t*)(dst)+c) = (uint16_t)data;
        break;
      case PLY_UINT32:
        *((uint32_t*)(dst)+c) = (uint32_t)data;
        break;
      case PLY_INT8:
        *((int8_t*)(dst)+c) = (int8_t)data;
        break;
      case PLY_INT16:
        *((int16_t*)(dst)+c) = (int16_t)data;
        break;
      case PLY_INT32:
        *((int32_t*)(dst)+c) = (int32_t)data;
        break;
      case PLY_FLOAT:
        *((float*)(dst)+c) = (float)data;
        break;
      case PLY_DOUBLE:
        *((double*)(dst)+c) = (double)data;
        break;
      default:
        break;
    }
  }
}

int
ply_file__get_property_from_element( ply_file_t* pf, const char* element_name, 
                                    const char** property_names, int num_requested_properties, 
                                    ply_type_id_t requested_type, ply_type_id_t requested_list_type, 
                                    void** data, void** list_data, int32_t *data_count,
                                    int32_t size_hint )
{
  // TODO(maciej): Errors!
  int8_t swap_endianness = pf->format != PLY_ASCII ? (pf->_system_format != pf->format) : 0;
  int32_t num_properties = -1;
  ply_element_t* el = ply_file_find_element(pf, element_name);

  if( !el ) { return PLY_ELEMENT_NOT_FOUND_ERR; }
  else
  {
    // double pt1 = msh_get_time(MSHT_MILLISECONDS);
    num_properties = msh_array_size(el->properties);
    if( el->data == NULL )
    {
      // Check if data layouts agree - if so, we can just copy and return
      int8_t can_simply_copy = 1;
      if( swap_endianness ) { can_simply_copy = 0; }
      if( num_requested_properties != num_properties ) { can_simply_copy = 0; }
      if( can_simply_copy )
      {
        for( int i = 0; i < num_properties; ++i )
        {
          ply_property_t* pr = &el->properties[i];
          const char* a = pr->name;
          const char* b = property_names[i];
          if( strcmp(a,b) ) { can_simply_copy = 0; }
          if( pr->type != requested_type ) { can_simply_copy = 0; }
          if( pr->list_type != PLY_INVALID ) { can_simply_copy = 0;}
        }
      }

      if( can_simply_copy )
      {
        ply_file_get_element_size( el, &el->data_size);
        *data_count = el->count;
        *data = malloc(el->data_size);
        ply_file_get_element_data( pf, el, &*data, el->data_size); 
        return PLY_NO_ERRORS;
      }
      
      ply_file_get_element_size( el, &el->data_size);
      el->data = malloc(el->data_size);
      ply_file_get_element_data( pf, el, &el->data, el->data_size); 
    }
    
    // Initialize output
    *data_count = el->count;
    uint8_t* dst_data = NULL; 
    uint8_t* dst_list = NULL;
    int data_byte_size = -1;
    int list_byte_size = -1;
    int requested_byte_size = ply_file__type_to_byte_size(requested_type);
    int requested_list_byte_size = ply_file__type_to_byte_size(requested_list_type);
    ply_file_get_properties_byte_size(el, property_names, num_requested_properties, 
                                      requested_type, requested_list_type,
                                      &data_byte_size, &list_byte_size);
    if( data != NULL ) 
    { 
      *data = malloc(data_byte_size); 
      dst_data = (uint8_t*)*data;
    }
    if( list_data != NULL  )
    { 
      *list_data = malloc(list_byte_size);
      dst_list = (uint8_t*)*list_data;
    }
    uint8_t* src = (uint8_t*)el->data;

    // Check if need to cast
    int need_cast = 0;
    int8_t requested_group_size[PLY_FILE_MAX_REQ_PROPERTIES] = {0};

    for( int i = 0; i < num_properties; ++i )
    {
      ply_property_t* pr = &el->properties[i];

      for( int j = 0; j < num_requested_properties; ++j )
      {
        if( !strcmp(pr->name, property_names[j]) ) 
        {
          if( pr->type != requested_type ) need_cast = 1;
          if( pr->list_type != PLY_INVALID &&
              pr->list_type != requested_list_type ) need_cast = 1;
        }
      }
    }

    int32_t num_groups = 0;
    for( int32_t i = 0; i < num_properties; ++i )
    {
      ply_property_t* pr = &el->properties[i];
      int32_t k = i;
      for( int32_t j = 0; j < num_requested_properties; ++j )
      {
        if( !strcmp(pr->name, property_names[j]) ) 
        { 
          num_groups++; 
        }

        while( !strcmp(pr->name, property_names[j]) )
        {
          requested_group_size[k]+=1;
          j++;
          i++;
          if( i >= num_properties || j >= num_requested_properties ) break;
          pr = &el->properties[i];
        }
      }
    }


    for( int32_t i = 0; i < num_properties; ++i )
    {
      ply_property_t* pr = &el->properties[i];
      printf("%s %d | %d\n", pr->name, requested_group_size[i], num_groups);
    }

    // TODO(maciej): Make into a struct?
    int32_t precalc_dst_row_size = 0;
    int32_t precalc_dst_list_row_size = 0;
    int32_t precalc_src_row_size = 0;
    int32_t precalc_dst_stride = 0;

    int32_t can_precalculate = ply_file__can_precalculate_sizes(pf, el);
    printf("PRECALC : %d | NEED_CAST : %d\n", can_precalculate, need_cast);
    if( can_precalculate )
    {
      // Determine data row sizes
      for( int32_t j = 0; j < num_properties; ++j )
      {
        ply_property_t* pr = &el->properties[j];
        // pr->list_count = 1;

        // if( pr->list_type != PLY_INVALID ) 
        // {
        //   for( int32_t k=0; k<msh_array_size(pf->_hints); ++k)
        //   {
        //     if( !strcmp(pr->name, pf->_hints[k].property_name) ) 
        //     { 
        //       pr->list_count = pf->_hints[k].expected_size;
        //     }
        //   }
        // }
        printf("List count: %d\n", pr->list_count);
    
        if( requested_group_size[j] )
        {
          if( pr->list_type == PLY_INVALID )
          {
            precalc_dst_stride = requested_group_size[j] * requested_byte_size;
            // printf(" STRIDE: %d\n", precalc_dst_stride);
          }
          else
          {
            precalc_dst_stride = pr->list_count * requested_byte_size;
          }
          precalc_dst_list_row_size += requested_list_byte_size;
          precalc_dst_row_size += precalc_dst_stride;
          pr->offset = precalc_src_row_size + pr->list_byte_size;
          pr->list_offset = precalc_src_row_size;
        }
        // printf("List count: %d\n", pr->list_count);
        precalc_src_row_size += pr->list_byte_size + pr->list_count * pr->byte_size;
      }
    }
  
    // printf( "%d\n", precalc_dst_row_size );
    // printf( "%d\n", precalc_dst_list_row_size );
    // printf( "%d\n", precalc_dst_stride );
    // printf( "%d\n", precalc_src_row_size );

    // Create separate list for just requested properties
    typedef struct ply_property_read_helper
    {
      int32_t offset;
      int32_t type;
      int32_t list_count;
      int32_t list_type;
      int32_t list_offset;
    } ply_property_read_helper_t;
    ply_property_read_helper_t requested_properties[PLY_FILE_MAX_REQ_PROPERTIES] = {{0}};

    int32_t pr_count = 0;
    for( int32_t i = 0; i < num_properties; ++i )
    {
      ply_property_t *pr = &el->properties[i];
      if( requested_group_size[i] )
      {
        requested_properties[pr_count].offset = pr->offset; 
        requested_properties[pr_count].list_count = pr->list_count; 
        requested_properties[pr_count].list_offset = pr->list_offset; 
        requested_properties[pr_count].type = pr->type;
        requested_properties[pr_count].list_type = pr->list_type;
        if( pr->list_type == PLY_INVALID )
        {
          requested_properties[pr_count].list_count  = requested_group_size[i];
        }
        pr_count += requested_group_size[i];
      }
    }
    assert(pr_count == num_requested_properties );



    // Start copying
    for( int i = 0; i < el->count; ++i )
    {
      int32_t dst_row_size = 0;
      int32_t dst_list_row_size = 0;
      int32_t dst_offset = 0; 
      int32_t dst_list_offset = 0;
      int32_t dst_stride = 0;
      int32_t src_row_size = 0;

      // Calculate the required data to read
      if( can_precalculate )
      {
        dst_row_size = precalc_dst_row_size;
        dst_list_row_size = precalc_dst_list_row_size;
        dst_stride = precalc_dst_stride;
        src_row_size = precalc_src_row_size;
      }
      else
      {
        int pr_count = 0;
        for( int j = 0; j < num_properties; ++j )
        {
          ply_property_t* pr = &el->properties[j];
          pr->list_count = 1;

          if( pr->list_type != PLY_INVALID )
          {
            pr->list_count = ply_file__get_data_as_int(src+src_row_size, pr->list_type, swap_endianness);
          }
          dst_stride = pr->list_count * requested_byte_size;

          if( requested_group_size[j] )
          {
            dst_list_row_size += requested_list_byte_size;
            dst_row_size += pr->list_count * requested_byte_size;
            pr->offset = src_row_size + pr->list_byte_size;
            pr->list_offset = src_row_size;

            requested_properties[pr_count].offset = pr->offset; 
            requested_properties[pr_count].list_count = pr->list_count; 
            requested_properties[pr_count].list_offset = pr->list_offset; 
            pr_count++;
          }
          src_row_size += pr->list_byte_size + pr->list_count * pr->byte_size;
        }
      }
   
      // TODO(maciej): Maybe make casting a separate function
      if( need_cast )
      {
        for( int32_t j = 0; j < num_groups; ++j )
        {
          ply_property_read_helper_t* prh = &requested_properties[j];
          if( dst_data )
          {
            void* dst_ptr = (dst_data + dst_offset);
            void* src_ptr = (src + prh->offset);
            ply_file__data_assign_cast( dst_ptr, src_ptr, requested_type, prh->type, prh->list_count );
            dst_offset += dst_stride; // NOTE(maciej): Would need to make per property to enable swizzle
            if( swap_endianness ) ply_file__swap_bytes( (uint8_t*)dst_ptr, requested_byte_size, prh->list_count );
          }

          if( dst_list )
          {
            void* dst_ptr = (dst_list + dst_list_offset);
            void* src_ptr = (src + prh->list_offset);
            ply_file__data_assign_cast( dst_ptr, src_ptr, requested_list_type, prh->list_type, 1 );
            dst_list_offset += requested_list_byte_size;
            if( swap_endianness ) ply_file__swap_bytes( (uint8_t*)dst_ptr, requested_list_byte_size, 1 );
          }
        }
      }
      else
      {
        for( int32_t j = 0; j < num_groups; ++j )
        {
          ply_property_read_helper_t* prh = &requested_properties[j];
          if( dst_data )
          {
            void* dst_ptr = (dst_data + dst_offset);
            void* src_ptr = (src + prh->offset);
            ply_file__data_assign( dst_ptr, src_ptr, prh->type, prh->list_count );
            dst_offset += dst_stride;
            if( swap_endianness ) ply_file__swap_bytes( (uint8_t*)dst_ptr, requested_byte_size, prh->list_count );
          }

          if( dst_list )
          {
            void* dst_ptr = (dst_list + dst_list_offset);
            void* src_ptr = (src + prh->list_offset);
            ply_file__data_assign( dst_ptr, src_ptr, prh->list_type, 1 );
            dst_list_offset += requested_list_byte_size;
            if( swap_endianness ) ply_file__swap_bytes( (uint8_t*)dst_ptr, requested_list_byte_size, 1 );
          }
        }
      }

      src += src_row_size;
      if(dst_data) dst_data += dst_row_size;
      if(dst_list) dst_list += dst_list_row_size;
    }
  }


  return PLY_NO_ERRORS;
}

int
ply_file_get_property_from_element( ply_file_t* pf, ply_file_property_desc_t* desc )
{
  // TODO(maciej): Check for null ptrs etc.
  return ply_file__get_property_from_element( pf, desc->element_name, 
                                desc->property_names, desc->num_requested_properties, 
                                desc->requested_type, desc->requested_list_type, 
                                (void**)desc->requested_data, (void**)desc->requested_list_data, 
                                desc->requested_data_count, desc->size_hint );
}


// ENCODER

int  
ply_file__add_element(ply_file_t* pf, const char* element_name, const int element_count)
{
  ply_element_t el = {{0}};
  strncpy( &el.name[0], element_name, 64 );
  el.count = element_count;
  el.properties = NULL;
  msh_array_push( pf->elements, el );
  return PLY_NO_ERRORS;
}


void
ply_file__fprint_data_at_offset(const ply_file_t* pf, const void* data, const int32_t offset, const int32_t type)
{
  switch(type)
  {
    case PLY_UINT8:  fprintf(pf->_fp,"%d ", *(uint8_t*)(data)+offset); break;
    case PLY_UINT16: fprintf(pf->_fp,"%d ", *(uint16_t*)(data)+offset); break;
    case PLY_UINT32: fprintf(pf->_fp,"%d ", *(uint32_t*)(data)+offset); break;
    case PLY_INT8:   fprintf(pf->_fp,"%d ", *(int8_t*)(data)+offset); break;
    case PLY_INT16:  fprintf(pf->_fp,"%d ", *(int16_t*)(data)+offset); break;
    case PLY_INT32:  fprintf(pf->_fp,"%d ", *(int32_t*)(data)+offset); break;
    case PLY_FLOAT:  fprintf(pf->_fp,"%f ", *(float*)(data)+offset); break;
    case PLY_DOUBLE: fprintf(pf->_fp,"%f ", *(double*)(data)+offset); break;
  }
}


// TODO(maciej): This function needs a rework, it is bit messy
int  
ply_file_add_property_to_element(ply_file_t* pf, const char* element_name, 
                  const char** property_names, int num_properties,
                  const ply_type_id_t data_type, const ply_type_id_t list_type, void* data, void* list_data,
                  int element_count )
{
  // Check if list type is integral type
  if( list_type == PLY_FLOAT || list_type == PLY_DOUBLE )
  {
    return PLY_INVALID_LIST_TYPE_ERR;
  }

  int8_t swap_endianness = pf->format != PLY_ASCII ? (pf->_system_format != pf->format) : 0;

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
    for( int i = 0; i < num_properties; ++i )
    {
      ply_property_t pr;
      strncpy(&pr.name[0], property_names[i], 32);

      // check for hints
      int hinted_list_count = 0;
      if( list_type != PLY_INVALID ) 
      {
        for( int k=0; k<msh_array_size(pf->_hints); ++k)
        {
          if( !strcmp(pr.name, pf->_hints[k].property_name) ) 
          { 
            hinted_list_count = pf->_hints[k].expected_size;
          }
        }
      }

      pr.type = data_type;
      pr.list_type = list_type;
      pr.byte_size = ply_file__type_to_byte_size( pr.type );
      pr.list_byte_size = ply_file__type_to_byte_size( pr.list_type );
      pr.list_count = (list_type == PLY_INVALID) ? 1 : hinted_list_count; // TODO(If se have a hint, set this to a hint)
      pr.data = data;
      pr.list_data = list_data;
      pr.offset = pr.byte_size * i;
      pr.stride = pr.byte_size * num_properties;
      pr.list_offset = pr.list_byte_size * i;
      pr.list_stride = pr.list_byte_size * num_properties;
      pr.total_byte_size = (pr.list_byte_size + pr.list_count * pr.byte_size) * el->count;
     
      if( pr.list_count == 0 ) // No hint was present
      {
        pr.total_byte_size = 0;
        for( int j = 0; j < element_count; ++j )
        {
          // NOTE(list type needs to be dereferenced to correct type)
          int offset = (pr.list_offset + j * pr.list_stride);
          int list_count = ply_file__get_data_as_int((uint8_t*)list_data + offset, list_type, swap_endianness);
          pr.total_byte_size += pr.list_byte_size + list_count * pr.byte_size;
          if( j == 0 ) // We care only about initial offset, so first element of list counts
          {
            pr.offset = init_offset;
            init_offset += list_count * pr.byte_size;
            printf("Offset %lld\n", pr.offset);
          }
        }
      }
      else
      {
        pr.stride *= pr.list_count;
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
                                         msh_array(ply_property_t) el_properties,
                                         int swap_endianness )
{
  int64_t stride = 0;
  int64_t offsets[PLY_FILE_MAX_PROPERTIES] = {0};
  for( int l = 0; l < msh_array_size(el_properties); ++l )
  {
    ply_property_t* qr = &el_properties[l];
    offsets[l] = qr->list_offset;
  }

  for( int l = 0; l < msh_array_size(el_properties); ++l )
  {
    ply_property_t* qr = &el_properties[l];
    if( pr->data == qr->data )
    {
      int32_t list_count = ply_file__get_data_as_int((uint8_t*)qr->list_data + offsets[l], qr->list_type, swap_endianness );
      stride += list_count * qr->byte_size;
      offsets[l] += qr->list_stride;
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
      case PLY_ASCII: { format_string = (char*)"ascii"; break; }
      case PLY_LITTLE_ENDIAN: { format_string = (char*)"binary_little_endian"; break; }
      case PLY_BIG_ENDIAN: { format_string = (char*)"binary_big_endian"; break;  }
      default: { return PLY_UNRECOGNIZED_FORMAT_ERR; }
    }

    fprintf(pf->_fp, "ply\nformat %s %2.1f\n", 
                      format_string, (float)pf->format_version);
    for( int i = 0; i < msh_array_size(pf->elements); ++i )
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
          pr->stride = ply_file__calculate_list_property_stride( pr, el->properties, 0 );

          int list_count = ply_file__get_data_as_int( (uint8_t*)pr->list_data + pr->list_offset, pr->list_type, 0 );
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


// TODO(maciej): Test if splitting into completly separate functions helps
int
ply_file__write_data_binary( const ply_file_t* pf )
{
  int8_t swap_endianness = (pf->_system_format != pf->format);
  for( int i = 0 ; i < msh_array_size(pf->elements); ++i )
  {
    ply_element_t* el = &pf->elements[i];
    int32_t buffer_size = 0;
    uint32_t hinted_list_counts[PLY_FILE_MAX_PROPERTIES] = {0};
    for( int j = 0 ; j < msh_array_size(el->properties); ++j )
    {
      ply_property_t* pr = &el->properties[j];
      if( pr->list_type != PLY_INVALID ) 
      {
        for( int k = 0; k < msh_array_size(pf->_hints); ++k)
        {
          if( !strcmp(pr->name, pf->_hints[k].property_name) ) 
          {
            hinted_list_counts[j] = pf->_hints[k].expected_size;
          }
        }
      }
      buffer_size += el->properties[j].total_byte_size;
    }

    uint8_t* dst = (uint8_t*)malloc( buffer_size );
    int64_t dst_offset = 0;
    
    for( int j = 0; j < el->count; ++j )
    {
      for( int k = 0; k < msh_array_size(el->properties); ++k )
      {
        ply_property_t* pr = &el->properties[k];
        if( pr->list_type == PLY_INVALID )
        {
          ply_file__data_assign( dst + dst_offset, (uint8_t*)pr->data + pr->offset, pr->type, 1 );
          pr->offset += pr->stride;
          dst_offset += pr->byte_size;
        }
        else
        {
          // figure out stride. 
          // NOTE(maciej): make sure that no data and no hint is an error
          if( hinted_list_counts[k] == 0 )
          {
            pr->stride = ply_file__calculate_list_property_stride( pr, el->properties, swap_endianness );
            pr->list_count = ply_file__get_data_as_int((uint8_t*)pr->list_data + pr->list_offset, pr->list_type, swap_endianness );
            
            ply_file__data_assign( dst + dst_offset, (uint8_t*)pr->list_data + pr->list_offset, pr->list_type, 1 );
            pr->list_offset += pr->list_stride;
            dst_offset += pr->list_byte_size;
          }
          else
          {
            pr->list_count = hinted_list_counts[k];
            // TODO(maciej): Just write hinted_list_counts[k] into memory
            // TODO(maciej): Check for errors.
            switch( pr->list_type )
            {
              case PLY_UINT8:
              {
                uint8_t* dst_ptr = (uint8_t*)dst + dst_offset;
                *dst_ptr = (uint8_t)pr->list_count;
                break;
              }
              case PLY_UINT16:
              {
                uint16_t* dst_ptr = (uint16_t*)dst + dst_offset;
                *dst_ptr = (uint16_t)pr->list_count;
                break;
              }
              case PLY_UINT32:
              {
                uint32_t* dst_ptr = (uint32_t*)dst + dst_offset;
                *dst_ptr = (uint32_t)pr->list_count;
                break;
              }
              case PLY_INT8:
              {
                int8_t* dst_ptr = (int8_t*)dst + dst_offset;
                *dst_ptr = (int8_t)pr->list_count;
                break;
              }
              case PLY_INT16:
              {
                int16_t* dst_ptr = (int16_t*)dst + dst_offset;
                *dst_ptr = (int16_t)pr->list_count;
                break;
              }
              case PLY_INT32:
              {
                int32_t* dst_ptr = (int32_t*)dst + dst_offset;
                *dst_ptr = (int32_t)pr->list_count;
                break;
              }
              default:
              {
                uint8_t* dst_ptr = (uint8_t*)dst + dst_offset;
                *dst_ptr = (uint8_t)pr->list_count;
                break;
              }
            }
            dst_offset += pr->list_byte_size;
          }

          ply_file__data_assign( dst + dst_offset, (uint8_t*)pr->data + pr->offset, pr->type, pr->list_count );
          pr->offset += pr->stride;
          dst_offset += pr->byte_size * pr->list_count;
        }
      }
    }
    fwrite( dst, buffer_size, 1, pf->_fp );
  }
  return PLY_NO_ERRORS;
}

int
ply_file__write_data( const ply_file_t* pf )
{
  int error = PLY_NO_ERRORS;
  if( pf->format == PLY_ASCII ) { error = ply_file__write_data_ascii(pf); }
  else                          { error = ply_file__write_data_binary(pf); }
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

int32_t 
ply_file__synchronize_list_sizes( ply_file_t *pf )
{
  for( int k = 0 ; k < msh_array_size( pf->descriptors ); ++k )
  {
    ply_file_property_desc_t* desc = pf->descriptors[k];
    ply_element_t* el = NULL;
    for( int32_t i = 0 ; i < msh_array_size(pf->elements); ++i )
    {
      ply_element_t* cur_el = &pf->elements[i];
      if( !strcmp(desc->element_name, cur_el->name) )
      {
        el = cur_el;
        break;
      }
    }
    if( el == NULL ) { return PLY_REQUESTED_ELEMENT_IS_MISSING; }

    for( int32_t i = 0; i < desc->num_requested_properties; ++i )
    {
      int32_t found = 0;
      for( int32_t j = 0; j < msh_array_size( el->properties ); ++j )
      {
        ply_property_t* pr = &el->properties[j];
        if( !strcmp( pr->name, desc->property_names[i] ) )
        {
          if( pr->list_type != PLY_INVALID ) { pr->list_count = desc->size_hint; }
          else { pr->list_count = 1; desc->size_hint = 1; }
          found = 1;
          break;
        }
      }
      if( !found ) { return PLY_REQUESTED_PROPERTY_IS_MISSING; }
    }

    for( int32_t i = 0; i < msh_array_size( el->properties ); ++i )
    {
      printf("%s -> %d\n", el->properties[i].name, el->properties[i].list_count );
    }
    printf("\n");
  }

  return PLY_NO_ERRORS;
}

int32_t 
ply_file_add_descriptor( ply_file_t *pf, ply_file_property_desc_t* desc )
{
  if( !pf ) { return PLY_FILE_NOT_OPEN_ERR; }
  msh_array_push( pf->descriptors, desc);
  return PLY_NO_ERRORS;
}


int
ply_file_read(ply_file_t* pf)
{
  int error = PLY_NO_ERRORS;
  if( !pf->_fp ) { return PLY_FILE_NOT_OPEN_ERR; }

  if( msh_array_size( pf->descriptors ) == 0 ) { return PLY_NO_REQUESTS; }

  error = ply_file_parse_header( pf );
  if( error ) { return error; }
  error = ply_file__synchronize_list_sizes( pf ); 
  if( error ) { return error; }
  error = ply_file_parse_contents( pf );
  if( error ) { return error; }
  
  for( int32_t i = 0; i < msh_array_size( pf->descriptors ); ++i )
  {
    ply_file_property_desc_t *desc = pf->descriptors[i];
    error = ply_file_get_property_from_element( pf, desc );
    if( error ) { return error; }
  }

  return error;
}

void
ply_file_print_header(ply_file_t* pf)
{
  // Find property & element with longest name
  size_t max_pr_length = 0;

  for( int i = 0 ; i < msh_array_size(pf->elements); ++i )
  {
    for( int j = 0 ; j < msh_array_size(pf->elements[i].properties); ++j )
    {
      max_pr_length = msh_max(strlen(pf->elements[i].properties[j].name),
                              max_pr_length);
    }
  }

  char *type_str = NULL;
  char *list_type_str = NULL;
  char spaces[PLY_FILE_MAX_STR_LEN];
  for( int i = 0 ; i < PLY_FILE_MAX_STR_LEN; ++i ) { spaces[i] = ' ';}
  spaces[PLY_FILE_MAX_STR_LEN-1] = 0;

  char *format_str = NULL;
  if(pf->format == PLY_ASCII)         { format_str = (char*)"Ascii"; }
  if(pf->format == PLY_LITTLE_ENDIAN) { format_str = (char*)"Binary Little Endian"; }
  if(pf->format == PLY_BIG_ENDIAN)    { format_str = (char*)"Binary Big Endian"; }
  printf("PLY: %s %d\n", format_str, pf->format_version);
  for( int i = 0 ; i < msh_array_size(pf->elements); ++i )
  {
    printf("   '%s' count: %d\n", pf->elements[i].name,
                                    pf->elements[i].count);
    for( int j = 0 ; j < msh_array_size(pf->elements[i].properties); ++j )
    {
      size_t n_spaces = max_pr_length - strlen(pf->elements[i].properties[j].name);
      spaces[n_spaces] = 0;
      ply_file__property_type_to_string( pf->elements[i].properties[j].type, &type_str );
      if(pf->elements[i].properties[j].list_type == PLY_INVALID)
      {
        printf("      '%s'%s | %7s(%2d bytes)\n", pf->elements[i].properties[j].name,
                                            spaces, type_str, pf->elements[i].properties[j].byte_size );
      }
      else
      {
        ply_file__property_type_to_string( pf->elements[i].properties[j].list_type, &list_type_str );
        printf("      '%s'%s | %7s(%2d bytes) %7s(%2d bytes)\n", 
                                        pf->elements[i].properties[j].name, spaces, 
                                        list_type_str, pf->elements[i].properties[j].list_byte_size,
                                        type_str, pf->elements[i].properties[j].byte_size );
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
    pf->elements       = 0;
    pf->descriptors    = 0;
    pf->_cur_element   = 0;
    pf->_hints         = 0;
    pf->_fp            = fp;
  
    // endianness check
    int n = 1;
    if(*(char *)&n == 1) { pf->_system_format = PLY_LITTLE_ENDIAN; }
    else                 { pf->_system_format = PLY_BIG_ENDIAN; }
  }
  if( mode[0] == 'w' )
  {
    pf->format_version = 1;
    pf->format = PLY_ASCII;
    if( strlen(mode) > 1 && mode[1] == 'b' ) { pf->format = pf->_system_format; }
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
  if(pf->descriptors) msh_array_free(pf->descriptors);
  if(pf->_hints) msh_array_free(pf->_hints);
  free(pf);
  return 1;
}

////////////////////////////////////////////////////////////////////////////////
// void write_ply_file( const char* filename )
// {
//   double t1 = msh_get_time(MSHT_MILLISECONDS);
//   const char* positions_names[] = {"x", "y", "z"};
//   const char* normals_names[]   = {"nx", "ny", "nz"};
//   const char* colors_names[]    = {"red", "green", "blue", "alpha"};
//   float positions[] = { 1.0f, 0.0f, 1.0f, 
//                         0.0f, 0.0f, 1.0f,
//                         1.0f, 0.0f, 0.0f };
//   float normals[] = { 0.0f, 1.0f, 0.0f, 
//                       0.0f, 1.0f, 0.0f,
//                       0.0f, 1.0f, 0.0f };
//   uint8_t colors[] = { 255, 125, 125, 255, 
//                      0, 255, 0, 255,
//                      0, 0, 255, 255 };
//   const char* face_names[] = {"vertex_indices"};
//   int face_ind[] = {0,1,2, 3,4,5,6, 7,8};
//   unsigned char counts[] = {3,4,2};

//   ply_file_t* pf = ply_file_open(filename, "wb");
//   pf->format = PLY_BIG_ENDIAN;
//   ply_file_add_property_to_element(pf, "vertex", positions_names, 3, PLY_FLOAT, PLY_INVALID, positions, NULL, 3 );
//   ply_file_add_property_to_element(pf, "vertex", normals_names, 3, PLY_FLOAT, PLY_INVALID, normals, NULL, 3 );
//   ply_file_add_property_to_element(pf, "vertex", colors_names, 4, PLY_UINT8, PLY_INVALID, colors, NULL, 3 );
//   ply_file_add_property_to_element(pf, "face", face_names, 1, PLY_INT32, PLY_UINT8, face_ind, counts, 3 );


//   ply_file_write(pf);
//   ply_file_close(pf);
//   double t2 = msh_get_time(MSHT_MILLISECONDS);
//   printf("Done in %f milliseconds\n", t2-t1);
// }

// void read_ply_file( const char* filename )
// {
//   printf("Reading file : %s\n", filename);
//   ply_file_t* pf = ply_file_open(filename, "rb");
//   ply_hint_t indices_size_hint = {.property_name="vertex_indices", .expected_size=3};
//   ply_file_add_hint(pf, indices_size_hint);
//   if( pf )
//   {
//     ply_file_parse(pf);
//     const char* positions_names[] = {"x","y","z"};
//     const char* vertex_indices_names[] = {"vertex_indices"};
//     float* positions = NULL;
//     int n_verts = -1;
//     int* indices = NULL;
//     int n_faces = -1;
//     uint8_t* indices_counts = NULL;
//     ply_file_get_property_from_element(pf, "vertex", positions_names, 3, PLY_FLOAT, PLY_INVALID, 
//                                       (void**)&positions, NULL, &n_verts );
//     ply_file_get_property_from_element(pf, "face", vertex_indices_names, 1, PLY_INT32, PLY_UINT8, 
//                                        (void**)&indices, NULL, &n_faces);

//     printf("Vertex count: %d\n", n_verts);
//     for( int i = 0 ; i < 3; ++i )
//     {
//       printf("%f %f %f\n", positions[3*i+0], positions[3*i+1], positions[3*i+2]);
//     }
//     for( int i = n_verts-1 ; i > n_verts-4 ; --i )
//     {
//       printf("%f %f %f\n", positions[3*i+0], positions[3*i+1], positions[3*i+2]);
//     }

//     printf("Face Count: %d\n", n_faces);
//     for( int i = 0; i < n_faces; ++i )
//     {
//       int should_print = (i < 3 || i > n_faces - 4);
//       int count = 3;//indices_counts[i];
//       msh_cprintf(should_print, "%d | ", count);
//       for( int j = 0 ; j < count; ++j )
//       {
//         msh_cprintf(should_print, "%d ", *indices );
//         indices+=1;
//       }
//       msh_cprintf(should_print,"\n");
//     }
//   }
//   // ply_file_print_header(pf);
//   ply_file_close(pf);
// }

// int main(int argc, char** argv)
// {
//   if(argc < 2) {printf("Please provide .ply filename\n"); return 0;} 
//   char* filename = argv[1];

//   // write_ply_file(filename);
//   read_ply_file(filename);

//   return 1;
// }
// 