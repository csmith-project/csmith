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

using std::cerr;
using std::cin;
using std::clog;
using std::cout;
using std::endl;
using std::fstream;
using std::ifstream;
using std::istream;
using std::istringstream;
using std::map;
using std::multimap;
using std::ofstream;
using std::ostream;
using std::ostringstream;
using std::pair;
using std::set;
using std::size_t;
using std::string;
using std::stringstream;
using std::vector;

#endif // STDLIB_ALIASES_H
