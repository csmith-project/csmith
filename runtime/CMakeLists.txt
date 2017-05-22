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
project(csmith_runtime)

cmake_policy(SET CMP0042 NEW) # Quiet warning about shared libs on OS X.

set(INCLUDE_DIR "${CMAKE_INSTALL_PREFIX}/include/csmith-${csmith_VERSION}")
set(LIB_DIR     "${CMAKE_INSTALL_PREFIX}/lib")

###############################################################################

include_directories(${CMAKE_BINARY_DIR})

# Build and install the static library.

add_library(libcsmith_a STATIC
  ${CMAKE_BINARY_DIR}/config.h
  volatile_runtime.c
  volatile_runtime.h
  )
set_target_properties(libcsmith_a PROPERTIES OUTPUT_NAME "csmith")

install(TARGETS libcsmith_a
  ARCHIVE DESTINATION "${LIB_DIR}"
  )

#####

# Build and install the shared library.

add_library(libcsmith_so SHARED
  ${CMAKE_BINARY_DIR}/config.h
  volatile_runtime.c
  volatile_runtime.h
  )
set_target_properties(libcsmith_so PROPERTIES OUTPUT_NAME "csmith")
# Set library build and API versions to mimic what the Automake-based path
# installs (for Linux).  This is probably not very useful.
set_target_properties(libcsmith_so PROPERTIES VERSION 0.0.0 SOVERSION 0)

# Linux, OS X, FreeBSD et al. look for the LIBRARY specification.
# Windows ("DLL platforms") looks for the RUNTIME specification.
# See, e.g., https://stackoverflow.com/questions/14990343/cmake-error-targets-given-no-library-destination-for-shared-library-target
install(TARGETS libcsmith_so
  LIBRARY DESTINATION "${LIB_DIR}"
  RUNTIME DESTINATION "${LIB_DIR}"
  )

###############################################################################

# Build and install the runtime headers.

set(generated_headers "")
list(APPEND generated_headers
  safe_math.h
  safe_math_macros.h
  safe_math_macros_notmp.h
  )

foreach(hdr IN LISTS generated_headers)
  string(REGEX REPLACE "[.]h$" ".m4" hdr_src ${hdr})
  add_custom_command(
    OUTPUT ${hdr}
    COMMAND ${M4} < ${PROJECT_SOURCE_DIR}/${hdr_src} > ${hdr}
    DEPENDS ${PROJECT_SOURCE_DIR}/${hdr_src}
    VERBATIM
    )
endforeach(hdr)

add_custom_target(
  headers ALL
  DEPENDS ${generated_headers}
  )

install(FILES
  csmith.h
  csmith_minimal.h
  custom_limits.h
  custom_stdint_x86.h
  platform_avr.h
  platform_generic.h
  platform_msp430.h
  random_inc.h
  safe_abbrev.h
  stdint_avr.h
  stdint_ia32.h
  stdint_ia64.h
  stdint_msp430.h
  volatile_runtime.c
  volatile_runtime.h
  DESTINATION "${INCLUDE_DIR}"
  )
foreach(hdr IN LISTS generated_headers)
  install(FILES
    "${PROJECT_BINARY_DIR}/${hdr}"
    DESTINATION "${INCLUDE_DIR}"
    )
endforeach(hdr)
install(FILES
  windows/stdint.h
  DESTINATION "${INCLUDE_DIR}/windows"
  )

###############################################################################

## End of file.
