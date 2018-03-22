/*
TODOs:
[x] Inform program of expected number of list elements
[x] Clean properties request API
[x] Ascii format reading
[x]   Add ascii format size calculation for list properties
[ ] Ply creation API
  [ ] Add multiple properties add command
  [ ] Add actual writing of files
[ ] Encoder/decorder split
[ ] Error reporting
[ ] Revise errors
[ ] Code cleanup
  [ ] Replace duplicated code
  [ ] Decide what to do with malloc(ie. Should we alloc memory? We are using FILE* anyway..)
[ ] Simplify API
[ ] Fix the header names to be mply
[ ] Replace msh_array
[ ] Extensive testing
[ ] Optimize
*/

#include <stdlib.h>
#include <stdint.h>
#include "msh.h"


////////////////////////////////////////////////////////////////////////////////////////////////////
// This ply library only provides set of functionalities to read/write your own specific ply file.
// pf_file_t does not actually store any mesh data. You need to provide buffers to which such data
// can be written.
////////////////////////////////////////////////////////////////////////////////////////////////////

#define PLY_MAX_STR_LEN 512

// the variables are non-descriptive here
typedef struct ply_property
{
  char name[64];

  int type;
  int list_type;
  int list_count;

  // Should we store those or calculate on the fly
  int byte_size;
  int list_byte_size;

  int total_count;
  int total_byte_size;
} ply_property_t;

