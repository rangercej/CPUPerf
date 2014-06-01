/*******************************************************************************
**
** This class is based on code from CodeProject, using the snippets on the main 
** page. Saves me having to put together something to use pdh.lib manually :-)
**
** Changes made to support multiple counters on a single query - this enables me
** to report various CPU and memory stats in one file, rather than separate files
** per counter.
**
** Source:
** http://www.codeproject.com/atl/desktop_perf_mon.asp
**
** Original class by chad_busche@hotmail.com
**
** Modifications by cej@nightwolf.org.uk, December 2005 & January 2006
**
*******************************************************************************/

#include "stdafx.h"
#include "CPerfMon.h"
#include "CLogger.h"
#include <sstream>
#include <string>
#include <iomanip>

/******************************************************************************/
CPerfMon::CPerfMon(CLogger *logger)
{
	m_hQuery = NULL;
	m_hCounter.clear();
	mLogger = logger;
}

/******************************************************************************/
CPerfMon::~CPerfMon()
{
	Clear();
}

/******************************************************************************/
void CPerfMon::GetPdhErrorMessage (PDH_STATUS status, sPERFCOUNTER *counter, std::string arg)
{
	std::string counterName;

	if (counter) {
		counterName = counter->name;
	} else {
		counterName = "";
	}

	GetPdhErrorMessage (status, counterName, arg);
}

/******************************************************************************/
void CPerfMon::GetPdhErrorMessage (PDH_STATUS status, LPTSTR counter, std::string arg)
{
	std::string counterName;

	if (counter) {
		counterName = counter;
	} else {
		counterName = "";
	}
}

