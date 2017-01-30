#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>

#define MSH_ARGPARSE_IMPLEMENTATION
#include "msh_argparse.h"

typedef struct options
{
  char * filename;
  int lucky_number;
  float rectangle_size[2];
  double cuboid_size[3];
  bool print_verbose;
  char *your_name[2];
} options_t;


int main( int argc, char** argv )
{
  options_t opts = {.filename       = NULL,
                    .lucky_number   = -1.0f,
                    .rectangle_size = {-1.0f, -1.0f},
                    .cuboid_size    = {-1.0f, -1.0f, -1.0f},
                    .print_verbose  = false,
                    .your_name      = {NULL, NULL} };

  msh_argparse_t parser;
  msh_init_argparse( "Argparse Example Program",
                     "This program showcases the capabilities of "
                     "msh_argparse_t", &parser );
  msh_add_string_argument("filename", NULL, "Name of a file we need to read",
                           &opts.filename, 1, &parser );
  msh_add_int_argument("--lucky_number", "-l", "Your lucky number",
                       &opts.lucky_number, 1, &parser );
  msh_add_float_argument("--rectangle_size", "-r", "Size of some rectangle",
                         &opts.rectangle_size[0], 2, &parser);
  msh_add_double_argument("--cuboid_size", "-c", "Size of some cuboid",
                          &opts.cuboid_size[0], 3, &parser );
  msh_add_bool_argument("--verbose", "-v", "Print verbose information",
                        &opts.print_verbose, 0, &parser );
  msh_add_string_argument("--your_name", "-n", "Your first and last name",
                          &opts.your_name[0], 2, &parser );

  if( !msh_parse_arguments(argc, argv, &parser) )
  {
    msh_display_help( &parser );
    exit(-1);
  }
  
  printf( "Supplied filename to read: %s\n", opts.filename );
  
  // if ( opts.print_verbose );
}
