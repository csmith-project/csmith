// -*- mode: C++ -*-
//
// Narrow standard library aliases for headers.
// Prefer these over `using namespace std;` to avoid importing all names.

#ifndef STDLIB_ALIASES_H
#define STDLIB_ALIASES_H

#include <cstddef>
#include <fstream>
#include <iosfwd>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using std::endl;
using std::fstream;
using std::ifstream;
using std::istringstream;
using std::istream;
using std::map;
using std::multimap;
using std::ostream;
using std::ostringstream;
using std::ofstream;
using std::pair;
using std::set;
using std::size_t;
using std::string;
using std::stringstream;
using std::vector;
using std::cerr;
using std::cin;
using std::clog;
using std::cout;

#endif // STDLIB_ALIASES_H