/******************************************************************************/
void CPerfMon::GetPdhErrorMessage (PDH_STATUS status, std::string counterName, std::string arg)
{
	std::ostringstream errMsg;

	errMsg << "Error in PDH lib: 0x" << std::setbase(16) << status << ": " << arg << ": ";

	// Error descriptions come straight from the MSDN. This is every PDH error
	// status listed; some of these probably can't happen in this service (e.g.,
	// the SQL errors), but it's nice to be complete :-)
	switch (status) {
		case PDH_CSTATUS_VALID_DATA:
			errMsg << "'" << counterName << "': PDH_CSTATUS_VALID_DATA: The returned data is valid.";
			break;
		case PDH_CSTATUS_NEW_DATA:
			errMsg << "'" << counterName << "': PDH_CSTATUS_NEW_DATA: The return data value is valid and different from the last sample.";
			break;
		case PDH_CSTATUS_NO_MACHINE:
			errMsg << "'" << counterName << "': PDH_CSTATUS_NO_MACHINE: Unable to connect to specified machine or machine is off line.";
			break;
		case PDH_CSTATUS_NO_INSTANCE:
			errMsg << "'" << counterName << "': PDH_CSTATUS_NO_INSTANCE: The specified instance is not present.";
			break;
		case PDH_MORE_DATA:
			errMsg << "'" << counterName << "': PDH_MORE_DATA: There is more data to return than would fit in the supplied buffer. Allocate a larger buffer and call the function again.";
			break;
		case PDH_CSTATUS_ITEM_NOT_VALIDATED:
			errMsg << "'" << counterName << "': PDH_CSTATUS_ITEM_NOT_VALIDATED: The data item has been added to the query but has not been validated nor accessed. No other status information on this data item is available.";
			break;
		case PDH_RETRY:
			errMsg << "'" << counterName << "': PDH_RETRY: The selected operation should be retried.";
			break;
		case PDH_NO_DATA:
			errMsg << "'" << counterName << "': PDH_NO_DATA: No data to return.";
			break;
		case PDH_CALC_NEGATIVE_DENOMINATOR:
			errMsg << "'" << counterName << "': PDH_CALC_NEGATIVE_DENOMINATOR: A counter with a negative denominator value was detected.";
			break;
		case PDH_CALC_NEGATIVE_TIMEBASE:
			errMsg << "'" << counterName << "': PDH_CALC_NEGATIVE_TIMEBASE: A counter with a negative time base value was detected.";
			break;
		case PDH_CALC_NEGATIVE_VALUE:
			errMsg << "'" << counterName << "': PDH_CALC_NEGATIVE_VALUE: A counter with a negative value was detected.";
			break;
		case PDH_DIALOG_CANCELLED:
			errMsg << "'" << counterName << "': PDH_DIALOG_CANCELLED: The user canceled the dialog box.";
			break;
		case PDH_END_OF_LOG_FILE:
			errMsg << "'" << counterName << "': PDH_END_OF_LOG_FILE: The end of the log file was reached.";
			break;
		case PDH_ASYNC_QUERY_TIMEOUT:
			errMsg << "'" << counterName << "': PDH_ASYNC_QUERY_TIMEOUT: Time out while waiting for asynchronous counter collection thread to end.";
			break;
		case PDH_CANNOT_SET_DEFAULT_REALTIME_DATASOURCE:
			errMsg << "'" << counterName << "': PDH_CANNOT_SET_DEFAULT_REALTIME_DATASOURCE: Cannot change set default real-time data source. There are real-time query sessions collecting counter data.";
			break;
		case PDH_CSTATUS_NO_OBJECT:
			errMsg << "'" << counterName << "': PDH_CSTATUS_NO_OBJECT: The specified object is not found on the system.";
			break;
		case PDH_CSTATUS_NO_COUNTER:
			errMsg << "'" << counterName << "': PDH_CSTATUS_NO_COUNTER: The specified counter could not be found.";
			break;
		case PDH_CSTATUS_INVALID_DATA:
			errMsg << "'" << counterName << "': PDH_CSTATUS_INVALID_DATA: The returned data is not valid.";
			break;
		case PDH_MEMORY_ALLOCATION_FAILURE:
			errMsg << "'" << counterName << "': PDH_MEMORY_ALLOCATION_FAILURE: A PDH function could not allocate enough temporary memory to complete the operation. Close some applications or extend the page file and retry the function.";
			break;
		case PDH_INVALID_HANDLE:
			errMsg << "'" << counterName << "': PDH_INVALID_HANDLE: The handle is not a valid PDH object.";
			break;
		case PDH_INVALID_ARGUMENT:
			errMsg << "'" << counterName << "': PDH_INVALID_ARGUMENT: A required argument is missing or incorrect.";
			break;
		case PDH_FUNCTION_NOT_FOUND:
			errMsg << "'" << counterName << "': PDH_FUNCTION_NOT_FOUND: Unable to find the specified function.";
			break;
		case PDH_CSTATUS_NO_COUNTERNAME:
			errMsg << "'" << counterName << "': PDH_CSTATUS_NO_COUNTERNAME: No counter was specified.";
			break;
		case PDH_CSTATUS_BAD_COUNTERNAME:
			errMsg << "'" << counterName << "': PDH_CSTATUS_BAD_COUNTERNAME: Unable to parse the counter path. Check the format and syntax of the specified path.";
			break;
		case PDH_INVALID_BUFFER:
			errMsg << "'" << counterName << "': PDH_INVALID_BUFFER: The buffer passed by the caller is invalid.";
			break;
		case PDH_INSUFFICIENT_BUFFER:
			errMsg << "'" << counterName << "': PDH_INSUFFICIENT_BUFFER: The requested data is larger than the buffer supplied. Unable to return the requested data.";
			break;
		case PDH_CANNOT_CONNECT_MACHINE:
			errMsg << "'" << counterName << "': PDH_CANNOT_CONNECT_MACHINE: Unable to connect to the requested machine.";
			break;
		case PDH_INVALID_PATH:
			errMsg << "'" << counterName << "': PDH_INVALID_PATH: The specified counter path could not be interpreted.";
			break;
		case PDH_INVALID_INSTANCE:
			errMsg << "'" << counterName << "': PDH_INVALID_INSTANCE: The instance name could not be read from the specified counter path.";
			break;
		case PDH_INVALID_DATA:
			errMsg << "'" << counterName << "': PDH_INVALID_DATA: The data is not valid.";
			break;
		case PDH_NO_DIALOG_DATA:
			errMsg << "'" << counterName << "': PDH_NO_DIALOG_DATA: The dialog box data block was missing or invalid.";
			break;
		case PDH_CANNOT_READ_NAME_STRINGS:
			errMsg << "'" << counterName << "': PDH_CANNOT_READ_NAME_STRINGS: Unable to read the counter and/or explain text from the specified machine.";
			break;
		case PDH_LOG_FILE_CREATE_ERROR:
			errMsg << "'" << counterName << "': PDH_LOG_FILE_CREATE_ERROR: Unable to create the specified log file.";
			break;
		case PDH_LOG_FILE_OPEN_ERROR:
			errMsg << "'" << counterName << "': PDH_LOG_FILE_OPEN_ERROR: Unable to open the specified log file.";
			break;
		case PDH_LOG_TYPE_NOT_FOUND:
			errMsg << "'" << counterName << "': PDH_LOG_TYPE_NOT_FOUND: The specified log file type has not been installed on this system.";
			break;
		case PDH_NO_MORE_DATA:
			errMsg << "'" << counterName << "': PDH_NO_MORE_DATA: No more data is available.";
			break;
		case PDH_ENTRY_NOT_IN_LOG_FILE:
			errMsg << "'" << counterName << "': PDH_ENTRY_NOT_IN_LOG_FILE: The specified record was not found in the log file.";
			break;
		case PDH_DATA_SOURCE_IS_LOG_FILE:
			errMsg << "'" << counterName << "': PDH_DATA_SOURCE_IS_LOG_FILE: The specified data source is a log file.";
			break;
		case PDH_DATA_SOURCE_IS_REAL_TIME:
			errMsg << "'" << counterName << "': PDH_DATA_SOURCE_IS_REAL_TIME: The specified data source is the current activity.";
			break;
		case PDH_UNABLE_READ_LOG_HEADER:
			errMsg << "'" << counterName << "': PDH_UNABLE_READ_LOG_HEADER: The log file header could not be read.";
			break;
		case PDH_FILE_NOT_FOUND:
			errMsg << "'" << counterName << "': PDH_FILE_NOT_FOUND: Unable to find the specified file.";
			break;
		case PDH_FILE_ALREADY_EXISTS:
			errMsg << "'" << counterName << "': PDH_FILE_ALREADY_EXISTS: There is already a file with the specified file name.";
			break;
		case PDH_NOT_IMPLEMENTED:
			errMsg << "'" << counterName << "': PDH_NOT_IMPLEMENTED: The function referenced has not been implemented.";
			break;
		case PDH_STRING_NOT_FOUND:
			errMsg << "'" << counterName << "': PDH_STRING_NOT_FOUND: Unable to find the specified string in the list of performance name and explain text strings.";
			break;
		case PDH_UNABLE_MAP_NAME_FILES:
			errMsg << "'" << counterName << "': PDH_UNABLE_MAP_NAME_FILES: Unable to map to the performance counter name data files. The data will be read from the registry and stored locally.";
			break;
		case PDH_UNKNOWN_LOG_FORMAT:
			errMsg << "'" << counterName << "': PDH_UNKNOWN_LOG_FORMAT: The format of the specified log file is not recognized by the PDH DLL.";
			break;
		case PDH_UNKNOWN_LOGSVC_COMMAND:
			errMsg << "'" << counterName << "': PDH_UNKNOWN_LOGSVC_COMMAND: The specified Log Service command value is not recognized.";
			break;
		case PDH_LOGSVC_QUERY_NOT_FOUND:
			errMsg << "'" << counterName << "': PDH_LOGSVC_QUERY_NOT_FOUND: The specified Query from the Log Service could not be found or could not be opened.";
			break;
		case PDH_LOGSVC_NOT_OPENED:
			errMsg << "'" << counterName << "': PDH_LOGSVC_NOT_OPENED: The Performance Data Log Service key could not be opened. This may be due to insufficient privilege or because the service has not been installed.";
			break;
		case PDH_WBEM_ERROR:
			errMsg << "'" << counterName << "': PDH_WBEM_ERROR: An error occurred while accessing the WBEM data store.";
			break;
		case PDH_ACCESS_DENIED:
			errMsg << "'" << counterName << "': PDH_ACCESS_DENIED: Unable to access the desired machine or service. Check the permissions and authentication of the log service or the interactive user session against those on the machine or service being monitored.";
			break;
		case PDH_LOG_FILE_TOO_SMALL:
			errMsg << "'" << counterName << "': PDH_LOG_FILE_TOO_SMALL: The maximum log file size specified is too small to log the selected counters. No data will be recorded in this log file. Specify a smaller set of counters to log or a larger file size and retry this call.";
			break;
		case PDH_INVALID_DATASOURCE:
			errMsg << "'" << counterName << "': PDH_INVALID_DATASOURCE: Cannot connect to ODBC DataSource Name.";
			break;
		case PDH_INVALID_SQLDB:
			errMsg << "'" << counterName << "': PDH_INVALID_SQLDB: SQL Database does not contain a valid set of tables for Perfmon; use PdhCreateSQLTables.";
			break;
		case PDH_NO_COUNTERS:
			errMsg << "'" << counterName << "': PDH_NO_COUNTERS: No counters were found for this Perfmon SQL Log Set.";
			break;
		case PDH_SQL_ALLOC_FAILED:
			errMsg << "'" << counterName << "': PDH_SQL_ALLOC_FAILED: Call to SQLAllocStmt failed with %1.";
			break;
		case PDH_SQL_ALLOCCON_FAILED:
			errMsg << "'" << counterName << "': PDH_SQL_ALLOCCON_FAILED: Call to SQLAllocConnect failed with %1.";
			break;
		case PDH_SQL_EXEC_DIRECT_FAILED:
			errMsg << "'" << counterName << "': PDH_SQL_EXEC_DIRECT_FAILED: Call to SQLExecDirect failed with %1.";
			break;
		case PDH_SQL_FETCH_FAILED:
			errMsg << "'" << counterName << "': PDH_SQL_FETCH_FAILED: Call to SQLFetch failed with %1.";
			break;
		case PDH_SQL_ROWCOUNT_FAILED:
			errMsg << "'" << counterName << "': PDH_SQL_ROWCOUNT_FAILED: Call to SQLRowCount failed with %1.";
			break;
		case PDH_SQL_MORE_RESULTS_FAILED:
			errMsg << "'" << counterName << "': PDH_SQL_MORE_RESULTS_FAILED: Call to SQLMoreResults failed with %1.";
			break;
		case PDH_SQL_CONNECT_FAILED:
			errMsg << "'" << counterName << "': PDH_SQL_CONNECT_FAILED: Call to SQLConnect failed with %1.";
			break;
		case PDH_SQL_BIND_FAILED:
			errMsg << "'" << counterName << "': PDH_SQL_BIND_FAILED: Call to SQLBindCol failed with %1.";
			break;
		case PDH_CANNOT_CONNECT_WMI_SERVER:
			errMsg << "'" << counterName << "': PDH_CANNOT_CONNECT_WMI_SERVER: Unable to connect to the WMI server on requested machine.";
			break;
		case PDH_PLA_COLLECTION_ALREADY_RUNNING:
			errMsg << "'" << counterName << "': PDH_PLA_COLLECTION_ALREADY_RUNNING: Collection '%1!s!' is already running.";
			break;
		case PDH_PLA_ERROR_SCHEDULE_OVERLAP:
			errMsg << "'" << counterName << "': PDH_PLA_ERROR_SCHEDULE_OVERLAP: The specified start time is after the end time.";
			break;
		case PDH_PLA_COLLECTION_NOT_FOUND:
			errMsg << "'" << counterName << "': PDH_PLA_COLLECTION_NOT_FOUND: Collection '%1!s!' does not exist.";
			break;
		case PDH_PLA_ERROR_SCHEDULE_ELAPSED:
			errMsg << "'" << counterName << "': PDH_PLA_ERROR_SCHEDULE_ELAPSED: The specified end time has already elapsed.";
			break;
		case PDH_PLA_ERROR_NOSTART:
			errMsg << "'" << counterName << "': PDH_PLA_ERROR_NOSTART: Collection '%1!s!' did not start check the application event log for any errors.";
			break;
		case PDH_PLA_ERROR_ALREADY_EXISTS:
			errMsg << "'" << counterName << "': PDH_PLA_ERROR_ALREADY_EXISTS: Collection '%1!s!' already exists.";
			break;
		case PDH_PLA_ERROR_TYPE_MISMATCH:
			errMsg << "'" << counterName << "': PDH_PLA_ERROR_TYPE_MISMATCH: There is a mismatch in the settings type.";
			break;
		case PDH_PLA_ERROR_FILEPATH:
			errMsg << "'" << counterName << "': PDH_PLA_ERROR_FILEPATH: The information specified does not resolve to a valid path name.";
			break;
		case PDH_PLA_SERVICE_ERROR:
			errMsg << "'" << counterName << "': PDH_PLA_SERVICE_ERROR: The 'Performance Logs & Alerts' service did not respond.";
			break;
		case PDH_PLA_VALIDATION_ERROR:
			errMsg << "'" << counterName << "': PDH_PLA_VALIDATION_ERROR: The information passed is not valid.";
			break;
		case PDH_PLA_VALIDATION_WARNING:
			errMsg << "'" << counterName << "': PDH_PLA_VALIDATION_WARNING: The information passed is not valid.";
			break;
		case PDH_PLA_ERROR_NAME_TOO_LONG:
			errMsg << "'" << counterName << "': PDH_PLA_ERROR_NAME_TOO_LONG: The name supplied is too long.";
			break;
		case PDH_INVALID_SQL_LOG_FORMAT:
			errMsg << "'" << counterName << "': PDH_INVALID_SQL_LOG_FORMAT: SQL log format is incorrect. Correct format is 'SQL:<DSN-name>!<LogSet-Name>'.";
			break;
		case PDH_COUNTER_ALREADY_IN_QUERY:
			errMsg << "'" << counterName << "': PDH_COUNTER_ALREADY_IN_QUERY: Performance counter in PdhAddCounter call has already been added in the performance query. This counter is ignored.";
			break;
		case PDH_BINARY_LOG_CORRUPT:
			errMsg << "'" << counterName << "': PDH_BINARY_LOG_CORRUPT: Unable to read counter information and data from input binary log files.";
			break;
		case PDH_LOG_SAMPLE_TOO_SMALL:
			errMsg << "'" << counterName << "': PDH_LOG_SAMPLE_TOO_SMALL: At least one of the input binary log files contain fewer than two data samples.";
			break;
		case PDH_OS_LATER_VERSION:
			errMsg << "'" << counterName << "': PDH_OS_LATER_VERSION: The version of the operating system on the computer named %1 is later than that on the local computer. This operation is not available from the local computer.";
			break;
		case PDH_OS_EARLIER_VERSION:
			errMsg << "'" << counterName << "': PDH_OS_EARLIER_VERSION: supports %2 or later. Check the operating system version on the computer named %3.";
			break;
		case PDH_INCORRECT_APPEND_TIME:
			errMsg << "'" << counterName << "': PDH_INCORRECT_APPEND_TIME: The output file must contain earlier data than the file to be appended.";
			break;
		case PDH_UNMATCHED_APPEND_COUNTER:
			errMsg << "'" << counterName << "': PDH_UNMATCHED_APPEND_COUNTER: Both files must have identical counters in order to append.";
			break;
		case PDH_SQL_ALTER_DETAIL_FAILED:
			errMsg << "'" << counterName << "': PDH_SQL_ALTER_DETAIL_FAILED: Cannot alter CounterDetail table layout in SQL database.";
			break;
		case PDH_QUERY_PERF_DATA_TIMEOUT:
			errMsg << "'" << counterName << "': PDH_QUERY_PERF_DATA_TIMEOUT: System is busy. Timeout when collecting counter data. Please retry later or increase the CollectTime registry value.";
			break;
		default:
			errMsg << "'" << counterName << "': Unknown error code";
	}
	mLogger->writeLine (errMsg.str());
}


