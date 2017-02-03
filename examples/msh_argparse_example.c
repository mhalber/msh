/*
  msh_argparse_example.c
  Author: Maciej Halber (macikuh@gmail.com)

  ===========================================
  An example program showcae msh_argparse.h library. Running it as:
 
  msh_argparse_example test.txt --your_name Maciej Halber --lucky_number 13
 
  Should print:
 
  Supplied filename to read: test.txt
  Your lucky number is 13
  Your name is Maciej Halber
 
  =====================================
  Not providing the required filename parameter will result in a failure:
 
  msh_argparse_example --your_name Maciej Halber --lucky_number 13
 
  Should print:

  Argparse Error: Wrong number of parameters for argument filename. Correct value is: 1

  |Argparse Example Program
  |       This program showcases the capabilities of msh_argparse_t
  |
  |Usage:
  | Required Arguments:
  |       filename                         - Name of a file we need to read <1 string>
  |
  | Optional Arguments:
  |       --cuboid_size, -c                - Size of some cuboid <3 double>
  |       --lucky_number, -l               - Your lucky number <1 int>
  |       --rectangle_size, -r             - Size of some rectangle <2 float>
  |       --verbose, -v                    - Print verbose information <0 bool>
  |       --your_name, -n                  - Your first and last name <2 string>
  \----------------------------------------------------------------

  =====================================
  You can try running it with different parameters. Providing an argument that
  is not in the above list should result in error and program will return early.
 */

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
  char *your_name[2];
  bool print_verbose;
} options_t;

void 
test ( const char* a, const char* b)
{
  printf("%s %s\n", a, b);
}

int main( int argc, char** argv )
{
  options_t opts = { .filename       = NULL,
                     .lucky_number   = 0,
                     .rectangle_size = {0.0f, 0.0f},
                     .cuboid_size    = {0.0f, 0.0f, 0.0f},
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
  msh_add_string_argument("--your_name", "-n", "Your first and last name",
                          &opts.your_name[0], 2, &parser );
  msh_add_bool_argument("--verbose", "-v", "Print verbose information",
                        &opts.print_verbose, 0, &parser );

  if( !msh_parse_arguments(argc, argv, &parser) )
  {
    msh_display_help( &parser );
    exit(-1);
  }

  printf( "Supplied filename to read: %s\n", opts.filename );
  
  if( opts.lucky_number )
  { 
    printf( "Your lucky number is %d\n", opts.lucky_number);
  }

  if( opts.rectangle_size[0] && opts.rectangle_size[1] )
  {
    printf("Rectangle size : %fx%f\n", opts.rectangle_size[0], 
                                       opts.rectangle_size[1] );
  }

  if( opts.cuboid_size[0] && opts.cuboid_size[1] && opts.cuboid_size[2] )
  {
    printf("Cuboid size : %fx%fx%f\n", opts.cuboid_size[0], 
                                       opts.cuboid_size[1],
                                       opts.cuboid_size[2] );
  }
  
  if( opts.your_name[0] && opts.your_name[1] )
  {
    printf("Your name is %s %s\n", opts.your_name[0], opts.your_name[1] );
  }

  if( opts.print_verbose )
  {
    printf("Verbosity requested! Below you can see the auto-generated "
           "help message:");
    msh_display_help(&parser);
  }

}

