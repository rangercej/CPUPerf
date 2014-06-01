///////////////////////////////////////////////////////////////////////////////
//
// CIniFileReader
// Does exactly what it says on the tin.
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

#include "CIniFileReader.h"

void CIniFileReader::init(std::string filename)
{
	std::string line;
	std::string section = "";
	std::vector<std::string> lineparts;
	char inbuf[BUFSIZ];

	m_filename = filename;

	m_iniFileStream = std::ifstream(m_filename.c_str(), std::ios_base::in); 

	while (m_iniFileStream.good()) {
		m_iniFileStream.getline(inbuf, BUFSIZ);
		line = inbuf;
		if (line[0] != ';') {
			if (line[0] == '[') {
				section = line.substr(1, line.length() - 2);
			} else {
				lineparts = splitString (line, "=");
				m_iniFileLines[section][lineparts[0]] = lineparts[1];
			}
		}
	}
}

std::vector<std::string> CIniFileReader::getSection (std::string sectionName)
{
}


std::vector<std::string> CIniFileReader::splitString (std::string instring, std::string separator)
{
   std::vector<std::string> retval;
   std::string::size_type start = 0;
   std::string::size_type end = 0;

   while ((end=instring.find (separator, start)) != std::string::npos)
   {
		retval.push_back (instring.substr (start, end-start));
		start = end + separator.size();
	}

	retval.push_back (instring.substr (start));

   return retval;
} 