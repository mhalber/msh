#ifndef MSH_MESH
#define MSH_MESH

#ifndef MSH_VEC_MATH
#error "Please include msh_vec_math.h before including msh_mesh!"
#endif

typedef union msh_imesh_triangle_facet
{
  uint32_t ind[3];
  struct { uint32_t i0; uint32_t i1; uint32_t i2; };
} msh_tri_ind_t;

// TODO(maciej): Add some color stuff to std? lerping, color conversions?
typedef union msh_rgba
{
  uint8_t color[4];
  struct { uint8_t r; uint8_t g; uint8_t b; uint8_t a; };
} msh_rgba_t;

typedef struct msh_imesh
{
  msh_vec3_t* positions;
  msh_vec3_t* normals;
  msh_rgba_t* colors;
  uint32_t* indices;

  size_t n_vertices;
  size_t n_faces;
} msh_imesh_t;

typedef struct msh_imesh msh_pointcloud_t;

typedef enum msh_indexed_mesh_error_codes
{
  MSH_IMESH_NO_ERR,
  MSH_IMESH_INIT_FAILURE_ERR,
  MSH_IMESH_MISSING_EXTENSION_ERR,
  MSH_IMESH_FORMAT_NOT_SUPPORTED_ERR,
  MSH_IMESH_PLY_MISSING_VERTEX_POSITION,
  MSH_IMESH_FILE_NOT_FOUND_ERR,
  MSH_IMESH_PLY_IN_ERR,
  MSH_IMESH_PLY_OUT_ERR
} msh_imesh_error_t;

int32_t
msh_imesh_init( msh_imesh_t* mesh, size_t n_vertices, size_t n_faces )
{
  mesh->n_vertices   = n_vertices;
  mesh->n_faces      = n_faces;
  mesh->positions    = (msh_vec3_t*)malloc( n_vertices * sizeof(msh_vec3_t) );
  mesh->normals      = NULL;
  mesh->colors       = NULL;
  mesh->indices = (uint32_t*)malloc( n_faces * 3 * sizeof(uint32_t) );
  if( !mesh->positions || !mesh->indices ) { return MSH_IMESH_INIT_FAILURE_ERR; }
  return MSH_IMESH_NO_ERR;
}

void
msh_imesh_free( msh_imesh_t* mesh )
{
  assert( mesh );

  mesh->n_vertices = 0;
  mesh->n_faces    = 0;
  free( mesh->positions );
  free( mesh->normals );
  free( mesh->indices );
}

void
msh_imesh_calculate_vertex_normals( msh_imesh_t* mesh )
{
  assert( mesh );
  assert( mesh->indices ); /* NOTE(maciej): Could use a msg here.. */
  assert( mesh->positions );
  free( mesh->normals );
  mesh->normals = (msh_vec3_t*)malloc( mesh->n_vertices * sizeof(msh_vec3_t) );
  double* normals_tmp = (double*)malloc( mesh->n_vertices * 4 * sizeof(double) );

  for( size_t i = 0; i < mesh->n_vertices; ++i )
  {
    normals_tmp[ 4 * i + 0 ] = 0.0;
    normals_tmp[ 4 * i + 1 ] = 0.0;
    normals_tmp[ 4 * i + 2 ] = 0.0;
    normals_tmp[ 4 * i + 3 ] = 0.0;
  }

  for( size_t i = 0; i < mesh->n_faces; ++i )
  {
    uint32_t* ind = &mesh->indices[3*i];
    msh_vec3_t p0 = mesh->positions[ ind[0] ];
    msh_vec3_t p1 = mesh->positions[ ind[1] ];
    msh_vec3_t p2 = mesh->positions[ ind[2] ];

    msh_vec3_t v0 = msh_vec3_sub(p1, p0);
    msh_vec3_t v1 = msh_vec3_sub(p2, p0);

    msh_vec3_t n = msh_vec3_cross( v0, v1 );

    normals_tmp[ 4 * ind[0] + 0 ] = normals_tmp[ 4 * ind[0] + 0 ] + n.x;
    normals_tmp[ 4 * ind[0] + 1 ] = normals_tmp[ 4 * ind[0] + 1 ] + n.y;
    normals_tmp[ 4 * ind[0] + 2 ] = normals_tmp[ 4 * ind[0] + 2 ] + n.z;

    normals_tmp[ 4 * ind[1] + 0 ] = normals_tmp[ 4 * ind[1] + 0 ] + n.x;
    normals_tmp[ 4 * ind[1] + 1 ] = normals_tmp[ 4 * ind[1] + 1 ] + n.y;
    normals_tmp[ 4 * ind[1] + 2 ] = normals_tmp[ 4 * ind[1] + 2 ] + n.z;

    normals_tmp[ 4 * ind[2] + 0 ] = normals_tmp[ 4 * ind[2] + 0 ] + n.x;
    normals_tmp[ 4 * ind[2] + 1 ] = normals_tmp[ 4 * ind[2] + 1 ] + n.y;
    normals_tmp[ 4 * ind[2] + 2 ] = normals_tmp[ 4 * ind[2] + 2 ] + n.z;

    normals_tmp[ 4 * ind[0] + 3] += 1.0;
    normals_tmp[ 4 * ind[1] + 3] += 1.0;
    normals_tmp[ 4 * ind[2] + 3] += 1.0;
  }
  
  for( size_t i = 0; i < mesh->n_vertices; ++i )
  {
    double weight = normals_tmp[ 4 * i + 3 ];
    double x = normals_tmp[ 4 * i + 0 ] / weight;
    double y = normals_tmp[ 4 * i + 1 ] / weight;
    double z = normals_tmp[ 4 * i + 2 ] / weight;
    double inv_norm = 1.0 / sqrt(x*x + y*y + z*z);
    mesh->normals[i] = msh_vec3( x * inv_norm, y * inv_norm, z * inv_norm );
    float norm = msh_vec3_norm( mesh->normals[i] );
    if( fabs(norm - 1.0) > 0.000001 ) { printf("WRONG LENGHT!\n"); }
  }

  free( normals_tmp );
}

