///
/// \file	m_raw_channel.h
///		Mode class for a raw channel
///

/*
    Copyright (C) 2005-2010, Net Direct Inc. (http://www.netdirect.ca/)
    Portions Copyright (C) 2010 RealVNC Ltd.

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

#ifndef __BARRY_M_RAW_CHANNEL_H__
#define __BARRY_M_RAW_CHANNEL_H__

#include "dll.h"
#include "m_mode_base.h"
#include "socket.h"

#include <string>
#include <pthread.h>

namespace Barry {

class semaphore;

namespace Mode {

// Callback from the raw channel.

class BXEXPORT RawChannelDataCallback
{
public:
	// Called when data has been received on the channel
	virtual void DataReceived(Data& data) = 0;
	// Called when the channel has an error
	virtual void ChannelError(std::string msg) = 0;
	// Called when the channel has been asked to close by the other side
	virtual void ChannelClose() = 0;
};

//
// Raw channel class
//
/// The main class for creating a raw channel session.
///
/// To use this class, use the following steps:
///
/// - Implement RawChannelDataCallback
///	- Create a Controller object (see Controller class for more details)
///	- Create this Mode::RawChannel object, passing in the Controller
///		object during construction
///	- Call Open() to open the channel and finish constructing.
///	- Call GetData() to fetch data
///	- Call SendData() to send data
///
class BXEXPORT RawChannel : public Mode
{
	// Mutex for signalling between read and write threads
	pthread_mutex_t m_mutex;
	bool m_mutex_valid;
	// Condvar for signalling between read and write threads
	pthread_cond_t m_cv;
	bool m_cv_valid;
	
	semaphore* m_semaphore;
	RawChannelDataCallback& m_callback;
	unsigned char *m_sendBuffer;
	bool m_zeroRegistered;

protected:
	void UnregisterZeroSocketInterest();

public:
	RawChannel(Controller &con, RawChannelDataCallback& callback);
	virtual ~RawChannel();

	//////////////////////////////////
	// Raw channel mode specific methods

	// Send some data on the raw channel.
	// Will throw a Barry::Error if data is longer than
	// MaximumPacketContentsSize or a Barry::Usb::Error if there
	// is an underlying USB error.
	void Send(Data& data, int timeout = -1);

	// Returns the maximum quantity of data which
	// can be sent
	size_t MaximumSendSize();

	// Not intended for use by users of this class.
	// Instead data received will come in via the 
	// RawChannelDataCallback::DataReceived callback.
	void HandleReceivedData(Data& data);

	// Not intended for use by users of this class.
	// This method is called by the internals of
	// Barry when setting up a connection.
	void OnOpen();
};

}} // namespace Barry::Mode

#endif
