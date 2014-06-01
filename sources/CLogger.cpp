///////////////////////////////////////////////////////////////////////////////
//
// CLogger
// A logging class for CPUPerf. Hopefully it'll be thread-safe(!)
//
///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006 Chris Johnson, Sheffield, UK <cej@nightwolf.org.uk>
// All rights reserved.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or (at
// your option) any later version.
// 
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
//
///////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "CLogger.h"
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <strstream>

//-----------------------------------------------------------------------------
CLogger::CLogger (std::string cpuPerfPath, bool serviceFlag)
{
	mLogFileName = cpuPerfPath;
	mLogFileName.append ("\\messages");

	mMutex = NULL;
	mLogFile = NULL;
	mServiceFlag = serviceFlag;
}

//-----------------------------------------------------------------------------
CLogger::~CLogger()
{
	if (mLogFile) {
		mLogFile->close();
	}
	mLogFile = NULL;

	if (mMutex) {
		CloseHandle (mMutex);
	}
	mMutex = NULL;
}

//-----------------------------------------------------------------------------
bool CLogger::init()
{
	bool retval = true;
	std::ofstream outFile;

	if (mServiceFlag) {
		mLogFile = new std::ofstream();
		mLogFile->open (mLogFileName.c_str(), std::ofstream::out | std::ofstream::app);
		if (mLogFile->fail()) {
			retval = false;
		} else {
			mMutex = CreateMutex(NULL, FALSE, NULL);
			if (mMutex == NULL) {
				retval = false;
			}
		}
	}

	return retval;
}
	
//-----------------------------------------------------------------------------
bool CLogger::writeLine(std::string msg)
{
	// Cast required to get this to call 'writeLine (char *)' below, otherwise
	// it recurses into itself.
	return writeLine ((char *)msg.c_str());
}

//-----------------------------------------------------------------------------
bool CLogger::writeLine(char *msg)
{
	char						*months[]	= {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
													 "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	bool						retval		= true;
	BOOL						f;
	DWORD						result;
	struct _SYSTEMTIME	ts;
	char						timestamp[BUFSIZ];

	GetSystemTime(&ts);

	memset (timestamp, 0, sizeof(timestamp));
	_snprintf_s(timestamp, sizeof(timestamp) - 1, _TRUNCATE, "%02d %s %4d %02d:%02d:%02d", ts.wDay, months[ts.wMonth - 1], ts.wYear, ts.wHour, ts.wMinute, ts.wSecond);
	
	if (!mServiceFlag) {
		std::cout << timestamp << " " << msg << std::endl;
	} else {
		if (!mLogFile) {
			retval = false;
		} else {
			result = WaitForSingleObject (mMutex, 5000L);

			switch (result) {
				case WAIT_OBJECT_0:
					(*mLogFile) << timestamp << " " << msg << std::endl;
					f = ReleaseMutex (mMutex);
					if (!f) {
						(*mLogFile) << "Failed to release mutex!! CPUPerf may now hang." << std::endl;
						retval = false;
					}
					break;

				case WAIT_TIMEOUT:
				case WAIT_ABANDONED:
					retval = false;
					break;

				default:
					retval = false;
			}
		}
	}
	return retval;
}