msh_vec3_t msh_imesh_get_centroid( const msh_imesh_t *mesh )
{
  assert( mesh );
  assert( mesh->positions );

  double cx = 0.0;
  double cy = 0.0;
  double cz = 0.0;
  for( size_t i = 0; i < mesh->n_vertices; ++i )
  {
    cx = cx + mesh->positions[i].x;
    cy = cy + mesh->positions[i].y;
    cz = cz + mesh->positions[i].z;
  }
  cx /= (double)mesh->n_vertices;
  cy /= (double)mesh->n_vertices;
  cz /= (double)mesh->n_vertices;

  return msh_vec3(cx, cy, cz);
}

void
msh_imesh_get_bbox( const msh_imesh_t* mesh, msh_vec3_t* min_pt, msh_vec3_t* max_pt )
{
  assert( mesh );
  assert( mesh->positions );

  (*min_pt) = msh_vec3( MSH_F32_MAX, MSH_F32_MAX, MSH_F32_MAX );
  (*max_pt) = msh_vec3( MSH_F32_MIN, MSH_F32_MIN, MSH_F32_MIN );
  for( size_t i = 0; i < mesh->n_vertices; ++i )
  {
    msh_vec3_t* p = &mesh->positions[i];
    if( min_pt->x > p->x ) { min_pt->x = p->x; }
    if( min_pt->y > p->y ) { min_pt->y = p->y; }
    if( min_pt->x > p->z ) { min_pt->x = p->z; }
    
    if( max_pt->x < p->x ) { max_pt->x = p->x; }
    if( max_pt->y < p->y ) { max_pt->y = p->y; }
    if( max_pt->x < p->z ) { max_pt->x = p->z; }
  }
}

msh_mat4_t 
msh_imesh_get_normalizing_transform( const msh_imesh_t* mesh )
{
  assert( mesh );
  assert( mesh->positions );

  msh_vec3_t centroid = msh_imesh_get_centroid( mesh );
  msh_vec3_t t = msh_vec3_invert( centroid );

  msh_vec3_t min_pt, max_pt, s;
  msh_imesh_get_bbox( mesh, &min_pt, &max_pt );
  real32_t sx = 1.0f / (max_pt.x - min_pt.x);
  real32_t sy = 1.0f / (max_pt.y - min_pt.y);
  real32_t sz = 1.0f / (max_pt.y - min_pt.y);
  real32_t ms = msh_max3( sx, sy, sz);
  s = msh_vec3( ms, ms, ms );

  msh_mat4_t xform = msh_mat4_identity();
  xform = msh_scale( xform, s );
  xform = msh_translate( xform, t );

  return xform;
}

void
msh_imesh_translate( msh_imesh_t* mesh, msh_vec3_t v )
{
  assert( mesh );
  assert( mesh->positions );

  for( size_t i = 0; i < mesh->n_vertices; ++i )
  {
    mesh->positions[i] = msh_vec3_add( mesh->positions[i], v );
  }
}

