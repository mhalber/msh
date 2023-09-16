/*
  ==============================================================================
  Licensing information can be found at the end of the file.
  ==============================================================================
  
  MSH_PLY.H v1.01
  
  A single header library for reading and writing ply files.

  To use the library you simply add:
    #define MSH_PLY_IMPLEMENTATION
    #include "msh_ply.h"

  ==============================================================================
  API DOCUMENTATION

  Customization
  -------------------
  'msh_ply.h' performs dynamic memory allocation. You have an option to provide alternate memory
  allocation functions, by defining following macros prior to inclusion of this file:
    - MSH_PLY_MALLOC
    - MSH_PLY_REALLOC
    - MSH_PLY_FREE
  
  You have an option to specify whether you need only encoding/decoding parts of the library.
    #define MSH_PLY_ENCODER_ONLY  - only pull in writing functionality
    #define MSH_PLY_DECODER_ONLY  - only pull in reading functionality

  msh_ply_open
  -------------------
    msh_ply_t* msh_ply_open( const char* filename, const char* mode );
  
  Creates a new ply file handle and returns pointer to it. 'filename' is the path to the ply file
  and 'mode' describes in what mode file should be used:
    'r' or 'rb' - read
    'w'         - write ASCII
    'wb'        - write binary(will write endianness based on your system)
  Note that this does not perform any reading / writing.

  msh_ply_add_descriptor
  -------------------
    int32_t msh_ply_add_descriptor( msh_ply_t *pf, msh_ply_desc_t *desc );
  
  Adds a data descriptor 'desc' to ply file object 'pf'. Descriptors provide information that is 
  requested from ply file object. See example for more details. Returns 0 on success and error
  code on failure.

  msh_ply_read
  -------------------
    int32_t msh_ply_read( msh_ply_t* pf );
  
  Performs reading of ply file described by 'pf'. Should be called after adding descriptors. 
  Returns 0 on success and error code on failure.
    
  msh_ply_write
  -------------------
    int32_t msh_ply_write( msh_ply_t* pf );
  
  Performs writing of ply file described by 'pf'. Should be called after adding descriptors.
  Returns 0 on success and error code on failure.

  msh_ply_parse_header
  -------------------
    int32_t msh_ply_parse_header( msh_ply_t* pf );

  Parses just the header of the file. Needs to be called to programatically check what
  contents file has, but will not read any of the actual data. Returns 0 on success and error 
  code on failure.
  
  msh_ply_has_properties
  -------------------
    bool msh_ply_has_properties( const msh_ply_t* pf, const msh_ply_desc_t* desc );
  
  Returns true if the ply file 'pf' contains properties contained in descriptor 'desc'. This is 
  the preffered  method to check whether some property exist in ply file 'pf'. 
  Can only be called after header has been parsed!

  msh_ply_find_element
  -------------------
    msh_ply_element_t*  msh_ply_find_element( const msh_ply_t* pf, const char* element_name );

  Returns a pointer to element if the element of given name has been found in 'pf'. Returns NULL 
  otherwise. Can only be called after header has been parsed!
 
  msh_ply_find_property
  -------------------
    msh_ply_property_t* msh_ply_find_property( const msh_ply_element_t* el, const char* property_name );

  Returns a pointer to property if the property of given name has been found in 'pf'.
  Returns NULL otherwise. Can be called after header has been parsed!

  msh_ply_close
  -------------------
    void msh_ply_close( msh_ply_t* pf );

  Closes the ply file 'pf'. 

  msh_ply_error_msg
  -------------------
    const char* msh_ply_error_msg( int32_t err );

  Get pointer to a error message string from the error code. 
  
  void msh_ply_print_header( msh_ply_t* pf );
  -------------------
    void msh_ply_print_header( msh_ply_t* pf );

  Pretty print of the ply file header.
    
  ==============================================================================
  EXAMPLES:

  This library usage focuses around the concept of descriptors. User describes his/her/their 
  data with a set of descriptors. Descriptors tell us what element and properties is
  the data describing, what is the data type, as well as storing pointer to actual data.
  Once descriptors are prepared they can be added to each of ply files and written out.
  System for reading is exactly the same, but you'd be describing requested data.
  Note that ply file does not own pointers to the data, so you will need to delete these
  separately. Additionally, every function returns an error code, which can be turned 
  into string message using "msh_ply_error_msg( ply_err_t err )" function. Not used
  below for clarity.

  ------------------------------
  Writing example:

  typedef struct TriMesh
  {
    Vec3f* vertices;
    Vec3i* faces;
    int32_t n_vertices;
    int32_t n_faces;
  } TriMesh_t;

  TriMesh_t mesh = {0};
  your_function_to_initialize_mesh( &mesh );

  msh_ply_desc_t descriptors[2];
  descriptors[0] = { .element_name = "vertex",
                     .property_names = (const char*[]){"x", "y", "z"},
                     .num_properties = 3,
                     .data_type = MSH_PLY_FLOAT,
                     .data = &mesh->vertices,
                     .data_count = &mesh->n_vertices };

  descriptors[1] = { .element_name = "face",
                     .property_names = (const char*[]){"vertex_indices"},
                     .num_properties = 1,
                     .data_type = MSH_PLY_INT32,
                     .list_type = MSH_PLY_UINT8,
                     .data = &mesh->faces,
                     .data_count = &mesh->n_faces,
                     .list_size_hint = 3 };

  // Create new ply file
  msh_ply_t* ply_file = msh_ply_open( filenames[i], "wb" );

  // Add descriptors to ply file
  msh_ply_add_descriptor( ply_file, &descriptors[0] );
  msh_ply_add_descriptor( ply_file, &descriptors[1] );

  // Write data to disk
  msh_ply_write( ply_file );

  // Close ply file
  msh_ply_close( ply_file );


  ------------------------------
  Reading example:

  typedef struct TriMesh
  {
    Vec3f* vertices;
    Vec3i* faces;
    int32_t n_vertices;
    int32_t n_faces;
  } TriMesh_t;

  TriMesh_t* meshes = NULL;
  int32_t n_meshes = 10;
  for (int32_t i = 0; i < n_meshes; ++i)
  {
    meshes[i].vertices   = NULL;
    meshes[i].faces      = NULL;
    meshes[i].n_faces    = 0;
    meshes[i].n_vertices = 0;
  }

  msh_ply_desc_t descriptors[2];
  descriptors[0] = { .element_name = "vertex",
                     .property_names = (const char*[]){"x", "y", "z"},
                     .num_properties = 3,
                     .data_type = MSH_PLY_FLOAT };
  descriptors[1] = { .element_name = "vertex",
                     .property_names = (const char*[]){"vertex_indices"},
                     .num_properties = 1,
                     .data_type = MSH_PLY_INT32,
                     .list_type = MSH_PLY_UINT8,
                     .list_size_hint = 3 };

  for (int32_t i = 0; i < n_meshes; ++i)
  {
    // Create new ply file
    msh_ply_t* ply_file = msh_ply_open( filenames[i], "rb" );

    // Add data to descriptors
    descriptors[0].data = &meshes[i].vertices;
    descriptors[0].data_count = &meshes[i].n_vertices;
    descriptors[1].data = &meshes[i].faces;
    descriptors[1].data_count = &meshes[i].n_faces;

    // Add descriptors to ply file
    msh_ply_add_descriptor( ply_file, &descriptors[0] );
    msh_ply_add_descriptor( ply_file, &descriptors[1] );

    // Read the file
    msh_ply_read( ply_file );

    // Close ply file
    msh_ply_close( ply_file );
  }

  ==============================================================================
  DEPENDENCIES

    This file requires following c stdlib headers:
    - assert.h
    - stdlib.h
    - stdint.h
    - string.h
    - stdio.h
    - stdbool.h
    Note that this file will not pull them in automatically to prevent pulling same
    files multiple time. If you do not like this behaviour and want this file to
    pull in c headers, simply define following before including the library:

    #define MSH_PLY_INCLUDE_LIBC_HEADERS

  ==============================================================================
  AUTHORS:
    Maciej Halber

  CREDITS:
    Inspiration for single header ply reader:   tinyply    by ddiakopoulos
    Dynamic array based on                      stb.h      by Sean T. Barrett

  ==============================================================================
  TODOs:
  [ ] Support buffering -> read data in chunks, then serve it out, rather than continuously fread.
  [ ] Add independence from c stdlib (like stdio etc.) -> Read from memory / Pass pointers to fopen?
  [ ] Better ascii support - check Vilya Harvey's miniply
  [ ] Getting raw data for the list property - Add different function.
  [ ] Check for duplicates in user-provided descriptor set.
  [ ] Writing optimization
    [ ] Profile lucy writing
  [ ] Error reporting
    [ ] Revise when errors are reported
    [ ] Just have a switch statement instead of static array
  [ ] Write C++ support(?)
  [ ] Add static function definition macro

  ==============================================================================
  REFERENCES:
  [1] stretchy_buffer https://github.com/nothings/stb/blob/master/stretchy_buffer.h
*/

#ifndef MSH_PLY
#define MSH_PLY

#if defined(MSH_PLY_INCLUDE_LIBC_HEADERS)
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/stat.h>
#endif

#ifndef MSH_PLY_MALLOC
#define MSH_PLY_MALLOC(x) malloc((x))
#endif

#ifndef MSH_PLY_REALLOC
#define MSH_PLY_REALLOC(x, y) realloc((x), (y))
#endif

#ifndef MSH_PLY_FREE
#define MSH_PLY_FREE(x) free((x))
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_MSC_VER)
#define MSH_PLY_INLINE __forceinline
#else
#define MSH_PLY_INLINE __attribute__((always_inline, unused)) inline
#endif

#ifndef MSH_PLY_DEF
#ifdef MSH_PLY_STATIC
#define MSH_PLY_DEF static
#else
#define MSH_PLY_DEF extern
#endif
#endif

#ifndef MSH_PLY_PRIVATE
#define MSH_PLY_PRIVATE static
#endif

#define MSH_PLY_MAX(a, b)          ((a) > (b) ? (a) : (b))
#define MSH_PLY_MAX_STR_LEN        1024
#define MSH_PLY_MAX_REQ_PROPERTIES 32
#define MSH_PLY_MAX_PROPERTIES     128
#define MSH_PLY_MAX_LIST_ELEMENTS  1024

typedef struct msh_ply_property msh_ply_property_t;
typedef struct msh_ply_element msh_ply_element_t;
typedef struct msh_ply_desc msh_ply_desc_t;
typedef struct msh_ply_file msh_ply_t;

typedef enum msh_ply_type_id
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
} msh_ply_type_id_t;

typedef enum msh_ply_format
{
  MSH_PLY_ASCII = 0,
  MSH_PLY_LITTLE_ENDIAN,
  MSH_PLY_BIG_ENDIAN
} msh_ply_format_t;

struct msh_ply_desc
{
  char* element_name;
  const char** property_names;
  int16_t num_properties;
  msh_ply_type_id_t data_type;
  msh_ply_type_id_t list_type;
  void* data;
  void* list_data;
  int32_t* data_count;
  uint8_t list_size_hint;
};

