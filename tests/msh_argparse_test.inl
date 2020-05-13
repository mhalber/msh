
MunitResult
test_msh_argparse(const MunitParameter params[], void* fixture) 
{
  (void) params;
  (void) fixture;

  typedef struct local_struct
  {
    char* filename;
    int an_integer;
    double a_real_number;
    uint8_t a_bunch_of_numbers[3];
    bool an_option;
    float dummy;
  } local_struct;

  local_struct ls = {0};
  msh_argparse_t parser = {0};
  msh_ap_init( &parser, "Test", "This is a test of argparse" );
  msh_ap_add_string_argument( &parser, "filename", NULL, "Name to a file", &ls.filename, 1 );
  msh_ap_add_int_argument( &parser, "--integer", NULL, "An integer", &ls.an_integer, 1 );
  msh_ap_add_double_argument( &parser, "--real", NULL, "A real number", &ls.a_real_number, 1 );
  msh_ap_add_unsigned_char_argument( &parser, "--some_numbers", "-s", "Bunch of numbers", &ls.a_bunch_of_numbers[0], 3 );
  msh_ap_add_float_argument( &parser, "--dummy", "-d", "Dummy", &ls.dummy, 1 );
  msh_ap_add_bool_argument( &parser, "--option", "-o", "Option", &ls.an_option, 0 );

  munit_assert_ptr_equal(ls.filename, NULL);
  munit_assert_int(ls.an_integer, ==, 0);
  munit_assert_double(ls.a_real_number, ==, 0.0 );

  char* argv1[] = {"program_name", "The_File.txt"};
  int argc1 = msh_count_of(argv1);

  msh_ap_parse(&parser, argc1, argv1);
  
  munit_assert_string_equal(ls.filename, "The_File.txt");
  munit_assert_float(ls.dummy,==,0.0f);

  char* argv2[] = { "program_name", "The_Other_File.txt",
                   "--integer", "10",
                   "--real", "2.0",
                   "--some_numbers", "1", "2", "3",
                   "--option" };
  int argc2 = msh_count_of(argv2);
  msh_ap_parse(&parser, argc2, argv2);
  munit_assert_string_equal(ls.filename, "The_Other_File.txt");
  munit_assert_int(ls.an_integer,==,10 );
  munit_assert_double( ls.a_real_number,==,2.0);
  munit_assert_uint8(ls.a_bunch_of_numbers[0],==,1);
  munit_assert_uint8(ls.a_bunch_of_numbers[1],==,2);
  munit_assert_uint8(ls.a_bunch_of_numbers[2],==,3);
  munit_assert_true(ls.an_option);
  munit_assert_float(ls.dummy,==,0.0f);

  return MUNIT_OK;
}