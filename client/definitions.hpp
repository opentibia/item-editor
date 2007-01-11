//////////////////////////////////////////////////////////////////////
// OTItemEditor
//////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//////////////////////////////////////////////////////////////////////

#ifndef __OTITEMEDITOR_DEFS_H__
#define __OTITEMEDITOR_DEFS_H__

#ifndef DEFINITIONS_RESOURCE
#include <iostream>
#endif

#define OTIE_VERSION_STRING "OTItemEditor 0.2.5 - 7.92"

#ifdef WIN32
#pragma warning(disable:4786) // msvc too long debug names in stl

#endif

#ifdef _DEBUG
#define __DEBUG_ASSERT__
#endif

#if defined  WIN32 && defined _DEBUG
#include "crtdbg.h"
#endif

#ifndef __DEBUG_ASSERT__
#define ASSERT(x) 
#else
#ifndef WIN32
#define ASSERT(x) \
	if (! (x) ){\
	std::cout << "ASSERT ERROR " << #x << std::endl;\
	std::cout << "file: " << __FILE__ << std::endl;\
	std::cout << "line: " << __LINE__ << std::endl;\
	exit(1);}
#else

#ifdef __FILE_REVISION
#define ASSERT(x) \
	if(! (x)){\
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_WNDW);\
	_CrtDbgReport(_CRT_ASSERT, __FILE__ " " __FILE_REVISION, __LINE__, NULL, #x);\
	_CrtDbgBreak();}
#else
#define ASSERT(x) \
	if(! (x)){\
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_WNDW);\
	_CrtDbgReport(_CRT_ASSERT, __FILE__ , __LINE__, NULL, #x);\
	_CrtDbgBreak();}
#endif //#ifdef __FILE_REVISION
#endif //#ifndef WIN32
#endif //#ifndef __DEBUG_ASSERT__

#endif
