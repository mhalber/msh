#!/usr/bin/env python

#   This file is a modification of gl3w, hosted at 
#   https://github.com/skaslev/gl3w
#
#   This is free and unencumbered software released into the public domain.
#
#   Anyone is free to copy, modify, publish, use, compile, sell, or
#   distribute this software, either in source code form or as a compiled
#   binary, for any purpose, commercial or non-commercial, and by any
#   means.
#
#   In jurisdictions that recognize copyright laws, the author or authors
#   of this software dedicate any and all copyright interest in the
#   software to the public domain. We make this dedication for the benefit
#   of the public at large and to the detriment of our heirs and
#   successors. We intend this dedication to be an overt act of
#   relinquishment in perpetuity of all present and future rights to this
#   software under copyright law.
#
#   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
#   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
#   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
#   IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
#   OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
#   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
#   OTHER DEALINGS IN THE SOFTWARE.

# Allow Python 2.6+ to use the print() function
from __future__ import print_function

import re
import os
import ssl

# Try to import Python 3 library urllib.request
# and if it fails, fall back to Python 2 urllib2
try:
    import urllib.request as urllib2
except ImportError:
    import urllib2

# UNLICENSE copyright header
UNLICENSE = br'''/*

    This file was generated with msh_glgen.py, modification of gl3w
    (hosted at https://github.com/skaslev/gl3w)

    This is free and unencumbered software released into the public domain.

    Anyone is free to copy, modify, publish, use, compile, sell, or
    distribute this software, either in source code form or as a compiled
    binary, for any purpose, commercial or non-commercial, and by any
    means.

    In jurisdictions that recognize copyright laws, the author or authors
    of this software dedicate any and all copyright interest in the
    software to the public domain. We make this dedication for the benefit
    of the public at large and to the detriment of our heirs and
    successors. We intend this dedication to be an overt act of
    relinquishment in perpetuity of all present and future rights to this
    software under copyright law.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
    OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
    ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    OTHER DEALINGS IN THE SOFTWARE.

*/

'''

# Download glcorearb.h
print('Downloading glcorearb.h...')
context = ssl._create_unverified_context()
web = urllib2.urlopen('https://www.opengl.org/registry/api/GL/glcorearb.h', context=context)
with open('../libs/glcorearb.h', 'wb') as f:
    f.writelines(web.readlines())

# Parse function names from glcorearb.h
print('Parsing glcorearb.h header...')
procs = []
p = re.compile(r'GLAPI.*APIENTRY\s+(\w+)')
with open('../libs/glcorearb.h', 'r') as f:
    for line in f:
        m = p.match(line)
        if m:
            procs.append(m.group(1))
procs.sort()

def proc_t(proc):
    return {
        'p': proc,
        'p_s': 'msh_ogl_' + proc[2:],
        'p_t': 'PFN' + proc.upper() + 'PROC'
    }

# Generate msh_ogl.h
print('Generating msh_ogl.h...')
with open('../msh_ogl.h', 'wb') as f:
    f.write(UNLICENSE)
    f.write(br'''#ifndef MSH_OGL_H
#define MSH_OGL_H

#include "libs/glcorearb.h"

#ifndef __gl_h_
#define __gl_h_
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*msh_ogl_proc_t)(void);

int msh_ogl_init(void);
int msh_ogl_is_supported(int major, int minor);
msh_ogl_proc_t msh_ogl_get_proc_address(const char *proc);

''')
    for proc in procs:
        f.write('extern {0[p_t]: <52} {0[p_s]};\n'.format(proc_t(proc)).encode('utf-8'))
    f.write(b'\n')
    for proc in procs:
        f.write('#define {0[p]: <45} {0[p_s]}\n'.format(proc_t(proc)).encode('utf-8'))
    f.write(br'''
#ifdef __cplusplus
}
#endif

#endif // MHS_OGL_H

#ifdef MSH_OGL_IMPLEMENTATION

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

static HMODULE libgl;

static void open_libgl(void)
{
	libgl = LoadLibraryA("opengl32.dll");
}

static void close_libgl(void)
{
	FreeLibrary(libgl);
}

static msh_ogl_proc_t get_proc(const char *proc)
{
	msh_ogl_proc_t res;

	res = (msh_ogl_proc_t)wglGetProcAddress(proc);
	if (!res)
		res = (msh_ogl_proc_t)GetProcAddress(libgl, proc);
	return res;
}
#elif defined(__APPLE__) || defined(__APPLE_CC__)
#include <Carbon/Carbon.h>

CFBundleRef bundle;
CFURLRef bundleURL;

static void open_libgl(void)
{
	bundleURL = CFURLCreateWithFileSystemPath(kCFAllocatorDefault,
		CFSTR("/System/Library/Frameworks/OpenGL.framework"),
		kCFURLPOSIXPathStyle, true);

	bundle = CFBundleCreate(kCFAllocatorDefault, bundleURL);
	assert(bundle != NULL);
}

static void close_libgl(void)
{
	CFRelease(bundle);
	CFRelease(bundleURL);
}

static msh_ogl_proc_t get_proc(const char *proc)
{
	msh_ogl_proc_t res;

	CFStringRef procname = CFStringCreateWithCString(kCFAllocatorDefault, proc,
		kCFStringEncodingASCII);
	*(void **)(&res) = CFBundleGetFunctionPointerForName(bundle, procname);
	CFRelease(procname);
	return res;
}
#else
#include <dlfcn.h>
#include <GL/glx.h>

static void *libgl;
static PFNGLXGETPROCADDRESSPROC glx_get_proc_address;

static void open_libgl(void)
{
	libgl = dlopen("libGL.so.1", RTLD_LAZY | RTLD_GLOBAL);
	*(void **)(&glx_get_proc_address) = dlsym(libgl, "glXGetProcAddressARB");
}

static void close_libgl(void)
{
	dlclose(libgl);
}

static msh_ogl_proc_t get_proc(const char *proc)
{
	msh_ogl_proc_t res;

	res = glx_get_proc_address((const GLubyte *)proc);
	if (!res)
		*(void **)(&res) = dlsym(libgl, proc);
	return res;
}
#endif

static struct {
	int major, minor;
} version;

static int parse_version(void)
{
	if (!glGetIntegerv)
		return -1;

	glGetIntegerv(GL_MAJOR_VERSION, &version.major);
	glGetIntegerv(GL_MINOR_VERSION, &version.minor);

	if (version.major < 3)
		return -1;
	return 0;
}

static void load_procs(void);

int msh_ogl_init(void)
{
	open_libgl();
	load_procs();
	close_libgl();
	return parse_version();
}

int msh_ogl_is_supported(int major, int minor)
{
	if (major < 3)
		return 0;
	if (version.major == major)
		return version.minor >= minor;
	return version.major >= major;
}

msh_ogl_proc_t msh_ogl_get_proc_address(const char *proc)
{
	return get_proc(proc);
}

''')
    for proc in procs:
        f.write('{0[p_t]: <52} {0[p_s]};\n'.format(proc_t(proc)).encode('utf-8'))
    f.write(br'''
static void load_procs(void)
{
''')
    for proc in procs:
        f.write('\t{0[p_s]} = ({0[p_t]})get_proc("{0[p]}");\n'.format(proc_t(proc)).encode('utf-8'))
    f.write(b'}\n')

    f.write(br'''#endif //MSH_OGL_IMPLEMENTATION''')
