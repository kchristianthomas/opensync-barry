///
/// \file	parser.cc
///		Virtual parser wrapper
///

/*
    Copyright (C) 2005-2010, Net Direct Inc. (http://www.netdirect.ca/)

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

#include "parser.h"
#include "r_calendar.h"
#include "r_calllog.h"
#include "r_bookmark.h"
#include "r_contact.h"
#include "r_memo.h"
#include "r_message.h"
#include "r_servicebook.h"
#include "r_task.h"
#include "r_pin_message.h"
#include "r_saved_message.h"
#include "r_sms.h"
#include "r_folder.h"
#include "r_timezone.h"

#include <iostream>
#include <memory>

#define ALL_KNOWN_RECORD_TYPES \
	HANDLE_TYPE(Contact) \
	HANDLE_TYPE(Message) \
	HANDLE_TYPE(Calendar) \
	HANDLE_TYPE(CalendarAll) \
	HANDLE_TYPE(CallLog) \
	HANDLE_TYPE(Bookmark) \
	HANDLE_TYPE(ServiceBook) \
	HANDLE_TYPE(Memo) \
	HANDLE_TYPE(Task) \
	HANDLE_TYPE(PINMessage) \
	HANDLE_TYPE(SavedMessage) \
	HANDLE_TYPE(Sms) \
	HANDLE_TYPE(Folder) \
	HANDLE_TYPE(Timezone)

namespace Barry {

//////////////////////////////////////////////////////////////////////////////
// HexDumpParser class

HexDumpParser::HexDumpParser(std::ostream &os)
	: m_os(os)
{
}

void HexDumpParser::ParseRecord(const Barry::DBData &data,
				const IConverter *ic)
{
	m_os << "Raw record dump for record: "
		<< std::hex << data.GetUniqueId() << std::endl;
	m_os << data.GetData() << std::endl;
}


//////////////////////////////////////////////////////////////////////////////
// MultiRecordParser class

// takes ownership of default_parser!
MultiRecordParser::MultiRecordParser(Parser *default_parser)
	: m_default(default_parser)
{
}

MultiRecordParser::~MultiRecordParser()
{
	map_type::iterator i = m_parsers.begin();
	for( ; i != m_parsers.end(); ++i ) {
		delete i->second;
	}

	// and the default parser
	delete m_default;
}

void MultiRecordParser::Add(const std::string &dbname, Parser *parser)
{
	std::auto_ptr<Parser> p(parser);

	map_type::iterator i = m_parsers.find(dbname);
	if( i != m_parsers.end() ) {
		// found existing parser, so delete it first
		delete i->second;

		// assign it
		i->second = p.release();
	}
	else {
		m_parsers[dbname] = p.get();
		p.release();
	}
}

// takes ownership of parser!
void MultiRecordParser::Add(RecordParserBase *parser)
{
	std::auto_ptr<Parser> p(parser);
	std::string name = parser->GetDBName();
	Add(name, p.release());
}

bool MultiRecordParser::Add(const std::string &dbname,
				std::ostream &os)
{
	std::auto_ptr<Parser> p;

#undef HANDLE_TYPE
#define HANDLE_TYPE(tname) if( dbname == tname::GetDBName() ) { p.reset( new RecordParser<tname, DumpStore<tname> > (new DumpStore<tname>(os)) ); }

	// check for recognized database names
	ALL_KNOWN_RECORD_TYPES

	if( !p.get() ) {
		// name not known
		return false;
	}

	Add(dbname, p.release());
	return true;
}

// Parser overrides
void MultiRecordParser::ParseRecord(const DBData &data, const IConverter *ic)
{
	// search for a named parser
	map_type::iterator i = m_parsers.find(data.GetDBName());
	if( i != m_parsers.end() ) {
		// found one, use it
		i->second->ParseRecord(data, ic);
	}
	else if( m_default ) {
		// use default parser
		m_default->ParseRecord(data, ic);
	}
}


//////////////////////////////////////////////////////////////////////////////
// AllRecordDumpStore class

void AllRecordDumpStore::operator() (const Barry::Contact &r)
{
	m_os << r << std::endl;
}

void AllRecordDumpStore::operator() (const Barry::Message &r)
{
	m_os << r << std::endl;
}

void AllRecordDumpStore::operator() (const Barry::Calendar &r)
{
	m_os << r << std::endl;
}

void AllRecordDumpStore::operator() (const Barry::CalendarAll &r)
{
	m_os << r << std::endl;
}

void AllRecordDumpStore::operator() (const Barry::CallLog &r)
{
	m_os << r << std::endl;
}

void AllRecordDumpStore::operator() (const Barry::Bookmark &r)
{
	m_os << r << std::endl;
}

void AllRecordDumpStore::operator() (const Barry::ServiceBook &r)
{
	m_os << r << std::endl;
}

void AllRecordDumpStore::operator() (const Barry::Memo &r)
{
	m_os << r << std::endl;
}

void AllRecordDumpStore::operator() (const Barry::Task &r)
{
	m_os << r << std::endl;
}

void AllRecordDumpStore::operator() (const Barry::PINMessage &r)
{
	m_os << r << std::endl;
}

void AllRecordDumpStore::operator() (const Barry::SavedMessage &r)
{
	m_os << r << std::endl;
}

void AllRecordDumpStore::operator() (const Barry::Sms &r)
{
	m_os << r << std::endl;
}

void AllRecordDumpStore::operator() (const Barry::Folder &r)
{
	m_os << r << std::endl;
}

void AllRecordDumpStore::operator() (const Barry::Timezone &r)
{
	m_os << r << std::endl;
}


//////////////////////////////////////////////////////////////////////////////
// AllRecordDumpParser class

AllRecordParser::AllRecordParser(std::ostream &os,
				Parser *default_parser,
				AllRecordStore *store)
	: MultiRecordParser(default_parser)
	, m_store(store)
{
#undef HANDLE_TYPE
#define HANDLE_TYPE(tname) if( m_store ) { Add( new RecordParser<tname, AllRecordStore>(*m_store)); } else { Add(tname::GetDBName(), os); }

	ALL_KNOWN_RECORD_TYPES;
}

AllRecordParser::~AllRecordParser()
{
	delete m_store;
}


//////////////////////////////////////////////////////////////////////////////
// TeeParser class

TeeParser::TeeParser()
{
}

TeeParser::~TeeParser()
{
	// free all the owned parser pointers
	for( parser_list_type::iterator i = m_owned_parsers.begin();
		i != m_owned_parsers.end();
		++i )
	{
		delete *i;
	}
}

// takes ownership of the pointer!
void TeeParser::Add(Parser *p)
{
	std::auto_ptr<Parser> ap(p);
	m_owned_parsers.push_back(ap.get());
	ap.release();
}

// does NOT take ownership
void TeeParser::Add(Parser &p)
{
	m_external_parsers.push_back(&p);
}

void TeeParser::ParseRecord(const DBData &data, const IConverter *ic)
{
	// call all owned parsers
	for( parser_list_type::iterator i = m_owned_parsers.begin();
		i != m_owned_parsers.end();
		++i )
	{
		(*i)->ParseRecord(data, ic);
	}

	// call all external parsers
	for( parser_list_type::iterator i = m_external_parsers.begin();
		i != m_external_parsers.end();
		++i )
	{
		(*i)->ParseRecord(data, ic);
	}
}

} // namespace Barry