MSH_PLY_DEF msh_ply_t* msh_ply_open(const char* filename, const char* mode);
MSH_PLY_DEF void msh_ply_close(msh_ply_t* pf);
MSH_PLY_DEF int32_t msh_ply_add_descriptor(msh_ply_t* pf, msh_ply_desc_t* desc);
MSH_PLY_DEF int32_t msh_ply_parse_header(msh_ply_t* pf);
MSH_PLY_DEF bool msh_ply_has_properties(const msh_ply_t* pf,
                                        const msh_ply_desc_t* desc);
MSH_PLY_DEF msh_ply_element_t* msh_ply_find_element(const msh_ply_t* pf,
                                                    const char* element_name);
MSH_PLY_DEF msh_ply_property_t*
msh_ply_find_property(const msh_ply_element_t* el, const char* property_name);
MSH_PLY_DEF const char* msh_ply_error_msg(int32_t err);
MSH_PLY_DEF void msh_ply_print_header(msh_ply_t* pf);

MSH_PLY_DEF int32_t msh_ply_add_property_to_element(msh_ply_t* pf,
                                                    const msh_ply_desc_t* desc);
MSH_PLY_DEF int32_t msh_ply_get_property_from_element(msh_ply_t* pf,
                                                      msh_ply_desc_t* desc);

#ifndef MSH_PLY_ENCODER_ONLY
MSH_PLY_DEF int32_t msh_ply_read(msh_ply_t* pf);
#endif

#ifndef MSH_PLY_DECODER_ONLY
MSH_PLY_DEF int32_t msh_ply_write(msh_ply_t* pf);
#endif

#ifdef __cplusplus
}
#endif

#endif /* MSH_PLY */

////////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////////
#ifdef MSH_PLY_IMPLEMENTATION

////////////////////////////////////////////////////////////////////////////////
// THIS IS A SIMPLIFIED VERSION OF MSH_ARRAY

#ifdef __cplusplus
extern "C" {
#endif

typedef struct msh_ply_array_header
{
  size_t len;
  size_t cap;
} msh_ply_array_hdr_t;

#define msh_ply_array(T) T*

MSH_PLY_PRIVATE void* msh_ply__array_grow(const void* array, size_t new_len, size_t elem_size);

#define msh_ply_array__grow_formula(x) ((2 * (x) + 5))
#define msh_ply_array__hdr(a)                                                  \
  ((msh_ply_array_hdr_t*)((char*)(a) - sizeof(msh_ply_array_hdr_t)))

#define msh_ply_array_len(a)   ((a) ? (msh_ply_array__hdr((a))->len) : 0)
#define msh_ply_array_cap(a)   ((a) ? (msh_ply_array__hdr((a))->cap) : 0)
#define msh_ply_array_front(a) ((a) ? (a) : NULL)
#define msh_ply_array_back(a)                                                  \
  (msh_ply_array_len((a)) ? ((a) + msh_ply_array_len((a)) - 1) : NULL)

#define msh_ply_array_free(a)                                                  \
  ((a) ? (MSH_PLY_FREE(msh_ply_array__hdr(a)), (a) = NULL) : 0)
#define msh_ply_array_fit(a, n)                                                \
  ((n) <= msh_ply_array_cap(a)                                                 \
     ? (0)                                                                     \
     : (*(void**)&(a) = msh_ply__array_grow((a), (n), sizeof(*(a)))))
#define msh_ply_array_push(a, ...)                                             \
  (msh_ply_array_fit((a), 1 + msh_ply_array_len((a))),                         \
   (a)[msh_ply_array__hdr(a)->len++] = (__VA_ARGS__))

#ifdef __cplusplus
}
#endif

struct msh_ply_property
{
  int32_t list_count;
  int16_t byte_size;
  int32_t offset;
  int32_t stride;
  int16_t list_byte_size;
  int32_t list_offset;
  int32_t list_stride;

  msh_ply_type_id_t type;
  msh_ply_type_id_t list_type;

  int32_t total_count;
  size_t total_byte_size;

  // Data storage
  char name[32];
  void* data;
  void* list_data;
};

struct msh_ply_element
{
  char name[64];
  int32_t count;
  msh_ply_array(msh_ply_property_t) properties;

  long file_anchor;
  void* data;
  size_t data_size;
};

struct msh_ply_file
{
  int32_t valid;
  int32_t format;
  int32_t format_version;

  msh_ply_array(msh_ply_element_t) elements;
  msh_ply_array(msh_ply_desc_t*) descriptors;

  FILE* _fp;
  int32_t _header_size;
  int32_t _system_format;
  int32_t _parsed;
};

enum msh_ply_err
{
  MSH_PLY_NO_ERR                             = 0,
  MSH_PLY_INVALID_FILE_ERR                   = 1,
  MSH_PLY_INVALID_FORMAT_ERR                 = 2,
  MSH_PLY_FILE_OPEN_ERR                      = 3,
  MSH_PLY_FILE_NOT_OPEN_ERR                  = 4,
  MSH_PLY_LINE_PARSE_ERR                     = 5,
  MSH_PLY_FORMAT_CMD_ERR                     = 6,
  MSH_PLY_ELEMENT_CMD_ERR                    = 7,
  MSH_PLY_PROPERTY_CMD_ERR                   = 8,
  MSH_PLY_ELEMENT_NOT_FOUND_ERR              = 9,
  MSH_PLY_PROPERTY_NOT_FOUND_ERR             = 10,
  MSH_PLY_BINARY_PARSE_ERR                   = 11,
  MSH_PLY_CONFLICTING_NUMBER_OF_ELEMENTS_ERR = 12,
  MSH_PLY_UNRECOGNIZED_CMD_ERR               = 13,
  MSH_PLY_NO_REQUESTS                        = 14,
  MSH_PLY_NULL_DESCRIPTOR_ERR                = 15,
  MSH_PLY_NULL_ELEMENT_NAME_ERR              = 16,
  MSH_PLY_NULL_PROPERTY_NAMES_ERR            = 17,
  MSH_PLY_NULL_PROPERTY_NAME_ERR             = 18,
  MSH_PLY_NO_REQUESTED_PROPERTIES_ERR        = 19,
  MSH_PLY_NULL_DATA_PTR_ERR                  = 20,
  MSH_PLY_NULL_DATA_COUNT_PTR_ERR            = 21,
  MSH_PLY_INVALID_DATA_TYPE_ERR              = 22,
  MSH_PLY_INVALID_LIST_TYPE_ERR              = 23,
  MSH_PLY_ASCII_FILE_READ_ERR                = 24,
  MSH_PLY_ASCII_FILE_EOF_ERR                 = 25,
  MSH_PLY_READ_REQUIRED_PROPERTY_IS_MISSING  = 26,
  MSH_PLY_WRITE_REQUIRED_PROPERTY_IS_MISSING = 27,
  MSH_PLY_NUM_OF_ERRORS
};

static const char* msh_ply_error_msgs[MSH_PLY_NUM_OF_ERRORS] = {
  "MSH_PLY: No errors.",
  "MSH_PLY: Invalid PLY file.",
  "MSH_PLY: Invalid PLY file: Invalid format in ply file.",
  "MSH_PLY: Could not open file.",
  "MSH_PLY: File was not open. Please open file first.",
  "MSH_PLY: Invalid PLY file: Could not parse line in a header.",
  "MSH_PLY: Invalid PLY file: Format of ply command is invalid.",
  "MSH_PLY: Invalid PLY file: Format of element command is invalid.",
  "MSH_PLY: Invalid PLY file: Format of property command is invalid.",
  "MSH_PLY: Could not find requested element in the input ply file.",
  "MSH_PLY: Could not find requested property in the input ply file.",
  "MSH_PLY: Error reading binary file.",
  "MSH_PLY: Number of requested elements does not match the descriptor.",
  "MSH_PLY: Invalid PLY file: Unrecognized command in a ply file.",
  "MSH_PLY: There is no requests for data to be read or writen. Aborting "
  "operation.",
  "MSH_PLY: Invalid descriptor: Desciptor pointer is NULL.",
  "MSH_PLY: Invalid descriptor: Element name pointer is NULL.",
  "MSH_PLY: Invalid descriptor: Property names pointer is NULL.",
  "MSH_PLY: Invalid descriptor: One of the property names is NULL.",
  "MSH_PLY: Invalid descriptor: Number of requested properties is zero or less "
  "than zero.",
  "MSH_PLY: Invalid descriptor: Data pointer is NULL.",
  "MSH_PLY: Invalid descriptor: Data Count pointer is NULL.",
  "MSH_PLY: Invalid descriptor: Incorrect data type.",
  "MSH_PLY: Invalid descriptor: Incorrect list type. List type cannot be float "
  "or double.",
  "MSH_PLY: Error reading ASCII PLY file.",
  "MSH_PLY: Reached EOF when reading ASCII PLY file."
  "MSH_PLY: When reading file, the required property not found in the input "
  "file",
  "MSH_PLY: When write file, the required property not found in the input file",
};

MSH_PLY_DEF const char*
msh_ply_error_msg(int32_t err)
{
  return msh_ply_error_msgs[err];
}

MSH_PLY_PRIVATE void*
msh_ply__array_grow(const void* array, size_t new_len, size_t elem_size)
{
  size_t old_cap  = msh_ply_array_cap(array);
  size_t new_cap  = (size_t)msh_ply_array__grow_formula(old_cap);
  new_cap         = (size_t)MSH_PLY_MAX(new_cap, MSH_PLY_MAX(new_len, 16));
  size_t new_size = sizeof(msh_ply_array_hdr_t) + new_cap * elem_size;
  msh_ply_array_hdr_t* new_hdr;

  if (array)
  {
    new_hdr = (msh_ply_array_hdr_t*)MSH_PLY_REALLOC(msh_ply_array__hdr(array),
                                                    new_size);
  }
  else
  {
    new_hdr      = (msh_ply_array_hdr_t*)MSH_PLY_MALLOC(new_size);
    new_hdr->len = 0;
  }
  new_hdr->cap = new_cap;
  return (void*)((char*)new_hdr + sizeof(msh_ply_array_hdr_t));
}

MSH_PLY_PRIVATE msh_ply_property_t
msh_ply__property_zero_init()
{
  msh_ply_property_t pr =
    {0, 0, 0, 0, 0, 0, 0, MSH_PLY_INVALID, MSH_PLY_INVALID, 0, 0, {0}, 0, 0};
  return pr;
}

MSH_PLY_PRIVATE msh_ply_element_t
msh_ply__element_zero_init()
{
  msh_ply_element_t el = {{0}, 0, 0, 0, 0, 0};
  return el;
}

MSH_PLY_DEF msh_ply_element_t*
msh_ply_find_element(const msh_ply_t* pf, const char* element_name)
{
  msh_ply_element_t* el = NULL;
  for (size_t i = 0; i < msh_ply_array_len(pf->elements); ++i)
  {
    if (!strcmp(element_name, pf->elements[i].name))
    {
      el = &pf->elements[i];
      break;
    }
  }
  return el;
}

MSH_PLY_DEF bool
msh_ply_has_properties(const msh_ply_t* pf, const msh_ply_desc_t* desc)
{
  msh_ply_element_t* el = msh_ply_find_element(pf, desc->element_name);
  if (!el) { return false; }

  for (int32_t i = 0; i < desc->num_properties; ++i)
  {
    const char* property_name = desc->property_names[i];
    bool found                = false;
    for (size_t j = 0; j < msh_ply_array_len(el->properties); ++j)
    {
      if (!strcmp(el->properties[j].name, property_name))
      {
        found = true;
        break;
      }
    }
    if (!found) { return false; }
  }

  return true;
}

