///
/// \file	os40.h
///		Wrapper class for opensync 0.4x syncing behaviour
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

#ifndef __BARRYDESKTOP_OS40_H__
#define __BARRYDESKTOP_OS40_H__

#include "dlopen.h"
#include <memory>
#include <vector>
#include <string>
#include <iosfwd>

class OpenSync40Private;

typedef std::vector<std::string>		string_list_type;

std::ostream& operator<< (std::ostream &os, const string_list_type &list);

class OpenSync40 : public DlOpen
{
public:

private:
	// private opensync 0.40 function pointers and data
	OpenSync40Private *m_priv;

protected:
	void SetupEnvironment(OpenSync40Private *p);

public:
	OpenSync40();
	~OpenSync40();

	// general tool API (based on command line options from osynctool)
	const char* GetVersion();
	void GetPluginNames(string_list_type &plugins);
};

#endif

