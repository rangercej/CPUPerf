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
#ifndef _CMONITOR_H_
#define _CMONITOR_H_

#include "stdafx.h"

#include "CMMapFile.h"
#include "CLogger.h"
#include <process.h>
#include <string>

class CMonitor
{
	public:
		CMonitor(std::string, CLogger *);
		~CMonitor();

		void onAfterConstruct();
		void onBeforeDestruct();

		void	 start();
		void	 stop();
		HANDLE getThreadHandle();

		static unsigned int __stdcall startThread(LPVOID *);

	public:
		virtual DWORD run();

	protected:
		void openOutput(std::string, DWORD);
		void openOutput(std::string);
		void closeOutput();
	
	protected:
		volatile bool	mRunning;		// Running flag
		CMMapFile *		mOutput;			// Pointer to output 
		CLogger *		mLogger;			// Logger
		std::string		mDirName;		// Directory for the output files

		HANDLE			mThreadHandle;
		unsigned int	mThreadID;
};

#endif