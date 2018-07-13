/*
TODOs:
[ ] Extensive testing
  [ ] Write tests for writing ply files
[ ] Encoder / decorder split
[ ] Error reporting
[ ] Getting raw data for list property - different function.
[ ] Inline msh_array if it is not available
[ ] Code cleanup
  [ ] Replace duplicated code
  [ ] Replace syscalls with redefineable macros
[ ] Optimize 
  [ ] Profile lucy writing, why is it showing a big slowdown.
[ ] Fix the header names to be mply
[ ] Better C++ support.
[ ] Move enums to implementation
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef enum   ply_format            ply_format_t;
typedef enum   ply_type_id           ply_type_id_t;
typedef struct ply_property          ply_property_t;
typedef struct ply_element           ply_element_t;
typedef struct msh_ply_property_desc msh_ply_property_desc_t;
typedef struct ply_file              msh_ply_t;

msh_ply_t* msh_ply_open( const char* filename, const char* mode );
int msh_ply_close( msh_ply_t* pf );

int msh_ply_read( msh_ply_t* pf );
int msh_ply_get_property_from_element( msh_ply_t* pf, msh_ply_property_desc_t* desc );

int msh_ply_write( msh_ply_t* pf );
int msh_ply_add_property_to_element( msh_ply_t* pf, const msh_ply_property_desc_t* desc );

ply_element_t* msh_ply_find_element( const msh_ply_t* pf, const char* element_name );
ply_property_t* msh_ply_find_property( const ply_element_t* el, const char* property_name);
int msh_ply_add_element( msh_ply_t* pf, const char* element_name, const int element_count );


enum ply_type_id
{
  MSH_PLY_INVALID,
  MSH_PLY_INT8,
  MSH_PLY_UINT8,
  MSH_PLY_INT16,
  MSH_PLY_UINT16,
  MSH_PLY_INT32,
  MSH_PLY_UINT32,
  MSH_PLY_FLOAT,
  MSH_PLY_DOUBLE,
  MSH_PLY_N_TYPES
};

enum ply_format
{
  MSH_PLY_ASCII = 0,
  MSH_PLY_LITTLE_ENDIAN,
  MSH_PLY_BIG_ENDIAN
};

struct ply_property
{
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

  // Data storage
  char name[32];
  void* data;
  void* list_data;

};

struct ply_element
{
  char name[64];
  int count;
  msh_array( ply_property_t ) properties;
  
  int file_anchor;
  void* data;
  uint64_t data_size;
};

struct ply_file
{
  int valid;
  int format;
  int format_version;
  
  msh_array(ply_element_t) elements;
  msh_array(msh_ply_property_desc_t*) descriptors;

  FILE* _fp;
  int _header_size;
  int _system_format;
  int _parsed;
};

struct msh_ply_property_desc
{
  char* element_name;
  const char** property_names;
  int32_t num_properties;
  ply_type_id_t data_type;
  ply_type_id_t list_type;
  void* data;
  void* list_data;
  int32_t data_count;

  int32_t list_size_hint;
};


#if defined(_MSC_VER)
#define MSH_PLY_INLINE __forceinline
#else
#define MSH_PLY_INLINE inline /* __attribute__((always_inline)) inline */
#endif

#define MSH_PLY_FILE_MAX_STR_LEN 1024
#define MSH_PLY_FILE_MAX_REQ_PROPERTIES 16
#define MSH_PLY_FILE_MAX_PROPERTIES 128

#ifdef __cplusplus
}
#endif

////////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////////
#ifdef MSH_PLY_IMPLEMENTATION


typedef enum ply_err
{
  MSH_PLY_NO_ERRORS = 0, 
  MSH_PLY_INVALID_FILE_ERR = 1, 
  MSH_PLY_INVALID_FORMAT_ERR = 2,
  MSH_PLY_FILE_OPEN_ERR = 3,
  MSH_PLY_FILE_NOT_OPEN_ERR = 4,
  MSH_PLY_LINE_PARSE_ERR = 5,
  MSH_PLY_FORMAT_CMD_ERR = 6,
  MSH_PLY_ELEMENT_CMD_ERR = 7,
  MSH_PLY_PROPERTY_CMD_ERR = 8,
  MSH_PLY_ELEMENT_NOT_FOUND_ERR = 9,
  MSH_PLY_PROPERTY_NOT_FOUND_ERR = 10,
  MSH_PLY_REQUESTED_PROPERTY_NOT_A_LIST_ERR = 11,
  MSH_PLY_PARSE_ERROR = 12,
  MSH_PLY_BIG_ENDIAN_ERR = 13,
  MSH_PLY_BINARY_FILE_READ_ERR = 14,
  MSH_PLY_CONFLICTING_NUMBER_OF_ELEMENTS_ERR = 15,
  MSH_PLY_UNRECOGNIZED_FORMAT_ERR = 16,
  MSH_PLY_UNRECOGNIZED_CMD_ERR = 17,
  MSH_PLY_UNSUPPORTED_FORMAT_ERR = 18,
  MSH_PLY_REQUESTED_ELEMENT_IS_MISSING = 19,
  MSH_PLY_REQUESTED_PROPERTY_IS_MISSING = 20,
  MSH_PLY_NO_REQUESTS = 21,
  MSH_PLY_INVALID_LIST_TYPE_ERR = 22 
} ply_err_t;

ply_property_t
ply_property_zero_init()
{
  ply_property_t pr = {   
    .list_count = 0,
    .byte_size = 0,
    .offset = 0,
    .stride = 0 ,
    .list_byte_size = 0,
    .list_offset = 0,
    .list_stride = 0,
    .type = (ply_type_id_t)0,
    .list_type= (ply_type_id_t)0,
    .total_count = 0,
    .total_byte_size = 0,
    .name = {0},
    .data = NULL,
    .list_data = NULL };
  return pr;
}


ply_element_t
ply_element_zero_init()
{
  ply_element_t el = { .name = {0},
                       .count  = 0,
                       .properties = NULL,
                       .file_anchor = 0,
                       .data = NULL,
                       .data_size = 0 };
  return el;
}



MSH_PLY_INLINE void 
msh_ply__swap_bytes( uint8_t* buffer, int32_t type_size, int32_t count )
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

MSH_PLY_INLINE ply_element_t*
msh_ply_find_element( const msh_ply_t* pf, const char* element_name )
{
  ply_element_t* el = NULL;
  for( size_t i = 0; i < msh_array_len(pf->elements); ++i )
  {
    if(!strcmp(element_name, pf->elements[i].name))
    {
      el = &pf->elements[i];
      break;
    }
  }
  return el;
}

MSH_PLY_INLINE ply_property_t*
msh_ply_find_property( const ply_element_t* el, const char* property_name)
{
  ply_property_t* pr = NULL;
  for( size_t i = 0; i < msh_array_len(el->properties) ; ++i )
  {
    if(!strcmp(el->properties[i].name, property_name)) { pr = &el->properties[i]; break; }
  }

  return pr;
}

