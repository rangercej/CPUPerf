///////////////////////////////////////////////////////////////////////////////
//
// CMonitor
//
// This is the base class that all monitoring classes should derive from. This
// takes care of starting and monitoring threads etc. so you don't have to do
// anything in your class.
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
#include "CMonitor.h"
#include "CLogger.h"
#include "CMMapFile.h"

#include <process.h>

//-----------------------------------------------------------------------------
CMonitor::CMonitor (std::string outputPath, CLogger *logger)
{
	mRunning = false;
	mDirName = outputPath;
	mLogger = logger;
	mOutput = NULL;
	mThreadHandle = INVALID_HANDLE_VALUE;
}

//-----------------------------------------------------------------------------
CMonitor::~CMonitor()
{
	mRunning = false;
	closeOutput();
	mOutput = NULL;
	mThreadHandle = INVALID_HANDLE_VALUE;
}

//-----------------------------------------------------------------------------
void CMonitor::start()
{
	mThreadHandle = (HANDLE)_beginthreadex (NULL, 0, (unsigned (__stdcall *)(void *))startThread, (LPVOID)this, 0, &mThreadID);
}

//-----------------------------------------------------------------------------
HANDLE CMonitor::getThreadHandle()
{
	return mThreadHandle;
}


//-----------------------------------------------------------------------------
// Allow us to start an asynchronous thread 
unsigned int __stdcall CMonitor::startThread(LPVOID *p)
{
	CMonitor *caller = (CMonitor *)p;

	return caller->run();
}

//-----------------------------------------------------------------------------
DWORD CMonitor::run()
{
	// Default action is to do nothing
	return 0;
}

//-----------------------------------------------------------------------------
void CMonitor::stop()
{
	mRunning = false;
}

//-----------------------------------------------------------------------------
void CMonitor::openOutput (std::string suffix)
{
	openOutput (suffix, 0x100);
}

//-----------------------------------------------------------------------------
void CMonitor::openOutput (std::string suffix, DWORD maxsize)
{
	if (mOutput == NULL) {
		std::string outputFile (mDirName);
		outputFile.append ("\\");
		outputFile.append (suffix);

		mOutput = new CMMapFile(mLogger);
		bool x = mOutput->openFile(outputFile, maxsize);
		if (!x) {
			std::cout << "Failed to open or create output file " << outputFile << std::endl;
			mOutput = NULL;
		}
	}
}

//-----------------------------------------------------------------------------
void CMonitor::closeOutput()
{
	if (mOutput != NULL) {
		mOutput->closeFile();
		delete mOutput;
	}
	mOutput = NULL;
}

