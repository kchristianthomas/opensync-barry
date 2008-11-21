///
/// \file	barry.h
///		Main header file for applications
///

/*
    Copyright (C) 2005-2008, Net Direct Inc. (http://www.netdirect.ca/)

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

#ifndef __BARRY_BARRY_H__
#define __BARRY_BARRY_H__

/**

	\mainpage Barry Reference Manual


	\section getting_started Getting Started

	Welcome to the Barry reference manual.  This entire manual was
	generated via Doxygen from comments in the code.  You can view
	the code here as well, in the Files section.

	The best place to get started at the moment is to examine the
	source code to the Barry command line tool: btool.cc


	\section classes Major Classes

	To get started with the API, see the Barry::Controller class.

*/


// This lists all the headers that the application needs.
// Only these headers get installed.

#include "data.h"
#include "usbwrap.h"			// to be moved to libusb someday
#include "common.h"			// Init()
#include "error.h"			// exceptions
#include "probe.h"			// device prober class
#include "dataqueue.h"
#include "socket.h"
#include "router.h"
#include "protocol.h"			// application-safe header
#include "parser.h"
#include "builder.h"
#include "ldif.h"
#include "controller.h"
#include "m_desktop.h"
#include "m_ipmodem.h"
#include "m_serial.h"
#include "version.h"
#include "log.h"
#include "sha1.h"
#include "iconv.h"
#include "record.h"

// Include all parser classes, to make it easy for the application to use.
#include "r_calendar.h"
#include "r_contact.h"
#include "r_memo.h"
#include "r_message.h"
#include "r_servicebook.h"
#include "r_task.h"
#include "r_pin_message.h"
#include "r_saved_message.h"
#include "r_folder.h"
#include "r_timezone.h"

// Include the template helpers after the record classes
#include "m_desktoptmpl.h"

#ifdef __BARRY_BOOST_MODE__
// Boost serialization seems to be picky about header order, do them all here
#include <iomanip>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/archive_exception.hpp>
#include "s11n-boost.h"
#endif

#endif

