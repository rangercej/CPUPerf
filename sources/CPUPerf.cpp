///////////////////////////////////////////////////////////////////////////////
//
// CPUPerf
// Performance monitor
//
// cej@nightwolf.org.uk, December 2005 -> February 2006
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
#include "CMonitorLoadAverage.h"
#include "CMonitorUptime.h"
#include "CMonitorCPU.h"
#include "CMonitorMemory.h"
#include "CLogger.h"
#include <string>
#include <iostream>
#include <sstream>
#include <windows.h>
#include <winsvc.h>

#define SERVICE_NAME				"CPUPerf"
#define CPUPERF_VERSION			"0.03"

static SERVICE_STATUS			m_ServiceStatus;
static SERVICE_STATUS_HANDLE	m_ServiceStatusHandle;
volatile static DWORD			serviceRunState = SERVICE_STOPPED;

static HANDLE						mCPUPerfThread;
static unsigned int				mCPUPerfThreadID;

//-----------------------------------------------------------------------------
void showLastError()
{
	LPVOID lpMsgBuf;
	DWORD err;

	err = GetLastError();

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER
					| FORMAT_MESSAGE_FROM_SYSTEM 
					| FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,	err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf, 0, NULL);

	std::cout << "Recieved an error: " << (LPCTSTR)lpMsgBuf << std::endl;

	LocalFree (lpMsgBuf);
}

//-----------------------------------------------------------------------------
std::string init()
{
	char systemRoot[1024];

	memset (systemRoot, 0, sizeof(systemRoot));
	GetEnvironmentVariable ("SYSTEMROOT", systemRoot, sizeof(systemRoot) - 1);

	strncat_s (systemRoot, "\\CPUPerf", 8);
	systemRoot[sizeof(systemRoot) - 1] = 0;

	CreateDirectory (systemRoot, NULL);

	return std::string(systemRoot);
}


