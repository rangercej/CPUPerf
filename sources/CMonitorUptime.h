///////////////////////////////////////////////////////////////////////////////
//
// CMonitorUptime
// Monitor uptime
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
#ifndef _CMONITORUPTIME_H_
#define _CMONITORUPTIME_H_

#include "stdafx.h"

#include "CPerfMon.h"
#include "CMonitor.h"
#include <string>

#define SAMPLE_PERIOD		1000		// Sample period in milliseconds

class CMonitorUptime: public CMonitor
{
	public:
		CMonitorUptime(std::string suffix, CLogger *);
		DWORD run();
};

#endif