typedef struct ply_element
{
  char name[64];
  int count;
  msh_array(ply_property_t) properties;
  int file_anchor;
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

// IO calls
ply_file_t* ply_file_open(const char* filename, const char* mode);
int  ply_file_close(ply_file_t* pf);

// Helper
void ply_file_print_header(ply_file_t* pf);
int  ply_file_add_hint(ply_file_t* pf, ply_hint_t hint);

// DECODER : Parsing
int  ply_file_parse_header(ply_file_t* pf);
int  ply_file_parse_contents(ply_file_t* pf);

// DECODER : Reading API - getting data from ply
int  ply_file_get_element(ply_file_t* pf, const char* element_name, ply_element_t** el);
int  ply_file_get_element_count(ply_file_t* pf, const char* element_name, int* count);
int  ply_file_get_element_size(ply_file_t* pf, const char* element_name, int* size);
int  ply_file_get_element_data(ply_file_t* pf, const char* element_name, void** data);

int  ply_file_get_property(ply_file_t* pf, const char* element_name, const char* property_name, 
                           ply_property_t** pr);
int  ply_file_get_properties_size(ply_file_t* pf, const char* element_name, 
                                  const char** properties_names, int n_properties, int* size);


int ply_file_get_properties_from_element(ply_file_t* pf, const char* element_name,  void* element_data,
                                          const char** req_properties, void** req_data );
int  ply_file_get_list_property_from_element(ply_file_t* pf,  const char* element_name, 
    	                                      void* element_data, const char* property_name, 
       		                                  void** list_lengths, void** list_data);

// ENCODER : Writing API - getting data to ply
int  ply_file_add_element(ply_file_t* pf, const char* element_name, const int element_count);
// NOTE(maciej): Should make this to be closer to  encoder part.
int  ply_file_add_property_to_element(ply_file_t* pf, 
                           const char* element_name, const char* property_name, int property_type);
int  ply_file_add_list_property(ply_file_t* pf, 
             const char* element_name, const char* property_name, int list_type, int property_type);
int  ply_file_write_header(ply_file_t* pf);
int  ply_file_write_element_data( ply_file_t* pf, const char* element_name, void* data );







////////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////////


typedef enum ply_err
{
  PLY_NO_ERRORS = 0,
  PLY_INVALID_FILE_ERR,
  PLY_INVALID_FORMAT_ERR,
  PLY_FILE_OPEN_ERR,
  PLY_LINE_PARSE_ERR,
  PLY_FORMAT_CMD_ERR,
  PLY_ELEMENT_CMD_ERR,
  PLY_PROPERTY_CMD_ERR,
  PLY_FORMAT_UNRECOGNIZED_ERR,
  PLY_ELEMENT_NOT_FOUND_ERR,
  PLY_PROPERTY_NOT_FOUND_ERR,
  PLY_REQUESTED_PROPERTY_NOT_A_LIST_ERR,
  PLY_PARSE_ERROR,
  PLY_BIG_ENDIAN_ERR,
  PLY_BINARY_FILE_READ_ERR,
  PLY_UNRECOGNIZED_CMD_ERR
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

static char* ply_type_str[PLY_N_TYPES] = {"%s","%c","%uc","%s","%us","%d","%ud","%f","%g"};



int
ply__ply_parse_cmd(char* line, ply_file_t* pf)
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
ply__parse_format_cmd(char* line, ply_file_t* pf)
{
  char cmd[PLY_MAX_STR_LEN];
  char frmt_str[PLY_MAX_STR_LEN];
  char frmt_ver_str[PLY_MAX_STR_LEN];
  if(sscanf(line, "%s %s %s", &cmd[0], &frmt_str[0], &frmt_ver_str[0]) != 3) { return PLY_FORMAT_CMD_ERR; } 
  if(!strcmp("ascii", frmt_str)){ pf->format = (int)PLY_ASCII; }
  else if(!strcmp("binary_little_endian", frmt_str)){ pf->format = (int)PLY_LITTLE_ENDIAN; }
  else if(!strcmp("binary_big_endian", frmt_str)){ pf->format = (int)PLY_BIG_ENDIAN; }
  else{ return PLY_FORMAT_UNRECOGNIZED_ERR; }
  pf->format_version = atoi(frmt_ver_str);
  return PLY_NO_ERRORS;
}

int
ply__parse_element_cmd(char* line, ply_file_t* pf)
{
  char cmd[PLY_MAX_STR_LEN];
  ply_element_t el;
  el.properties = NULL;
  if(sscanf(line, "%s %s %d", &cmd[0], &el.name[0], &el.count) != 3) { return PLY_ELEMENT_CMD_ERR; }
  msh_array_push(pf->elements, el);
  pf->cur_element = msh_array_back(pf->elements);
  return PLY_NO_ERRORS;
}

void
ply__string_to_property_type(char* type_str, int* pr_type, int* pr_size)
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
ply__parse_property_cmd(char* line, ply_file_t* pf)
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
    ply__string_to_property_type(type_str, &pr.type, &pr.byte_size);
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
    
    ply__string_to_property_type(type_str, &pr.type, &pr.byte_size);
    ply__string_to_property_type(list_type_str, &pr.list_type, &pr.list_byte_size);
    pr.list_count = -1;//UNKNOWN (TODO: Add maybe enum to indicate this?)
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
ply__parse_command(char* cmd, char* line, ply_file_t* pf)
{
  if(!strcmp(cmd, "ply"))      { return ply__ply_parse_cmd(line, pf); }
  if(!strcmp(cmd, "format"))   { return ply__parse_format_cmd(line, pf); }
  if(!strcmp(cmd, "element"))  { return ply__parse_element_cmd(line, pf); }
  if(!strcmp(cmd, "property")) { return ply__parse_property_cmd(line, pf); }
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
    err_code = ply__parse_command(cmd, line, pf);
  }
  pf->_header_size = ftell(pf->_fp);
  return err_code;
}

#define PLY_CONVERT_AND_COPY(C, D, T, conv_funct) {\
  T n = (T)conv_funct(C); \
  memcpy((void*)D, (void*)&n, sizeof(T)); \
  D+=sizeof(T);}


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
  printf("------------\n");
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

  char type[PLY_MAX_STR_LEN];
  char list_type[PLY_MAX_STR_LEN];
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
      switch( pf->elements[i].properties[j].type )
      {
        case PLY_INT8:   { snprintf(type, 7, "char"); break; }
        case PLY_INT16:  { snprintf(type, 7, "short"); break; }
        case PLY_INT32:  { snprintf(type, 7, "int"); break; }
        case PLY_UINT8:  { snprintf(type, 7, "uchar"); break; }
        case PLY_UINT16: { snprintf(type, 7, "ushort"); break; }
        case PLY_UINT32: { snprintf(type, 7, "uint"); break; }
        case PLY_FLOAT:  { snprintf(type, 7, "float"); break; }
        case PLY_DOUBLE: { snprintf(type, 7, "double"); break; }
      }
      if(pf->elements[i].properties[j].list_type == PLY_INVALID)
      {
        printf("      '%s'%s | %7s(%2d bytes)\n", pf->elements[i].properties[j].name,
                                            spaces, type, pf->elements[i].properties[j].byte_size );
      }
      else
      {
        switch( pf->elements[i].properties[j].list_type )
        {
          case PLY_INT8:   { snprintf(list_type, 7, "char"); break; }
          case PLY_INT16:  { snprintf(list_type, 7, "short"); break; }
          case PLY_INT32:  { snprintf(list_type, 7, "int"); break; }
          case PLY_UINT8:  { snprintf(list_type, 7, "uchar"); break; }
          case PLY_UINT16: { snprintf(list_type, 7, "ushort"); break; }
          case PLY_UINT32: { snprintf(list_type, 7, "uint"); break; }
          case PLY_FLOAT:  { snprintf(list_type, 7, "float"); break; }
          case PLY_DOUBLE: { snprintf(list_type, 7, "double"); break; }
        }
        printf("      '%s'%s | %7s(%2d bytes) %7s(%2d bytes)\n", 
                                        pf->elements[i].properties[j].name, spaces, 
                                        list_type, pf->elements[i].properties[j].list_byte_size,
                                        type, pf->elements[i].properties[j].byte_size );
      }
      spaces[n_spaces] = ' ';
    }
 }
}

