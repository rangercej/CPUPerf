///////////////////////////////////////////////////////////////////////////////
//
// CMonitorUptime
// Calculate running load average on Win32 box
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
#include "CPerfMon.h"
#include "CMonitorUptime.h"

//-----------------------------------------------------------------------------
CMonitorUptime::CMonitorUptime(std::string suffix, CLogger *logger) : CMonitor(suffix, logger)
{
	// Calls base-class constructor
}

//-----------------------------------------------------------------------------
DWORD CMonitorUptime::run()
{
	CPerfMon *pm = NULL;	// Performance counter accessor
	char buf[256];			// Output string buffer
	ULONG uptime;			// Length of run queue as provided by Win32
	ULONG count = 0;		// Just a counter that tracks #readings

	mLogger->writeLine ("Starting uptime monitor");
	
	// Open the output interface - a memory mapped file
	openOutput("uptime", 40);

	if (mOutput != NULL) {

		// Now start process monitoring
		pm = new CPerfMon(mLogger);
		pm->Add("\\System\\System Up Time");

		// Discard first value which just gives a summary
		uptime = pm->GetValue();

		mRunning = true;
		while (mRunning == true) {
			count++;
			uptime = pm->GetValue();

			// Send to MMIO
			memset (buf, 0, sizeof(buf));
			_snprintf_s (buf, sizeof(buf) - 1, _TRUNCATE, "%d %lu\n", count, uptime);
			mOutput->writeFile(buf, strlen(buf));

			Sleep(SAMPLE_PERIOD);
		}
	}

	mLogger->writeLine ("Stopping uptime monitor");
	closeOutput();

	if (pm) {
		pm->Clear();
		delete pm;
	}
	pm = NULL;

	return 0;
}
