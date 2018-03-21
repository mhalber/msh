/*
  ==============================================================================
  
  MSH_MACHINE_LEARNING.H - WIP!
  
  A single header library for simple machine learning algorithms. 

  To use the library you simply add:
  
  #define MSH_MACHINE_LEARNING_IMPLEMENTATION
  #include "msh_ml.h"

  The define should only include once in your source. If you need to include 
  library in multiple places, simply use the include:

  #include "msh_ml.h"

  All functions can be made static by definining:

  #ifdef MSH_MACHINE_LEARNING_STATIC

  before including the "msh_ml.h"

  ==============================================================================
  DEPENDENCIES

  This library requires anonymous structs, which is a C11 extension.

  This library depends on following standard headers:

  TODO: Add dependencies list

  By default this library does not import these headers. Please see 
  docs/no_headers.md for explanation. Importing heades is enabled by:

  #define MSH_MACHINE_LEARNING_INCLUDE_HEADERS

  ==============================================================================
  AUTHORS

    Maciej Halber (macikuh@gmail.com)

  ==============================================================================
  LICENSE
  The MIT License (MIT)

  Copyright (c) 2017 Maciej Halber

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
  
  
  ==============================================================================
  NOTES 

  ==============================================================================
  TODOs
  [ ] clustering: k-means, mean-shift
  [ ] classifiers: decision trees, boosting?

  ==============================================================================
  REFERENCES:

 */


#ifndef MSH_MACHINE_LEARNING_H
#define MSH_MACHINE_LEARNING_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MSH_MACHINE_LEARNING_INCLUDE_HEADERS
#include <stdio.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#endif

#ifdef MSH_MACHINE_LEARNING_STATIC
#define MSHMLDEF static
#else
#define MSHMLDEF extern
#endif




#ifdef __cplusplus
}
#endif

#endif /*MSH_MACHINE_LEARNING_H*/

#ifdef MSH_MACHINE_LEARNING_IMPLEMENTATION


#endif /*MSH_MACHINE_LEARNING_IMPLEMENTATION*/