MSH_PLY_DEF msh_ply_property_t*
msh_ply_find_property(const msh_ply_element_t* el, const char* property_name)
{
  msh_ply_property_t* pr = NULL;
  for (size_t i = 0; i < msh_ply_array_len(el->properties); ++i)
  {
    if (!strcmp(el->properties[i].name, property_name))
    {
      pr = &el->properties[i];
      break;
    }
  }

  return pr;
}

MSH_PLY_PRIVATE int32_t
msh_ply__validate_descriptor(const msh_ply_desc_t* desc)
{
  if (!desc) { return MSH_PLY_NULL_DESCRIPTOR_ERR; }
  if (!desc->element_name) { return MSH_PLY_NULL_ELEMENT_NAME_ERR; }
  if (!desc->property_names) { return MSH_PLY_NULL_PROPERTY_NAMES_ERR; }
  if (desc->num_properties <= 0) { return MSH_PLY_NO_REQUESTED_PROPERTIES_ERR; }
  if (!desc->data) { return MSH_PLY_NULL_DATA_PTR_ERR; }
  if (!desc->data_count) { return MSH_PLY_NULL_DATA_COUNT_PTR_ERR; }
  if (desc->data_type <= MSH_PLY_INVALID || desc->data_type >= MSH_PLY_N_TYPES)
  {
    return MSH_PLY_INVALID_DATA_TYPE_ERR;
  }
  if (desc->list_type < MSH_PLY_INVALID || desc->list_type >= MSH_PLY_FLOAT)
  {
    return MSH_PLY_INVALID_LIST_TYPE_ERR;
  }
  for (int32_t i = 0; i < desc->num_properties; ++i)
  {
    if (!desc->property_names[i]) { return MSH_PLY_NULL_PROPERTY_NAME_ERR; }
  }

  return MSH_PLY_NO_ERR;
}

MSH_PLY_DEF int32_t
msh_ply_add_descriptor(msh_ply_t* pf, msh_ply_desc_t* desc)
{
  if (!pf) { return MSH_PLY_FILE_NOT_OPEN_ERR; }
  int32_t desc_err = msh_ply__validate_descriptor(desc);
  if (desc_err) { return desc_err; }
  msh_ply_array_push(pf->descriptors, desc);
  return MSH_PLY_NO_ERR;
}

MSH_PLY_PRIVATE MSH_PLY_INLINE void
msh_ply__swap_bytes(uint8_t* buffer, int32_t type_size, int32_t count)
{
  for (int32_t i = 0; i < count; ++i)
  {
    int32_t offset = i * type_size;
    for (int32_t j = 0; j < type_size >> 1; ++j)
    {
      uint8_t temp                       = buffer[offset + j];
      buffer[offset + j]                 = buffer[offset + type_size - 1 - j];
      buffer[offset + type_size - 1 - j] = temp;
    }
  }
}

MSH_PLY_PRIVATE MSH_PLY_INLINE int32_t
msh_ply__get_data_as_int(void* data, int32_t type, int8_t swap_endianness)
{
  int32_t retval = 0;
  switch (type)
  {
    case MSH_PLY_INT8:
      retval = ((char*)data)[0];
      break;
    case MSH_PLY_INT16:
    {
      retval = ((int16_t*)data)[0];
      if (swap_endianness) msh_ply__swap_bytes((uint8_t*)&retval, 2, 1);
      break;
    }
    case MSH_PLY_INT32:
    {
      retval = ((int32_t*)data)[0];
      if (swap_endianness) msh_ply__swap_bytes((uint8_t*)&retval, 4, 1);
      break;
    }
    case MSH_PLY_UINT8:

      retval = ((uint8_t*)data)[0];
      break;
    case MSH_PLY_UINT16:
    {
      retval = ((uint16_t*)data)[0];
      if (swap_endianness) msh_ply__swap_bytes((uint8_t*)&retval, 2, 1);
      break;
    }
    case MSH_PLY_UINT32:
    {
      retval = ((uint32_t*)data)[0];
      if (swap_endianness) msh_ply__swap_bytes((uint8_t*)&retval, 4, 1);
      break;
    }
    default:
      retval = 0;
      break;
  }
  return retval;
}

// TODO(maciej): Static table instead of switch statement?
MSH_PLY_PRIVATE MSH_PLY_INLINE int16_t
msh_ply__type_to_byte_size(msh_ply_type_id_t type)
{
  int16_t retval = 0;
  switch (type)
  {
    case MSH_PLY_INT8:
    {
      retval = 1;
      break;
    }
    case MSH_PLY_INT16:
    {
      retval = 2;
      break;
    }
    case MSH_PLY_INT32:
    {
      retval = 4;
      break;
    }
    case MSH_PLY_UINT8:
    {
      retval = 1;
      break;
    }
    case MSH_PLY_UINT16:
    {
      retval = 2;
      break;
    }
    case MSH_PLY_UINT32:
    {
      retval = 4;
      break;
    }
    case MSH_PLY_FLOAT:
    {
      retval = 4;
      break;
    }
    case MSH_PLY_DOUBLE:
    {
      retval = 8;
      break;
    }
    default:
    {
      retval = 0;
      break;
    }
  }
  return retval;
}

// TODO(maciej): Static table instead of switch statement?
MSH_PLY_PRIVATE MSH_PLY_INLINE void
msh_ply__property_type_to_string(msh_ply_type_id_t type, char** string)
{
  switch (type)
  {
    case MSH_PLY_INT8:
    {
      *string = (char*)"char";
      break;
    }
    case MSH_PLY_INT16:
    {
      *string = (char*)"short";
      break;
    }
    case MSH_PLY_INT32:
    {
      *string = (char*)"int";
      break;
    }
    case MSH_PLY_UINT8:
    {
      *string = (char*)"uchar";
      break;
    }
    case MSH_PLY_UINT16:
    {
      *string = (char*)"ushort";
      break;
    }
    case MSH_PLY_UINT32:
    {
      *string = (char*)"uint";
      break;
    }
    case MSH_PLY_FLOAT:
    {
      *string = (char*)"float";
      break;
    }
    case MSH_PLY_DOUBLE:
    {
      *string = (char*)"double";
      break;
    }
    default:
    {
      break;
    }
  }
}

//TODO(maciej): String interning?
MSH_PLY_PRIVATE MSH_PLY_INLINE void
msh_ply__string_to_property_type(char* type_str,
                                 msh_ply_type_id_t* pr_type,
                                 int16_t* pr_size)
{
  if (!strcmp("int8", type_str) || !strcmp("char", type_str))
  {
    *pr_type = MSH_PLY_INT8;
    *pr_size = 1;
  }
  else if (!strcmp("uint8", type_str) || !strcmp("uchar", type_str))
  {
    *pr_type = MSH_PLY_UINT8;
    *pr_size = 1;
  }
  else if (!strcmp("int16", type_str) || !strcmp("short", type_str))
  {
    *pr_type = MSH_PLY_INT16;
    *pr_size = 2;
  }
  else if (!strcmp("uint16", type_str) || !strcmp("ushort", type_str))
  {
    *pr_type = MSH_PLY_UINT16;
    *pr_size = 2;
  }
  else if (!strcmp("int32", type_str) || !strcmp("int", type_str))
  {
    *pr_type = MSH_PLY_INT32;
    *pr_size = 4;
  }
  else if (!strcmp("uint32", type_str) || !strcmp("uint", type_str))
  {
    *pr_type = MSH_PLY_UINT32;
    *pr_size = 4;
  }
  else if (!strcmp("float32", type_str) || !strcmp("float", type_str))
  {
    *pr_type = MSH_PLY_FLOAT;
    *pr_size = 4;
  }
  else if (!strcmp("float64", type_str) || !strcmp("double", type_str))
  {
    *pr_type = MSH_PLY_DOUBLE;
    *pr_size = 8;
  }
  else
  {
    *pr_type = MSH_PLY_INVALID;
    *pr_size = 0;
  }
}

#define MSH_PLY__COPY_DATA_AS_TYPE(dst, src, T)                                \
  do {                                                                         \
    T* dst_ptr = (T*)(dst);                                                    \
    T* src_ptr = (T*)(src);                                                    \
    for (int32_t c = 0; c < a; c++)                                            \
    {                                                                          \
      *(dst_ptr) = *(src_ptr);                                                 \
      dst_ptr++;                                                               \
      src_ptr++;                                                               \
      *(dst_ptr) = *(src_ptr);                                                 \
      dst_ptr++;                                                               \
      src_ptr++;                                                               \
      *(dst_ptr) = *(src_ptr);                                                 \
      dst_ptr++;                                                               \
      src_ptr++;                                                               \
    }                                                                          \
    for (int32_t c = 0; c < b; c++)                                            \
    {                                                                          \
      *(dst_ptr) = *(src_ptr);                                                 \
      dst_ptr++;                                                               \
      src_ptr++;                                                               \
    }                                                                          \
  } while (0)

MSH_PLY_PRIVATE void
msh_ply__data_assign(void* dst, void* src, int32_t type, int32_t count)
{
  int32_t a = count / 3;
  int32_t b = count % 3;

  switch (type)
  {
    case MSH_PLY_UINT8:
    {
      MSH_PLY__COPY_DATA_AS_TYPE(dst, src, uint8_t);
      break;
    }
    case MSH_PLY_UINT16:
    {
      MSH_PLY__COPY_DATA_AS_TYPE(dst, src, uint16_t);
      break;
    }
    case MSH_PLY_UINT32:
    {
      MSH_PLY__COPY_DATA_AS_TYPE(dst, src, uint32_t);
      break;
    }
    case MSH_PLY_INT8:
    {
      MSH_PLY__COPY_DATA_AS_TYPE(dst, src, int8_t);
      break;
    }
    case MSH_PLY_INT16:
    {
      MSH_PLY__COPY_DATA_AS_TYPE(dst, src, int16_t);
      break;
    }
    case MSH_PLY_INT32:
    {
      MSH_PLY__COPY_DATA_AS_TYPE(dst, src, int32_t);
      break;
    }
    case MSH_PLY_FLOAT:
    {
      MSH_PLY__COPY_DATA_AS_TYPE(dst, src, float);
      break;
    }
    case MSH_PLY_DOUBLE:
    {
      MSH_PLY__COPY_DATA_AS_TYPE(dst, src, double);
      break;
    }
    default:
      break;
  }
}
#undef MSH_PLY__COPY_DATA_AS_TYPE

#ifndef MSH_PLY_ENCODER_ONLY

MSH_PLY_PRIVATE int32_t
msh_ply__parse_ply_cmd(char* line, msh_ply_t* pf)
{
  char cmd_str[MSH_PLY_MAX_STR_LEN];
  if (sscanf(line, "%s", cmd_str))
  {
    if (!strcmp(cmd_str, "ply"))
    {
      pf->valid = true;
      return MSH_PLY_NO_ERR;
    }
  }
  return MSH_PLY_INVALID_FILE_ERR;
}

