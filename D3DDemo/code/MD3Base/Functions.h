// (c) Beem Media. All rights reserved.

#pragma once

#include "MD3Types.h"
#include <windows.h>

class CDataStream;

BOOL RemoveDirectoryFromStringA(char szLineOut[], const char szLineIn[]);
BOOL RemoveDirectoryFromStringW(WCHAR szLineOut[], const WCHAR szLineIn[]);

BOOL GetDirectoryFromStringA(char szLineOut[], const char szLineIn[]);
BOOL GetDirectoryFromStringW(WCHAR szLineOut[], const WCHAR szLineIn[]);


#define RLERR_INVALIDFILE 0x80000001l
#define RLERR_NOTALINE    0x80000002l

#define RLSUC_READSUCCESS 0x0000000l
#define RLSUC_EOF         0x00000001l
#define RLSUC_FINISHED    0x00000002l

DWORD GetNumLinesInFile(HANDLE hFile);
HRESULT ReadLine(HANDLE hFile, LPSTR szLine);

HRESULT ReadWordFromLine(LPSTR szLineOut, LPSTR szLine, DWORD dwStart, DWORD* dwEnd);

namespace Functions
{
	std::vector<std::string> ReadLines(CDataStream& In);
	std::string ReadWordFromLine(const std::string& Line, std::size_t Start, std::size_t* End);

	std::string RemoveDirectoryFromString(const std::string& In);
}
