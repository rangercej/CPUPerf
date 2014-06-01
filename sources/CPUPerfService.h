#ifndef _CPUPERFSERVICE_H_
#define _CPUPERFSERVICE_H_

#include "stdafx.h"
#include "ntserv.h"

#define STATUS_STOPPED		0
#define STATUS_STOPPING		1
#define STATUS_STARTING		2
#define STATUS_PAUSED		3
#define STATUS_STARTED		4

class CCPUPerfService : public CNTService
{
	public:
		CCPUPerfService();
		~CCPUPerfService();

		BOOL OnInit();
		BOOL Run();

	private:
		std::string init();

	private:
		int mStatus;
};

#endif