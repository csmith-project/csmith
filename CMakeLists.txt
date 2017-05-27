## -*- mode: CMake -*-
##
## Copyright (c) 2017 The University of Utah
## All rights reserved.
##
## This file is part of `csmith', a random generator of C programs.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions are met:
##
##   * Redistributions of source code must retain the above copyright notice,
##     this list of conditions and the following disclaimer.
##
##   * Redistributions in binary form must reproduce the above copyright
##     notice, this list of conditions and the following disclaimer in the
##     documentation and/or other materials provided with the distribution.
##
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
## AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
## IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
## ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
## LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
## CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
## SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
## INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
## CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
## ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
## POSSIBILITY OF SUCH DAMAGE.

###############################################################################

cmake_minimum_required(VERSION 2.8.12)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(CheckIncludeFile)
include(CheckSymbolExists)
include(GetGitRevisionDescription)

project(csmith)

###############################################################################

# Determine the short git hash for the source tree.  The logic here follows the
# logic in the `git-hash.sh` script.
#
## METHOD 1: The source tree is the result of `git archive'.
# `git archive' inserts the abbreviated hash of the archive's commit into this
# file.  (See the `.gitattributes' file.)
#
set(GIT_HASH "$Format:%h$")
if(GIT_HASH MATCHES "^\\$")
  ## METHOD 2: The source tree is a git repository.
  get_git_head_revision(GIT_REFSPEC GIT_HASH)
  if(NOT GIT_HASH STREQUAL "GITDIR-NOTFOUND")
    # Trim to the short hash.
    string(SUBSTRING "${GIT_HASH}" 0 7 GIT_HASH)
  else()
    ## METHOD 3: Give up.
    set(GIT_HASH "unknown")
  endif()
endif()

# `arc4random' and friends are available on BSD in the standard C library, and
# on Linux in `libbsd'.  We use `arc4random_buf' if it is available, but we do
# not require it.
#
message(STATUS "Check for libbsd")
find_library(BSD_LIBRARY NAMES bsd)
if(BSD_LIBRARY)
  message(STATUS "Check for libbsd: found ${BSD_LIBRARY}")
else()
  message(STATUS "Check for libbsd: not found")
endif()

###############################################################################

# Generate the "config.h" file.
#
check_include_file("bsd/stdlib.h"	HAVE_BSD_STDLIB_H)
check_include_file("dlfcn.h"		HAVE_DLFCN_H)
check_include_file("inttypes.h"		HAVE_INTTYPES_H)
check_include_file("memory.h"		HAVE_MEMORY_H)
check_include_file("stdint.h"		HAVE_STDINT_H)
check_include_file("stdlib.h"		HAVE_STDLIB_H)
check_include_file("strings.h"		HAVE_STRINGS_H)
check_include_file("string.h"		HAVE_STRING_H)
check_include_file("sys/stat.h"		HAVE_SYS_STAT_H)
check_include_file("sys/types.h"	HAVE_SYS_TYPES_H)
check_include_file("unistd.h"		HAVE_UNISTD_H)

set(arc4random_hdrs "stdlib.h")
if(HAVE_BSD_STDLIB_H)
  list(APPEND arc4random_hdrs "bsd/stdlib.h")
endif()
if(BSD_LIBRARY)
  set(CMAKE_REQUIRED_LIBRARIES "${BSD_LIBRARY}")
endif()
check_symbol_exists(arc4random_buf
		    "${arc4random_hdrs}" HAVE_ARC4RANDOM_BUF)
set(CMAKE_REQUIRED_LIBRARIES "")

check_symbol_exists(lrand48
		    "stdlib.h"		HAVE_LRAND48)
check_symbol_exists(srand48_deterministic
		    "stdlib.h"		HAVE_SRAND48_DETERMINISTIC)

set(csmith_PACKAGE			"csmith")
set(csmith_PACKAGE_BUGREPORT		"csmith-bugs@flux.utah.edu")
set(csmith_PACKAGE_NAME			"csmith")
set(csmith_PACKAGE_STRING		"csmith 2.3.0")
set(csmith_PACKAGE_TARNAME		"csmith")
set(csmith_PACKAGE_URL			"http://embed.cs.utah.edu/csmith/")
set(csmith_PACKAGE_VERSION		"2.3.0")
set(csmith_VERSION			"2.3.0")

configure_file("cmake_config.h.in" "${PROJECT_BINARY_DIR}/config.h")
add_definitions("-DHAVE_CONFIG_H")

###############################################################################

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU"
    OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-long-long")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
endif()

# Locate M4, which is needed to create some of the runtime headers.
#
message(STATUS "Check for M4 macro processor")
find_program(M4
  NAMES "m4${CMAKE_EXECUTABLE_SUFFIX}" "m4${CMAKE_EXECUTABLE_SUFFIX}"
  DOC "Location of the M4 macro processor"
  )
if(NOT M4)
  message(FATAL_ERROR
    "The `m4' macro processor was not found.  "
    "You must install `m4' before you can to compile Csmith.")
endif()
message(STATUS "Check for M4 macro processor: found ${M4}")

###############################################################################

add_subdirectory(doc)
add_subdirectory(runtime)
add_subdirectory(scripts)
add_subdirectory(src)

###############################################################################

## End of file.