MSH_PLY_INLINE int
msh_ply__get_data_as_int( void* data, int type, int8_t swap_endianness )
{
  int retval = 0;
  switch( type )
  {
    case MSH_PLY_INT8:  
      retval  = ((char*)data)[0];
      break;
    case MSH_PLY_INT16: 
    {
      retval = ((int16_t*)data)[0];
      if( swap_endianness ) msh_ply__swap_bytes((uint8_t*)&retval, 2, 1);
      break;
    }
    case MSH_PLY_INT32:
    {
      retval = ((int32_t*)data)[0]; 
      if( swap_endianness ) msh_ply__swap_bytes((uint8_t*)&retval, 4, 1);
      break;
    }
    case MSH_PLY_UINT8:

      retval = ((uint8_t*)data)[0];
      break;
    case MSH_PLY_UINT16: 
    {
      retval = ((uint16_t*)data)[0];
      if( swap_endianness ) msh_ply__swap_bytes((uint8_t*)&retval, 2, 1);
      break;
    }
    case MSH_PLY_UINT32:
    { 
      retval = ((uint32_t*)data)[0]; 
      if( swap_endianness ) msh_ply__swap_bytes((uint8_t*)&retval, 4, 1);
      break;
    }
    default: retval = 0; break;
  }
  return retval;
}


MSH_PLY_INLINE int 
msh_ply__type_to_byte_size( ply_type_id_t type )
{
  int retval = 0;
  switch( type )
  {
    case MSH_PLY_INT8:   { retval = 1; break; }
    case MSH_PLY_INT16:  { retval = 2; break; }
    case MSH_PLY_INT32:  { retval = 4; break; }
    case MSH_PLY_UINT8:  { retval = 1; break; }
    case MSH_PLY_UINT16: { retval = 2; break; }
    case MSH_PLY_UINT32: { retval = 4; break; }
    case MSH_PLY_FLOAT:  { retval = 4; break; }
    case MSH_PLY_DOUBLE: { retval = 8; break; }
    default: { retval = 0; break; }
  }
  return retval;
}

MSH_PLY_INLINE void msh_ply__property_type_to_string( ply_type_id_t type, char** string )
{
  switch( type )
  {
    case MSH_PLY_INT8:   { *string = (char*)"char"; break; }
    case MSH_PLY_INT16:  { *string = (char*)"short"; break; }
    case MSH_PLY_INT32:  { *string = (char*)"int"; break; }
    case MSH_PLY_UINT8:  { *string = (char*)"uchar"; break; }
    case MSH_PLY_UINT16: { *string = (char*)"ushort"; break; }
    case MSH_PLY_UINT32: { *string = (char*)"uint"; break; }
    case MSH_PLY_FLOAT:  { *string = (char*)"float"; break; }
    case MSH_PLY_DOUBLE: { *string = (char*)"double"; break; }
    default: { break; }
  }
}

MSH_PLY_INLINE void
msh_ply_string_to_property_type(char* type_str, ply_type_id_t* pr_type, int16_t* pr_size)
{
  if      (!strcmp("int8",    type_str) || !strcmp("char",   type_str)) {*pr_type=MSH_PLY_INT8;   *pr_size=1;} 
  else if (!strcmp("uint8",   type_str) || !strcmp("uchar",  type_str)) {*pr_type=MSH_PLY_UINT8;  *pr_size=1;} 
  else if (!strcmp("int16",   type_str) || !strcmp("short",  type_str)) {*pr_type=MSH_PLY_INT16;  *pr_size=2;} 
  else if (!strcmp("uint16",  type_str) || !strcmp("ushort", type_str)) {*pr_type=MSH_PLY_UINT16; *pr_size=2;} 
  else if (!strcmp("int32",   type_str) || !strcmp("int",    type_str)) {*pr_type=MSH_PLY_INT32;  *pr_size=4;} 
  else if (!strcmp("uint32",  type_str) || !strcmp("uint",   type_str)) {*pr_type=MSH_PLY_UINT32; *pr_size=4;} 
  else if (!strcmp("float32", type_str) || !strcmp("float",  type_str)) {*pr_type=MSH_PLY_FLOAT;  *pr_size=4;} 
  else if (!strcmp("float64", type_str) || !strcmp("double", type_str)) {*pr_type=MSH_PLY_DOUBLE; *pr_size=8;} 
  else { *pr_type = MSH_PLY_INVALID; *pr_size = 0;}
}

int
msh_ply_parse_ply_cmd(char* line, msh_ply_t* pf)
{
  char cmd_str[MSH_PLY_FILE_MAX_STR_LEN];
  if( sscanf(line, "%s", cmd_str) )
  { 
    if( !strcmp(cmd_str, "ply") )
    {
      pf->valid = true; 
      return MSH_PLY_NO_ERRORS; 
    }
  }
  return MSH_PLY_INVALID_FILE_ERR;
}

int
msh_ply_parse_format_cmd(char* line, msh_ply_t* pf)
{
  char cmd[MSH_PLY_FILE_MAX_STR_LEN];
  char frmt_str[MSH_PLY_FILE_MAX_STR_LEN];
  char frmt_ver_str[MSH_PLY_FILE_MAX_STR_LEN];
  if(sscanf(line, "%s %s %s", &cmd[0], &frmt_str[0], &frmt_ver_str[0]) != 3) { return MSH_PLY_FORMAT_CMD_ERR; } 
  if(!strcmp("ascii", frmt_str)){ pf->format = (int)MSH_PLY_ASCII; }
  else if(!strcmp("binary_little_endian", frmt_str)){ pf->format = (int)MSH_PLY_LITTLE_ENDIAN; }
  else if(!strcmp("binary_big_endian", frmt_str)){ pf->format = (int)MSH_PLY_BIG_ENDIAN; }
  else{ return MSH_PLY_UNRECOGNIZED_FORMAT_ERR; }
  pf->format_version = atoi(frmt_ver_str);
  return MSH_PLY_NO_ERRORS;
}

int
msh_ply_parse_element_cmd(char* line, msh_ply_t* pf)
{
  char cmd[MSH_PLY_FILE_MAX_STR_LEN];
  ply_element_t el = ply_element_zero_init();
  el.properties = NULL;
  if(sscanf(line, "%s %s %d", &cmd[0], &el.name[0], &el.count) != 3) { return MSH_PLY_ELEMENT_CMD_ERR; }
  msh_array_push( pf->elements, el );
  return MSH_PLY_NO_ERRORS;
}

