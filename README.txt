CPUPerf 0.03
============

cej@nightwolf.org.uk, February 2006

Thingy to report various bits of system information in various files.

INSTALLATION
------------
1. Copy CPUPerf.exe somewhere (e.g., c:\winnt)
2. Open a command prompt and change to the directory where you copied the file
3. Issue the command: CPUPerf -i
4. Start the service! (from a command prompt: net start cpuperf)
5. Stats will now be reported in %WINDIR%\CPUPerf (e.g., C:\WINNT\CPUPerf),
   and any messages raised in the log file %WINDIR%\CPUPerf\messages


UNINSTALLING
------------
1. Stop the CPUPerf service (from a command prompt: net stop cpuperf)
2. Open a command prompt and change to the directory where you copied the file
3. Issue the command: CPUPerf -d
4. You may need to reboot
5. Delete the CPUPerf executable
6. Delete the directory %WINDIR%\CPUPerf


STATS REPORTED
--------------
Currently these stats are reported:
    loadaverage: format is:
        <samples> <current run queue> <1 min LA> <5 min LA> <15 min LA>

    uptime: format is:
        <samples> <uptime in seconds>

    cpu: format is:
        <samples> <processor %> <system %>

    memory: format is:
        <samples> <available bytes> <cache bytes> <committed bytes> <page faults/sec> <page reads/sec> <page writes/sec>

the <samples> count can not really be used as an indication of how long the
service has been running; samples are taken every second, but there may be
delays or other Windows system events that prevent it counting accurately. It
is merely provided as a guide, and a good indicator that the service is still
running :-)

More detail can be found in COUNTERS.txt


ADDING YOUR OWN MONITOR
-----------------------
Just create your own class, deriving it from CMonitor and hooking it up in
main() so it actually does something. See the code for the CMonitor* classes
to see how it all hangs together. 

You'll need Visual C++ to do builds. CPUPerf has been developed with Visual
C++ 2005 Express, however it should be possible to build it in previous
versions by creating an empty console project and adding all the CPP and H
files to the build. As well as the default libraries, You'll need to link
against advapi32.lib and pdh.lib.

See COUNTERS.txt for information on how to determine paths to counters,
and which counters make up which files in CPUPerf.


KNOWN BUGS
----------
- Nothing known at the moment. It's not ran in anger for several days, so
	if there are any issues like memory leaks, let me know and I'll
	do some poking.


OTHER THINGS
------------
This uses code and snippets in the following files from other sources:
- LinuxLoadAverage.h
  Linux 2.4.26, file: include/linux/sched.h for load averaging code

- CPUPerf.cpp (portions)
  Anish C.V. for Win32 service interface sample
	http://www.codeguru.com/cpp/w-p/system/ntservices/article.php/c5701/

- CPerfMon.cpp, CPerfMon.h
  Chad Busche (chad_busche@hotmail.com) for a simple class that acts
	as a simple wrapper round Microsoft's PDH library
	(http://www.codeproject.com/atl/desktop_perf_mon.asp)

Serious analysis of the Linux LA algorthm has been written by Neil Gunther
and can be found at:
	http://www.teamquest.com/resources/gunther/ldavg1.shtml
There's an alternative version of it here:
	http://www.luv.asn.au/overheads/NJG_LUV_2002/luvSlides.html
	

AUTHOR INFO
-----------
Thrown together over several days (although in reality only a few hours
tinkering) by Chris Johnson <cej@nightwolf.org.uk>, in December 2005 and
January to February 2006.


LICENSE INFO
------------
Copyright (C) 2006 Chris Johnson, Sheffield, UK <cej@nightwolf.org.uk>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

