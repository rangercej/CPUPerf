///////////////////////////////////////////////////////////////////////////////
//
// CMonitorLoadAverage
// Calculate running load average on Win32 box
//
///////////////////////////////////////////////////////////////////////////////
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
#include "CMonitorLoadAverage.h"
#include "LinuxLoadAverage.h"
#include <string>

//-----------------------------------------------------------------------------
CMonitorLoadAverage::CMonitorLoadAverage(std::string suffix, CLogger *logger) : CMonitor(suffix, logger)
{
	// Calls base-class constructor
}

//-----------------------------------------------------------------------------
DWORD CMonitorLoadAverage::run()
{
	CPerfMon *pm = NULL;				// Performance counter accessor

	char buf[256];						// Output string buffer

	ULONG runQueueLen;				// Length of run queue as provided by Win32
	ULONG fixedRunQueueLen;			// As above, but converted to fixed precision
	ULONG avenRun[3] = { 0,0,0 };	// Contains load averages at 1, 5 and 15 min intervals
	SYSTEM_INFO si;					// Contains system info

	ULONG count = 0;					// Just a counter that tracks #readings

	mLogger->writeLine ("Starting load average monitor");

	// Find out how many processors the machine has; we divide the run queue length
	// between them all...
	GetSystemInfo (&si);

	// Open the output interface - a memory mapped file
	openOutput("loadaverage", 40);

	if (mOutput != NULL) {

		// Now start process monitoring
		pm = new CPerfMon(mLogger);
		pm->Add("\\System\\Processor Queue Length");

		// Discard first value which just gives a summary
		runQueueLen = pm->GetValue();

		mRunning = true;
		while (mRunning == true) {
			count++;
			runQueueLen = ((pm->GetValue()) / si.dwNumberOfProcessors);
			
			// Convert the run queue length to a fixed-precision number for CALC_LOAD
			fixedRunQueueLen = runQueueLen * FIXED_1;
			
			// This is how Linux does its load average
			CALC_LOAD(avenRun[0], EXP_1, fixedRunQueueLen);
			CALC_LOAD(avenRun[1], EXP_5, fixedRunQueueLen);
			CALC_LOAD(avenRun[2], EXP_15, fixedRunQueueLen);

			// Send to MMIO
			memset (buf, 0, sizeof(buf));
			_snprintf_s (buf, sizeof(buf) - 1, _TRUNCATE, "%d %lu %.2f %.2f %.2f", 
						count, runQueueLen, 
						GET_LA(avenRun[0]), GET_LA(avenRun[1]), GET_LA(avenRun[2]));
			mOutput->writeFile(buf, strlen(buf));

			Sleep(SAMPLE_PERIOD);
		}
	}

	mLogger->writeLine ("Stopping load average monitor");
	closeOutput();

	if (pm) {
		pm->Clear();
		delete pm;
	}
	pm = NULL;

	return 0;
}
