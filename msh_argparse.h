
/*
  ==============================================================================
  
  MSH_ARGPARSE.H 
  

  ==============================================================================
  DEPENDENCIES

  ==============================================================================
  TODOS
    1. Ask Kyle for review
    2. Add program description / options and example
    2. Fill in dependencies info

  ==============================================================================
  AUTHORS

    Maciej Halber (macikuh@gmail.com)

  ==============================================================================
  LICENSE

  This software is in the public domain. Where that dedication is not
  recognized, you are granted a perpetual, irrevocable license to copy,
  distribute, and modify this file as you see fit.

  ==============================================================================
  NOTES 
    1. This code is macro heavy, which is probably not great. However, macros
       are uses solely for the purpose of code-generating functions.
    2. User should not touch any of the members of defined structs 
       msh_arg and msh_argparse.
 */

/*
 * =============================================================================
 *       INCLUDES, TYPES AND DEFINES
 * =============================================================================
 */


#ifndef MSH_ARGPARSE
#define MSH_ARGPARSE

/* Main options. Should be sane defaults. */

#ifndef MSH_MAX_NAME_LEN
#define MSH_MAX_NAME_LEN     64
#endif

#ifndef MSH_MAX_STR_LEN
#define MSH_MAX_STR_LEN      512
#endif

#ifndef MSH_MAX_N_ARGS
#define MSH_MAX_N_ARGS       100
#endif

/* If you're confident all your arguments are setup properly it is fine to 
   define MSH_NO_DEBUG, to prevent bounds check from hapenning. */

/* #define MSH_NO_DEBUG */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MSH_ARGPARSE_INCLUDE_HEADERS
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#endif

#ifdef MSH_ARGPARSE_STATIC
#define MSHAPDEF static
#else
#define MSHAPDEF extern
#endif

typedef enum 
{
  BOOL,
  CHAR,
  UNSIGNED_CHAR,
  SHORT,
  UNSIGNED_SHORT,
  INT,
  UNSIGNED_INT,
  LONG,
  UNSIGNED_LONG,
  FLOAT,
  DOUBLE,
  STRING,
  N_TYPES
} msh_type_t;


// to make these private -> first make them void, then move them to implementation, then remove them again!
typedef struct msh_arg
{
  char *name;
  char *shorthand;
  char *message;
  char position;
  msh_type_t type;
  size_t num_vals;
  void* values;
} msh_arg_t;

typedef struct test_struct test_t;

typedef struct msh_argparse
{
  char *program_name; 
  char *program_description;

  msh_arg_t args[ MSH_MAX_N_ARGS ];
  size_t n_args;
  size_t n_required;

  char * typenames[ N_TYPES ];

} msh_argparse_t;


MSHAPDEF int msh_init_argparse( char * program_name, 
                                char * program_description,
                                msh_argparse_t * argparse );

MSHAPDEF int msh_parse_arguments( int argc, 
                                  char **argv, 
                                  msh_argparse_t * argparse );

MSHAPDEF int msh_display_help( msh_argparse_t * argparse );

/* Argument addition function prototype.  */
/* typename follows the convention of msh_typenames */
#define MSH_ADD_ARGUMENT(typename,val_t)                                       \
  MSHAPDEF int msh_add_##typename##_argument( char * name,                     \
                                              char * shorthand,                \
                                              char * message,                  \
                                              val_t *value,                    \
                                              const size_t num_vals,           \
                                              msh_argparse_t * argparse ); 

