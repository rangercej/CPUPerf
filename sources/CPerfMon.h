/*******************************************************************************
**
** This class is based on code from CodeProject, using the snippets on the main 
** page.
**
** Source:
** http://www.codeproject.com/atl/desktop_perf_mon.asp
**
** Original class by chad_busche@hotmail.com
**
** Modifications by cej@nightwolf.org.uk, December 2005 & January 2006
**
*******************************************************************************/

#ifndef _CPERFMON_H_
#define _CPERFMON_H_

#include "stdafx.h"
#include "CLogger.h"
#include <vector>
#include <pdh.h>
#include <pdhmsg.h>
 
typedef std::vector<LONG> v_long;

typedef struct {
	std::string		name;
	HCOUNTER			handle;
} sPERFCOUNTER;

/*#define CHECK_PDH_STATUS(expr,ctr) { \
	int mac_rv = expr; \
	switch (mac_rv) { \
		case PDH_INVALID_ARGUMENT: \
			std::cout << ctr << ": Invalid counter << std::endl; break; \
	}
*/

class CPerfMon
{
   public:
      CPerfMon(CLogger *);   
      virtual ~CPerfMon();
   
      BOOL Add(LPTSTR);
      VOID Clear();
      LONG GetValue();
      LONG GetValue(unsigned int);
		BOOL GetValues(v_long &results);

	private:
		void GetPdhErrorMessage(PDH_STATUS, sPERFCOUNTER *, std::string);
		void GetPdhErrorMessage(PDH_STATUS, std::string   , std::string);
		void GetPdhErrorMessage(PDH_STATUS, LPTSTR        , std::string);

   private:
      HQUERY								m_hQuery;
		std::vector<sPERFCOUNTER *>	m_hCounter;
		CLogger*								mLogger;

};

#endif