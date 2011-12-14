#! /bin/sh
#
## Copyright (c) 2011 The University of Utah
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

# git-hash.sh: Return the short git hash for a source tree
#
# usage: git-hash.sh [dir]
#
# This script is intended to be invoked from `make' so that one can insert the
# current git commit hash into compiled programs.  The command-line argument
# should identify the source tree to be checked.  If it is not provided, the
# current directory is assumed to identify the source tree.

###############################################################################

## METHOD 1: The source tree is the result of `git archive'.

# `git archive' inserts the abbreviated hash of the archive's commit into this
# script.  (See the `.gitattributes' file.)
git_hash='$Format:%h$'

if test "${git_hash}" != '$''Format:%h$'; then
  echo "${git_hash}"
  exit 0
fi

###############################################################################

## METHOD 2: The source tree is a git repository.

case $# in
  0) ;;
  1) if test ! -d "$1"; then
       echo "error"
       exit 1
     fi
     cd "$1"
     if test $? -eq 1; then
       echo "error"
       exit 1
     fi;;
  *) echo "error"
     exit 1;;
esac

git_hash=`git show -s --format=%h . 2>/dev/null`
if test $? -eq 0; then
  echo "${git_hash}"
  exit 0
fi

###############################################################################

## METHOD 3: Give up.

echo "unknown"
exit 0

###############################################################################

# End of file.