/* let's ask preprocessor to generate functions for us */
/* msh_add_bool_argument(...) */
MSH_ADD_ARGUMENT(bool, bool)
/* msh_add_char_argument(...) */
MSH_ADD_ARGUMENT(char, char)
/* msh_add_unsigned_char_argument(...) */
MSH_ADD_ARGUMENT(unsigned_char, unsigned char)  
/* msh_add_short_argument(...) */
MSH_ADD_ARGUMENT(short, short)
/* msh_add_unsigned_short_argument(...) */
MSH_ADD_ARGUMENT(unsigned_short, unsigned short)
/* msh_add_int_argument(...) */
MSH_ADD_ARGUMENT(int,int)
/* msh_add_unsigned_int_argument(...) */
MSH_ADD_ARGUMENT(unsigned_int, unsigned int)
/* msh_add_long_argument(...) */
MSH_ADD_ARGUMENT(long,long)
/* msh_add_unsigned_long_argument(...) */
MSH_ADD_ARGUMENT(unsigned_long, unsigned long)
/* msh_add_float_argument(...) */
MSH_ADD_ARGUMENT(float,float)
/* msh_add_double_argument(...) */
MSH_ADD_ARGUMENT(double,double)
/* msh_add_sting_argument(...) */
MSH_ADD_ARGUMENT(string,char*)

#ifdef __cplusplus
}
#endif

#endif /* MSH_ARGPARSE */

/*
 * =============================================================================
 *       VECTORS
 * =============================================================================
 */
#ifdef MSH_ARGPARSE_IMPLEMENTATION

typedef struct test_struct
{
  int a;
  float b;
  double c;
} test_t;


static int 
msh__arg_compare ( const void * data_a, const void * data_b )
{
  const msh_arg_t * arg_a = data_a; 
  const msh_arg_t * arg_b = data_b;

  if (  arg_a->position >= 0 && arg_b->position >= 0 ) 
    return 0;

  if ( arg_a->position < 0 && arg_b->position < 0 ) 
    return strncmp( arg_a->name, arg_b->name, MSH_MAX_NAME_LEN );

  if (  arg_a->position >= 0 && arg_b->position < 0 )
    return -1;

  if ( arg_a->position < 0 &&  arg_b->position >= 0 ) 
    return 1;
  
  return 0;
}

static int
msh__are_options_valid( const char * name, 
                        const char * shorthand, 
                        const char * message, 
                        const void * values, 
                        const msh_argparse_t *argparse )
{
#ifndef MSH_NO_DEBUG
  if ( argparse->n_args >= MSH_MAX_N_ARGS )
  {
    printf("Reached maxiumum numbers of arguments!\n"
            "Did not add argument %s\n"
            "Please modify your options as necessary!\n",
            name );
    return 0;
  }

  if ( strlen(name) > MSH_MAX_NAME_LEN )
  {
    printf("Name for argument %s is too long (more than %d). "
            "Please shorten or modify your options as necessary!\n", 
            name, MSH_MAX_NAME_LEN );
    return 0;
  }

  /* TODO: Enforce that if argument its optional its length must be at least 3 and 
     must have "--" */
  if (strlen(name) > 3 && name[0] == '-' && name[1] != '-' ) 
  {
    printf("All optional arguments must start with '--'. "
           "Please fix argument %s.\n", 
            name );
  }

  if ( values == NULL )
  {
    printf("Storage for argument %s is invlaid (NULL pointer).\n", 
            name );
    return 0;
  }

  if ( shorthand && ( strlen(shorthand) != 2 || shorthand[0] != '-' ) ) 
  {
    printf("Shorthand for argument %s has invalid format! "
           "Correct shorthand format '-<single_letter>'.\n",
            name );
    return 0;
  }

  if ( message && strlen(message) > MSH_MAX_STR_LEN )
  {
    printf("Message for argument %s is too long (more than %d). "
            "Please shorten or modify your options as necessary!\n", 
            name, MSH_MAX_STR_LEN );
    return 0;
  }

#endif
  /* If no debug defined this will just return 1 */
  return 1;
}
  
