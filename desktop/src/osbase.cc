///
/// \file	osbase.cc
///		Base API class helpers
///

/*
    Copyright (C) 2009, Net Direct Inc. (http://www.netdirect.ca/)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the GNU General Public License in the COPYING file at the
    root directory of this project for more details.
*/

#include "osbase.h"
#include <iostream>

using namespace std;

std::ostream& operator<< (std::ostream &os, const string_list_type &list)
{
	string_list_type::const_iterator b = list.begin(), e = list.end();
	for( ; b != e; ++b ) {
		os << *b << endl;
	}
	return os;
}

