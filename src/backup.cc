///
/// \file	backup.cc
///		Special parser class to support creation of Barry Backup files
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

#include "backup.h"
#include "tarfile.h"
#include "error.h"
#include <sstream>
#include <iomanip>
#include <iostream>

namespace Barry {

Backup::Backup(const std::string &tarpath)
{
	try {
		m_tar.reset( new reuse::TarFile(tarpath.c_str(), true,
				&reuse::gztar_ops_nonthread, true) );
	}
	catch( reuse::TarFile::TarError &te ) {
		throw Barry::BackupError(te.what());
	}
}

Backup::~Backup()
{
	std::cout << "Closing Backup parser" << std::endl;
	try {
		Close();
	}
	catch( Barry::BackupError & ) {
		// throw it away
	}
}

void Backup::Close()
{
	if( m_tar.get() ) try {
		m_tar->Close();
		m_tar.reset();
	}
	catch( reuse::TarFile::TarError &te ) {
		throw Barry::BackupError(te.what());
	}
}


//////////////////////////////////////////////////////////////////////////////
// Barry::Parser overrides

void Backup::Clear()
{
}

void Backup::SetIds(const std::string &DbName,
		    uint8_t RecType,
		    uint32_t UniqueId)
{
	m_current_dbname = DbName;

	std::ostringstream oss;
	oss << std::hex << UniqueId << " " << (unsigned int)RecType;
	m_tar_id_text = oss.str();

	if( m_current_dbname.size() == 0 )
		throw Barry::BackupError("Backup: No database name available");
	if( m_tar_id_text.size() == 0 )
		throw Barry::BackupError("Backup: No unique ID available!");
}

void Backup::ParseHeader(const Barry::Data &data, size_t &offset)
{
}

void Backup::ParseFields(const Barry::Data &data,
				  size_t &offset,
				  const Barry::IConverter *ic)
{
	m_record_data.assign((const char*)data.GetData() + offset, data.GetSize() - offset);
}

void Backup::Store()
{
	std::string tarname = m_current_dbname + "/" + m_tar_id_text;
std::cout << "Saving: " << tarname << std::endl;
	m_tar->AppendFile(tarname.c_str(), m_record_data);
}

} // namespace Barry