static void                                      
msh__print_arguments( const msh_argparse_t * argparse, 
                      size_t start, 
                      size_t end )
{
  for ( size_t i = start ; i < end ; ++i )
  {
    const msh_arg_t * argument = &argparse->args[i];
    if ( argument->shorthand == NULL )
    {
      printf("\t%-24s - %s <%lu %s> %d\n", argument->name,
                                           argument->message,
                                           argument->num_vals,
                                           argparse->typenames[argument->type],
                                           argument->position );
    }
    else
    {
      char name_and_shorthand[MSH_MAX_NAME_LEN + 10];
      sprintf( name_and_shorthand, "%s, %s", argument->name, 
                                             argument->shorthand );
      printf("\t%-24s - %s <%lu %s> %d\n", name_and_shorthand,
                                           argument->message,
                                           argument->num_vals,
                                           argparse->typenames[argument->type],
                                           argument->position );
    }
  }
}

static msh_type_t
msh__find_type( const msh_argparse_t * argparse, char * type_id )
{
  for ( size_t i = 0 ; i < N_TYPES ; ++i )
  {
    if ( !strncmp( type_id, argparse->typenames[i], 15 ) )
      return i;
  }
  return -1;
}

static msh_arg_t *
msh__find_argument( const char * arg_name,
                    const char * shorthand,
                    const int position,
                    msh_argparse_t * argparse )
{
  for ( size_t i = 0 ; i < argparse->n_args ; ++i )
  {
    msh_arg_t * cur_arg = &argparse->args[ i ];
    if ( arg_name && !strcmp( cur_arg->name, arg_name ) )
    {
      return cur_arg;
    }
    if ( shorthand && cur_arg->shorthand &&
         !strcmp(cur_arg->shorthand, shorthand) )
    {
      return cur_arg;
    }
    if ( position >= 0 && cur_arg->position == position )
    {
      return cur_arg;
    }
  }
  return NULL;
}

#define MSH__PARSE_ARGUMENT(typename, val_t, cfunc)                            \
  static int                                                                   \
  msh__parse_##typename##_argument( msh_arg_t * arg,                           \
                                    size_t argc,                               \
                                    char **argv,                               \
                                    size_t *argv_index )                       \
  {                                                                            \
    val_t * values = arg->values;                                              \
    if ( arg->num_vals <= 0 )                                                  \
    {                                                                          \
      values[0] = (val_t)1;                                                    \
      return 1;                                                                \
    }                                                                          \
    for ( size_t j = 0 ; j < arg->num_vals ; j++ )                             \
    {                                                                          \
      *argv_index += 1;                                                        \
      if ( *argv_index >= argc || (argv[*argv_index][0] == '-' &&              \
                                  !isdigit(argv[*argv_index][1]) ) )           \
      {                                                                        \
        printf( "Wrong number of parameters for argument %s. "                 \
                "Correct value is: %lu\n",                                     \
                arg->name,                                                     \
                arg->num_vals );                                               \
        return 0;                                                              \
      }                                                                        \
      values[j] = cfunc( argv[*argv_index] );                                  \
    }                                                                          \
    return 1;                                                                  \
  }                                                                            
MSH__PARSE_ARGUMENT(bool, bool, (bool)atoi)
MSH__PARSE_ARGUMENT(char, char, atoi)
MSH__PARSE_ARGUMENT(unsigned_char, unsigned char, atoi)
MSH__PARSE_ARGUMENT(short, short, atoi)
MSH__PARSE_ARGUMENT(unsigned_short, unsigned short, atoi)
MSH__PARSE_ARGUMENT(int, int, atoi)
MSH__PARSE_ARGUMENT(unsigned_int, unsigned int, atoi)
MSH__PARSE_ARGUMENT(long, long, atoi)
MSH__PARSE_ARGUMENT(unsigned_long, unsigned long, atoi)
MSH__PARSE_ARGUMENT(float, float, atof)
MSH__PARSE_ARGUMENT(double, double, atof)
MSH__PARSE_ARGUMENT(string, char*, /*nothing*/ )