/******************************************************************************/
BOOL CPerfMon::Add(LPTSTR lpstrCounter)
{
	PDH_STATUS		pdhStatus;
	HCOUNTER			hCounter;
	sPERFCOUNTER	*perfCounter;

	if (!m_hQuery) {
		pdhStatus = PdhOpenQuery(NULL, 0, &m_hQuery);
	} else {
		pdhStatus = ERROR_SUCCESS;
	}
 
	if (pdhStatus != ERROR_SUCCESS) {
		GetPdhErrorMessage (pdhStatus, lpstrCounter, "[PdhOpenQuery failed]");
		m_hQuery = NULL;
	} else {
		/* This seems to take a second or two in Win2k...? If omitted, then
		   the delay moves to PdhAddCounter() below. Curious... it's fine on
		   WinXP SP2 */
		pdhStatus = PdhValidatePath(lpstrCounter);
 
		if (pdhStatus == ERROR_SUCCESS) {
			pdhStatus = PdhAddCounter(m_hQuery, lpstrCounter, 0, &hCounter);
		} else {
			GetPdhErrorMessage (pdhStatus, lpstrCounter, "[PdhAddCounter failed]");
		}
	}
 
	if (pdhStatus == ERROR_SUCCESS) {
		perfCounter = new sPERFCOUNTER;
		perfCounter->name = std::string(lpstrCounter);
		perfCounter->handle = hCounter;
		m_hCounter.push_back(perfCounter);
	}

	return (pdhStatus == ERROR_SUCCESS);
}