int
msh_ply_parse_property_cmd(char* line, msh_ply_t* pf)
{
  char cmd[MSH_PLY_FILE_MAX_STR_LEN];
  char type_str[MSH_PLY_FILE_MAX_STR_LEN];
  char list_str[MSH_PLY_FILE_MAX_STR_LEN];
  char list_type_str[MSH_PLY_FILE_MAX_STR_LEN];
  int valid_format = false;
  
  ply_element_t* el = msh_array_back(pf->elements);
  ply_property_t pr = ply_property_zero_init();
  // Try to parse regular property format
  if(sscanf(line, "%s %s %s", &cmd[0], &type_str[0], (char*)&pr.name) == 3) 
  {
    msh_ply_string_to_property_type(type_str, &pr.type, &pr.byte_size);
    pr.list_type = MSH_PLY_INVALID;
    pr.list_byte_size = 0;
    pr.list_count = 1;
    valid_format = true;
  }

  // Try to parse list property format
  cmd[0] = 0; type_str[0] = 0; list_str[0] = 0; list_type_str[0] = 0;
  if( sscanf(line, "%s %s %s %s %s", &cmd[0], &list_str[0], &list_type_str[0], &type_str[0], (char*)&pr.name) == 5)
  {
    if( strcmp(list_str, "list") ) { return MSH_PLY_PROPERTY_CMD_ERR; }
    msh_ply_string_to_property_type(type_str, &pr.type, &pr.byte_size);
    msh_ply_string_to_property_type(list_type_str, &pr.list_type, &pr.list_byte_size);
    pr.list_count = 0;
    valid_format = true;
  }
  pr.total_byte_size = 0;
  pr.total_count = 0;
  // Both failed
  if(!valid_format) { return MSH_PLY_PROPERTY_CMD_ERR; }

  // Either succeded
  msh_array_push(el->properties, pr);

  return MSH_PLY_NO_ERRORS;
}

int 
msh_ply__parse_command(char* cmd, char* line, msh_ply_t* pf)
{
  if(!strcmp(cmd, "ply"))      { return msh_ply_parse_ply_cmd(line, pf); }
  if(!strcmp(cmd, "format"))   { return msh_ply_parse_format_cmd(line, pf); }
  if(!strcmp(cmd, "element"))  { return msh_ply_parse_element_cmd(line, pf); }
  if(!strcmp(cmd, "property")) { return msh_ply_parse_property_cmd(line, pf); }
  return MSH_PLY_UNRECOGNIZED_CMD_ERR;
}

int
msh_ply_parse_header(msh_ply_t* pf)
{
  int line_no = 0;
  char line[MSH_PLY_FILE_MAX_STR_LEN];
  int err_code = 0;
  while( fgets( &line[0], MSH_PLY_FILE_MAX_STR_LEN, pf->_fp ) )
  {
    line_no++;
    char cmd[MSH_PLY_FILE_MAX_STR_LEN];
    if(sscanf(line, "%s", cmd)!=(unsigned) 1) { return MSH_PLY_LINE_PARSE_ERR; }
    if(!strcmp(cmd,"end_header")) break;
    if(!strcmp(cmd,"comment")) continue;
    if(!strcmp(cmd,"obj_info")) continue;
    err_code = msh_ply__parse_command(cmd, line, pf);
    if( err_code ) break;
  }
  pf->_header_size = ftell(pf->_fp);
  if( err_code == MSH_PLY_NO_ERRORS ) { pf->_parsed = 1; }
  return err_code;
}