MSH_PLY_PRIVATE int32_t
msh_ply__parse_format_cmd(char* line, msh_ply_t* pf)
{
  char cmd[MSH_PLY_MAX_STR_LEN];
  char frmt_str[MSH_PLY_MAX_STR_LEN];
  char frmt_ver_str[MSH_PLY_MAX_STR_LEN];
  if (sscanf(line, "%s %s %s", &cmd[0], &frmt_str[0], &frmt_ver_str[0]) != 3)
  {
    return MSH_PLY_FORMAT_CMD_ERR;
  }
  if (!strcmp("ascii", frmt_str)) { pf->format = (int32_t)MSH_PLY_ASCII; }
  else if (!strcmp("binary_little_endian", frmt_str))
  {
    pf->format = (int32_t)MSH_PLY_LITTLE_ENDIAN;
  }
  else if (!strcmp("binary_big_endian", frmt_str))
  {
    pf->format = (int32_t)MSH_PLY_BIG_ENDIAN;
  }
  else
  {
    return MSH_PLY_INVALID_FORMAT_ERR;
  }
  pf->format_version = atoi(frmt_ver_str);
  return MSH_PLY_NO_ERR;
}

MSH_PLY_PRIVATE int32_t
msh_ply__parse_element_cmd(char* line, msh_ply_t* pf)
{
  char cmd[MSH_PLY_MAX_STR_LEN];
  msh_ply_element_t el = msh_ply__element_zero_init();
  int32_t el_count     = 0;
  el.properties        = NULL;
  if (sscanf(line, "%s %s %d", &cmd[0], &el.name[0], &el_count) != 3)
  {
    return MSH_PLY_ELEMENT_CMD_ERR;
  }
  el.count = el_count;
  msh_ply_array_push(pf->elements, el);
  return MSH_PLY_NO_ERR;
}

MSH_PLY_PRIVATE int32_t
msh_ply__parse_property_cmd(char* line, msh_ply_t* pf)
{
  char cmd[MSH_PLY_MAX_STR_LEN];
  char type_str[MSH_PLY_MAX_STR_LEN];
  char list_str[MSH_PLY_MAX_STR_LEN];
  char list_type_str[MSH_PLY_MAX_STR_LEN];
  int32_t valid_format = 0;

  msh_ply_element_t* el = msh_ply_array_back(pf->elements);
  msh_ply_property_t pr = msh_ply__property_zero_init();
  // Try to parse regular property format
  if (sscanf(line, "%s %s %s", &cmd[0], &type_str[0], (char*)&pr.name) == 3)
  {
    msh_ply__string_to_property_type(type_str, &pr.type, &pr.byte_size);
    pr.list_type      = MSH_PLY_INVALID;
    pr.list_byte_size = 0;
    pr.list_count     = 1;
    valid_format      = 1;
  }

  // Try to parse list property format
  cmd[0]           = 0;
  type_str[0]      = 0;
  list_str[0]      = 0;
  list_type_str[0] = 0;
  if (sscanf(line,
             "%s %s %s %s %s",
             &cmd[0],
             &list_str[0],
             &list_type_str[0],
             &type_str[0],
             (char*)&pr.name) == 5)
  {
    if (strcmp(list_str, "list")) { return MSH_PLY_PROPERTY_CMD_ERR; }
    msh_ply__string_to_property_type(type_str, &pr.type, &pr.byte_size);
    msh_ply__string_to_property_type(list_type_str,
                                     &pr.list_type,
                                     &pr.list_byte_size);
    pr.list_count = 0;
    valid_format  = 1;
  }
  pr.total_byte_size = 0;
  pr.total_count     = 0;
  // Both failed
  if (!valid_format) { return MSH_PLY_PROPERTY_CMD_ERR; }

  // Either succeded
  msh_ply_array_push(el->properties, pr);

  return MSH_PLY_NO_ERR;
}

MSH_PLY_PRIVATE int32_t
msh_ply__parse_command(char* cmd, char* line, msh_ply_t* pf)
{
  if (!strcmp(cmd, "ply")) { return msh_ply__parse_ply_cmd(line, pf); }
  if (!strcmp(cmd, "format")) { return msh_ply__parse_format_cmd(line, pf); }
  if (!strcmp(cmd, "element")) { return msh_ply__parse_element_cmd(line, pf); }
  if (!strcmp(cmd, "property"))
  {
    return msh_ply__parse_property_cmd(line, pf);
  }
  return MSH_PLY_UNRECOGNIZED_CMD_ERR;
}

MSH_PLY_DEF int32_t
msh_ply_parse_header(msh_ply_t* pf)
{
  int32_t line_no = 0;
  char line[MSH_PLY_MAX_STR_LEN];
  int32_t err_code = 0;
  while (fgets(&line[0], MSH_PLY_MAX_STR_LEN, pf->_fp))
  {
    line_no++;
    char cmd[MSH_PLY_MAX_STR_LEN];
    if (sscanf(line, "%s", cmd) != (unsigned)1)
    {
      return MSH_PLY_LINE_PARSE_ERR;
    }
    if (!strcmp(cmd, "end_header")) break;
    if (!strcmp(cmd, "comment")) continue;
    if (!strcmp(cmd, "obj_info")) continue;
    err_code = msh_ply__parse_command(cmd, line, pf);
    if (err_code) break;
  }
  pf->_header_size = ftell(pf->_fp);
  if (err_code == MSH_PLY_NO_ERR) { pf->_parsed = 1; }
  return err_code;
}

MSH_PLY_PRIVATE int32_t
msh_ply__calculate_elem_size_ascii(msh_ply_t* pf, msh_ply_element_t* el)
{
  char line[MSH_PLY_MAX_STR_LEN];
  while (fgets(&line[0], MSH_PLY_MAX_STR_LEN, pf->_fp))
  {
    char* cp_up  = &line[0];
    char* cp_low = &line[0];

    for (size_t j = 0; j < msh_ply_array_len(el->properties); ++j)
    {
      msh_ply_property_t* pr = &el->properties[j];
      while (*cp_up != ' ' && *cp_up != '\n') { cp_up++; }
      char tmp_cp_up = *cp_up;
      *cp_up         = 0;   //Fake string end;

      if (pr->list_type != MSH_PLY_INVALID)
      {
        switch (pr->list_type)
        {
          case MSH_PLY_INT8:
          case MSH_PLY_INT16:
          case MSH_PLY_INT32:
          case MSH_PLY_UINT8:
          case MSH_PLY_UINT16:
          case MSH_PLY_UINT32:
            pr->list_count = (int32_t)atoi(cp_low);
            break;
          case MSH_PLY_FLOAT:
          case MSH_PLY_DOUBLE:
            pr->list_count = (int32_t)atof(cp_low);
            break;
          default:
            pr->list_count = 1;
            break;
        }

        // skip
        *cp_up = tmp_cp_up;
        cp_low = cp_up;
        cp_up++;
        for (int32_t k = 0; k < pr->list_count; ++k)
        {
          while (*cp_up != ' ' && *cp_up != '\n') { cp_up++; }
          cp_up++;
        }
      }

      // fixup pointers
      *cp_up = tmp_cp_up;
      if (*cp_up != '\n')
      {
        cp_up++;
        cp_low = cp_up;
      }
      pr->total_byte_size +=
        pr->list_byte_size + pr->list_count * pr->byte_size;
      pr->total_count += pr->list_count;
    }
  }
  return MSH_PLY_NO_ERR;
}

// NOTE(maciej): This is less than stellar, as it is actually reading file line by line.
// Should replace it with buffered reading / read the whole file and parse it
MSH_PLY_PRIVATE int32_t
msh_ply__calculate_elem_size_binary(msh_ply_t* pf, msh_ply_element_t* el)
{
  for (size_t i = 0; i < el->count; ++i)
  {
    for (size_t j = 0; j < msh_ply_array_len(el->properties); ++j)
    {
      msh_ply_property_t* pr = &el->properties[j];
      if (pr->list_type == MSH_PLY_INVALID)
      {
        pr->total_count += 1;
        pr->total_byte_size += pr->byte_size;
        double dummy_data = 0;
        size_t read_count = fread(&dummy_data, pr->byte_size, 1, pf->_fp);
        if (read_count != 1) { return MSH_PLY_BINARY_PARSE_ERR; }
      }
      else
      {
        int32_t count                                = 1;
        double dummy_data[MSH_PLY_MAX_LIST_ELEMENTS] = {0};

        size_t read_count = fread(&count, pr->list_byte_size, 1, pf->_fp);
        if (read_count != 1) { return MSH_PLY_BINARY_PARSE_ERR; }

        read_count = fread(&dummy_data[0], pr->byte_size, count, pf->_fp);
        if (read_count != (size_t)count) { return MSH_PLY_BINARY_PARSE_ERR; }

        pr->total_count += count;
        pr->total_byte_size += pr->list_byte_size + count * pr->byte_size;
      }
    }
  }
  return MSH_PLY_NO_ERR;
}

MSH_PLY_PRIVATE int32_t
msh_ply__can_precalculate_sizes(msh_ply_element_t* el)
{
  int32_t can_precalculate = 1;
  for (size_t i = 0; i < msh_ply_array_len(el->properties); ++i)
  {
    msh_ply_property_t* pr = &el->properties[i];
    if (pr->list_type != MSH_PLY_INVALID && pr->list_count == 0)
    {
      can_precalculate = 0;
      break;
    }
  }

  return can_precalculate;
}

MSH_PLY_PRIVATE int32_t
msh_ply__synchronize_list_sizes(msh_ply_t* pf)
{
  int32_t error_code = MSH_PLY_NO_ERR;
  for (size_t k = 0; k < msh_ply_array_len(pf->descriptors); ++k)
  {
    msh_ply_desc_t* desc  = pf->descriptors[k];
    msh_ply_element_t* el = NULL;
    for (size_t i = 0; i < msh_ply_array_len(pf->elements); ++i)
    {
      msh_ply_element_t* cur_el = &pf->elements[i];
      if (!strcmp(desc->element_name, cur_el->name))
      {
        el = cur_el;
        break;
      }
    }

    if (el == NULL)
    {
      error_code = MSH_PLY_ELEMENT_NOT_FOUND_ERR;
      return error_code;
    }

    for (int32_t i = 0; i < desc->num_properties; ++i)
    {
      int32_t found = 0;
      for (size_t j = 0; j < msh_ply_array_len(el->properties); ++j)
      {
        msh_ply_property_t* pr = &el->properties[j];
        if (!strcmp(pr->name, desc->property_names[i]))
        {
          if (pr->list_type != MSH_PLY_INVALID)
          {
            pr->list_count = desc->list_size_hint;
            if (desc->list_data == NULL) { desc->list_type = pr->list_type; }
          }
          else
          {
            pr->list_count       = 1;
            desc->list_size_hint = 1;
          }
          found = 1;
          break;
        }
      }
      if (!found)
      {
        error_code = MSH_PLY_PROPERTY_NOT_FOUND_ERR;
        return error_code;
      }
    }
  }

  return MSH_PLY_NO_ERR;
}