/******************************************************************************/
LONG CPerfMon::GetValue()
{
	return (GetValue(0));
}

/******************************************************************************/
LONG CPerfMon::GetValue(unsigned int idx)
{
	PDH_STATUS				pdhStatus			= ERROR_SUCCESS;
	PDH_FMT_COUNTERVALUE	pdhCounterValue;
	LONG						retval				= 0;
 
	if (m_hQuery) {
		pdhStatus = PdhCollectQueryData(m_hQuery);
		if (pdhStatus != ERROR_SUCCESS) {
			GetPdhErrorMessage (pdhStatus, m_hCounter[idx], "[GetValue failed - Collect]");
		} else {
			if (idx < m_hCounter.size()) {
				pdhStatus = PdhGetFormattedCounterValue(m_hCounter[idx]->handle, PDH_FMT_LONG, NULL, &pdhCounterValue);
 
				if (pdhStatus == ERROR_SUCCESS) {
					retval = pdhCounterValue.longValue;
				} else {
					GetPdhErrorMessage (pdhStatus, m_hCounter[idx], "[GetValue failed - Get]");
				}
			}
		}
	}

	return retval;      
}

/******************************************************************************/
BOOL CPerfMon::GetValues(v_long &results)
{
	PDH_STATUS				pdhStatus			= ERROR_SUCCESS;
	PDH_FMT_COUNTERVALUE	pdhCounterValue;
	BOOL						retval				= true;
	unsigned int i;
 
	results.clear();

	if (!m_hQuery) {
		retval = false;
	} else {
		pdhStatus = PdhCollectQueryData(m_hQuery);
		if (pdhStatus != ERROR_SUCCESS) {
			GetPdhErrorMessage (pdhStatus, "", "[GetValues failed - Collect]");
			retval = false;
		} else {
			for (i = 0; i < m_hCounter.size(); i++) {
				pdhStatus = PdhGetFormattedCounterValue(m_hCounter[i]->handle, PDH_FMT_LONG, NULL, &pdhCounterValue);
 
				if (pdhStatus != ERROR_SUCCESS) {
					results.push_back(0);
					retval = false;
					GetPdhErrorMessage (pdhStatus, m_hCounter[i], "[GetValues failed - Get]");
				} else {
					results.push_back(pdhCounterValue.longValue);
				}
			}
		}
	}

	return retval;      
}

/******************************************************************************/
VOID CPerfMon::Clear()
{
	unsigned int i;

	for (i = 0; i < m_hCounter.size(); i++) {
		if (m_hCounter[i]->handle) {
			PdhRemoveCounter(m_hCounter[i]->handle);
			delete (m_hCounter[i]);
		}
	}
	m_hCounter.clear();
 
	if (m_hQuery) {
		PdhCloseQuery (m_hQuery);
	}
	m_hQuery = NULL;
}
