///
/// \file	mimeio.cc
///		Storage, parser, builder classes for MIME objects
///		(vcard, vevent, vtodo, vjournal)
///

/*
    Copyright (C) 2010, Net Direct Inc. (http://www.netdirect.ca/)

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

#include "mimeio.h"
#include "vcard.h"
#include "vevent.h"
#include "vtodo.h"
#include "vjournal.h"
#include <iostream>
#include <fstream>
#include <strings.h>

using namespace std;

namespace Barry {

MimeBuilder::MimeBuilder(const std::string &filename)
	: m_ifs( new std::ifstream(filename.c_str()) )
	, m_is(*m_ifs)
{
}

MimeBuilder::MimeBuilder(std::istream &is)
	: m_is(is)
{
}

bool MimeBuilder::BuildRecord(DBData &data, size_t &offset,
				const IConverter *ic)
{
	string vrec, type;
	while( ReadMimeRecord(m_is, vrec, type) ) {
		if( !vrec.size() ) {
			// end of file
			return false;
		}
		else if( strcasecmp(type.c_str(), Sync::vCard::GetVName()) == 0 ) {
			Sync::vCard vcard;
			SetDBData(vcard.ToBarry(vrec.c_str(), 0),
				data, offset, ic);
			return true;
		}
		else if( strcasecmp(type.c_str(), Sync::vCalendar::GetVName()) == 0 ) {
			Sync::vTimeConverter vtc;
			Sync::vCalendar vcal(vtc);
			SetDBData(vcal.ToBarry(vrec.c_str(), 0),
				data, offset, ic);
			return true;
		}
		else if( strcasecmp(type.c_str(), Sync::vTodo::GetVName()) == 0 ) {
			Sync::vTimeConverter vtc;
			Sync::vTodo vtodo(vtc);
			SetDBData(vtodo.ToBarry(vrec.c_str(), 0),
				data, offset, ic);
			return true;
		}
		else if( strcasecmp(type.c_str(), Sync::vJournal::GetVName()) == 0 ) {
			Sync::vJournal vjournal;
			SetDBData(vjournal.ToBarry(vrec.c_str(), 0),
				data, offset, ic);
			return true;
		}
		else {
			// read the next one
		}
	}

	// end of file
	return false;
}

bool MimeBuilder::FetchRecord(DBData &data, const IConverter *ic)
{
	size_t offset = 0;
	return BuildRecord(data, offset, ic);
}

bool MimeBuilder::EndOfFile() const
{
	return !m_is;
}

// return false at end of file, true if a record was read
bool MimeBuilder::ReadMimeRecord(std::istream &is, std::string &vrec,
				std::string &type)
{
	vrec.clear();
	type.clear();

	string line;

	// ignore whitespace
	while( getline(is, line) ) {
		if( strcasecmp(line.substr(0, 6).c_str(), "BEGIN:") == 0 ) {
			vrec += line;
			vrec += "\n";
			type = line.substr(6);
			break;
		}
	}

	if( !vrec.size() )
		return false;

	// load until end
	int count = 0;
	while( getline(is, line) ) {
		// end on blank lines
		if( !line.size() )
			return true;

		vrec += line;
		vrec += "\n";

		// pick up innermost BEGIN line
		if( strcasecmp(line.substr(0, 6).c_str(), "BEGIN:") == 0 ) {
			type = line.substr(6);
			while( type.size() && type[type.size()-1] == '\r' ) {
				type = type.substr(0, type.size()-1);
			}
		}

		// place an upper limit on the number of lines...
		// since a MIME data block shouldn't be more than
		// a few pages of lines!
		count++;
		if( count > 200 )
			return false;
	}
	// assume that end of file is the same as "blank line"
	return true;
}

} // namespace Barry