MSH_PLY_DEF int32_t
msh_ply_parse_contents(msh_ply_t* pf)
{
  int32_t err_code = MSH_PLY_NO_ERR;
  err_code         = msh_ply__synchronize_list_sizes(pf);
  if (err_code) { return err_code; }

  for (size_t i = 0; i < msh_ply_array_len(pf->elements); ++i)
  {
    // If user did not ask for last element, we can skip reading it all together
    int32_t can_skip = (i == msh_ply_array_len(pf->elements) - 1);
    if (can_skip)
    {
      for (size_t j = 0; j < msh_ply_array_len(pf->descriptors); ++j)
      {
        if (!strcmp(pf->descriptors[j]->element_name, pf->elements[i].name))
        {
          can_skip = 0;
          break;
        }
      }
    }
    if (can_skip) { continue; }

    msh_ply_element_t* el  = &pf->elements[i];
    int32_t num_properties = (int32_t)msh_ply_array_len(el->properties);

    if (el->count <= 0 || num_properties <= 0) { continue; }
    el->file_anchor = ftell(pf->_fp);

    // Determine if any of the properties in the element has list
    int32_t can_precalculate_size = msh_ply__can_precalculate_sizes(el);
    if (can_precalculate_size)
    {
      // This is a faster path, as we can just calculate the size required by element in one go.
      int32_t elem_size = 0;
      for (int32_t j = 0; j < num_properties; ++j)
      {
        msh_ply_property_t* pr = &el->properties[j];
        pr->total_byte_size    = pr->byte_size * pr->list_count * el->count;
        elem_size += pr->byte_size * pr->list_count;
        if (pr->list_type != MSH_PLY_INVALID)
        {
          pr->total_byte_size += pr->list_byte_size * el->count;
          elem_size += pr->list_byte_size;
        }
        pr->total_count += pr->list_count * el->count;
      }

      if (pf->format != MSH_PLY_ASCII)
      {
        fseek(pf->_fp, el->count * elem_size, SEEK_CUR);
      }
      else
      {
        char line[MSH_PLY_MAX_STR_LEN];
        for (size_t j = 0; j < el->count; ++j)
        {
          char* ret = fgets(&line[0], MSH_PLY_MAX_STR_LEN, pf->_fp);
          if (!ret)
          {
            if (ferror(pf->_fp)) { return MSH_PLY_ASCII_FILE_READ_ERR; }
            if (feof(pf->_fp)) { return MSH_PLY_ASCII_FILE_EOF_ERR; }
          }
        }
      }
    }
    else
    {
      // There exists a list property. We need to calculate required size via pass through
      if (pf->format == MSH_PLY_ASCII)
      {
        err_code = msh_ply__calculate_elem_size_ascii(pf, el);
      }
      else
      {
        err_code = msh_ply__calculate_elem_size_binary(pf, el);
      }
    }
  }

  return err_code;
}

MSH_PLY_PRIVATE int32_t
msh_ply__get_element_count(const msh_ply_element_t* el, int32_t* count)
{
  int32_t err_code = MSH_PLY_NO_ERR;
  *count           = el->count;
  return err_code;
}

MSH_PLY_PRIVATE int32_t
msh_ply__get_element_size(const msh_ply_element_t* el, size_t* size)
{
  int32_t err_code = MSH_PLY_NO_ERR;

  *size = 0;
  for (size_t i = 0; i < msh_ply_array_len(el->properties); ++i)
  {
    *size += el->properties[i].total_byte_size;
  }
  return err_code;
}

// NOTE(maciej): this works better with an assignment
#define MSH_PLY__CONVERT_AND_ASSIGN(D, C, T, conv_funct)                       \
  do {                                                                         \
    T n        = (T)conv_funct(C);                                             \
    *((T*)(D)) = n;                                                            \
    (D) += sizeof(T);                                                          \
  } while (0)

MSH_PLY_PRIVATE MSH_PLY_INLINE void
msh_ply__ascii_to_value(char** dst, char* src, const int32_t type)
{
  switch (type)
  {
    case MSH_PLY_INT8:
      MSH_PLY__CONVERT_AND_ASSIGN(*dst, src, int8_t, atoi);
      break;
    case MSH_PLY_INT16:
      MSH_PLY__CONVERT_AND_ASSIGN(*dst, src, int16_t, atoi);
      break;
    case MSH_PLY_INT32:
      MSH_PLY__CONVERT_AND_ASSIGN(*dst, src, int32_t, atoi);
      break;
    case MSH_PLY_UINT8:
      MSH_PLY__CONVERT_AND_ASSIGN(*dst, src, uint8_t, atoi);
      break;
    case MSH_PLY_UINT16:
      MSH_PLY__CONVERT_AND_ASSIGN(*dst, src, uint16_t, atoi);
      break;
    case MSH_PLY_UINT32:
      MSH_PLY__CONVERT_AND_ASSIGN(*dst, src, uint32_t, atoi);
      break;
    case MSH_PLY_FLOAT:
      MSH_PLY__CONVERT_AND_ASSIGN(*dst, src, float, atof);
      break;
    case MSH_PLY_DOUBLE:
      MSH_PLY__CONVERT_AND_ASSIGN(*dst, src, double, atof);
      break;
  }
}

MSH_PLY_PRIVATE int32_t
msh_ply__get_element_data_ascii(msh_ply_t* pf,
                                const msh_ply_element_t* el,
                                void** storage)
{
  int32_t err_code       = MSH_PLY_NO_ERR;
  int32_t num_properties = (int32_t)msh_ply_array_len(el->properties);

  fseek(pf->_fp, el->file_anchor, SEEK_SET);
  char* dest = (char*)*storage;
  for (size_t i = 0; i < el->count; ++i)
  {
    char line[MSH_PLY_MAX_STR_LEN];
    fgets(line, MSH_PLY_MAX_STR_LEN, pf->_fp);
    char* cp_up  = &line[0];
    char* cp_low = &line[0];

    for (int32_t j = 0; j < num_properties; ++j)
    {
      msh_ply_property_t* pr = &el->properties[j];
      while (*cp_up != ' ' && *cp_up != '\n') { cp_up++; }
      char tmp_cp_up = *cp_up;
      *cp_up         = 0;                     // fake string end;
      if (pr->list_type == MSH_PLY_INVALID)   // regular property
      {
        msh_ply__ascii_to_value(&dest, cp_low, pr->type);
      }
      else   // list property
      {
        msh_ply__ascii_to_value(&dest, cp_low, pr->list_type);
        if (!pr->list_count)
        {
          pr->list_count = msh_ply__get_data_as_int(dest - pr->list_byte_size,
                                                    pr->list_type,
                                                    0);
        }
        *cp_up = tmp_cp_up;
        cp_low = cp_up;
        cp_up++;
        for (int32_t k = 0; k < pr->list_count; ++k)
        {
          while (*cp_up != ' ' && *cp_up != '\n') { cp_up++; }
          tmp_cp_up = *cp_up;
          *cp_up    = 0;   // fake string end;
          msh_ply__ascii_to_value(&dest, cp_low, pr->type);
          *cp_up = tmp_cp_up;
          cp_low = cp_up;
          cp_up++;
        }
      }

      // fixup pointers
      *cp_up = tmp_cp_up;
      if (*cp_up != '\n')
      {
        cp_up++;
        cp_low = cp_up;
      }
    }
  }

  return err_code;
}
#undef MSH_PLY_CONVERT_AND_ASSIGN

MSH_PLY_PRIVATE int32_t
msh_ply__get_element_data_binary(msh_ply_t* pf,
                                 const msh_ply_element_t* el,
                                 void** storage,
                                 size_t storage_size)
{
  int32_t err_code = MSH_PLY_NO_ERR;
  fseek(pf->_fp, el->file_anchor, SEEK_SET);
  if (fread(*storage, storage_size, 1, pf->_fp) != 1)
  {
    return MSH_PLY_BINARY_PARSE_ERR;
  }
  return err_code;
}

MSH_PLY_PRIVATE int32_t
msh_ply__get_element_data(msh_ply_t* pf,
                          const msh_ply_element_t* el,
                          void** storage,
                          size_t storage_size)
{
  int32_t err_code = MSH_PLY_NO_ERR;
  if (pf->format == MSH_PLY_ASCII)
  {
    err_code = msh_ply__get_element_data_ascii(pf, el, storage);
  }
  else
  {
    err_code = msh_ply__get_element_data_binary(pf, el, storage, storage_size);
  }

  return err_code;
}

MSH_PLY_PRIVATE int32_t
msh_ply__get_properties_byte_size(msh_ply_element_t* el,
                                  const char** properties_names,
                                  int32_t num_properties,
                                  msh_ply_type_id_t type,
                                  msh_ply_type_id_t list_type,
                                  size_t* data_size,
                                  size_t* list_size)
{
  int32_t n_found             = 0;
  size_t total_data_byte_size = 0;
  size_t total_list_byte_size = 0;
  int32_t byte_size           = msh_ply__type_to_byte_size(type);
  int32_t list_byte_size      = msh_ply__type_to_byte_size(list_type);
  for (int32_t i = 0; i < num_properties; ++i)
  {
    for (size_t j = 0; j < msh_ply_array_len(el->properties); ++j)
    {
      msh_ply_property_t* pr = &el->properties[j];
      if (!strcmp(pr->name, properties_names[i]))
      {
        n_found++;
        total_data_byte_size += pr->total_count * byte_size;
        total_list_byte_size += list_byte_size;
      }
    }
  }
  if (n_found != num_properties) return MSH_PLY_PROPERTY_NOT_FOUND_ERR;

  total_list_byte_size *= el->count;

  *data_size = total_data_byte_size;
  *list_size = total_list_byte_size;

  return MSH_PLY_NO_ERR;
}

MSH_PLY_PRIVATE void
msh_ply__data_assign_cast(void* dst,
                          void* src,
                          int32_t type_dst,
                          int32_t type_src,
                          int32_t count)
{
  static double data = 0;
  for (int32_t c = 0; c < count; c++)
  {
    switch (type_src)
    {
      // intermediate store
      case MSH_PLY_UINT8:
        data = (double)(*((uint8_t*)(src) + c));
        break;
      case MSH_PLY_UINT16:
        data = (double)(*((uint16_t*)(src) + c));
        break;
      case MSH_PLY_UINT32:
        data = (double)(*((uint32_t*)(src) + c));
        break;
      case MSH_PLY_INT8:
        data = (double)(*((int8_t*)(src) + c));
        break;
      case MSH_PLY_INT16:
        data = (double)(*((int16_t*)(src) + c));
        break;
      case MSH_PLY_INT32:
        data = (double)(*((int32_t*)(src) + c));
        break;
      case MSH_PLY_FLOAT:
        data = (double)(*((float*)(src) + c));
        break;
      case MSH_PLY_DOUBLE:
        data = (double)(*((double*)(src) + c));
        break;
      default:
        data = (double)(*((uint8_t*)(src) + c));
        break;
    }

    // write
    switch (type_dst)
    {
      case MSH_PLY_UINT8:
        *((uint8_t*)(dst) + c) = (uint8_t)data;
        break;
      case MSH_PLY_UINT16:
        *((uint16_t*)(dst) + c) = (uint16_t)data;
        break;
      case MSH_PLY_UINT32:
        *((uint32_t*)(dst) + c) = (uint32_t)data;
        break;
      case MSH_PLY_INT8:
        *((int8_t*)(dst) + c) = (int8_t)data;
        break;
      case MSH_PLY_INT16:
        *((int16_t*)(dst) + c) = (int16_t)data;
        break;
      case MSH_PLY_INT32:
        *((int32_t*)(dst) + c) = (int32_t)data;
        break;
      case MSH_PLY_FLOAT:
        *((float*)(dst) + c) = (float)data;
        break;
      case MSH_PLY_DOUBLE:
        *((double*)(dst) + c) = (double)data;
        break;
      default:
        *((uint8_t*)(dst) + c) = (uint8_t)data;
        break;
    }
  }
}