int
ply_file_get_element(ply_file_t* pf, const char* element_name, ply_element_t** el)
{
  int err_code = PLY_NO_ERRORS;
  // find requested element
  *el = NULL;
  for(int i=0;i<msh_array_size(pf->elements);++i)
  {
    if(!strcmp(pf->elements[i].name, element_name)) { *el = &pf->elements[i]; break; }
  }
  if( !*el ) return PLY_ELEMENT_NOT_FOUND_ERR;

  return err_code;
}

int
ply_file_get_property(ply_file_t* pf, const char* element_name,const char* property_name, 
                      ply_property_t** pr)
{
  int err_code = PLY_NO_ERRORS;
  // find requested element
  ply_element_t* el = NULL;
  err_code = ply_file_get_element(pf, element_name, &el);
  if( err_code ) return err_code;

  *pr = NULL;
  for(int i=0;i<msh_array_size(el->properties);++i)
  {
    if(!strcmp(el->properties[i].name, property_name)) { *pr = &el->properties[i]; break; }
  }
  if( !*pr ) return PLY_PROPERTY_NOT_FOUND_ERR;

  return err_code;
}

int
ply_file_get_element_count(ply_file_t* pf, const char* element_name, int* count)
{
  int err_code = PLY_NO_ERRORS;
  // find requested element
  ply_element_t* el = NULL;
  for(int i=0;i<msh_array_size(pf->elements);++i)
  {
    if(!strcmp(pf->elements[i].name, element_name)) el = &pf->elements[i];
  }
  if( !el ) return PLY_ELEMENT_NOT_FOUND_ERR;

  *count = el->count;

  return err_code;
}

int
ply_file_get_element_size(ply_file_t* pf, const char* element_name, int* size)
{
  int err_code = PLY_NO_ERRORS;
  // find requested element
  ply_element_t* el = NULL;
  for(int i=0;i<msh_array_size(pf->elements);++i)
  {
    if(!strcmp(pf->elements[i].name, element_name)) el = &pf->elements[i];
  }
  if( !el ) return PLY_ELEMENT_NOT_FOUND_ERR;

  *size = 0;
  for(int i= 0; i<msh_array_size(el->properties); ++i)
  {
    *size += el->properties[i].total_byte_size;
  }
  return err_code;
}