void
msh_imesh_transform( msh_imesh_t* mesh, msh_mat4_t mat )
{
  assert( mesh );
  assert( mesh->positions );
  assert( mesh->normals );

  msh_mat4_t normal_mat = msh_mat4_transpose( msh_mat4_inverse( mat ) );
  for( size_t i = 0; i < mesh->n_vertices; ++i )
  {
    mesh->positions[i] = msh_mat4_vec3_mul( mat, mesh->positions[i], 1 );
    mesh->normals[i]   = msh_vec3_normalize( msh_mat4_vec3_mul( normal_mat, mesh->normals[i], 0 ) );
    
  }
}

void
msh_imesh_split_vertices( msh_imesh_t* mesh )
{
  assert( mesh );
  assert( mesh->positions );
  assert( mesh->indices );

  size_t new_n_vertices = 3 * mesh->n_faces;
  msh_vec3_t* new_positions = NULL;
  msh_vec3_t* new_normals = NULL;
  msh_rgba_t* new_colors = NULL;
  new_positions = (msh_vec3_t*)malloc( new_n_vertices * sizeof(msh_vec3_t) );
  if( mesh->normals ) new_normals = (msh_vec3_t*)malloc( new_n_vertices * sizeof(msh_vec3_t) );
  if( mesh->colors ) new_colors = (msh_rgba_t*)malloc( new_n_vertices * sizeof(msh_rgba_t) );

  size_t j = 0;
  for( size_t i = 0; i < mesh->n_faces; ++i )
  {
    uint32_t* ind = &mesh->indices[ 3 * i ];
    uint32_t i0 = ind[0];
    uint32_t i1 = ind[1];
    uint32_t i2 = ind[2];

    new_positions[ j + 0 ] = mesh->positions[ i0 ];
    new_positions[ j + 1 ] = mesh->positions[ i1 ];
    new_positions[ j + 2 ] = mesh->positions[ i2 ];

    if( mesh->normals )
    {
      msh_vec3_t v1 = msh_vec3_sub(new_positions[j+1], new_positions[j+0]);
      msh_vec3_t v2 = msh_vec3_sub(new_positions[j+2], new_positions[j+0]);
      msh_vec3_t n = msh_vec3_normalize( msh_vec3_cross( v1, v2 ) );
      new_normals[ j + 0 ] = n;//mesh->normals[ i0 ];
      new_normals[ j + 1 ] = n;//mesh->normals[ i0 ];
      new_normals[ j + 2 ] = n;//mesh->normals[ i0 ];
    }

    if( mesh->colors )
    {
      new_colors[ j + 0 ] = mesh->colors[ i0 ];
      new_colors[ j + 1 ] = mesh->colors[ i1 ];
      new_colors[ j + 2 ] = mesh->colors[ i2 ];
    }

    ind[0] = j + 0;
    ind[1] = j + 1;
    ind[2] = j + 2;

    j += 3;
  }

  assert( j == new_n_vertices );
  free( mesh->positions );
  mesh->positions = new_positions;
  mesh->n_vertices = new_n_vertices;

  if( mesh->normals )
  {
    free( mesh->normals );
    mesh->normals = new_normals;
  }
  if( mesh->colors )
  {
    free( mesh->colors );
    mesh->colors = new_colors;
  }

}


#ifdef MSH_PLY
#define MSH_IMESH__INIT_PLY_DESCRIPTORS()                                                             \
descriptors[0] = (msh_ply_desc_t){ .element_name = "vertex",                                       \
                                   .property_names = (const char*[]){"x", "y", "z"},               \
                                   .num_properties = 3,      \
                                   .data = &mesh->positions,      \
                                   .data_count = &mesh->n_vertices,      \
                                   .data_type = MSH_PLY_FLOAT };      \
descriptors[1] = (msh_ply_desc_t){ .element_name = "vertex",      \
                                   .property_names = (const char*[]){"nx", "ny", "nz"},      \
                                   .num_properties = 3,      \
                                   .data = &mesh->normals,      \
                                   .data_count = &mesh->n_vertices,      \
                                   .data_type = MSH_PLY_FLOAT };      \
descriptors[2] = (msh_ply_desc_t){ .element_name = "vertex",      \
                                   .property_names = (const char*[]){"red", "green", "blue", "alpha"},      \
                                   .num_properties = 4,      \
                                   .data = &mesh->colors,      \
                                   .data_count = &mesh->n_vertices,      \
                                   .data_type = MSH_PLY_UINT8 };      \