int 
msh_ply__calculate_elem_size_ascii(msh_ply_t* pf, ply_element_t* el)
{
  char line[MSH_PLY_FILE_MAX_STR_LEN];
  while(fgets(&line[0], MSH_PLY_FILE_MAX_STR_LEN, pf->_fp))
  {
    char* cp_up  = &line[0];
    char* cp_low = &line[0];

  for( size_t j = 0; j<msh_array_len(el->properties); ++j )
    {
      ply_property_t* pr = &el->properties[j];
      while( *cp_up != ' ' && *cp_up != '\n' ) { cp_up++; }
      char tmp_cp_up = *cp_up;
      *cp_up = 0; //Fake string end;

      if(pr->list_type != MSH_PLY_INVALID)
      {
        switch(pr->list_type)
        {
          case MSH_PLY_INT8:   
          case MSH_PLY_INT16:  
          case MSH_PLY_INT32:  
          case MSH_PLY_UINT8:  
          case MSH_PLY_UINT16: 
          case MSH_PLY_UINT32: pr->list_count = (int32_t)atoi(cp_low);  break;
          case MSH_PLY_FLOAT:
          case MSH_PLY_DOUBLE: pr->list_count = (int32_t)atof(cp_low);  break;
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
  return MSH_PLY_NO_ERRORS;
}


int 
msh_ply__calculate_elem_size_binary(msh_ply_t* pf, ply_element_t* el)
{
  for( int i = 0; i < el->count; ++i )
  {
    for( size_t j = 0; j < msh_array_len(el->properties); ++j )
    {
      ply_property_t* pr = &el->properties[j];
      int count = 1;
      if( pr->list_type != MSH_PLY_INVALID ) 
      { 
        int read_count = fread(&count, pr->list_byte_size, 1, pf->_fp);
        if( read_count != 1) { return MSH_PLY_PARSE_ERROR;}
        pr->total_byte_size += pr->list_byte_size;
      }
      fseek(pf->_fp, count * pr->byte_size, SEEK_CUR);
      pr->total_byte_size += count * pr->byte_size;
      pr->total_count += count;
    }
  }
  return MSH_PLY_NO_ERRORS;
}

int32_t 
msh_ply__can_precalculate_sizes( ply_element_t* el )
{
  int32_t can_precalculate = 1;
  for( size_t i = 0; i < msh_array_len(el->properties); ++i )
  {
    ply_property_t* pr = &el->properties[i];
    if( pr->list_type != MSH_PLY_INVALID && pr->list_count == 0) 
    { 
      can_precalculate = 0;
      break;
    }
  }

  return can_precalculate;
}

int
msh_ply_parse_contents( msh_ply_t* pf )
{
  int err_code = MSH_PLY_NO_ERRORS;
  for( size_t i = 0; i < msh_array_len(pf->elements); ++i )
  {
    ply_element_t* el = &pf->elements[i];
    int num_properties = msh_array_len(el->properties);
    if( el->count <= 0 || num_properties <= 0 ) { continue; }
    el->file_anchor = ftell(pf->_fp);

    // Determine if any of the properties in the element has list
    int can_precalculate_size = msh_ply__can_precalculate_sizes( el );
    if( can_precalculate_size )
    {
      // This is a faster path, as we can just calculate the size required by element in one go.
      int elem_size = 0;
      for( int j = 0; j < num_properties; ++j ) 
      {
        ply_property_t* pr = &el->properties[j];
        pr->total_byte_size = pr->byte_size * pr->list_count * el->count;
        elem_size += pr->byte_size * pr->list_count; 
        if(pr->list_type != MSH_PLY_INVALID )
        {
          pr->total_byte_size += pr->list_byte_size*el->count;
          elem_size += pr->list_byte_size;
        }
        pr->total_count += pr->list_count * el->count;
      }
      if( pf->format != MSH_PLY_ASCII ) { fseek( pf->_fp, el->count * elem_size, SEEK_CUR ); }
      else
      {
        char line[MSH_PLY_FILE_MAX_STR_LEN];
        for( int j =0; j < el->count; ++j ) { fgets( &line[0], MSH_PLY_FILE_MAX_STR_LEN, pf->_fp ); }
      }
    }
    else 
    {
      // There exists a list property. We need to calculate required size via pass through
      if( pf->format == MSH_PLY_ASCII ) { err_code = msh_ply__calculate_elem_size_ascii(pf, el); }
      else                              { err_code = msh_ply__calculate_elem_size_binary(pf, el); }

    }
  }

  return err_code;
}

int
msh_ply__get_element_count( const ply_element_t* el, int* count )
{
  int err_code = MSH_PLY_NO_ERRORS;
  *count = el->count;
  return err_code;
}

int
msh_ply__get_element_size( const ply_element_t *el, uint64_t* size )
{
  int err_code = MSH_PLY_NO_ERRORS;
  
  *size = 0;
  for( size_t i = 0; i < msh_array_len( el->properties ); ++i )
  {
    *size += el->properties[i].total_byte_size;
  }
  return err_code;
}

// NOTE(maciej): this works better with an assignment
#define MSH_PLY_CONVERT_AND_ASSIGN(D, C, T, conv_funct) {\
  T n = (T)conv_funct(C); \
  *((T*)(D)) = n; \
  (D) += sizeof(T); }

MSH_PLY_INLINE void 
msh_ply__ascii_to_value( char** dst, char* src, const int type )
{
  switch(type)
  {
    case MSH_PLY_INT8:   MSH_PLY_CONVERT_AND_ASSIGN(*dst, src, int8_t,   atoi); break;
    case MSH_PLY_INT16:  MSH_PLY_CONVERT_AND_ASSIGN(*dst, src, int16_t,  atoi); break;
    case MSH_PLY_INT32:  MSH_PLY_CONVERT_AND_ASSIGN(*dst, src, int32_t,  atoi); break;
    case MSH_PLY_UINT8:  MSH_PLY_CONVERT_AND_ASSIGN(*dst, src, uint8_t,  atoi); break;
    case MSH_PLY_UINT16: MSH_PLY_CONVERT_AND_ASSIGN(*dst, src, uint16_t, atoi); break;
    case MSH_PLY_UINT32: MSH_PLY_CONVERT_AND_ASSIGN(*dst, src, uint32_t, atoi); break;
    case MSH_PLY_FLOAT:  MSH_PLY_CONVERT_AND_ASSIGN(*dst, src, float,    atof); break;
    case MSH_PLY_DOUBLE: MSH_PLY_CONVERT_AND_ASSIGN(*dst, src, double,   atof); break;
  }
}


int32_t
msh_ply__get_element_data_ascii( msh_ply_t* pf, const ply_element_t* el, void** storage )
{
  int32_t err_code = MSH_PLY_NO_ERRORS;
  int32_t num_properties = msh_array_len( el->properties );

  fseek(pf->_fp, el->file_anchor, SEEK_SET);
  char *dest = (char*)*storage;
  for( int i = 0; i < el->count; ++i )
  {
    char line[MSH_PLY_FILE_MAX_STR_LEN];
    fgets(line, MSH_PLY_FILE_MAX_STR_LEN, pf->_fp);
    char* cp_up  = &line[0];
    char* cp_low = &line[0];

    for( int j = 0; j < num_properties; ++j )
    {
      ply_property_t* pr = &el->properties[j];
      while( *cp_up!=' ' && *cp_up!='\n' ) { cp_up++; }
      char tmp_cp_up = *cp_up;
      *cp_up = 0; // fake string end;
      if(pr->list_type == MSH_PLY_INVALID) // regular property
      { 
        msh_ply__ascii_to_value( &dest, cp_low, pr->type );
      }
      else // list property
      {
        msh_ply__ascii_to_value( &dest, cp_low, pr->list_type);
        if( !pr->list_count )
        {
          pr->list_count = msh_ply__get_data_as_int( dest-pr->list_byte_size, pr->list_type, 0 );
        }
        *cp_up = tmp_cp_up;
        cp_low = cp_up;
        cp_up++;
        for( int k = 0; k < pr->list_count; ++k) 
        {
          while(*cp_up!=' ' && *cp_up!='\n') {cp_up++;}
          tmp_cp_up = *cp_up;
          *cp_up = 0; // fake string end;
          msh_ply__ascii_to_value( &dest, cp_low, pr->type);
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
#undef MSH_PLY_CONVERT_AND_ASSIGN

int
msh_ply__get_element_data_binary( msh_ply_t* pf, const ply_element_t* el, 
                                   void** storage, int64_t storage_size )
{
  int err_code = MSH_PLY_NO_ERRORS;
  fseek( pf->_fp, el->file_anchor, SEEK_SET );
  if( fread( *storage, storage_size, 1, pf->_fp ) != 1 ) { return MSH_PLY_PARSE_ERROR; }
  return err_code;
}

MSH_PLY_INLINE int
msh_ply__get_element_data(msh_ply_t* pf, const ply_element_t* el, void** storage, int64_t storage_size)
{
  int err_code = MSH_PLY_NO_ERRORS;
  if(pf->format == MSH_PLY_ASCII)  { err_code = msh_ply__get_element_data_ascii( pf, el, storage ); }
  else                         { err_code = msh_ply__get_element_data_binary( pf, el, storage, storage_size ); }

  return err_code;
}

int
msh_ply__get_properties_byte_size( ply_element_t* el, 
                                   const char** properties_names, int num_properties,
                                   ply_type_id_t type, ply_type_id_t list_type,
                                   int* data_size, int* list_size )
{
  int n_found              = 0;
  int total_data_byte_size = 0;
  int total_list_byte_size = 0;
  int byte_size = msh_ply__type_to_byte_size( type );
  int list_byte_size = msh_ply__type_to_byte_size( list_type );
  for( int i = 0; i < num_properties; ++i )
  {
    for( size_t j = 0; j < msh_array_len( el->properties ); ++j )
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
  if( n_found != num_properties ) return MSH_PLY_PROPERTY_NOT_FOUND_ERR;

  total_list_byte_size *= el->count;

  *data_size = total_data_byte_size;
  *list_size = total_list_byte_size;

  return MSH_PLY_NO_ERRORS;
}

MSH_PLY_INLINE void
msh_ply__data_assign( void* dst, void* src, int32_t type, int32_t count )
{
  int32_t a = count / 3;
  int32_t b = count % 3;
  
  switch( type )
  {
    case MSH_PLY_UINT8:
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
      
    case MSH_PLY_UINT16:
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
      
    case MSH_PLY_UINT32:
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
      
    case MSH_PLY_INT8:
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
      
    case MSH_PLY_INT16:
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
      
    case MSH_PLY_INT32:
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
      
    case MSH_PLY_FLOAT:
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
      
    case MSH_PLY_DOUBLE:
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
MSH_PLY_INLINE void
msh_ply__data_assign_cast( void* dst, void* src, int32_t type_dst, int32_t type_src, int32_t count )
{
  double data;
  for( int32_t c = 0; c < count ; c++ )
  {
    switch( type_src )
    {
      // intermediate store
      case MSH_PLY_UINT8:
        data = (double)(*((uint8_t*)(src)+c));
        break;
      case MSH_PLY_UINT16:
        data = (double)(*((uint16_t*)(src)+c));
        break;
      case MSH_PLY_UINT32:
        data = (double)(*((uint32_t*)(src)+c));
        break;
      case MSH_PLY_INT8:
        data = (double)(*((int8_t*)(src)+c));
        break;
      case MSH_PLY_INT16:
        data = (double)(*((int16_t*)(src)+c));
        break;
      case MSH_PLY_INT32:
        data = (double)(*((int32_t*)(src)+c));
        break;
      case MSH_PLY_FLOAT:
        data = (double)(*((float*)(src)+c));
        break;
      case MSH_PLY_DOUBLE:
        data = (double)(*((double*)(src)+c));
        break;
      default:
        data = (double)(*((uint8_t*)(src)+c));
        break;
    }

    // write
    switch( type_dst )
    {
      case MSH_PLY_UINT8:
        *((uint8_t*)(dst)+c) = (uint8_t)data;
        break;
      case MSH_PLY_UINT16:
        *((uint16_t*)(dst)+c) = (uint16_t)data;
        break;
      case MSH_PLY_UINT32:
        *((uint32_t*)(dst)+c) = (uint32_t)data;
        break;
      case MSH_PLY_INT8:
        *((int8_t*)(dst)+c) = (int8_t)data;
        break;
      case MSH_PLY_INT16:
        *((int16_t*)(dst)+c) = (int16_t)data;
        break;
      case MSH_PLY_INT32:
        *((int32_t*)(dst)+c) = (int32_t)data;
        break;
      case MSH_PLY_FLOAT:
        *((float*)(dst)+c) = (float)data;
        break;
      case MSH_PLY_DOUBLE:
        *((double*)(dst)+c) = (double)data;
        break;
      default:
        *((uint8_t*)(dst)+c) = (uint8_t)data;
        break;
    }
  }
}

int
msh_ply__get_property_from_element( msh_ply_t* pf, const char* element_name, 
                                    const char** property_names, int num_requested_properties, 
                                    ply_type_id_t requested_type, ply_type_id_t requested_list_type, 
                                    void** data, void** list_data, int32_t *data_count )
{
  // TODO(maciej): Errors!
  int8_t swap_endianness = pf->format != MSH_PLY_ASCII ? (pf->_system_format != pf->format) : 0;
  int32_t num_properties = -1;
  ply_element_t* el = msh_ply_find_element(pf, element_name);

  if( !el ) { return MSH_PLY_ELEMENT_NOT_FOUND_ERR; }
  else
  {
    // double pt1 = size_t MSHT_MILLISECONDS);
    num_properties = msh_array_len(el->properties);
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
          if( strcmp(a, b) ) { can_simply_copy = 0; }
          if( pr->type != requested_type ) { can_simply_copy = 0; }
          if( pr->list_type != MSH_PLY_INVALID ) { can_simply_copy = 0;}
        }
      }

      if( can_simply_copy )
      {
        msh_ply__get_element_size( el, &el->data_size );
        *data_count = el->count;
        *data = malloc(el->data_size);
        msh_ply__get_element_data( pf, el, &*data, el->data_size ); 
        return MSH_PLY_NO_ERRORS;
      }
      
      msh_ply__get_element_size( el, &el->data_size );
      el->data = malloc( el->data_size );
      msh_ply__get_element_data( pf, el, &el->data, el->data_size ); 
    }
    
    // Initialize output
    *data_count = el->count;
    uint8_t* dst_data = NULL; 
    uint8_t* dst_list = NULL;
    int data_byte_size = -1;
    int list_byte_size = -1;
    int requested_byte_size = msh_ply__type_to_byte_size( requested_type );
    int requested_list_byte_size = msh_ply__type_to_byte_size( requested_list_type );
    msh_ply__get_properties_byte_size( el, property_names, num_requested_properties, 
                                       requested_type, requested_list_type,
                                       &data_byte_size, &list_byte_size );
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
    int8_t requested_group_size[MSH_PLY_FILE_MAX_REQ_PROPERTIES] = {0};

    for( int i = 0; i < num_properties; ++i )
    {
      ply_property_t* pr = &el->properties[i];

      for( int j = 0; j < num_requested_properties; ++j )
      {
        if( !strcmp(pr->name, property_names[j]) ) 
        {
          if( pr->type != requested_type ) need_cast = 1;
          if( pr->list_type != MSH_PLY_INVALID &&
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

    // TODO(maciej): Make into a struct?
    int32_t precalc_dst_row_size = 0;
    int32_t precalc_dst_list_row_size = 0;
    int32_t precalc_src_row_size = 0;
    int32_t precalc_dst_stride = 0;

    int32_t can_precalculate = msh_ply__can_precalculate_sizes( el );
    if( can_precalculate )
    {
      // Determine data row sizes
      for( int32_t j = 0; j < num_properties; ++j )
      {
        ply_property_t* pr = &el->properties[j];
    
        if( requested_group_size[j] )
        {
          if( pr->list_type == MSH_PLY_INVALID )
          {
            precalc_dst_stride = requested_group_size[j] * requested_byte_size;
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
        precalc_src_row_size += pr->list_byte_size + pr->list_count * pr->byte_size;
      }
    }

    // Create separate list for just requested properties
    typedef struct ply_property_read_helper
    {
      int32_t offset;
      int32_t type;
      int32_t list_count;
      int32_t list_type;
      int32_t list_offset;
    } ply_property_read_helper_t;

    ply_property_read_helper_t requested_properties[MSH_PLY_FILE_MAX_REQ_PROPERTIES];
    for( int32_t i = 0; i < MSH_PLY_FILE_MAX_REQ_PROPERTIES; ++i )
    {
      ply_property_read_helper_t prh = { .offset = 0, .type = 0, .list_count = 0,
                                         .list_type = 0, .list_offset = 0 };
      requested_properties[i] = prh;
    }

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
        if( pr->list_type == MSH_PLY_INVALID )
        {
          requested_properties[pr_count].list_count  = requested_group_size[i];
        }
        pr_count += requested_group_size[i];
      }
    }

    assert( pr_count == num_requested_properties );

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

          if( pr->list_type != MSH_PLY_INVALID )
          {
            pr->list_count = msh_ply__get_data_as_int(src+src_row_size, pr->list_type, swap_endianness);
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
            msh_ply__data_assign_cast( dst_ptr, src_ptr, requested_type, prh->type, prh->list_count );
            dst_offset += dst_stride; // NOTE(maciej): Would need to make per property to enable swizzle
            if( swap_endianness ) msh_ply__swap_bytes( (uint8_t*)dst_ptr, requested_byte_size, prh->list_count );
          }

          if( dst_list )
          {
            void* dst_ptr = (dst_list + dst_list_offset);
            void* src_ptr = (src + prh->list_offset);
            msh_ply__data_assign_cast( dst_ptr, src_ptr, requested_list_type, prh->list_type, 1 );
            dst_list_offset += requested_list_byte_size;
            if( swap_endianness ) msh_ply__swap_bytes( (uint8_t*)dst_ptr, requested_list_byte_size, 1 );
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
            msh_ply__data_assign( dst_ptr, src_ptr, prh->type, prh->list_count );
            dst_offset += dst_stride;
            if( swap_endianness ) msh_ply__swap_bytes( (uint8_t*)dst_ptr, requested_byte_size, prh->list_count );
          }

          if( dst_list )
          {
            void* dst_ptr = (dst_list + dst_list_offset);
            void* src_ptr = (src + prh->list_offset);
            msh_ply__data_assign( dst_ptr, src_ptr, prh->list_type, 1 );
            dst_list_offset += requested_list_byte_size;
            if( swap_endianness ) msh_ply__swap_bytes( (uint8_t*)dst_ptr, requested_list_byte_size, 1 );
          }
        }
      }

      src += src_row_size;
      if(dst_data) dst_data += dst_row_size;
      if(dst_list) dst_list += dst_list_row_size;
    }
  }


  return MSH_PLY_NO_ERRORS;
}

int
msh_ply_get_property_from_element( msh_ply_t* pf, msh_ply_property_desc_t* desc )
{
  // TODO(maciej): Check for null ptrs etc.
  return msh_ply__get_property_from_element( pf, desc->element_name, 
                                desc->property_names, desc->num_properties, 
                                desc->data_type, desc->list_type, 
                                (void**)desc->data, (void**)desc->list_data, 
                                &desc->data_count );
}


// ENCODER
int
msh_ply_add_element( msh_ply_t* pf, const char* element_name, const int element_count )
{
  ply_element_t el = ply_element_zero_init();
  strncpy( &el.name[0], element_name, 64 );
  el.count = element_count;
  el.properties = NULL;
  msh_array_push( pf->elements, el );
  return MSH_PLY_NO_ERRORS;
}


void
msh_ply__fprint_data_at_offset(const msh_ply_t* pf, const void* data, const int32_t offset, const int32_t type)
{
  switch(type)
  {
    case MSH_PLY_UINT8:  fprintf(pf->_fp,"%d ", *(uint8_t*)(data+offset)); break;
    case MSH_PLY_UINT16: fprintf(pf->_fp,"%d ", *(uint16_t*)(data+offset)); break;
    case MSH_PLY_UINT32: fprintf(pf->_fp,"%d ", *(uint32_t*)(data+offset)); break;
    case MSH_PLY_INT8:   fprintf(pf->_fp,"%d ", *(int8_t*)(data+offset)); break;
    case MSH_PLY_INT16:  fprintf(pf->_fp,"%d ", *(int16_t*)(data+offset)); break;
    case MSH_PLY_INT32:  fprintf(pf->_fp,"%d ", *(int32_t*)(data+offset)); break;
    case MSH_PLY_FLOAT:  fprintf(pf->_fp,"%f ", *(float*)(data+offset)); break;
    case MSH_PLY_DOUBLE: fprintf(pf->_fp,"%f ", *(double*)(data+offset)); break;
  }
}


// TODO(maciej): This function needs a rework, it is bit messy
int32_t
msh_ply__add_property_to_element( msh_ply_t* pf, const char* element_name, 
                                  const char** property_names, int32_t num_properties,
                                  const ply_type_id_t data_type, const ply_type_id_t list_type, 
                                  void* data, void* list_data, 
                                  int32_t element_count, int32_t size_hint )
{
  // Check if list type is integral type
  if( list_type == MSH_PLY_FLOAT || list_type == MSH_PLY_DOUBLE )
  {
    return MSH_PLY_INVALID_LIST_TYPE_ERR;
  }

  int8_t swap_endianness = pf->format != MSH_PLY_ASCII ? (pf->_system_format != pf->format) : 0;

  // Find / Create element
  ply_element_t* el = msh_ply_find_element(pf, element_name);
  if( !el )
  {
    msh_ply_add_element(pf, element_name, element_count);
    el = msh_array_back(pf->elements);
  }

  if( el )
  {
    if( el->count != element_count ) 
    {
      return MSH_PLY_CONFLICTING_NUMBER_OF_ELEMENTS_ERR;
    }

    int init_offset = 0; // Helper variable for storing initial list offsets.
    for( int i = 0; i < num_properties; ++i )
    {
      ply_property_t pr;
      strncpy(&pr.name[0], property_names[i], 32);

      pr.type = data_type;
      pr.list_type = list_type;
      pr.byte_size = msh_ply__type_to_byte_size( pr.type );
      pr.list_byte_size = msh_ply__type_to_byte_size( pr.list_type );
      pr.list_count = (list_type == MSH_PLY_INVALID) ? 1 : size_hint; // TODO(If se have a hint, set this to a hint)
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
          int list_count = msh_ply__get_data_as_int((uint8_t*)list_data + offset, list_type, swap_endianness);
          pr.total_byte_size += pr.list_byte_size + list_count * pr.byte_size;
          if( j == 0 ) // We care only about initial offset, so first element of list counts
          {
            pr.offset = init_offset;
            init_offset += list_count * pr.byte_size;
            // printf("Offset %lld\n", pr.offset);
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
    return MSH_PLY_ELEMENT_NOT_FOUND_ERR;
  }
  return MSH_PLY_NO_ERRORS;
}

int
msh_ply_add_property_to_element( msh_ply_t* pf, const msh_ply_property_desc_t* desc )
{
  // TODO(maciej): Check for null ptrs etc.
  return msh_ply__add_property_to_element( pf, desc->element_name, desc->property_names, 
                                           desc->num_properties, desc->data_type, desc->list_type,
                                           desc->data, desc->list_data,
                                           desc->data_count, desc->list_size_hint);
}


// NOTE(maciej): I don't really like how this works. Might need to redo it
int
msh_ply__calculate_list_property_stride( const ply_property_t* pr, 
                                         msh_array(ply_property_t) el_properties,
                                         int swap_endianness )
{
  int64_t stride = 0;
  int64_t offsets[MSH_PLY_FILE_MAX_PROPERTIES] = {0};
  for( size_t l = 0; l < msh_array_len(el_properties); ++l )
  {
    ply_property_t* qr = &el_properties[l];
    offsets[l] = qr->list_offset;
  }

  for( size_t l = 0; l < msh_array_len(el_properties); ++l )
  {

    ply_property_t* qr = &el_properties[l];
   
    if( pr->data == qr->data )
    {
      int32_t list_count = msh_ply__get_data_as_int((uint8_t*)qr->list_data + offsets[l], qr->list_type, swap_endianness );
      stride += list_count * qr->byte_size;
      offsets[l] += qr->list_stride;
    }
  }
  return ((stride > 0) ? stride : 0);
}


int
msh_ply__write_header( const msh_ply_t* pf )
{
  if( !pf->_fp ) { return MSH_PLY_INVALID_FILE_ERR; }
  else
  {
    char* format_string = NULL;
    switch(pf->format)
    {
      case MSH_PLY_ASCII: { format_string = (char*)"ascii"; break; }
      case MSH_PLY_LITTLE_ENDIAN: { format_string = (char*)"binary_little_endian"; break; }
      case MSH_PLY_BIG_ENDIAN: { format_string = (char*)"binary_big_endian"; break;  }
      default: { return MSH_PLY_UNRECOGNIZED_FORMAT_ERR; }
    }

    fprintf(pf->_fp, "ply\nformat %s %2.1f\n", 
                      format_string, (float)pf->format_version);
    for( size_t i = 0; i < msh_array_len(pf->elements); ++i )
    {
      ply_element_t* el = &pf->elements[i];
      fprintf(pf->_fp, "element %s %d\n", el->name, el->count);
      for( size_t j = 0; j < msh_array_len(el->properties); j++)
      {
        ply_property_t* pr = &el->properties[j];
        char* pr_type_str = NULL;
        msh_ply__property_type_to_string( pr->type, &pr_type_str );
        
        if( pr->list_type == MSH_PLY_INVALID )
        {
          fprintf(pf->_fp, "property %s %s\n", pr_type_str, pr->name );
        }
        else
        {
          char* pr_list_type_str = NULL;
          msh_ply__property_type_to_string( pr->list_type, &pr_list_type_str );
          fprintf(pf->_fp, "property list %s %s %s\n", pr_list_type_str, 
                                                       pr_type_str, 
                                                       pr->name );
        }
      }
    }
    fprintf( pf->_fp, "end_header\n");
  }
  return MSH_PLY_NO_ERRORS;
}


int
msh_ply__write_data_ascii( const msh_ply_t* pf )
{
  for( size_t i = 0; i < msh_array_len(pf->elements); ++i )
  {
    ply_element_t* el = &pf->elements[i];

    for( int j = 0; j < el->count; ++j )
    {
      for( size_t k = 0; k < msh_array_len(el->properties); ++k )
      {
        ply_property_t* pr = &el->properties[k];
        if( pr->list_type == MSH_PLY_INVALID )
        {
          msh_ply__fprint_data_at_offset( pf, pr->data, pr->offset, pr->type );
          pr->offset += pr->stride;
        }
        else
        {
          // figure out stride.
          // TODO(maciej): Check stride + hints to decide on fastest way of getting stride.
          int list_count = 0;
          if( pr->list_count != 0 )
          {
            list_count = pr->list_count;
            msh_ply__fprint_data_at_offset( pf, &list_count, 0, pr->list_type );
          }
          else
          {
            pr->stride = msh_ply__calculate_list_property_stride( pr, el->properties, 0 );
            list_count = msh_ply__get_data_as_int( (uint8_t*)pr->list_data + pr->list_offset, pr->list_type, 0 );
            msh_ply__fprint_data_at_offset( pf, pr->list_data, pr->list_offset, pr->list_type );
          }
          pr->list_offset += pr->list_stride;

          for( int l = 0; l < list_count; ++l )
          {
            int cur_offset = pr->offset + l * pr->byte_size;
            msh_ply__fprint_data_at_offset( pf, pr->data, cur_offset, pr->type );
          }
          pr->offset += pr->stride;
        }
      }
      fprintf(pf->_fp, "\n");
    }
  }
  return MSH_PLY_NO_ERRORS;
}


// TODO(maciej): Test if splitting into completly separate functions helps
int
msh_ply__write_data_binary( const msh_ply_t* pf )
{
  int8_t swap_endianness = (pf->_system_format != pf->format);
  for( size_t i = 0; i < msh_array_len(pf->elements); ++i )
  {
    ply_element_t* el = &pf->elements[i];
    int32_t buffer_size = 0;
    // uint32_t hinted_list_counts[MSH_PLY_FILE_MAX_PROPERTIES] = {0};
    for( size_t j = 0; j < msh_array_len(el->properties); ++j )
    {
      // ply_property_t* pr = &el->properties[j];
      // if( pr->list_type != MSH_PLY_INVALID ) 
      // {
        // for( size_t k = 0; k < msh_array_len(pf->_hints); ++k)
        // {
          // if( !strcmp(pr->name, pf->_hints[k].property_name) ) 
          // {
            // hinted_list_counts[j] = pf->_hints[k].expected_size;
          // }
        // }
      // }
      buffer_size += el->properties[j].total_byte_size;
    }

    uint8_t* dst = (uint8_t*)malloc( buffer_size );
    int64_t dst_offset = 0;
    
    for( int j = 0; j < el->count; ++j )
    {
      for( size_t k = 0; k < msh_array_len(el->properties); ++k )
      {
        ply_property_t* pr = &el->properties[k];
        if( pr->list_type == MSH_PLY_INVALID )
        {
          msh_ply__data_assign( dst + dst_offset, (uint8_t*)pr->data + pr->offset, pr->type, 1 );
          pr->offset += pr->stride;
          dst_offset += pr->byte_size;
        }
        else
        {
          // figure out stride. 
          if( !pr->list_count )
          {
            pr->stride = msh_ply__calculate_list_property_stride( pr, el->properties, swap_endianness );
            pr->list_count = msh_ply__get_data_as_int((uint8_t*)pr->list_data + pr->list_offset, pr->list_type, swap_endianness );
            
            msh_ply__data_assign( dst + dst_offset, (uint8_t*)pr->list_data + pr->list_offset, pr->list_type, 1 );
            pr->list_offset += pr->list_stride;
            dst_offset += pr->list_byte_size;
          }
          else
          {
            switch( pr->list_type )
            {
              case MSH_PLY_UINT8:
              {
                uint8_t* dst_ptr = (uint8_t*)dst + dst_offset;
                *dst_ptr = (uint8_t)pr->list_count;
                break;
              }
              case MSH_PLY_UINT16:
              {
                uint16_t* dst_ptr = (uint16_t*)dst + dst_offset;
                *dst_ptr = (uint16_t)pr->list_count;
                break;
              }
              case MSH_PLY_UINT32:
              {
                uint32_t* dst_ptr = (uint32_t*)dst + dst_offset;
                *dst_ptr = (uint32_t)pr->list_count;
                break;
              }
              case MSH_PLY_INT8:
              {
                int8_t* dst_ptr = (int8_t*)dst + dst_offset;
                *dst_ptr = (int8_t)pr->list_count;
                break;
              }
              case MSH_PLY_INT16:
              {
                int16_t* dst_ptr = (int16_t*)dst + dst_offset;
                *dst_ptr = (int16_t)pr->list_count;
                break;
              }
              case MSH_PLY_INT32:
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

          msh_ply__data_assign( dst + dst_offset, (uint8_t*)pr->data + pr->offset, pr->type, pr->list_count );
          pr->offset += pr->stride;
          dst_offset += pr->byte_size * pr->list_count;
        }
      }
    }
    fwrite( dst, buffer_size, 1, pf->_fp );
  }
  return MSH_PLY_NO_ERRORS;
}

int
msh_ply__write_data( const msh_ply_t* pf )
{
  int error = MSH_PLY_NO_ERRORS;
  if( pf->format == MSH_PLY_ASCII ) { error = msh_ply__write_data_ascii(pf); }
  else                              { error = msh_ply__write_data_binary(pf); }
  return error;
}

int 
msh_ply_write( msh_ply_t* pf )
{
  int error = MSH_PLY_NO_ERRORS;
  
  for( size_t i = 0; i < msh_array_len(pf->descriptors); ++i )
  {
    msh_ply_add_property_to_element( pf, pf->descriptors[i] );
  }

  // TODO(maciej): What happens if two same descriptors are added?
  if( msh_array_len(pf->elements) == 0 ) { return MSH_PLY_NO_REQUESTS; }

  error = msh_ply__write_header( pf );
  if( error ) { return error; }
  error = msh_ply__write_data( pf );
  return error;
}

int32_t 
msh_ply_synchronize_list_sizes( msh_ply_t *pf )
{
  int error_code = MSH_PLY_NO_ERRORS;
  for( size_t k = 0; k < msh_array_len( pf->descriptors ); ++k )
  {
    msh_ply_property_desc_t* desc = pf->descriptors[k];
    ply_element_t* el = NULL;
    for( size_t i = 0; i < msh_array_len(pf->elements); ++i )
    {
      ply_element_t* cur_el = &pf->elements[i];
      if( !strcmp( desc->element_name, cur_el->name ) )
      {
        el = cur_el;
        break;
      }
    }
    if( el == NULL ) { error_code = MSH_PLY_REQUESTED_ELEMENT_IS_MISSING; return error_code; }
    for( int32_t i = 0; i < desc->num_properties; ++i )
    {
      int32_t found = 0;
      for( size_t j = 0; j < msh_array_len( el->properties ); ++j )
      {
        ply_property_t* pr = &el->properties[j];
        if( !strcmp( pr->name, desc->property_names[i] ) )
        {
          if( pr->list_type != MSH_PLY_INVALID ) 
          { 
            pr->list_count = desc->list_size_hint; 
            if( desc->list_data == NULL ) { desc->list_type = pr->list_type; }
          }
          else 
          { 
            pr->list_count = 1; 
            desc->list_size_hint = 1; 
          }
          found = 1;
          break;
        }
      }
      if( !found ) { error_code = MSH_PLY_REQUESTED_PROPERTY_IS_MISSING; return error_code; }
    }

    // for( size_t i = 0; i < msh_array_len( el->properties ); ++i )
    // {
    //   printf("%s -> %d\n", el->properties[i].name, el->properties[i].list_count );
    // }
    // printf("\n");
  }

  return MSH_PLY_NO_ERRORS;
}

int32_t 
msh_ply_add_descriptor( msh_ply_t *pf, msh_ply_property_desc_t *desc )
{
  if( !pf ) { return MSH_PLY_FILE_NOT_OPEN_ERR; }
  msh_array_push( pf->descriptors, desc);
  return MSH_PLY_NO_ERRORS;
}

void
msh_ply_print_header(msh_ply_t* pf);

int
msh_ply_read( msh_ply_t* pf )
{
  int error = MSH_PLY_NO_ERRORS;
  if( !pf->_fp ) { return MSH_PLY_FILE_NOT_OPEN_ERR; }
  if( msh_array_len( pf->descriptors ) == 0 ) { return MSH_PLY_NO_REQUESTS; }
  if( !pf->_parsed ) { error = msh_ply_parse_header( pf ); }
  if( error ) { return error; }
  error = msh_ply_synchronize_list_sizes( pf ); 
  if( error ) { return error; }

  error = msh_ply_parse_contents( pf );

  if( error ) { return error; }
  
  for( size_t i = 0; i < msh_array_len( pf->descriptors ); ++i )
  {
    msh_ply_property_desc_t *desc = pf->descriptors[i];
    error = msh_ply_get_property_from_element( pf, desc );
    if( error ) { return error; }

  }

  return error;
}

void
msh_ply_print_header(msh_ply_t* pf)
{
  // Find property & element with longest name
  size_t max_pr_length = 0;

  for( size_t i = 0; i < msh_array_len(pf->elements); ++i )
  {
    for( size_t j = 0; j < msh_array_len(pf->elements[i].properties); ++j )
    {
      max_pr_length = msh_max(strlen(pf->elements[i].properties[j].name),
                              max_pr_length);
    }
  }

  char *type_str = NULL;
  char *list_type_str = NULL;
  char spaces[MSH_PLY_FILE_MAX_STR_LEN];
  for( int i = 0; i < MSH_PLY_FILE_MAX_STR_LEN; ++i ) { spaces[i] = ' ';}
  spaces[MSH_PLY_FILE_MAX_STR_LEN-1] = 0;

  char *format_str = NULL;
  if(pf->format == MSH_PLY_ASCII)         { format_str = (char*)"Ascii"; }
  if(pf->format == MSH_PLY_LITTLE_ENDIAN) { format_str = (char*)"Binary Little Endian"; }
  if(pf->format == MSH_PLY_BIG_ENDIAN)    { format_str = (char*)"Binary Big Endian"; }
  printf("PLY: %s %d\n", format_str, pf->format_version);
  for( size_t i = 0; i < msh_array_len(pf->elements); ++i )
  {
    printf("   '%s' count: %d\n", pf->elements[i].name,
                                    pf->elements[i].count);
    for( size_t j = 0; j < msh_array_len(pf->elements[i].properties); ++j )
    {
      size_t n_spaces = max_pr_length - strlen(pf->elements[i].properties[j].name);
      spaces[n_spaces] = 0;
      msh_ply__property_type_to_string( pf->elements[i].properties[j].type, &type_str );
      if(pf->elements[i].properties[j].list_type == MSH_PLY_INVALID)
      {
        printf("      '%s'%s | %7s(%d bytes)\n", pf->elements[i].properties[j].name,
                                            spaces, type_str, pf->elements[i].properties[j].byte_size );
      }
      else
      {
        msh_ply__property_type_to_string( pf->elements[i].properties[j].list_type, &list_type_str );
        printf("      '%s'%s | %7s(%d bytes) %7s(%d bytes)\n", 
                                        pf->elements[i].properties[j].name, spaces, 
                                        list_type_str, pf->elements[i].properties[j].list_byte_size,
                                        type_str, pf->elements[i].properties[j].byte_size );
      }
      spaces[n_spaces] = ' ';
    }
  }
}


msh_ply_t*
msh_ply_open( const char* filename, const char* mode )
{
  msh_ply_t *pf = NULL;
  if( mode[0] != 'r' && mode[0] != 'w' ) return NULL;
  FILE* fp = fopen(filename, mode);
  
  
  if( fp )
  {
    pf                 = (msh_ply_t*)malloc(sizeof(msh_ply_t));
    pf->valid          = 0;
    pf->format         = -1;
    pf->format_version = 0;
    pf->elements       = 0;
    pf->descriptors    = 0;
    pf->_fp            = fp;
    pf->_parsed        = 0;
  
    // endianness check
    int n = 1;
    if(*(char *)&n == 1) { pf->_system_format = MSH_PLY_LITTLE_ENDIAN; }
    else                 { pf->_system_format = MSH_PLY_BIG_ENDIAN; }
  }
  if( mode[0] == 'w' )
  {
    pf->format_version = 1;
    pf->format = MSH_PLY_ASCII;
    if( strlen(mode) > 1 && mode[1] == 'b' ) { pf->format = pf->_system_format; }
  }
  return pf;
}

int
msh_ply_close( msh_ply_t* pf )
{
  if(pf->_fp) { fclose(pf->_fp); pf->_fp = NULL; }
  // NOTE(maciej): When we drop dynamic arrays we will need to change this
  if(pf->elements)
  {
    for( size_t i = 0; i < msh_array_len(pf->elements); ++i )
    {
      ply_element_t* el = &pf->elements[i];
      if(el->properties) { msh_array_free(el->properties); }
      if(el->data) { free(el->data); }
    }
    msh_array_free(pf->elements);
  }
  if(pf->descriptors) msh_array_free(pf->descriptors);
  free(pf);
  return 1;
}

#endif /* MSH_PLY_IMPLEMENTATION */