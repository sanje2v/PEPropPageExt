#pragma once
#include "stdafx.h"
#include <Windows.h>

/* This file is derived from 'cxxabi.h' provided in GCC.
	These functions will be manually delay loaded as the
	accompanying header had too many external dependencies
	(macros and otherwise) and could only be statically linked.*/

/**
   *  @brief Demangling routine.
   *  ABI-mandated entry point in the C++ runtime library for demangling.
   *
   *  @param __mangled_name A NULL-terminated character string
   *  containing the name to be demangled.
   *
   *  @param __output_buffer A region of memory, allocated with
   *  malloc, of @a *__length bytes, into which the demangled name is
   *  stored.  If @a __output_buffer is not long enough, it is
   *  expanded using realloc.  @a __output_buffer may instead be NULL;
   *  in that case, the demangled name is placed in a region of memory
   *  allocated with malloc.
   *
   *  @param __length If @a __length is non-NULL, the length of the
   *  buffer containing the demangled name is placed in @a *__length.
   *
   *  @param __status @a *__status is set to one of the following values:
   *   0: The demangling operation succeeded.
   *  -1: A memory allocation failure occurred.
   *  -2: @a mangled_name is not a valid name under the C++ ABI mangling rules.
   *  -3: One of the arguments is invalid.
   *
   *  @return A pointer to the start of the NULL-terminated demangled
   *  name, or NULL if the demangling fails.  The caller is
   *  responsible for deallocating this memory using @c free.
   *
   *  The demangling is performed using the C++ ABI mangling rules,
   *  with GNU extensions. For example, this function is used in
   *  __gnu_cxx::__verbose_terminate_handler.
   *
   *  See http://gcc.gnu.org/onlinedocs/libstdc++/manual/bk01pt12ch39.html
   *  for other examples of use.
   *
   *  @note The same demangling functionality is available via
   *  libiberty (@c <libiberty/demangle.h> and @c libiberty.a) in GCC
   *  3.1 and later, but that requires explicit installation (@c
   *  --enable-install-libiberty) and uses a different API, although
   *  the ABI is unchanged.
   */
	typedef char* (*p__cxa_demangle)(const char*, char*, size_t*, int*);
	
	char*
	__cxa_demangle(const char* __mangled_name, char* __output_buffer,
		 size_t* __length, int* __status);