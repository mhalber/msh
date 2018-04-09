/*
This file is a simple test and some notes on tinyply2.0 by @ddiakopoulos.
Goal here is to identify how tinyply deals with non-standard ply files,
as well as extract some timing information

Notes: 
  - For output tinyply is quite nice. Just pass the properties to element in
  one pass. All data is passed as cast to uint8_t. All methods pass
  the info about count of the property to discriminate between regular and list properties.
  Does not seem to be possible to write data that have variable number of properties
  per element, i.e. mixed triangle/quad meshes.
  -Even though I requested binary file, an ascii file has been written. binary is decided by looking
  at the parameter in the write command. I find this a tad confusing.
  format of output command is: <element_name> <properties> <property_type> <property_count> <data> <list_type> <list_count> 
  -The property list is nice, as it is able to deal with multiple list properties.
  Still it seems like a big limitation that the count is constant per property, rendering this aspect of ply file 
  completly unusable.

  - for input, tinyply2 has this really nice concept of ply data as intermediate data representation.
  Will need to figure out how much memory does that take...
  

To compile:
clang++ -I../../ -std=c++11 tinyply.cpp tinyply2_test.cpp -o ../../bin/tinyply2_test
*/
#include <thread>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include "tinyply.h"

#define MSH_IMPLEMENTATION
#include "msh.h"



void
write_ply( const char* filename )
{
  using namespace tinyply;
  std::vector<float> pos;
  std::vector<float> nor;
  std::vector<uint8_t> col;

  std::vector<uint32_t> ind;

  pos = {
    /*  0 */0.f, 1.f, 2.f,
    /*  1 */3.f, 4.f, 5.f,
    /*  2 */6.f, 7.f, 8.f,
    /*  3 */9.f, 10.f, 11.f,
    /*  4 */12.f, 13.f, 14.f,
    /*  5 */15.f, 16.f, 17.f,
    /*  6 */18.f, 19.f, 20.f,
    /*  7 */21.f, 22.f, 23.f,
    /*  8 */24.f, 25.f, 26.f,
    /*  9 */27.f, 28.f, 29.f,
    /* 10 */30.f, 31.f, 32.f,
    /* 11 */33.f, 34.f, 35.f,
    };

    nor = {
    /*  0 */0.5f, 1.5f, 2.5f,
    /*  1 */3.5f, 4.5f, 5.5f,
    /*  2 */6.5f, 7.5f, 8.5f,
    /*  3 */9.5f, 10.5f, 11.5f,
    /*  4 */12.5f, 13.5f, 14.5f,
    /*  5 */15.5f, 16.5f, 17.5f,
    /*  6 */18.5f, 19.5f, 20.5f,
    /*  7 */21.5f, 22.5f, 23.5f,
    /*  8 */24.5f, 25.5f, 26.5f,
    /*  9 */27.5f, 28.5f, 29.5f,
    /* 10 */30.5f, 31.5f, 32.5f,
    /* 11 */33.5f, 34.5f, 35.5f,
    };

    col = {
    /*  0 */10, 20, 30, 244,
    /*  1 */15, 25, 35, 245,
    /*  2 */20, 30, 40, 246,
    /*  3 */25, 35, 45, 247,
    /*  4 */30, 40, 50, 248,
    /*  5 */35, 45, 55, 249,
    /*  6 */40, 50, 60, 250,
    /*  7 */45, 55, 65, 251,
    /*  8 */50, 60, 70, 252,
    /*  9 */55, 65, 75, 253,
    /* 10 */60, 70, 80, 254,
    /* 11 */65, 75, 85, 255
    };
    
    ind = { 
        0, 1, 2, 0, 1, 2, 
        3, 4, 5,  3, 4, 5,
        6, 7, 8,  6, 7, 8, 
        9, 10, 11, 9, 10, 11 
    };

    // Tinyply does not perform any file i/o internally
    std::filebuf fb;
    fb.open(filename, std::ios::binary | std::ios::out  );
    std::ostream output_stream(&fb);

    PlyFile ply_out;
    ply_out.add_properties_to_element("vertex", { "x", "y", "z" }, Type::FLOAT32, pos.size(),
                                            reinterpret_cast<uint8_t*>(pos.data()), Type::INVALID, 0);
    ply_out.add_properties_to_element("vertex", { "nx", "ny", "nz" }, Type::FLOAT32, pos.size(), 
                                            reinterpret_cast<uint8_t*>(nor.data()), Type::INVALID, 0);
    ply_out.add_properties_to_element("vertex", { "red", "green", "blue", "alpha" }, Type::UINT8, pos.size(),
                                            reinterpret_cast<uint8_t*>(col.data()), Type::INVALID, 0);

    ply_out.add_properties_to_element("face", { "vertex_indices", "ind2" }, Type::UINT32, ind.size()/2, 
                                            reinterpret_cast<uint8_t*>(ind.data()), Type::UINT16, 3);

    ply_out.write(output_stream, false);

    fb.close();
}

void
read_ply( const char* filename )
{
  using namespace tinyply;
  std::ifstream ss(filename, std::ios::binary);
  PlyFile file;
  file.parse_header(ss);
  
  std::shared_ptr<PlyData> pos, ind;
  pos = file.request_properties_from_element("vertex", {"x", "y", "z"});
  ind = file.request_properties_from_element("face", { "vertex_indices" });

  file.read(ss);
}
int
main( int argc, char** argv )
{
  double t1, t2;
  printf("TinyPly2 testing\n");
  
  // t1 = msh_get_time(MSHT_MILLISECONDS);
  // write_ply("test.ply");
  // t2 = msh_get_time(MSHT_MILLISECONDS);
  // printf("Ply file written in %lf milliseconds\n", t2-t1);

  t1 = msh_get_time(MSHT_MILLISECONDS);
  printf("%s\n", argv[1]);
  read_ply(argv[1]);
  t2 = msh_get_time(MSHT_MILLISECONDS);
  printf("Ply file read in %lf milliseconds\n", t2-t1);
}