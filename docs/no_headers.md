Why not #include <libx.h>?
===================
  
Following the advice from an exellent article by Rob Pike
(https://www.lysator.liu.se/c/pikestyle.html), we do not explicity import
headers by default. This should be done once, before the msh libraries are
included. 

If you are unhappy with this behaviour, you can add:
~~~~~~~~~~~~~
#define MSH_<LIBNAME>_INCLUDE_HEADERS
~~~~~~~~~~~~~

Or simply modify the source to include them without that flag :)