static int 
msh__parse_argument( msh_arg_t * arg, 
                     int argc, 
                     char** argv, 
                     size_t * argv_index )
{
  /* NOTE: CAN I AVOID THIS SWITCH SOMEHOW...? */
  switch ( arg->type )
  {
    case BOOL:
      if ( !msh__parse_bool_argument(arg, argc, argv, argv_index) ) 
        { return 0; }
      break;
    case CHAR:
      if ( !msh__parse_char_argument(arg, argc, argv, argv_index) ) 
        { return 0; }
      break;
    case UNSIGNED_CHAR:
      if ( !msh__parse_unsigned_char_argument(arg, argc, argv, argv_index) )
        { return 0; }
      break;
    case SHORT:
      if ( !msh__parse_short_argument(arg, argc, argv, argv_index) ) 
        { return 0; }
      break;
    case UNSIGNED_SHORT:
      if ( !msh__parse_unsigned_short_argument(arg, argc, argv, argv_index) )
        { return 0; }
      break;
    case INT:
      if ( !msh__parse_int_argument(arg, argc, argv, argv_index) ) 
        { return 0; }
      break;
    case UNSIGNED_INT:
      if ( !msh__parse_unsigned_int_argument(arg, argc, argv, argv_index) )
        { return 0; }
      break;
    case LONG:
      if ( !msh__parse_long_argument(arg, argc, argv, argv_index) ) 
        { return 0; }
      break;
    case UNSIGNED_LONG:
      if ( !msh__parse_unsigned_long_argument(arg, argc, argv, argv_index) )
        { return 0; }
      break;
    case FLOAT:
      if ( !msh__parse_float_argument(arg, argc, argv, argv_index) )
        { return 0; }
      break;
    case DOUBLE:
      if ( !msh__parse_double_argument(arg, argc, argv, argv_index) )
        { return 0; }
      break;
    case STRING:
      if ( !msh__parse_string_argument(arg, argc, argv, argv_index) )
        { return 0; }
      break;
    default:
      return 0;
  }
  return 1;
}