int
ply_file__get_element_data_ascii(ply_file_t* pf, const char* element_name, void** storage)
{
  int err_code = PLY_NO_ERRORS;
  // find requested element
  ply_element_t* el = NULL;
  for(int i=0;i<msh_array_size(pf->elements);++i)
  {
    if(!strcmp(pf->elements[i].name, element_name)) el = &pf->elements[i];
  }
  if( !el ) return PLY_ELEMENT_NOT_FOUND_ERR;

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
        switch(pr->type)
        {
          case PLY_INT8:   PLY_CONVERT_AND_COPY(cp_low, dest, int8_t,   atoi); break;
          case PLY_INT16:  PLY_CONVERT_AND_COPY(cp_low, dest, int16_t,  atoi); break;
          case PLY_INT32:  PLY_CONVERT_AND_COPY(cp_low, dest, int32_t,  atoi); break;
          case PLY_UINT8:  PLY_CONVERT_AND_COPY(cp_low, dest, uint8_t,  atoi); break;
          case PLY_UINT16: PLY_CONVERT_AND_COPY(cp_low, dest, uint16_t, atoi); break;
          case PLY_UINT32: PLY_CONVERT_AND_COPY(cp_low, dest, uint32_t, atoi); break;
          case PLY_FLOAT:  PLY_CONVERT_AND_COPY(cp_low, dest, float,    atof); break;
          case PLY_DOUBLE: PLY_CONVERT_AND_COPY(cp_low, dest, double,   atof); break;
        }
      }
      else //list pr
      {
        switch(pr->list_type)
        {
          case PLY_INT8:   PLY_CONVERT_AND_COPY(cp_low, dest, int8_t,   atoi); break;
          case PLY_INT16:  PLY_CONVERT_AND_COPY(cp_low, dest, int16_t,  atoi); break;
          case PLY_INT32:  PLY_CONVERT_AND_COPY(cp_low, dest, int32_t,  atoi); break;
          case PLY_UINT8:  PLY_CONVERT_AND_COPY(cp_low, dest, uint8_t,  atoi); break;
          case PLY_UINT16: PLY_CONVERT_AND_COPY(cp_low, dest, uint16_t, atoi); break;
          case PLY_UINT32: PLY_CONVERT_AND_COPY(cp_low, dest, uint32_t, atoi); break;
          case PLY_FLOAT:  PLY_CONVERT_AND_COPY(cp_low, dest, float,    atof); break;
          case PLY_DOUBLE: PLY_CONVERT_AND_COPY(cp_low, dest, double,   atof); break;
        }
        *cp_up = tmp_cp_up;
        cp_low = cp_up;
        cp_up++;
        for(int k=0; k<pr->list_count;++k) 
        {
          while(*cp_up!=' ' && *cp_up!='\n') {cp_up++;}
          tmp_cp_up = *cp_up;
          *cp_up = 0; //Fake string end;
          switch(pr->type)
          {
            case PLY_INT8:   PLY_CONVERT_AND_COPY(cp_low, dest, int8_t,   atoi); break;
            case PLY_INT16:  PLY_CONVERT_AND_COPY(cp_low, dest, int16_t,  atoi); break;
            case PLY_INT32:  PLY_CONVERT_AND_COPY(cp_low, dest, int32_t,  atoi); break;
            case PLY_UINT8:  PLY_CONVERT_AND_COPY(cp_low, dest, uint8_t,  atoi); break;
            case PLY_UINT16: PLY_CONVERT_AND_COPY(cp_low, dest, uint16_t, atoi); break;
            case PLY_UINT32: PLY_CONVERT_AND_COPY(cp_low, dest, uint32_t, atoi); break;
            case PLY_FLOAT:  PLY_CONVERT_AND_COPY(cp_low, dest, float,    atof); break;
            case PLY_DOUBLE: PLY_CONVERT_AND_COPY(cp_low, dest, double,   atof); break;
          }
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

  // NOTE(maciej): Here we need to go and parse all;


  return err_code;
}

int
ply_file__get_element_data_le(ply_file_t* pf, const char* element_name, void** storage)
{
  int err_code = PLY_NO_ERRORS;
  // find requested element
  ply_element_t* el = NULL;
  for(int i=0;i<msh_array_size(pf->elements);++i)
  {
    if(!strcmp(pf->elements[i].name, element_name)) el = &pf->elements[i];
  }
  if( !el ) return PLY_ELEMENT_NOT_FOUND_ERR;

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
ply_file_get_element_data(ply_file_t* pf, const char* element_name, void** storage)
{
  int err_code = PLY_NO_ERRORS;
  if(pf->format==PLY_ASCII)         err_code = ply_file__get_element_data_ascii(pf, element_name, storage);
  if(pf->format==PLY_LITTLE_ENDIAN) err_code = ply_file__get_element_data_le(pf, element_name, storage);
  if(pf->format==PLY_BIG_ENDIAN)    err_code = PLY_BIG_ENDIAN_ERR;

  return err_code;
}

int
ply_file_get_properties_size(ply_file_t* pf, const char* element_name, 
                             const char** properties_names, int n_properties, int* size)
{
  int err_code = PLY_NO_ERRORS;
  // find requested element
  ply_element_t* el = NULL;
  for(int i=0;i<msh_array_size(pf->elements);++i)
  {
    if(!strcmp(pf->elements[i].name, element_name)) el = &pf->elements[i];
  }
  if( !el ) return PLY_ELEMENT_NOT_FOUND_ERR;
  
  // find requested properties
  int n_found         = 0;
  int total_byte_size = 0;
  for( int i=0;i<n_properties;++i)
  {
    for( int j=0;j<msh_array_size(el->properties);++j)
    {
      if( !strcmp(el->properties[j].name, properties_names[i]) )
      {
        n_found++;
        total_byte_size += el->properties[j].total_byte_size;
      } 
    }
  }
  if( n_found != n_properties ) return PLY_PROPERTY_NOT_FOUND_ERR;

  // only update 'size' here - don't provide user with partial information
  *size = total_byte_size;

  return err_code;
}

int _ply_file_get_properties_from_element(ply_file_t* pf, const char* element_name,  void* element_data,
                                          const char** req_properties, int n_req_properties,
                                          void** req_data )
{
  // TODO(maciej):report that requested property is a list
  ply_element_t* el = NULL;
  for(int i = 0; i < msh_array_size(pf->elements); ++i)
  {
    if(!strcmp(element_name, pf->elements[i].name))
    {
      el = &pf->elements[i];
      break;
    }
  }
  if(el == NULL) return PLY_ELEMENT_NOT_FOUND_ERR;
  
  //TODO(maciej): Remove msh_array
  ply_property_t* properties = el->properties;
  int src_offsets[4] = {0};
  int byte_sizes[4] = {0};
  int dst_row_size = 0;
  int src_row_size = 0;
  for(int i = 0; i < msh_array_size(properties); ++i)
  {
    for(int j = 0; j < n_req_properties; ++j)
    {
      if(!strcmp(properties[i].name, req_properties[j])) 
      {
        dst_row_size += properties[i].byte_size;
        src_offsets[j] = src_row_size;
        byte_sizes[j] = properties[i].byte_size;
      }
    }
    src_row_size += properties[i].byte_size;
  }

  char* dst = (char*)*req_data;
  char* src = (char*)element_data;
  for(int i = 0; i < el->count; ++i )
  {
    int dst_offset = 0;
    for( int j = 0; j < n_req_properties; ++j)
    {
      memcpy(dst+dst_offset,src+src_offsets[j], byte_sizes[j]); 
      dst_offset += byte_sizes[j];
    }
    src += src_row_size;
    dst += dst_row_size;
  }

  return 1;
}

int 
ply_file_get_list_property_from_element(ply_file_t* pf, const char* element_name, void* element_data, 
																				const char* property_name,  
                                        void** list_lengths, void** list_data)
{
  //TODO(maciej): Add internal parameter searching
  ply_element_t* el = NULL;
  for(int i = 0; i < msh_array_size(pf->elements); ++i)
  {
    if(!strcmp(element_name, pf->elements[i].name))
    {
      el = &pf->elements[i];
      break;
    }
  }
  
  if(el == NULL) return PLY_ELEMENT_NOT_FOUND_ERR;
  msh_array(ply_property_t) properties = el->properties;
  
  ply_property_t *list_property = NULL;
  int list_property_index = 0;
  for(int i=0; i<msh_array_size(properties); ++i)
  {
    if(!strcmp(properties[i].name, property_name))
    {
      list_property = &properties[i];
      list_property_index = i;
    }
  }
  
  char* ll_dst = (char*)*list_lengths;
  char* ld_dst = (char*)*list_data;
  char* src = (char*)element_data;
  
  for(int i = 0; i < el->count; ++i )
  {  
    for(int j=0; j<msh_array_size(properties); ++j)
    {
      if(j == list_property_index)
      {
        memcpy(ll_dst, src, properties[j].list_byte_size);
        uint64_t count = 0;
        if( properties[j].list_count != -1)
        {
          count = properties[j].list_count;
        }
        else
        {
          switch(properties[j].type)
          {
            case PLY_INT8:   { int8_t c_tmp = *ll_dst;   count = (int64_t)c_tmp; break; }
            case PLY_INT16:  { int16_t c_tmp = *ll_dst;  count = (int64_t)c_tmp; break; }
            case PLY_INT32:  { int32_t c_tmp = *ll_dst;  count = (int64_t)c_tmp; break; }
            case PLY_UINT8:  { uint8_t c_tmp = *ll_dst;  count = (int64_t)c_tmp; break; }
            case PLY_UINT16: { uint16_t c_tmp = *ll_dst; count = (int64_t)c_tmp; break; }
            case PLY_UINT32: { uint32_t c_tmp = *ll_dst; count = (int64_t)c_tmp; break; }
            default: printf("INVALID TYPE\n"); break;
          }
        }
        memcpy(ld_dst, src+properties[j].list_byte_size, count * properties[j].byte_size);
        src += properties[j].list_byte_size + count * properties[j].byte_size; 
        ll_dst += properties[j].list_byte_size;
        ld_dst += count * properties[j].byte_size;
      }
      else
      {
        int64_t count = 0;
        switch(properties[j].type)
        {
          case PLY_INT8:   { int8_t c_tmp = *src;   count = (int64_t)c_tmp; break; }
          case PLY_INT16:  { int16_t c_tmp = *src;  count = (int64_t)c_tmp; break; }
          case PLY_INT32:  { int32_t c_tmp = *src;  count = (int64_t)c_tmp; break; }
          case PLY_UINT8:  { uint8_t c_tmp = *src;  count = (int64_t)c_tmp; break; }
          case PLY_UINT16: { uint16_t c_tmp = *src; count = (int64_t)c_tmp; break; }
          case PLY_UINT32: { uint32_t c_tmp = *src; count = (int64_t)c_tmp; break; }
          default: printf("INVALID TYPE\n"); break;
        }
        src += properties[j].list_byte_size + count * properties[j].byte_size;
      }
    }
  }
  return 1;
}


//NOTE(maciej): void** is not generic : http://c-faq.com/ptrs/genericpp.html
#define ply_file_get_properties_from_element(PF, EN, ED, RP, RD) do{\
void* RD_ptr = *RD;\
_ply_file_get_properties_from_element(PF, EN, ED, RP, sizeof(RP)/sizeof(*RP), &RD_ptr);\
*RD = RD_ptr;\
}while(0)\





/// ENCODER
// int  
// ply_file_add_element(ply_file_t* pf, const char* element_name, const int element_count)
// {
// 	ply_element_t el;
// 	strncpy( &el.name[0], element_name, 64 );
// 	el.count = element_count;
// 	el.properties = NULL;
// 	msh_array_push( pf->elements, el );
// 	return PLY_NO_ERRORS;
// }

// int  
// ply_file_add_property_to_element(ply_file_t* pf, const char* element_name, const char* property_name, int property_type)
// {
// 	ply_property_t pr;
// 	pr.type = property_type;
// 	strncpy( &pr.name[0], property_name, 64 );

// 	// Find element
//   ply_element_t* el = NULL;
//   for(int i = 0; i < msh_array_size(pf->elements); ++i)
//   {
//     if(!strcmp(element_name, pf->elements[i].name))
//     {
//       el = &pf->elements[i];
//       break;
//     }
//   }

//   if(el)
//   { 
//     ply_property_t pr;
//     strncpy(&pr.name[0], property_name, 64);
//     pr.type = property_type;
//     pr.list_type = PLY_INVALID;
//       switch( property_type )
//       {
//         case PLY_INT8:   {  pr.byte_size = 1; break; }
//         case PLY_INT16:  {  pr.byte_size = 2; break; }
//         case PLY_INT32:  {  pr.byte_size = 4; break; }
//         case PLY_UINT8:  {  pr.byte_size = 1; break; }
//         case PLY_UINT16: {  pr.byte_size = 2; break; }
//         case PLY_UINT32: {  pr.byte_size = 4; break; }
//         case PLY_FLOAT:  {  pr.byte_size = 4; break; }
//         case PLY_DOUBLE: {  pr.byte_size = 8; break; }
//       }
//     pr.list_byte_size = 0;
//     pr.list_count = 1;
//     msh_array_push( el->properties, pr );
//   }
//   else 
//   {
//     return PLY_PROPERTY_NOT_FOUND_ERR;
//   }

// 	// Add 'pr' to it
// 	return PLY_NO_ERRORS;
// }

// int  ply_file_write_header(ply_file_t* pf);
// int  ply_file_write_element_data( ply_file_t* pf, const char* element_name, void* data );

/// IO


int
ply_file_add_hint(ply_file_t* pf, ply_hint_t hint)
{
  msh_array_push(pf->_hints, hint);
  return PLY_NO_ERRORS;
}

ply_file_t*
ply_file_open(const char* filename, const char* mode)
{
  ply_file_t *pf = NULL;
  if( mode[0] != 'r' && mode[0] != 'w' ) return NULL;
  FILE* fp = fopen(filename, mode);
  if( fp )
  {
    pf                 = (ply_file_t*)malloc(sizeof(ply_file_t)); //THIS USES MALLOC!:<
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
    }
    msh_array_free(pf->elements);
  }
  if(pf->_hints) msh_array_free(pf->_hints);
  free(pf);
  return 1;
}