// TODO(maciej): This is gigantic function, I should break it down
MSH_PLY_PRIVATE int32_t
msh_ply__get_property_from_element(msh_ply_t* pf,
                                   const char* element_name,
                                   const char** property_names,
                                   int32_t num_requested_properties,
                                   msh_ply_type_id_t requested_type,
                                   msh_ply_type_id_t requested_list_type,
                                   void** data,
                                   void** list_data,
                                   int32_t* data_count)
{
  // TODO(maciej): Errors!
  int8_t swap_endianness =
    pf->format != MSH_PLY_ASCII ? (pf->_system_format != pf->format) : 0;
  int32_t num_properties = -1;
  msh_ply_element_t* el  = msh_ply_find_element(pf, element_name);

  if (!el) { return MSH_PLY_ELEMENT_NOT_FOUND_ERR; }
  else
  {
    num_properties = (int32_t)msh_ply_array_len(el->properties);
    if (el->data == NULL)
    {
      // Check if data layouts agree - if so, we can just copy and return
      int8_t can_simply_copy = 1;
      if (swap_endianness) { can_simply_copy = 0; }
      if (num_requested_properties != num_properties) { can_simply_copy = 0; }
      if (can_simply_copy)
      {
        for (int32_t i = 0; i < num_properties; ++i)
        {
          msh_ply_property_t* pr = &el->properties[i];
          const char* a          = pr->name;
          const char* b          = property_names[i];
          if (strcmp(a, b)) { can_simply_copy = 0; }
          if (pr->type != requested_type) { can_simply_copy = 0; }
          if (pr->list_type != MSH_PLY_INVALID) { can_simply_copy = 0; }
        }
      }

      if (can_simply_copy)
      {
        msh_ply__get_element_size(el, &el->data_size);
        *data_count = el->count;
        *data       = MSH_PLY_MALLOC(el->data_size);
        msh_ply__get_element_data(pf, el, &*data, el->data_size);
        return MSH_PLY_NO_ERR;
      }

      // If we can't simply copy the data, we will copy everything from the file and parse that
      // NOTE(maciej): Maybe this is a source of slowdown - possibly a huge read here
      msh_ply__get_element_size(el, &el->data_size);
      el->data = MSH_PLY_MALLOC(el->data_size);
      msh_ply__get_element_data(pf, el, &el->data, el->data_size);
    }

    // Initialize output
    *data_count                 = el->count;
    uint8_t* dst_data           = NULL;
    uint8_t* dst_list           = NULL;
    size_t data_byte_size       = 0;
    size_t list_byte_size       = 0;
    int32_t requested_byte_size = msh_ply__type_to_byte_size(requested_type);
    int32_t requested_list_byte_size =
      msh_ply__type_to_byte_size(requested_list_type);
    msh_ply__get_properties_byte_size(el,
                                      property_names,
                                      num_requested_properties,
                                      requested_type,
                                      requested_list_type,
                                      &data_byte_size,
                                      &list_byte_size);
    if (data != NULL)
    {
      *data    = MSH_PLY_MALLOC(data_byte_size);
      dst_data = (uint8_t*)*data;
    }
    else
    {
      return MSH_PLY_NULL_DATA_PTR_ERR;
    }
    if (list_data != NULL)
    {
      *list_data = MSH_PLY_MALLOC(list_byte_size);
      dst_list   = (uint8_t*)*list_data;
    }
    uint8_t* src = (uint8_t*)el->data;

    // Check if need to cast
    int32_t need_cast                                       = 0;
    int8_t requested_group_size[MSH_PLY_MAX_REQ_PROPERTIES] = {0};

    for (int32_t i = 0; i < num_properties; ++i)
    {
      msh_ply_property_t* pr = &el->properties[i];

      for (int32_t j = 0; j < num_requested_properties; ++j)
      {
        if (!strcmp(pr->name, property_names[j]))
        {
          if (pr->type != requested_type) need_cast = 1;
          if (pr->list_type != MSH_PLY_INVALID &&
              pr->list_type != requested_list_type)
            need_cast = 1;
        }
      }
    }

    int32_t num_groups = 0;
    for (int32_t i = 0; i < num_properties; ++i)
    {
      msh_ply_property_t* pr = &el->properties[i];
      int32_t k              = i;

      for (int32_t j = 0; j < num_requested_properties; ++j)
      {
        if (!strcmp(pr->name, property_names[j])) { num_groups++; }

        while (!strcmp(pr->name, property_names[j]))
        {
          requested_group_size[k] += 1;
          j++;
          i++;
          if (i >= num_properties || j >= num_requested_properties) { break; }
          pr = &el->properties[i];
        }
      }
    }

    // TODO(maciej): Make into a struct?
    int32_t precalc_dst_row_size      = 0;
    int32_t precalc_dst_list_row_size = 0;
    int32_t precalc_src_row_size      = 0;
    int32_t precalc_dst_stride        = 0;

    int32_t can_precalculate = msh_ply__can_precalculate_sizes(el);
    if (can_precalculate)
    {
      // Determine data row sizes
      for (int32_t j = 0; j < num_properties; ++j)
      {
        msh_ply_property_t* pr = &el->properties[j];

        if (requested_group_size[j])
        {
          if (pr->list_type == MSH_PLY_INVALID)
          {
            precalc_dst_stride = requested_group_size[j] * requested_byte_size;
          }
          else
          {
            precalc_dst_stride = pr->list_count * requested_byte_size;
          }
          precalc_dst_list_row_size += requested_list_byte_size;
          precalc_dst_row_size += precalc_dst_stride;
          pr->offset      = precalc_src_row_size + pr->list_byte_size;
          pr->list_offset = precalc_src_row_size;
        }
        precalc_src_row_size +=
          pr->list_byte_size + pr->list_count * pr->byte_size;
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

    ply_property_read_helper_t requested_properties[MSH_PLY_MAX_REQ_PROPERTIES];
    for (int32_t i = 0; i < MSH_PLY_MAX_REQ_PROPERTIES; ++i)
    {
      ply_property_read_helper_t prh = {0, 0, 0, 0, 0};
      requested_properties[i]        = prh;
    }

    int32_t pr_count = 0;
    for (int32_t i = 0; i < num_properties; ++i)
    {
      msh_ply_property_t* pr = &el->properties[i];
      if (requested_group_size[i])
      {
        requested_properties[pr_count].offset      = pr->offset;
        requested_properties[pr_count].list_count  = pr->list_count;
        requested_properties[pr_count].list_offset = pr->list_offset;
        requested_properties[pr_count].type        = pr->type;
        requested_properties[pr_count].list_type   = pr->list_type;
        if (pr->list_type == MSH_PLY_INVALID)
        {
          requested_properties[pr_count].list_count = requested_group_size[i];
        }
        pr_count += requested_group_size[i];
      }
    }

    // Start copying
    for (size_t i = 0; i < el->count; ++i)
    {
      int32_t dst_row_size      = 0;
      int32_t dst_list_row_size = 0;
      int32_t dst_offset        = 0;
      int32_t dst_list_offset   = 0;
      int32_t dst_stride        = 0;
      int32_t src_row_size      = 0;

      // Calculate the required data to read
      if (can_precalculate)
      {
        dst_row_size      = precalc_dst_row_size;
        dst_list_row_size = precalc_dst_list_row_size;
        dst_stride        = precalc_dst_stride;
        src_row_size      = precalc_src_row_size;
      }
      else
      {
        pr_count = 0;
        for (int32_t j = 0; j < num_properties; ++j)
        {
          msh_ply_property_t* pr = &el->properties[j];
          pr->list_count         = 1;

          if (pr->list_type != MSH_PLY_INVALID)
          {
            pr->list_count = msh_ply__get_data_as_int(src + src_row_size,
                                                      pr->list_type,
                                                      swap_endianness);
          }
          dst_stride = pr->list_count * requested_byte_size;

          if (requested_group_size[j])
          {
            dst_list_row_size += requested_list_byte_size;
            dst_row_size += pr->list_count * requested_byte_size;
            pr->offset      = src_row_size + pr->list_byte_size;
            pr->list_offset = src_row_size;

            requested_properties[pr_count].offset      = pr->offset;
            requested_properties[pr_count].list_count  = pr->list_count;
            requested_properties[pr_count].list_offset = pr->list_offset;
            pr_count++;
          }
          src_row_size += pr->list_byte_size + pr->list_count * pr->byte_size;
        }
      }

      // TODO(maciej): Maybe make casting a separate function
      if (need_cast)
      {
        for (int32_t j = 0; j < num_groups; ++j)
        {
          ply_property_read_helper_t* prh = &requested_properties[j];
          if (dst_data)
          {
            void* dst_ptr = (dst_data + dst_offset);
            void* src_ptr = (src + prh->offset);
            msh_ply__data_assign_cast(dst_ptr,
                                      src_ptr,
                                      requested_type,
                                      prh->type,
                                      prh->list_count);
            dst_offset += dst_stride;
            if (swap_endianness)
            {
              msh_ply__swap_bytes((uint8_t*)dst_ptr,
                                  requested_byte_size,
                                  prh->list_count);
            }
          }

          if (dst_list)
          {
            void* dst_ptr = (dst_list + dst_list_offset);
            void* src_ptr = (src + prh->list_offset);
            msh_ply__data_assign_cast(dst_ptr,
                                      src_ptr,
                                      requested_list_type,
                                      prh->list_type,
                                      1);
            dst_list_offset += requested_list_byte_size;
            if (swap_endianness)
            {
              msh_ply__swap_bytes((uint8_t*)dst_ptr,
                                  requested_list_byte_size,
                                  1);
            }
          }
        }
      }
      else
      {
        for (int32_t j = 0; j < num_groups; ++j)
        {
          ply_property_read_helper_t* prh = &requested_properties[j];
          if (dst_data)
          {
            void* dst_ptr = (dst_data + dst_offset);
            void* src_ptr = (src + prh->offset);
            msh_ply__data_assign(dst_ptr, src_ptr, prh->type, prh->list_count);
            dst_offset += dst_stride;
            if (swap_endianness)
            {
              msh_ply__swap_bytes((uint8_t*)dst_ptr,
                                  requested_byte_size,
                                  prh->list_count);
            }
          }

          if (dst_list)
          {
            void* dst_ptr = (dst_list + dst_list_offset);
            void* src_ptr = (src + prh->list_offset);
            msh_ply__data_assign(dst_ptr, src_ptr, prh->list_type, 1);
            dst_list_offset += requested_list_byte_size;
            if (swap_endianness)
            {
              msh_ply__swap_bytes((uint8_t*)dst_ptr,
                                  requested_list_byte_size,
                                  1);
            }
          }
        }
      }

      src += src_row_size;
      if (dst_data) dst_data += dst_row_size;
      if (dst_list) dst_list += dst_list_row_size;
    }
  }
  free(el->data);
  el->data=NULL;
  return MSH_PLY_NO_ERR;
}

MSH_PLY_DEF int32_t
msh_ply_get_property_from_element(msh_ply_t* pf, msh_ply_desc_t* desc)
{
  assert(pf);
  assert(desc);
  return msh_ply__get_property_from_element(pf,
                                            desc->element_name,
                                            desc->property_names,
                                            desc->num_properties,
                                            desc->data_type,
                                            desc->list_type,
                                            (void**)desc->data,
                                            (void**)desc->list_data,
                                            desc->data_count);
}

MSH_PLY_DEF int32_t
msh_ply_read(msh_ply_t* pf)
{
  int32_t error = MSH_PLY_NO_ERR;
  if (!pf->_fp) { return MSH_PLY_FILE_NOT_OPEN_ERR; }
  if (msh_ply_array_len(pf->descriptors) == 0) { return MSH_PLY_NO_REQUESTS; }

  if (!pf->_parsed) { error = msh_ply_parse_header(pf); }
  if (error) { return error; }

  error = msh_ply_parse_contents(pf);
  if (error) { return error; }

  for (size_t i = 0; i < msh_ply_array_len(pf->descriptors); ++i)
  {
    msh_ply_desc_t* desc = pf->descriptors[i];
    error                = msh_ply_get_property_from_element(pf, desc);
    if (error) { return error; }
  }
  return error;
}
#endif /* MSH_PLY_ENCODER_ONLY */

// ENCODER

#ifndef MSH_PLY_DECODER_ONLY

MSH_PLY_PRIVATE int32_t
msh_ply__add_element(msh_ply_t* pf,
                     const char* element_name,
                     const int32_t element_count)
{
  msh_ply_element_t el = msh_ply__element_zero_init();
  strncpy(&el.name[0], element_name, 63);
  el.name[63]   = '\0';
  el.count      = element_count;
  el.properties = NULL;
  msh_ply_array_push(pf->elements, el);
  return MSH_PLY_NO_ERR;
}

MSH_PLY_PRIVATE void
msh_ply__fprint_data_at_offset(const msh_ply_t* pf,
                               const void* data,
                               const int32_t offset,
                               const int32_t type)
{
  switch (type)
  {
    case MSH_PLY_UINT8:
      fprintf(pf->_fp, "%d ", *(uint8_t*)((uint8_t*)data + offset));
      break;
    case MSH_PLY_UINT16:
      fprintf(pf->_fp, "%d ", *(uint16_t*)((uint8_t*)data + offset));
      break;
    case MSH_PLY_UINT32:
      fprintf(pf->_fp, "%d ", *(uint32_t*)((uint8_t*)data + offset));
      break;
    case MSH_PLY_INT8:
      fprintf(pf->_fp, "%d ", *(int8_t*)((uint8_t*)data + offset));
      break;
    case MSH_PLY_INT16:
      fprintf(pf->_fp, "%d ", *(int16_t*)((uint8_t*)data + offset));
      break;
    case MSH_PLY_INT32:
      fprintf(pf->_fp, "%d ", *(int32_t*)((uint8_t*)data + offset));
      break;
    case MSH_PLY_FLOAT:
      fprintf(pf->_fp, "%f ", *(float*)((uint8_t*)data + offset));
      break;
    case MSH_PLY_DOUBLE:
      fprintf(pf->_fp, "%f ", *(double*)((uint8_t*)data + offset));
      break;
  }
}

MSH_PLY_PRIVATE int32_t
msh_ply__add_property_to_element(msh_ply_t* pf,
                                 const char* element_name,
                                 const char** property_names,
                                 int32_t num_properties,
                                 const msh_ply_type_id_t data_type,
                                 const msh_ply_type_id_t list_type,
                                 void** data,
                                 void** list_data,
                                 int32_t element_count,
                                 int32_t size_hint)
{
  // Check if list type is integral type
  if (list_type == MSH_PLY_FLOAT || list_type == MSH_PLY_DOUBLE)
  {
    return MSH_PLY_INVALID_LIST_TYPE_ERR;
  }
  int8_t swap_endianness =
    pf->format != MSH_PLY_ASCII ? (pf->_system_format != pf->format) : 0;

  // Find / Create element
  msh_ply_element_t* el = msh_ply_find_element(pf, element_name);
  if (!el)
  {
    msh_ply__add_element(pf, element_name, element_count);
    el = msh_ply_array_back(pf->elements);
  }
  if (el)
  {
    if (el->count != element_count)
    {
      return MSH_PLY_CONFLICTING_NUMBER_OF_ELEMENTS_ERR;
    }

    int32_t init_offset =
      0;   // Helper variable for storing initial list offsets.
    for (int32_t i = 0; i < num_properties; ++i)
    {
      msh_ply_property_t pr;
      strncpy(&pr.name[0], property_names[i], 31);
      pr.name[31] = '\0';

      if (data == NULL) { return MSH_PLY_NULL_DATA_PTR_ERR; }

      pr.type           = data_type;
      pr.list_type      = list_type;
      pr.byte_size      = msh_ply__type_to_byte_size(pr.type);
      pr.list_byte_size = msh_ply__type_to_byte_size(pr.list_type);
      pr.list_count     = (list_type == MSH_PLY_INVALID) ? 1 : size_hint;
      pr.data           = *data;
      pr.list_data      = (list_data != NULL) ? *list_data : NULL;
      pr.offset         = pr.byte_size * i;
      pr.stride         = pr.byte_size * num_properties;
      pr.list_offset    = pr.list_byte_size * i;
      pr.list_stride    = pr.list_byte_size * num_properties;
      pr.total_byte_size =
        (pr.list_byte_size + pr.list_count * pr.byte_size) * el->count;

      if (pr.list_count == 0)   // No hint was present
      {
        pr.total_byte_size = 0;
        for (int32_t j = 0; j < element_count; ++j)
        {
          // NOTE(maciej): list type needs to be dereferenced to correct type
          int32_t offset = (pr.list_offset + j * pr.list_stride);
          int32_t list_count =
            msh_ply__get_data_as_int((uint8_t*)(*list_data) + offset,
                                     list_type,
                                     swap_endianness);
          pr.total_byte_size += pr.list_byte_size + list_count * pr.byte_size;
          if (
            j ==
            0)   // We care only about initial offset, so first element of list counts
          {
            pr.offset = init_offset;
            init_offset += list_count * pr.byte_size;
          }
        }
      }
      else
      {
        pr.stride *= pr.list_count;
      }
      msh_ply_array_push(el->properties, pr);
    }
  }
  else
  {
    return MSH_PLY_ELEMENT_NOT_FOUND_ERR;
  }

  return MSH_PLY_NO_ERR;
}

MSH_PLY_DEF int32_t
msh_ply_add_property_to_element(msh_ply_t* pf, const msh_ply_desc_t* desc)
{
  if (!pf) { return MSH_PLY_FILE_NOT_OPEN_ERR; }
  int32_t desc_err = msh_ply__validate_descriptor(desc);
  if (desc_err) { return desc_err; }
  return msh_ply__add_property_to_element(pf,
                                          desc->element_name,
                                          desc->property_names,
                                          desc->num_properties,
                                          desc->data_type,
                                          desc->list_type,
                                          (void**)desc->data,
                                          (void**)desc->list_data,
                                          *desc->data_count,
                                          desc->list_size_hint);
}

MSH_PLY_PRIVATE int32_t
msh_ply__calculate_list_property_stride(const msh_ply_property_t* pr,
                                        msh_ply_array(msh_ply_property_t)
                                          el_properties,
                                        int8_t swap_endianness)
{
  int32_t stride                          = 0;
  int32_t offsets[MSH_PLY_MAX_PROPERTIES] = {0};
  for (size_t l = 0; l < msh_ply_array_len(el_properties); ++l)
  {
    msh_ply_property_t* qr = &el_properties[l];
    offsets[l]             = qr->list_offset;
  }

  for (size_t l = 0; l < msh_ply_array_len(el_properties); ++l)
  {
    msh_ply_property_t* qr = &el_properties[l];

    if (pr->data == qr->data)
    {
      int32_t list_count =
        msh_ply__get_data_as_int((uint8_t*)qr->list_data + offsets[l],
                                 qr->list_type,
                                 swap_endianness);
      stride += list_count * qr->byte_size;
      offsets[l] += qr->list_stride;
    }
  }
  return ((stride > 0) ? stride : 0);
}

MSH_PLY_PRIVATE int32_t
msh_ply__write_header(const msh_ply_t* pf)
{
  if (!pf->_fp) { return MSH_PLY_INVALID_FILE_ERR; }
  else
  {
    char* format_string = NULL;
    switch (pf->format)
    {
      case MSH_PLY_ASCII:
      {
        format_string = (char*)"ascii";
        break;
      }
      case MSH_PLY_LITTLE_ENDIAN:
      {
        format_string = (char*)"binary_little_endian";
        break;
      }
      case MSH_PLY_BIG_ENDIAN:
      {
        format_string = (char*)"binary_big_endian";
        break;
      }
      default:
      {
        return MSH_PLY_INVALID_FORMAT_ERR;
      }
    }

    fprintf(pf->_fp,
            "ply\nformat %s %2.1f\n",
            format_string,
            (float)pf->format_version);
    for (size_t i = 0; i < msh_ply_array_len(pf->elements); ++i)
    {
      msh_ply_element_t* el = &pf->elements[i];
      fprintf(pf->_fp, "element %s %d\n", el->name, (int32_t)el->count);
      for (size_t j = 0; j < msh_ply_array_len(el->properties); j++)
      {
        msh_ply_property_t* pr = &el->properties[j];
        char* pr_type_str      = NULL;
        msh_ply__property_type_to_string(pr->type, &pr_type_str);

        if (pr->list_type == MSH_PLY_INVALID)
        {
          fprintf(pf->_fp, "property %s %s\n", pr_type_str, pr->name);
        }
        else
        {
          char* pr_list_type_str = NULL;
          msh_ply__property_type_to_string(pr->list_type, &pr_list_type_str);
          fprintf(pf->_fp,
                  "property list %s %s %s\n",
                  pr_list_type_str,
                  pr_type_str,
                  pr->name);
        }
      }
    }
    fprintf(pf->_fp, "end_header\n");
  }
  return MSH_PLY_NO_ERR;
}

MSH_PLY_PRIVATE int32_t
msh_ply__write_data_ascii(const msh_ply_t* pf)
{
  for (size_t i = 0; i < msh_ply_array_len(pf->elements); ++i)
  {
    msh_ply_element_t* el = &pf->elements[i];

    for (size_t j = 0; j < el->count; ++j)
    {
      for (size_t k = 0; k < msh_ply_array_len(el->properties); ++k)
      {
        msh_ply_property_t* pr = &el->properties[k];
        if (pr->list_type == MSH_PLY_INVALID)
        {
          msh_ply__fprint_data_at_offset(pf, pr->data, pr->offset, pr->type);
          pr->offset += pr->stride;
        }
        else
        {
          // figure out stride.
          int32_t list_count = 0;
          if (pr->list_count != 0)
          {
            list_count = pr->list_count;
            msh_ply__fprint_data_at_offset(pf, &list_count, 0, pr->list_type);
          }
          else
          {
            pr->stride =
              msh_ply__calculate_list_property_stride(pr, el->properties, 0);
            list_count = msh_ply__get_data_as_int((uint8_t*)pr->list_data +
                                                    pr->list_offset,
                                                  pr->list_type,
                                                  0);
            msh_ply__fprint_data_at_offset(pf,
                                           pr->list_data,
                                           pr->list_offset,
                                           pr->list_type);
          }
          pr->list_offset += pr->list_stride;

          for (int32_t l = 0; l < list_count; ++l)
          {
            int32_t cur_offset = pr->offset + l * pr->byte_size;
            msh_ply__fprint_data_at_offset(pf, pr->data, cur_offset, pr->type);
          }
          pr->offset += pr->stride;
        }
      }
      fprintf(pf->_fp, "\n");
    }
  }
  return MSH_PLY_NO_ERR;
}

// TODO(maciej): Test if splitting into completly separate functions helps
// TODO(maciej): Check if writing data to file directly ends up being faster...
int32_t
msh_ply__write_data_binary(const msh_ply_t* pf)
{
  int8_t swap_endianness = (pf->_system_format != pf->format);

  for (size_t i = 0; i < msh_ply_array_len(pf->elements); ++i)
  {
    msh_ply_element_t* el = &pf->elements[i];
    size_t buffer_size    = 0;
    for (size_t j = 0; j < msh_ply_array_len(el->properties); ++j)
    {
      buffer_size += el->properties[j].total_byte_size;
    }

    uint8_t* dst       = (uint8_t*)MSH_PLY_MALLOC(buffer_size);
    int64_t dst_offset = 0;

    for (size_t j = 0; j < el->count; ++j)
    {
      for (size_t k = 0; k < msh_ply_array_len(el->properties); ++k)
      {
        msh_ply_property_t* pr = &el->properties[k];
        if (pr->list_type == MSH_PLY_INVALID)
        {
          msh_ply__data_assign(dst + dst_offset,
                               (uint8_t*)pr->data + pr->offset,
                               pr->type,
                               1);
          pr->offset += pr->stride;
          dst_offset += pr->byte_size;
        }
        else
        {
          // figure out stride
          if (!pr->list_count)
          {
            pr->stride =
              msh_ply__calculate_list_property_stride(pr,
                                                      el->properties,
                                                      swap_endianness);
            pr->list_count = msh_ply__get_data_as_int((uint8_t*)pr->list_data +
                                                        pr->list_offset,
                                                      pr->list_type,
                                                      swap_endianness);

            msh_ply__data_assign(dst + dst_offset,
                                 (uint8_t*)pr->list_data + pr->list_offset,
                                 pr->list_type,
                                 1);
            pr->list_offset += pr->list_stride;
            dst_offset += pr->list_byte_size;
          }
          else
          {
            switch (pr->list_type)
            {
              case MSH_PLY_UINT8:
              {
                uint8_t* dst_ptr = (uint8_t*)dst + dst_offset;
                *dst_ptr         = (uint8_t)pr->list_count;
                break;
              }
              case MSH_PLY_UINT16:
              {
                uint16_t* dst_ptr = (uint16_t*)dst + dst_offset;
                *dst_ptr          = (uint16_t)pr->list_count;
                break;
              }
              case MSH_PLY_UINT32:
              {
                uint32_t* dst_ptr = (uint32_t*)dst + dst_offset;
                *dst_ptr          = (uint32_t)pr->list_count;
                break;
              }
              case MSH_PLY_INT8:
              {
                int8_t* dst_ptr = (int8_t*)dst + dst_offset;
                *dst_ptr        = (int8_t)pr->list_count;
                break;
              }
              case MSH_PLY_INT16:
              {
                int16_t* dst_ptr = (int16_t*)dst + dst_offset;
                *dst_ptr         = (int16_t)pr->list_count;
                break;
              }
              case MSH_PLY_INT32:
              {
                int32_t* dst_ptr = (int32_t*)dst + dst_offset;
                *dst_ptr         = (int32_t)pr->list_count;
                break;
              }
              default:
              {
                uint8_t* dst_ptr = (uint8_t*)dst + dst_offset;
                *dst_ptr         = (uint8_t)pr->list_count;
                break;
              }
            }
            dst_offset += pr->list_byte_size;
          }

          msh_ply__data_assign(dst + dst_offset,
                               (uint8_t*)pr->data + pr->offset,
                               pr->type,
                               pr->list_count);
          pr->offset += pr->stride;
          dst_offset += pr->byte_size * pr->list_count;
        }
      }
    }

    // NOTE(maciej): Can someone explain why below is faster than a single call to fwrite..?
    size_t block_size       = 32 * 65536;
    size_t remaining_buffer = buffer_size;
    uint8_t* mem            = dst;
    while (true)
    {
      if (remaining_buffer < block_size) break;
      fwrite(mem, block_size, 1, pf->_fp);
      mem += block_size;
      remaining_buffer -= block_size;
    }
    fwrite(mem, remaining_buffer, 1, pf->_fp);

    MSH_PLY_FREE(dst);
  }
  return MSH_PLY_NO_ERR;
}

MSH_PLY_PRIVATE int32_t
msh_ply__write_data(const msh_ply_t* pf)
{
  int32_t error = MSH_PLY_NO_ERR;
  if (pf->format == MSH_PLY_ASCII) { error = msh_ply__write_data_ascii(pf); }
  else
  {
    error = msh_ply__write_data_binary(pf);
  }
  return error;
}

MSH_PLY_DEF int32_t
msh_ply_write(msh_ply_t* pf)
{
  int32_t error = MSH_PLY_NO_ERR;

  if (msh_ply_array_len(pf->descriptors) == 0) { return MSH_PLY_NO_REQUESTS; }

  if (msh_ply_array_len(pf->elements) == 0)
  {
    for (size_t i = 0; i < msh_ply_array_len(pf->descriptors); ++i)
    {
      msh_ply_desc_t* desc = pf->descriptors[i];
      msh_ply_add_property_to_element(pf, desc);
    }
  }

  error = msh_ply__write_header(pf);
  if (error) { return error; }

  error = msh_ply__write_data(pf);

  return error;
}
#endif /* MSH_PLY_DECODER_ONLY */

MSH_PLY_DEF void
msh_ply_print_header(msh_ply_t* pf)
{
  // Find property & element with longest name
  size_t max_pr_length = 0;

  for (size_t i = 0; i < msh_ply_array_len(pf->elements); ++i)
  {
    for (size_t j = 0; j < msh_ply_array_len(pf->elements[i].properties); ++j)
    {
      max_pr_length =
        MSH_PLY_MAX(strlen(pf->elements[i].properties[j].name), max_pr_length);
    }
  }

  char* type_str      = NULL;
  char* list_type_str = NULL;
  char spaces[MSH_PLY_MAX_STR_LEN];
  for (int32_t i = 0; i < MSH_PLY_MAX_STR_LEN; ++i) { spaces[i] = ' '; }
  spaces[MSH_PLY_MAX_STR_LEN - 1] = 0;

  char* format_str = NULL;
  if (pf->format == MSH_PLY_ASCII) { format_str = (char*)"Ascii"; }
  if (pf->format == MSH_PLY_LITTLE_ENDIAN)
  {
    format_str = (char*)"Binary Little Endian";
  }
  if (pf->format == MSH_PLY_BIG_ENDIAN)
  {
    format_str = (char*)"Binary Big Endian";
  }
  printf("PLY: %s %d\n", format_str, pf->format_version);
  for (size_t i = 0; i < msh_ply_array_len(pf->elements); ++i)
  {
    printf("   '%s' count: %d\n",
           pf->elements[i].name,
           (int32_t)pf->elements[i].count);
    for (size_t j = 0; j < msh_ply_array_len(pf->elements[i].properties); ++j)
    {
      size_t n_spaces =
        max_pr_length - strlen(pf->elements[i].properties[j].name);
      spaces[n_spaces] = 0;
      msh_ply__property_type_to_string(pf->elements[i].properties[j].type,
                                       &type_str);
      if (pf->elements[i].properties[j].list_type == MSH_PLY_INVALID)
      {
        printf("      '%s'%s | %7s(%d bytes)\n",
               pf->elements[i].properties[j].name,
               spaces,
               type_str,
               pf->elements[i].properties[j].byte_size);
      }
      else
      {
        msh_ply__property_type_to_string(
          pf->elements[i].properties[j].list_type,
          &list_type_str);
        printf("      '%s'%s | %7s(%d bytes) %7s(%d bytes)\n",
               pf->elements[i].properties[j].name,
               spaces,
               list_type_str,
               pf->elements[i].properties[j].list_byte_size,
               type_str,
               pf->elements[i].properties[j].byte_size);
      }
      spaces[n_spaces] = ' ';
    }
  }
}

MSH_PLY_DEF msh_ply_t*
msh_ply_open(const char* filename, const char* mode)
{
  msh_ply_t* pf = NULL;
  if (mode[0] != 'r' && mode[0] != 'w') return NULL;

  const char* mode_str;
  if (mode[0] == 'r')
  {
    mode_str = "rb";
  }   // We always wanna read file as binary for ftell.
  else
  {
    mode_str = mode;
  }
  FILE* fp = fopen(filename, mode_str);

  if (fp)
  {
    pf                 = (msh_ply_t*)MSH_PLY_MALLOC(sizeof(msh_ply_t));
    pf->valid          = 0;
    pf->format         = -1;
    pf->format_version = 0;
    pf->elements       = 0;
    pf->descriptors    = 0;
    pf->_fp            = fp;
    pf->_parsed        = 0;

    // Endianness check
    int32_t n = 1;
    if (*(char*)&n == 1) { pf->_system_format = MSH_PLY_LITTLE_ENDIAN; }
    else
    {
      pf->_system_format = MSH_PLY_BIG_ENDIAN;
    }
  }
  if (mode[0] == 'w')
  {
    pf->format_version = 1;
    pf->format         = MSH_PLY_ASCII;
    if (strlen(mode) > 1 && mode[1] == 'b') { pf->format = pf->_system_format; }
  }
  return pf;
}

MSH_PLY_DEF void
msh_ply_close(msh_ply_t* pf)
{
  if (pf->_fp)
  {
    fclose(pf->_fp);
    pf->_fp = NULL;
  }

  if (pf->elements)
  {
    for (size_t i = 0; i < msh_ply_array_len(pf->elements); ++i)
    {
      msh_ply_element_t* el = &pf->elements[i];
      if (el->properties) { msh_ply_array_free(el->properties); }
    }
    msh_ply_array_free(pf->elements);
  }
  if (pf->descriptors) msh_ply_array_free(pf->descriptors);
  MSH_PLY_FREE(pf);
}

#endif /* MSH_PLY_IMPLEMENTATION */

/*
------------------------------------------------------------------------------

This software is available under 2 licenses - you may choose the one you like.

------------------------------------------------------------------------------

ALTERNATIVE A - MIT License

Copyright (c) 2018-2020 Maciej Halber

Permission is hereby granted, free of charge, to any person obtaining a copy of 
this software and associated documentation files (the "Software"), to deal in 
the Software without restriction, including without limitation the rights to 
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies 
of the Software, and to permit persons to whom the Software is furnished to do 
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all 
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
SOFTWARE.

------------------------------------------------------------------------------

ALTERNATIVE B - Public Domain (www.unlicense.org)

This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or distribute this 
software, either in source code form or as a compiled binary, for any purpose, 
commercial or non-commercial, and by any means.

In jurisdictions that recognize copyright laws, the author or authors of this 
software dedicate any and all copyright interest in the software to the public 
domain. We make this dedication for the benefit of the public at large and to 
the detriment of our heirs and successors. We intend this dedication to be an 
overt act of relinquishment in perpetuity of all present and future rights to 
this software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN 
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION 
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

------------------------------------------------------------------------------
*/