#define MSH_ADD_ARGUMENT_IMPL(typename, val_t)                                 \
  MSHAPDEF int                                                                 \
  msh_add_##typename##_argument( char * name,                                  \
                                 char * shorthand,                             \
                                 char * message,                               \
                                 val_t *values,                                \
                                 const size_t num_vals,                        \
                                 msh_argparse_t * argparse )                   \
  {                                                                            \
    if ( !name )                                                               \
    {                                                                          \
      printf( "Please provide valid name for an argument\n" );                 \
      return 0;                                                                \
    }                                                                          \
    msh_arg_t *argument = msh__find_argument( name,                            \
                                              shorthand,                       \
                                              -1,                              \
                                              argparse );                      \
    if ( !argument )                                                           \
    {                                                                          \
      if ( !msh__are_options_valid( name,                                      \
                                    shorthand,                                 \
                                    message,                                   \
                                    values,                                    \
                                    argparse ) )                               \
      {                                                                        \
        return 0;                                                              \
      }                                                                        \
                                                                               \
      /* create argument */                                                    \
      msh_arg_t *argument = &(argparse->args[argparse->n_args++]);             \
                                                                               \
      /* is argument an option, or required? */                                \
      argument->position = -1;                                                 \
      if ( name[0] != '-' )                                                    \
      {                                                                        \
        argument->position = ++argparse->n_required - 1;                       \
      }                                                                        \
                                                                               \
      /* assign properties */                                                  \
      argument->name      = name;                                              \
      argument->shorthand = shorthand;                                         \
      argument->message   = message;                                           \
                                                                               \
      /* determine type */                                                     \
      argument->type = msh__find_type( argparse, #typename );                  \
                                                                               \
      /* copy length and pointer */                                            \
      argument->num_vals = num_vals;                                           \
      argument->values = (void*)values;                                        \
      return 1;                                                                \
    }                                                                          \
    else                                                                       \
    {                                                                          \
      printf("Argument %s already exists. Skipping!\n", name );                \
      return 0;                                                                \
    }                                                                          \
  }
MSH_ADD_ARGUMENT_IMPL(bool, bool)
MSH_ADD_ARGUMENT_IMPL(char, char)
MSH_ADD_ARGUMENT_IMPL(unsigned_char, unsigned char)
MSH_ADD_ARGUMENT_IMPL(short, short)
MSH_ADD_ARGUMENT_IMPL(unsigned_short, unsigned short)
MSH_ADD_ARGUMENT_IMPL(int, int)
MSH_ADD_ARGUMENT_IMPL(unsigned_int, unsigned int)
MSH_ADD_ARGUMENT_IMPL(long, long)
MSH_ADD_ARGUMENT_IMPL(unsigned_long, unsigned long)
MSH_ADD_ARGUMENT_IMPL(float, float)
MSH_ADD_ARGUMENT_IMPL(double, double)
MSH_ADD_ARGUMENT_IMPL(string, char*)


MSHAPDEF int 
msh_init_argparse( char * program_name,
                   char * program_description,
                   msh_argparse_t * argparse )
{
#ifndef MSH_NO_DEBUG
  if( strlen(program_name) >= MSH_MAX_NAME_LEN ) 
  { 
    printf("Name %s is too long (more than %d). "
           "Please shorten or modify your options as necessary!\n", 
           program_name, MSH_MAX_NAME_LEN );
  }
  
  if( strlen(program_description) >= MSH_MAX_STR_LEN )
  { 
    printf("Description %s is too long (more than %d). "
           "Please shorten or modify your options as necessary!\n", 
           program_description, MSH_MAX_STR_LEN );
  }
#endif

  argparse->program_name = program_name;
  argparse->program_description = program_description;

  argparse->n_args = 0;
  argparse->n_required = 0;

  argparse->typenames[ 0] = "bool";
  argparse->typenames[ 1] = "char";
  argparse->typenames[ 2] = "unsigned_char";
  argparse->typenames[ 3] = "short";
  argparse->typenames[ 4] = "unsigned_short";
  argparse->typenames[ 5] = "int";
  argparse->typenames[ 6] = "unsigned_int";
  argparse->typenames[ 7] = "long";
  argparse->typenames[ 8] = "unsigned_long";
  argparse->typenames[ 9] = "float";
  argparse->typenames[10] = "double";
  argparse->typenames[11] = "string";

  return 1;
}

MSHAPDEF int
msh_parse_arguments( int argc, 
                     char **argv, 
                     msh_argparse_t * argparse )
{
  /* sort arguments */
  qsort( argparse->args, 
         argparse->n_args, 
         sizeof(msh_arg_t), 
         msh__arg_compare );

  size_t argv_index = 0;
  size_t i;
  for ( i = 0 ; i < argparse->n_required ; ++i )
  {
    msh_arg_t * cur_arg = &(argparse->args[i]);
    if ( !msh__parse_argument( cur_arg, argc, argv, &argv_index ) ) return 0;
  }
 
  argv_index++;
  for ( ; (int)argv_index < argc ; ++argv_index )
  {
     msh_arg_t * cur_arg = msh__find_argument( argv[argv_index], 
                                               argv[argv_index], 
                                               -1,
                                               argparse );
    if ( !cur_arg ) 
    { 
      printf( "Unknown argument %s encountered!\n", argv[argv_index] );
      return 0;
    }
    if ( !msh__parse_argument( cur_arg, argc, argv, &argv_index ) ) return 0;
  }
  return 1;
}

MSHAPDEF int 
msh_display_help( msh_argparse_t *argparse )
{
  printf("\n|%s\n", argparse->program_name );
  printf("|\t%s\n", argparse->program_description );
  
  /* sort arguments */
  qsort( argparse->args, 
         argparse->n_args, 
         sizeof(msh_arg_t), 
         msh__arg_compare );

  /* Print out the info */
  if ( argparse->n_args )
  {
    printf("| Required Arguments:\n");
    msh__print_arguments( argparse, 0, argparse->n_required );

    printf("| Optional Arguments:\n");
    msh__print_arguments( argparse, argparse->n_required, argparse->n_args );
  }
  printf("-----------------------------------------------------------------\n");
  return 1;
}

#endif  /* MSH_ARGPARSE_IMPLEMENTATION */