descriptors[3] = (msh_ply_desc_t){ .element_name = "face",      \
                                   .property_names = (const char*[]){"vertex_indices"},      \
                                   .num_properties = 1,      \
                                   .data_type = MSH_PLY_INT32,      \
                                   .list_type = MSH_PLY_UINT8,      \
                                   .data = &mesh->indices,      \
                                   .data_count = &mesh->n_faces,      \
                                   .list_size_hint = 3 };
#endif

int32_t
msh_imesh_load_ply( msh_imesh_t* mesh, const char* filename )
{
  assert( mesh );
  assert( !mesh->indices );
  assert( !mesh->positions );
  assert( !mesh->normals );
  assert( !mesh->colors );

#ifdef MSH_PLY
  msh_ply_desc_t descriptors[4] = {0};
  MSH_IMESH__INIT_PLY_DESCRIPTORS();
  msh_ply_desc_t* vpos_desc = &descriptors[0];
  msh_ply_desc_t* vnor_desc = &descriptors[1];
  msh_ply_desc_t* vclr_desc = &descriptors[2];
  msh_ply_desc_t* find_desc = &descriptors[3];
  
  int32_t msh_mesh_err           = MSH_IMESH_NO_ERR;
  int32_t msh_ply_err            = MSH_PLY_NO_ERR;
  bool need_to_calculate_normals = false;
  bool need_to_init_color        = false;
  msh_ply_t* ply_file = msh_ply_open( filename, "rb" );
  if( !ply_file )
  {
    msh_mesh_err = MSH_IMESH_FILE_NOT_FOUND_ERR; 
    return msh_mesh_err;
  }

  msh_ply_parse_header( ply_file );

  if( !msh_ply_has_properties( ply_file, vpos_desc ) )
  {
    msh_mesh_err = MSH_IMESH_PLY_MISSING_VERTEX_POSITION;
    goto ply_io_failure;
  }
  else
  {
    msh_ply_err = msh_ply_add_descriptor( ply_file, vpos_desc );
    if( msh_ply_err ) { goto ply_io_failure; }
  }

  if( !msh_ply_has_properties( ply_file, vnor_desc ) )
  {
    need_to_calculate_normals = true;
  }
  else
  {
    msh_ply_err = msh_ply_add_descriptor( ply_file, vnor_desc );
    if( msh_ply_err ) { goto ply_io_failure; }
  }

  if( !msh_ply_has_properties( ply_file, vclr_desc ) )
  {
    need_to_init_color = true;
  }
  else
  {
    msh_ply_err = msh_ply_add_descriptor( ply_file, vclr_desc );
    if( msh_ply_err ) { goto ply_io_failure; }
  }

  if( !msh_ply_has_properties( ply_file, find_desc ) )
  {
    /* No face info - cancel vertex normal calculation */
    need_to_calculate_normals = false;
  }
  else
  {
    msh_ply_err = msh_ply_add_descriptor( ply_file, find_desc );
    if( msh_ply_err ) { goto ply_io_failure; }
  }

  msh_ply_err = msh_ply_read( ply_file );

  if( need_to_calculate_normals )
  {
    msh_imesh_calculate_vertex_normals( mesh );
  }

  if( need_to_init_color )
  {
    mesh->colors = (msh_rgba_t*)malloc( mesh->n_vertices * sizeof(msh_rgba_t) );
    for( size_t i = 0; i < mesh->n_vertices; ++i )
    {
      mesh->colors[i] = (msh_rgba_t){{ 255, 255, 255, 255 }};
    }
  }

ply_io_failure:
  if( msh_ply_err )
  {
    msh_eprintf( "%s\n", msh_ply_get_error_msg( msh_ply_err ) );
    msh_mesh_err = MSH_IMESH_PLY_IN_ERR;
  }
  msh_ply_close( ply_file );
  return msh_mesh_err;
#else
  return MSH_IMESH_FORMAT_NOT_SUPPORTED_ERR;
#endif
}