//-----------------------------------------------------------------------------
BOOL InstallService()
{

	char						strDir[1024];
	SC_HANDLE				schSCManager;
	SC_HANDLE				schService;
	LPCTSTR					lpszBinaryPathName	= strDir;
	BOOL						retval					= false;
	BOOL						result;
	SERVICE_DESCRIPTION	serviceDesc;
 

	memset (strDir, 0, sizeof(strDir));
	GetCurrentDirectory(1000, strDir);
	strncat_s (strDir, "\\CPUPerf.exe", 12); 

	schSCManager = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);  
 
	if (schSCManager == NULL) {
		showLastError();
	} else {
		schService = CreateService(schSCManager, SERVICE_NAME, SERVICE_NAME,           // service name to display 
			SERVICE_ALL_ACCESS,			// desired access 
			SERVICE_WIN32_OWN_PROCESS,	// service type 
			SERVICE_AUTO_START,			// start type 
			SERVICE_ERROR_NORMAL,		// error control type 
			lpszBinaryPathName,			// service's binary 
			NULL,								// no load ordering group 
			NULL,								// no tag identifier 
			NULL,								// no dependencies 
			NULL,								// LocalSystem account 
			NULL);							// no password 
 
		if (schService == NULL) {
			showLastError();
		} else {
			serviceDesc.lpDescription = "Reports various performance counters to regular files in %WINDIR%\\CPUPerf";
			result = ChangeServiceConfig2 (schService, SERVICE_CONFIG_DESCRIPTION, &serviceDesc);
			if (!result) {
				showLastError();
			}

			CloseServiceHandle(schService); 
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
BOOL UnInstallService()
{
	SC_HANDLE	schSCManager;
	SC_HANDLE	hService;
	BOOL			retval = false;

	schSCManager = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
 
	if (schSCManager == NULL) {
		showLastError();
	} else {
		hService=OpenService(schSCManager, SERVICE_NAME, SERVICE_ALL_ACCESS);

		if (hService == NULL) {
			showLastError();
		} else {
			if(DeleteService(hService)==0) {
				showLastError();
			} else {
				if(CloseServiceHandle(hService)==0) {
					showLastError();
				} else {
					retval = true;
				}
			}
		}
	}

	return retval;
}

//-----------------------------------------------------------------------------
void WINAPI ServiceCtrlHandler(DWORD Opcode)
{
	switch(Opcode) 
	{ 
		case SERVICE_CONTROL_PAUSE: 
			m_ServiceStatus.dwCurrentState = SERVICE_PAUSED; 
			break; 

		case SERVICE_CONTROL_CONTINUE: 
			m_ServiceStatus.dwCurrentState = SERVICE_RUNNING; 
			break; 

		case SERVICE_CONTROL_STOP: 
			m_ServiceStatus.dwWin32ExitCode = 0; 
			m_ServiceStatus.dwCurrentState  = SERVICE_STOP_PENDING; 
			m_ServiceStatus.dwCheckPoint    = 0; 
			m_ServiceStatus.dwWaitHint      = 0; 

			SetServiceStatus (m_ServiceStatusHandle,&m_ServiceStatus);

			// Tell running loop to stop, and wait for service thread to close
			// down cleanly...
			serviceRunState = SERVICE_STOP_PENDING;
			WaitForSingleObject (mCPUPerfThread, 10000);
			CloseHandle (mCPUPerfThread);

			m_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
			SetServiceStatus (m_ServiceStatusHandle,&m_ServiceStatus);
			break;

		case SERVICE_CONTROL_INTERROGATE: 
			break; 
	}      
	return; 
}

//-----------------------------------------------------------------------------
void startCPUPerf(bool serviceFlag)
{
	// Init CPUPerf for running
	std::string					outputPath;
	std::vector<CMonitor *>	monitors;
	std::ostringstream		version;
	std::string					version2;
	CMonitor *					monitor;
	CLogger*						logger;
	unsigned int				i;
	char							s[BUFSIZ];
	HANDLE						*counterHandles;
	unsigned int				numCounterHandles;

	outputPath = init();

	version << "Version: " << CPUPERF_VERSION << " - Built: " << __DATE__ << " " __TIME__ << std::ends;
	version2 = version.str();

	// Create a logger
	logger = new CLogger(outputPath, serviceFlag);
	logger->init();
	logger->writeLine ("-------------------------------------------------------");
	logger->writeLine ("CPUPerf - Performance monitor for Windows");
	logger->writeLine (version2);
	logger->writeLine ("Copyright (c) 2006 Chris Johnson <cej@nightwolf.org.uk>");
	logger->writeLine ("");
	logger->writeLine ("Starting CPUPerf...");

	// Initialise monitor classes
	monitor = (CMonitor *)(new CMonitorUptime(outputPath, logger));
	monitors.push_back(monitor);
	monitor = (CMonitor *)(new CMonitorLoadAverage(outputPath, logger));
	monitors.push_back(monitor);
	monitor = (CMonitor *)(new CMonitorCPU(outputPath, logger));
	monitors.push_back(monitor);
	monitor = (CMonitor *)(new CMonitorMemory(outputPath, logger));
	monitors.push_back(monitor);

	// Get memory to store handles for monitor threads
	numCounterHandles = monitors.size();
	counterHandles = (HANDLE *)calloc(numCounterHandles, sizeof(HANDLE));

	if (counterHandles == NULL) {
		logger->writeLine ("INTERNAL FATAL: Failed to allocate memory for monitor handles");
	} else {
		// Let's start monitoring!
		for (i = 0; i < monitors.size(); i++) {
			monitors[i]->start();
			// Put a pause in to try and put a bit of a gap between each class
			// so they don't all hit the CPU at the same time. This, of course,
			// is (probably) doomed to failure over the long life-cycle of a 
			// but it'll help a little bit.
			Sleep(100);
		}
		logger->writeLine ("CPUPerf started.");

		// Now we do nothing until the service is told to shut down.
		serviceRunState = SERVICE_RUNNING;
		if (serviceFlag) {
			while ((serviceRunState != SERVICE_STOP_PENDING) && (serviceRunState != SERVICE_STOPPED)) {
				Sleep(1000);
			}
		} else {
			logger->writeLine ("-z flag supplied. Running CPUPerf in foreground. Hit <ENTER> in console to stop CPUPerf.");
			std::cout << "Hit <ENTER> to exit CPUPerf." << std::endl;
			gets_s(s, sizeof(s));
		}
	}

	// Start to shut down...
	logger->writeLine ("CPUPerf stopping...");

	// Cleanly shut down each thread before destruction. First we
	// tell each thread to stop...
	for (i = 0; i < monitors.size(); i++) {
		if (counterHandles != NULL) {
			counterHandles[i] = monitors[i]->getThreadHandle();
		}
		monitors[i]->stop();
	}
	// ... then wait for each thread to singal its completion ...
	if (counterHandles != NULL) {
		WaitForMultipleObjects(numCounterHandles, counterHandles, TRUE, 5000);
	}
	// ... before saying KILL! KILL! KILL!
	for (i = 0; i < monitors.size(); i++) {
		delete monitors[i];
	}
	monitors.clear();
	free (counterHandles);

	// Almost there...
	logger->writeLine ("CPUPerf stopping logger.");
	delete logger;

	// And that's it. Leaving this function will exit the thread and the service
	// will be signalled to stop.
}

//-----------------------------------------------------------------------------
unsigned int WINAPI startCPUPerfThread(LPVOID params)
{
	startCPUPerf (true);
	return 0;
}

//-----------------------------------------------------------------------------
void WINAPI ServiceMain (DWORD argc, LPTSTR *argv)
{
	// Do service bits
	m_ServiceStatus.dwServiceType        = SERVICE_WIN32; 
	m_ServiceStatus.dwCurrentState       = SERVICE_START_PENDING; 
	m_ServiceStatus.dwControlsAccepted   = SERVICE_ACCEPT_STOP; 
	m_ServiceStatus.dwWin32ExitCode      = 0; 
	m_ServiceStatus.dwServiceSpecificExitCode = 0; 
	m_ServiceStatus.dwCheckPoint         = 0; 
	m_ServiceStatus.dwWaitHint           = 0; 

	m_ServiceStatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceCtrlHandler);  
	if (m_ServiceStatusHandle != (SERVICE_STATUS_HANDLE)0) {
		m_ServiceStatus.dwCurrentState       = SERVICE_RUNNING; 
		m_ServiceStatus.dwCheckPoint         = 0; 
		m_ServiceStatus.dwWaitHint           = 0;  
		if (!SetServiceStatus (m_ServiceStatusHandle, &m_ServiceStatus)) {

		}
	}
	// end of doing service bits

	mCPUPerfThread = (HANDLE)_beginthreadex (NULL, 0, startCPUPerfThread, NULL, 0, &mCPUPerfThreadID);
}

//-----------------------------------------------------------------------------
int main(int argc, char **argv)
{
	if (argc > 1) {
		std::cout << "CPUPerf Version " << CPUPERF_VERSION << std::endl;
		if (strcmp(argv[1],"-i") == 0) {
			if (InstallService()) {
				std::cout << "\nCPUPerf installed sucessfully." << std::endl;
			} else {
				std::cout << "\nCPUPerf failed to install." << std::endl;
			}
		} else if (strcmp(argv[1],"-d") == 0) {
			if (UnInstallService()) {
				std::cout << "\nCPUPerf uninstalled sucessfully." << std::endl;
			} else {
				std::cout << "\nCPUPerf failed to uninstall." << std::endl;
			}
		} else if (strcmp(argv[1],"-v") == 0) {
			std::cout << "Copyright (c) 2006 Chris Johnson, Sheffield, UK <cej@nightwolf.org.uk>\n" << std::endl;
			std::cout << "This software is distributed under the terms of the GPL Version 2. A copy" << std::endl;
			std::cout << "of the licence should have been distributed with this program. If not, a" << std::endl;
			std::cout << "copy may be seen at http://www.gnu.org/copyleft/gpl.html\n" << std::endl;
		} else if (strcmp(argv[1],"-z") == 0) {
			std::cout << "Starting CPUPerf in foreground." << std::endl;
			startCPUPerf(false);
		} else {
			std::cout << "\nInvalid command. Usage: " << argv[0] << " {-i|-d|-v|-z}\n" << std::endl;
			std::cout << "    -i    Install service into Windows Service Manager" << std::endl;
			std::cout << "    -d    Remove service from Windows Service Manager" << std::endl;
			std::cout << "    -v    Display version and copyright information\n" << std::endl;
			std::cout << "    -z    Run CPUPerf in foreground. All messages come to the console.\n" << std::endl;
		}
	} else {
		SERVICE_TABLE_ENTRY DispatchTable[]={{SERVICE_NAME,ServiceMain},{NULL,NULL}};  
		StartServiceCtrlDispatcher(DispatchTable); 
	}
	return 0;
}
