///////////////////////////////////////////////////////////////////////////////
//
// CMMapFile
// Write to a memory-mapped file
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
#include "CMMapFile.h"
#include <sstream>
#include <string>

//-----------------------------------------------------------------------------
CMMapFile::CMMapFile(CLogger *logger)
{
	mFile = INVALID_HANDLE_VALUE;
	mFileMap = INVALID_HANDLE_VALUE;
	mOutput = NULL;
	mLogger = logger;
}

//-----------------------------------------------------------------------------
CMMapFile::~CMMapFile()
{
	closeFile();
}

//-----------------------------------------------------------------------------
bool CMMapFile::openFile(std::string filename)
{
	return (openFile (filename, 0x100));
}


//-----------------------------------------------------------------------------
bool CMMapFile::openFile(std::string filename, DWORD maxSize)
{
	bool retval = true;

	// Close any existing file handled by this class instance
	closeFile();

	mFile = CreateFile (filename.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, 
				CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING, NULL);

	if (mFile == INVALID_HANDLE_VALUE) {
		showLastError();
		retval = false;
	} else {
		mMapSize = maxSize;
		mFileMap = CreateFileMapping (mFile, NULL, PAGE_READWRITE, 0, mMapSize, NULL);
		if (mFileMap == INVALID_HANDLE_VALUE) {
			showLastError();
			retval = false;
		} else {
			mOutput = MapViewOfFile (mFileMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
			if (mOutput == NULL) {
				showLastError();
				retval = false;
			}
		}
	}

	return retval;
}

//-----------------------------------------------------------------------------
void CMMapFile::writeFile(std::string s)
{
	writeFile (s.c_str(), s.length());
}

//-----------------------------------------------------------------------------
void CMMapFile::writeFile(const char *s, size_t len)
{
	int ok;

	if (mOutput != NULL) {
		memset (mOutput, 0, mMapSize);
		memcpy (mOutput, s, len);
		ok = FlushViewOfFile (mOutput, 0);
		if (!ok) {
			showLastError();
		}
	}
}

//-----------------------------------------------------------------------------
void CMMapFile::closeFile()
{
	if (mOutput != NULL) {
		UnmapViewOfFile (mOutput);
	}
	if (mFileMap != NULL) {
		CloseHandle (mFileMap);
	}
	if (mFile != NULL) {
		CloseHandle (mFile);
	}
	mFile = INVALID_HANDLE_VALUE;
	mFileMap = INVALID_HANDLE_VALUE;
	mOutput = NULL;
}

//-----------------------------------------------------------------------------
void CMMapFile::showLastError()
{
	LPVOID lpMsgBuf;
	DWORD err;

	err = GetLastError();
	std::ostringstream errMsg;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER
					| FORMAT_MESSAGE_FROM_SYSTEM 
					| FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,	err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf, 0, NULL);

	errMsg << "CCMapFile: recieved an error: " << (LPTSTR)lpMsgBuf;
	mLogger->writeLine (errMsg.str());

	LocalFree (lpMsgBuf);
}
