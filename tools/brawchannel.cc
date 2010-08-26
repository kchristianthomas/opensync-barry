///
/// \file	brawchannel.cc
///		Directs a named raw channel over STDIN/STDOUT
///

/*
    Copyright (C) 2010, RealVNC Ltd.

        Some parts are inspired from bjavaloader.cc

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


#include <barry/barry.h>
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <getopt.h>
#include <fstream>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>

#include "i18n.h"

using namespace std;
using namespace Barry;

// How long to wait between reads before checking if should shutdown
#define READ_TIMEOUT_SECONDS 1

static volatile bool signalReceived = false;

static void signalHandler(int signum)
{
	signalReceived = true;
}

class CallbackHandler : public Barry::Mode::RawChannelDataCallback
{
private:
	volatile bool* m_continuePtr;
	bool m_verbose;

public:
	CallbackHandler(volatile bool& keepGoing, bool verbose)
		: m_continuePtr(&keepGoing)
		, m_verbose(verbose)
		{
		}

    
public: // From RawChannelDataCallback
	virtual void DataReceived(Data& data);
	virtual void ChannelError(string msg);
	virtual void ChannelClose();
};


void CallbackHandler::DataReceived(Data& data)
{
	if( m_verbose ) {
		cerr << "From BB: ";
		data.DumpHex(cerr);
		cerr << "\n";
	}

	size_t toWrite = data.GetSize();
	size_t written = 0;

	while( written < toWrite && *m_continuePtr ) {
		ssize_t writtenThisTime = write(STDOUT_FILENO, &(data.GetData()[written]), toWrite - written);
		if( m_verbose ) {
			cerr.setf(ios::dec, ios::basefield);
			cerr << "Written " << writtenThisTime << " bytes over stdout" << endl;
		}
		fflush(stdout);
		if( writtenThisTime < 0 ) {
			ChannelClose();
		}
		else {
			written += writtenThisTime;
		}
	}	
}

void CallbackHandler::ChannelError(string msg)
{
	cerr << "CallbackHandler: Received error: " << msg << endl;
	ChannelClose();
}

void CallbackHandler::ChannelClose()
{
	*m_continuePtr = false;
}

// Class which extends the functionality of SocketRoutingQueue to add
// error detection and setting of a continue boolean to false when an
// error is detected.
// This code is heavily based on the thread
// creation code of SocketRoutingQueue, which sadly has too many
// private variables to just sub-class.
class ErrorHandlingSocketRoutingQueue
{
protected:
	static void* ReadThreadFunction(void* userPtr)
	{
		ErrorHandlingSocketRoutingQueue *q = (ErrorHandlingSocketRoutingQueue *)userPtr;

		// read from USB and write to stdout until finished
		string msg;
		while( q->m_runningThread ) {
			if( !q->m_socketRoutingQueue.DoRead(msg,  READ_TIMEOUT_SECONDS * 1000) &&
			    // Only report the first failure, so check m_continuePtr
				*q->m_continuePtr ) {
				cerr << "Error in ReadThread: " << msg << endl;
				*q->m_continuePtr = false;
			}
		}
		return 0;	
	}
		
	SocketRoutingQueue m_socketRoutingQueue;
	volatile bool* m_continuePtr;
	volatile bool m_runningThread;
	pthread_t m_usb_read_thread;
public:
	ErrorHandlingSocketRoutingQueue(volatile bool& continuePtr)
		: m_socketRoutingQueue()
		, m_continuePtr(&continuePtr)
		, m_runningThread(false)
	{
		// Nothing to do
	}

	~ErrorHandlingSocketRoutingQueue()
	{
		// Is the read thread still running
		if( m_runningThread ) {
			m_runningThread = false;
			pthread_join(m_usb_read_thread, NULL);
		}
	}

	// Utility function to make it easier to create the
	// USB pure-read thread.  
	// Throws Barry::ErrnoError on thread creation error.
	void SpinoffReadThread()
	{
		// signal that it's ok to run inside the thread
		if( m_runningThread )
			return;	// already running
		m_runningThread = true;

		// Start USB read thread, to handle all routing
		int ret = pthread_create(&m_usb_read_thread, NULL, &ReadThreadFunction, this);
		if( ret ) {
			m_runningThread = false;
			throw Barry::ErrnoError("SocketRoutingQueue: Error creating USB read thread.", ret);
		}
	}

	SocketRoutingQueue* GetSocketRoutingQueue()
	{
		return &m_socketRoutingQueue;
	}
};

void Usage()
{
	int major, minor;
	const char *Version = Barry::Version(major, minor);

	cerr
		<< "brawchannel - Command line USB Blackberry raw channel interface\n"
		<< "        Copyright 2010, RealVNC Ltd.\n"
		<< "        Using: " << Version << "\n"
		<< "\n"
		<< "Usage:\n"
		<< "brawchannel [options] <channel name>\n"
		<< "\n"
		<< "   -h        This help\n"
		<< "   -p pin    PIN of device to talk with\n"
		<< "             If only one device is plugged in, this flag is optional\n"
		<< "   -P pass   Simplistic method to specify device password\n"
		<< "   -v        Dump protocol data during operation\n"
		<< "             This will cause libusb output to appear on STDOUT unless\n"
		<< "             the environment variable USB_DEBUG is set to 0,1 or 2.\n"
		<< endl;
}

// Helper class to restore signal handlers when shutdown is occuring
// This class isn't responsible for setting up the signal handlers
// as they need to be restored before the Barry::Socket starts closing.
class SignalRestorer
{
private:
	int m_signum;
	sighandler_t m_handler;
public:
	SignalRestorer(int signum, sighandler_t handler)
		: m_signum(signum), m_handler(handler) {}
	~SignalRestorer() { signal(m_signum, m_handler); }
};

int main(int argc, char *argv[])
{
	INIT_I18N(PACKAGE);

	// Setup signal handling
	sighandler_t oldSigHup = signal(SIGHUP, &signalHandler);
	sighandler_t oldSigTerm = signal(SIGTERM, &signalHandler);
	sighandler_t oldSigInt = signal(SIGINT, &signalHandler);
	sighandler_t oldSigQuit = signal(SIGQUIT, &signalHandler);

	cerr.sync_with_stdio(true);	// since libusb uses
					// stdio for debug messages

	// Buffer to hold data read in from STDIN before sending it
	// to the BlackBerry.
	unsigned char* buf = NULL;
	try {
		uint32_t pin = 0;
		bool data_dump = false;
		string password;

		// process command line options
		for( ;; ) {
			int cmd = getopt(argc, argv, "hp:P:v");
			if( cmd == -1 ) {
				break;
			}

			switch( cmd )
			{
			case 'p':	// Blackberry PIN
				pin = strtoul(optarg, NULL, 16);
				break;

			case 'P':	// Device password
				password = optarg;
				break;

			case 'v':	// data dump on
				data_dump = true;
				break;

			case 'h':	// help
			default:
				Usage();
			return 0;
			}
		}

		argc -= optind;
		argv += optind;

		if( argc < 1 ) {
			cerr << "Error: Missing raw channel name." << endl;
			Usage();
			return 1;
		}

		if( argc > 1 ) {
			cerr << "Error: Too many arguments." << endl;
			Usage();
			return 1;
		}

		// Fetch command from remaining arguments
		string channelName = argv[0];
		argc --;
		argv ++;

		
		if( data_dump ) {
			// Warn if USB_DEBUG isn't set to 0, 1 or 2
			// as that usually means libusb will write to STDOUT
			char* val = getenv("USB_DEBUG");
			int parsedValue = -1;
			if( val ) {
				parsedValue = atoi(val);
			}
			if( parsedValue != 0 && parsedValue != 1 && parsedValue != 2 ) {
				cerr << "Warning: Protocol dump enabled without setting USB_DEBUG to 0, 1 or 2.\n"
				     << "         libusb might log to STDOUT and ruin data stream." << endl;
			}	
		}

		// Initialize the barry library.  Must be called before
		// anything else.
		Barry::Init(data_dump, &cerr);

		// Probe the USB bus for Blackberry devices.
		// If user has specified a PIN, search for it in the
		// available device list here as well
		Barry::Probe probe;
		int activeDevice = probe.FindActive(pin);
		if( activeDevice == -1 ) {
			cerr << "No device selected, or PIN not found" << endl;
			return 1;
		}

		// Now get setup to open the channel.
		if( data_dump ) {
			cerr << "Connected to device, starting read/write\n";
		}

		volatile bool running = true;

		// Create the thing which will write onto stdout
		// and perform other callback duties.
		CallbackHandler callbackHandler(running, data_dump);

		// Start a thread to handle any data arriving from
		// the BlackBerry.
		auto_ptr<ErrorHandlingSocketRoutingQueue> router;
		router.reset(new ErrorHandlingSocketRoutingQueue(running));
		router->SpinoffReadThread();

		// Create our controller object
		Barry::Controller con(probe.Get(activeDevice), *router->GetSocketRoutingQueue());

		Barry::Mode::RawChannel rawChannel(con, callbackHandler);

		// Try to open the requested channel now everything is setup
		rawChannel.Open(password.c_str(), channelName.c_str());

		// We now have a thread running to read from the
		// BB and write over stdout; in this thread we'll
		// read from stdin and write to the BB.
		const size_t bufSize = rawChannel.MaximumSendSize();
		buf = new unsigned char[bufSize];
		fd_set rfds;
		struct timeval tv;
		FD_ZERO(&rfds);

		// Set up the signal restorers to restore signal
		// handling (in their destructors) before the socket
		// starts to be closed. This allows, for example,
		// double control-c presses to stop graceful close
		// down.
		SignalRestorer srh(SIGHUP, oldSigHup);
		SignalRestorer srt(SIGTERM, oldSigTerm);
		SignalRestorer sri(SIGINT, oldSigInt);
		SignalRestorer srq(SIGQUIT, oldSigQuit);

		while( running && !signalReceived ) {
			FD_SET(STDIN_FILENO, &rfds);
			tv.tv_sec = READ_TIMEOUT_SECONDS;
			tv.tv_usec = 0;

			int ret = select(1, &rfds, NULL, NULL, &tv);
			if( ret < 0 ) {
				cerr << "Select failed with errno: " << errno << endl;
				running = false;
			}
			else if ( ret && FD_ISSET(STDIN_FILENO, &rfds) ) {
				ssize_t haveRead = read(STDIN_FILENO, buf, bufSize);
				if( haveRead > 0 ) {
					Data toWrite(buf, haveRead);
					if( data_dump ) {
						cerr.setf(ios::dec, ios::basefield);
						cerr << "Sending " << haveRead << " bytes stdin->USB\n";
						cerr << "To BB: ";
						toWrite.DumpHex(cerr);
						cerr << "\n";
					}
					rawChannel.Send(toWrite);
					if( data_dump ) {
						cerr.setf(ios::dec, ios::basefield);
						cerr << "Sent " << ios::dec << haveRead << " bytes stdin->USB\n";
					}
				}
				else if( haveRead < 0 ) {
					running = false;
				}
			}
		}
	}
	catch( Usb::Error &ue ) {
		cerr << "Usb::Error caught: " << ue.what() << endl;
		return 1;
	}
	catch( Barry::Error &se ) {
		cerr << "Barry::Error caught: " << se.what() << endl;
		return 1;
	}
	catch( exception &e ) {
		cerr << "exception caught: " << e.what() << endl;
		return 1;
	}

	delete[] buf;

	return 0;
}