int main(int argc, char** argv)
{
	if(argc < 2) {printf("Please provide .ply filename\n"); return 0;} 
	char* filename = argv[1];


  float vertices[] = { 1.0f, 0.0f, 0.0f, 
                       0.0f, 1.0f, 0.0f,
                       0.0f, 1.0f, 0.0f };
  int face_ind[] = {0,1,2};
  ply_file_t* pf = ply_file_open(filename, "wb");
  ply_file_add_property_to_element(pf, "vertex", "x", PLY_FLOAT );
  ply_file_add_property_to_element(pf, "vertex", "y", PLY_FLOAT );
  ply_file_add_property_to_element(pf, "vertex", "z", PLY_FLOAT );
  
  ply_file_print_header(pf);
  ply_file_close(pf);
  printf("Done\n");
#if 0
	printf("Reading file : %s\n", filename);
	ply_file_t *pf = ply_file_open(filename, "rb");
	ply_hint_t indices_size_hint = {.property_name="vertex_indices", .expected_size=3};
  ply_file_add_hint(pf, indices_size_hint);
	if( pf )
	{
		double ht1 = msh_get_time(MSHT_MICROSECONDS);
		ply_file_parse_header(pf);
		double ht2 = msh_get_time(MSHT_MICROSECONDS);
		printf("Header read in %fus\n", ht2-ht1);
		ply_file_print_header(pf);

		double pt1 = msh_get_time(MSHT_MILLISECONDS);
	  ply_file_parse_contents(pf);
		double pt2 = msh_get_time(MSHT_MILLISECONDS);
		printf("Parsing took %f ms\n", pt2-pt1);


		double t1 = msh_get_time(MSHT_MILLISECONDS);
    int vertices_size = -1;
    int faces_size = -1;
    ply_file_get_element_size(pf, "vertex", &vertices_size);
    ply_file_get_element_size(pf, "face", &faces_size);
    printf("vertices_size : %d\n", vertices_size );
    printf("faces_size : %d\n", faces_size );
    void *vertices_data = malloc(vertices_size);
    void *faces_data    = malloc(faces_size);
		t1 = msh_get_time(MSHT_MILLISECONDS);
    ply_file_get_element_data(pf, "vertex", &vertices_data);
		double t2 = msh_get_time(MSHT_MILLISECONDS);
		printf("data request : %fms\n", t2-t1);
    t1 = msh_get_time(MSHT_MILLISECONDS);
		ply_file_get_element_data(pf, "face", &faces_data);
    t2 = msh_get_time(MSHT_MILLISECONDS);
    printf("data request : %fms\n", t2-t1);

		int vertex_count = -1;
    int face_count = -1;
    ply_file_get_element_count(pf, "vertex", &vertex_count);
    ply_file_get_element_count(pf, "face", &face_count);

    t1 = msh_get_time(MSHT_MILLISECONDS);   
    int pos_size = -1;
    const char* pos_prop_names[] = {"nx","ny","nz"};
    ply_file_get_properties_size(pf, "vertex", pos_prop_names, 3, &pos_size);
    float* positions = malloc(pos_size);
    ply_file_get_properties_from_element(pf, "vertex", vertices_data, pos_prop_names, &positions );
    t2 = msh_get_time(MSHT_MILLISECONDS);
    printf("pos data copy : %fms\n", t2-t1);

	//TODO(maciej): Ply file property call is confusing - should be cnaged. We need api calls to
	// give use correct size
    t1 = msh_get_time(MSHT_MILLISECONDS);
    ply_property_t* ind_prop = NULL;
    ply_file_get_property(pf, "face", "vertex_indices", &ind_prop);
    unsigned char* ind_count = (unsigned char*)malloc(face_count * ind_prop->list_byte_size);  
    int* indices = (int*)malloc( ind_prop->total_count * ind_prop->byte_size);
    ply_file_get_list_property_from_element(pf, "face", faces_data, "vertex_indices", &ind_count, &indices);
    t2 = msh_get_time(MSHT_MILLISECONDS);
    printf("ind data copy : %fms\n", t2-t1);

	}
	ply_file_close(pf);
#endif

  return 1;
}