///
/// \file	usbwrap.h
///		USB API wrapper
///

/*
    Copyright (C) 2005-2011, Chris Frey
    Portions Copyright (C) 2011, RealVNC Ltd.

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


#ifndef __SB_USBWRAP_H__
#define __SB_USBWRAP_H__

#include "dll.h"

#include <memory>
#include <vector>
#include <map>
#include "error.h"

#define USBWRAP_DEFAULT_TIMEOUT	30000

#define VENDOR_RIM		0x0fca
#define PRODUCT_RIM_BLACKBERRY	0x0001
#define PRODUCT_RIM_PEARL_DUAL	0x0004
#define PRODUCT_RIM_PEARL_FLIP	0x8001		// 8200 series
#define PRODUCT_RIM_PEARL_8120	0x8004
#define PRODUCT_RIM_PEARL	0x0006
#define PRODUCT_RIM_STORM	0x8007
// Matches any product ID when calling DeviceList::MatchDevices
#define PRODUCT_ANY             0x10000
// Indicates an unknown product ID
#define PRODUCT_UNKNOWN         0x20000

#define BLACKBERRY_INTERFACE		0
#define BLACKBERRY_CONFIGURATION	1
// USB interface classes
#define BLACKBERRY_MS_CLASS             8
#define BLACKBERRY_DB_CLASS		0xff

#define IPRODUCT_RIM_HANDHELD		2
#define IPRODUCT_RIM_MASS_STORAGE	4
#define IPRODUCT_RIM_COMPOSITE		5

namespace Barry { class Data; }

/// Namespace for the libusb-related wrapper classes.  This namespace
/// may change in the future.
namespace Usb {

/// \addtogroup exceptions
/// @{

/// Thrown on low level USB errors.
class BXEXPORT Error : public Barry::Error
{
	int m_errcode;

public:
	Error(const std::string &str);
	Error(int errcode, const std::string &str);

	// can return 0 in some case, if unknown error code
	int errcode() const { return m_errcode; }
};

class BXEXPORT Timeout : public Error
{
public:
	Timeout(const std::string &str) : Error(str) {}
	Timeout(int errcode, const std::string &str)
		: Error(errcode, str) {}
};

/// @}

// Private struct for holding library specific
// a unique identifier to a connected device.
struct DeviceID;

// Private struct for holding a library specific
// device handle
struct DeviceHandle;

// Static functions for setting up USB
// The interface that usbwrap.cc uses
// to interact with the USB library
class BXEXPORT LibraryInterface
{
public:
	static std::string GetLastErrorString(int errcode);
	static int Init();
	static void Uninit();
	static void SetDataDump(bool data_dump_mode);
	static const char* BusName(DeviceID* dev);
	static uint16_t DeviceNumber(DeviceID* dev);
	static const char* FileName(DeviceID* dev);
	static uint16_t DeviceIdProduct(DeviceID* dev);
};

// Forward declaration of descriptor types.
class BXEXPORT DeviceDescriptor;
class BXEXPORT ConfigDescriptor;
class BXEXPORT InterfaceDescriptor;

// Private struct for holding library specific
// information about endpoint descriptors
struct EndpointDescriptorImpl;

// Encapsulates an endpoint descriptor
class BXEXPORT EndpointDescriptor
{
public:
	enum EpType
	{
		ControlType = 0,
		IsochronousType = 1,
		BulkType = 2,
		InterruptType = 3,
		InvalidType = 0xff
	};
private:
	const std::auto_ptr<EndpointDescriptorImpl> m_impl;
	bool m_read;
	uint8_t m_addr;
	EpType m_type;
public:
	EndpointDescriptor(InterfaceDescriptor& dev, int endpoint);
	~EndpointDescriptor();
	bool IsRead() const;
	uint8_t Address() const;
	EpType Type() const;
};

// Private struct for holding library specific
// information about interface descriptors
struct InterfaceDescriptorImpl;

// Encapsulates an interface descriptor
//
// The inherited vector methods look up endpoints
class BXEXPORT InterfaceDescriptor : public std::vector<EndpointDescriptor*>
{
	friend class EndpointDescriptor;
public:
	typedef std::vector<EndpointDescriptor*> base_type;
private:
	const std::auto_ptr<InterfaceDescriptorImpl> m_impl;
public:
	InterfaceDescriptor(ConfigDescriptor& cfgdesc,
			    int interface, int altsetting);
	~InterfaceDescriptor();
	uint8_t Class() const;
	uint8_t Number() const;
};

// Private struct for holding library specific
// information about config descriptors

struct ConfigDescriptorImpl;

// Encapsulates a configuration descriptor
//
// The inherited map methods look up interface descriptors
class BXEXPORT ConfigDescriptor : public std::map<int, InterfaceDescriptor*>
{
	friend class InterfaceDescriptor;
public:
	typedef std::map<int, InterfaceDescriptor*> base_type;
private:
	const std::auto_ptr<ConfigDescriptorImpl> m_impl;
public:
	ConfigDescriptor(DeviceDescriptor& dev, int cfgnumber);
	~ConfigDescriptor();
	uint8_t Number() const;
};

// Private struct for holding library specific
// information about a device descriptor
struct DeviceDescriptorImpl;

// Encapsulates a device descriptor
//
// The inherited map methods look up config descriptors
class BXEXPORT DeviceDescriptor : public std::map<int, ConfigDescriptor*>
{
	friend class ConfigDescriptor;
public:
	typedef std::map<int, ConfigDescriptor*> base_type;
private:
	const std::auto_ptr<DeviceDescriptorImpl> m_impl;
public:
	DeviceDescriptor(DeviceID* devid);
	~DeviceDescriptor();
};

// Private struct for holding library specific
// information for devices.
struct DeviceListImpl;

class BXEXPORT DeviceList
{
private:
	// Private implementation structure
	const std::auto_ptr<DeviceListImpl> m_impl;
protected:
	static bool ToNum(const char *str, long &num);
	static bool NameCompare(const char *n1, const char *n2);
public:
	DeviceList();
	~DeviceList();

	std::vector<DeviceID*> MatchDevices(int vendor, int product,
					    const char *busname, const char *devname);

};

struct PrivateDeviceData;

class BXEXPORT Device
{
private:
	Usb::DeviceID* m_id;
	std::auto_ptr<Usb::DeviceHandle> m_handle;

	int m_timeout;
	int m_lasterror;

public:
	Device(Usb::DeviceID *id, int timeout = USBWRAP_DEFAULT_TIMEOUT);
	~Device();

	/////////////////////////////
	// Data access

	const Usb::DeviceID* GetID() const { return &*m_id; }
	const Usb::DeviceHandle* GetHandle() const { return &*m_handle; }
	int GetLastError() const { return m_lasterror; } //< not thread safe...
		//< use the error code stored in the exceptions to track
		//< errors in threaded usage
	void SetLastError(int err) { m_lasterror = err; }
	int GetDefaultTimeout() const { return m_timeout; }

	/////////////////////////////
	// Device information

	int PowerLevel();
	int FindInterface(int ifaceClass);

	/////////////////////////////
	// Device manipulation

	bool SetConfiguration(unsigned char cfg);
	bool ClearHalt(int ep);
	bool Reset();
	bool DetachKernelDriver(int iface);

	/////////////////////////////
	// IO functions

	bool BulkRead(int ep, Barry::Data &data, int timeout = -1);
	bool BulkWrite(int ep, const Barry::Data &data, int timeout = -1);
	bool BulkWrite(int ep, const void *data, size_t size, int timeout = -1);
	bool InterruptRead(int ep, Barry::Data &data, int timeout = -1);
	bool InterruptWrite(int ep, const Barry::Data &data, int timeout = -1);

	void BulkDrain(int ep, int timeout = 100);

	bool ControlMsg(int requesttype, int request, int value,
			int index, char *bytes, int size, int timeout);

	/////////////////////////////
	// Combo functions

	bool GetConfiguration(unsigned char &cfg);
};

class BXEXPORT Interface
{
	Device &m_dev;
	int m_iface;
public:
	Interface(Device &dev, int iface);
	~Interface();
	bool SetAltInterface(int altSetting);
};

// Map of Endpoint numbers (not indexes) to endpoint descriptors
struct BXEXPORT EndpointPair
{
	unsigned char read;
	unsigned char write;
	EndpointDescriptor::EpType type;

	EndpointPair();
	bool IsTypeSet() const;
	bool IsComplete() const;
	bool IsBulk() const;
};

class BXEXPORT EndpointPairings : public std::vector<EndpointPair>
{
public:
	typedef std::vector<EndpointPair> base_type;
private:
	bool m_valid;
public:
	EndpointPairings(const std::vector<EndpointDescriptor*>& eps);
	~EndpointPairings();
	bool IsValid() const;
};

class BXEXPORT Match
{
private:
	std::vector<DeviceID*> m_list;
	std::vector<DeviceID*>::iterator m_iter;
public:
	// Due to USB libraries having different ownership ideas
	// about device IDs, Match objects must be constructed
	// with a device list.
	Match(DeviceList& devices,
	      int vendor, int product,
	      const char *busname = 0, const char *devname = 0);	
	~Match();

	// searches for next match, and if found, fills devid with
	// something you can pass on to DeviceDiscover, etc
	// returns true if next is found, false if no more
	bool next_device(Usb::DeviceID*& devid);
};

}

#endif
