///////////////////////////////////////////////////////////////////////////////
//
// CMonitorCPU
// Monitor CPU usage
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
#include "CMonitorCPU.h"

//-----------------------------------------------------------------------------
CMonitorCPU::CMonitorCPU(std::string suffix, CLogger *logger) : CMonitor(suffix, logger)
{
	// Calls base-class constructor
}

//-----------------------------------------------------------------------------
DWORD CMonitorCPU::run()
{
	CPerfMon *pm = NULL;	// Performance counter accessor
	char buf[256];			// Output string buffer
	v_long results;		// Performance counter results
	ULONG count = 0;		// Just a counter that tracks #readings
	BOOL ok;

	mLogger->writeLine ("Starting CPU monitor");

	// Open the output interface - a memory mapped file
	openOutput("cpu", 40);

	if (mOutput != NULL) {
		// Now start process monitoring
		pm = new CPerfMon(mLogger);
		pm->Add("\\Processor(_Total)\\% Processor Time");
		pm->Add("\\Processor(_Total)\\% Privileged Time");

		// Discard first value which just gives a summary
		ok = pm->GetValues(results);

		mRunning = true;
		while (mRunning == true) {
			count++;
			ok = pm->GetValues(results);

			if (ok) {
				// Send to MMIO
				memset (buf, 0, sizeof(buf));
				_snprintf_s (buf, sizeof(buf) - 1, _TRUNCATE, "%d %lu %lu\n", count, results[0], results[1]);
				mOutput->writeFile(buf, strlen(buf));
			}

			Sleep(SAMPLE_PERIOD);
		}
	}

	mLogger->writeLine ("Stopping CPU monitor");
	closeOutput();

	if (pm) {
		pm->Clear();
		delete pm;
	}
	pm = NULL;

	return 0;
}
