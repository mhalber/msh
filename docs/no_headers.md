Why not #include <libx.h>?
===================
  
Following the advice from an excellent article by Rob Pike
(https://www.lysator.liu.se/c/pikestyle.html), we do not explicitly import any
headers by default. This should be done once, before the msh libraries are
included. 

If you are unhappy with this behavior, you can add:
~~~~~~~~~~~~~
#define MSH_<LIBNAME>_INCLUDE_HEADERS
~~~~~~~~~~~~~

Or simply modify the source to include them without that flag :)