int32_t
msh_imesh_write_ply( msh_imesh_t* mesh, const char* filename )
{
  assert( mesh );
  assert( mesh->positions );


#ifdef MSH_PLY
  msh_ply_desc_t descriptors[4] = {0};
  MSH_IMESH__INIT_PLY_DESCRIPTORS();
  msh_ply_desc_t* vpos_desc = &descriptors[0];
  msh_ply_desc_t* vnor_desc = &descriptors[1];
  msh_ply_desc_t* vclr_desc = &descriptors[2];
  msh_ply_desc_t* find_desc = &descriptors[3];
  int32_t msh_mesh_err           = MSH_IMESH_NO_ERR;
  int32_t msh_ply_err            = MSH_PLY_NO_ERR;

  msh_ply_t* ply_file = msh_ply_open( filename, "wb" );
  if( !ply_file )
  {
    msh_mesh_err = MSH_IMESH_FILE_NOT_FOUND_ERR; 
    return msh_mesh_err;
  }

  msh_ply_err = msh_ply_add_descriptor( ply_file, vpos_desc );
  if( msh_ply_err ) { goto ply_io_failure; }

  if( mesh->normals ) 
  {
    msh_ply_err = msh_ply_add_descriptor( ply_file, vnor_desc );
    if( msh_ply_err )  { goto ply_io_failure; }
  }

  if( mesh->colors ) 
  { 
    msh_ply_err = msh_ply_add_descriptor( ply_file, vclr_desc ); 
    if( msh_ply_err ) { goto ply_io_failure; }
  }

  if( mesh->indices ) 
  { 
    msh_ply_err = msh_ply_add_descriptor( ply_file, find_desc );
    if( msh_ply_err ) { goto ply_io_failure; }
  }

  msh_ply_err = msh_ply_write( ply_file );

ply_io_failure:
  if( msh_ply_err )
  {
    msh_eprintf( "%s\n", msh_ply_get_error_msg( msh_ply_err ) );
    msh_mesh_err = MSH_IMESH_PLY_OUT_ERR;
  }
  msh_ply_close( ply_file );
  return msh_mesh_err;
#else
  return MSH_IMESH_FORMAT_NOT_SUPPORTED_ERR;
#endif
}

int32_t
msh_imesh_load_obj( )
{
#ifdef MSH_OBJ
  return MSH_IMESH_NO_ERR;
#else
  return MSH_IMESH_FORMAT_NOT_SUPPORTED_ERR;
#endif
}

int32_t
msh_imesh_write_obj( )
{
#ifdef MSH_OBJ
  return MSH_IMESH_NO_ERR;
#else
  return MSH_IMESH_FORMAT_NOT_SUPPORTED_ERR;
#endif
}

int32_t
msh_imesh_load( msh_imesh_t* mesh, const char* filename )
{
  char* ext = strrchr( filename, '.' );
  if( !ext ) { return MSH_IMESH_MISSING_EXTENSION_ERR; }

  int32_t err = MSH_IMESH_NO_ERR;
  if( !strcmp( ext, ".ply" ) )      { err = msh_imesh_load_ply( mesh, filename ); }
  else if( !strcmp( ext, ".obj" ) ) { err = msh_imesh_load_obj(); }
  else { err = MSH_IMESH_FORMAT_NOT_SUPPORTED_ERR; }

  return err;
}

int32_t
msh_imesh_write( msh_imesh_t* mesh, const char* filename )
{
  char* ext = strrchr( filename, '.' );
  if( !ext ) { return MSH_IMESH_MISSING_EXTENSION_ERR; }

  int32_t err = MSH_IMESH_NO_ERR;
  if( !strcmp( ext, ".ply" ) )      { err = msh_imesh_write_ply( mesh, filename ); }
  else if( !strcmp( ext, ".obj" ) ) { err = msh_imesh_write_obj(); }
  else { err = MSH_IMESH_FORMAT_NOT_SUPPORTED_ERR; }

  return err;
}

char*
msh_imesh_get_error_msg( int32_t error_code )
{
  switch( error_code )
  {
    case MSH_IMESH_NO_ERR:
      return "MSH_IMESH: No Errors";
    case MSH_IMESH_INIT_FAILURE_ERR:
      return "MSH_IMESH: Failed to initialize memory for indexed mesh";
    case MSH_IMESH_MISSING_EXTENSION_ERR:
      return "MSH_IMESH: Provided filename is missing an extension";
    case MSH_IMESH_FORMAT_NOT_SUPPORTED_ERR:
      return "MSH_IMESH: Provided format is not supported. Supported formats: .ply";
    case MSH_IMESH_PLY_MISSING_VERTEX_POSITION:
      return "MSH_IMESH: Ply file is missing vertex data!";
    case MSH_IMESH_FILE_NOT_FOUND_ERR:
      return "MSH_IMESH: Could not open mesh file!";
    case MSH_IMESH_PLY_IN_ERR:
      return "MSH_IMESH: Issue reading ply file!";
    case MSH_IMESH_PLY_OUT_ERR:
      return "MSH_IMESH: Issue writing ply file!";
    default:
      return "No Errors";
  }
}
#endif /*MSH_MESH*/


#ifdef MSH_MESH_IMPLEMENTATION

#endif