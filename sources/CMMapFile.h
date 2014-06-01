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

#ifndef _CMMAPFILE_H_
#define _CMMAPFILE_H_

#include "stdafx.h"
#include "CLogger.h"
#include <string>

class CMMapFile
{
	public:
		CMMapFile(CLogger *);
		~CMMapFile();
		bool openFile(std::string);
		bool openFile(std::string, DWORD);
		void writeFile(std::string);
		void writeFile(const char *, size_t);
		void closeFile();

	private:
		void showLastError();	// Print the last error to screen

	private:
		HANDLE	mFile;			// The file acting as our MM I/O
		HANDLE	mFileMap;		// Handle to the mapped file 
		LPVOID	mOutput;			// Pointer to the MM file, primed for memcpy()
		DWORD		mMapSize;		// Max size of the file-map
		CLogger* mLogger;			// Logger
};

#endif