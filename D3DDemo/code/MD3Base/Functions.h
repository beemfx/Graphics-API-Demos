#ifndef __FUNCTIONS_H__
#define __FUNCTIONS_H__

#include <windows.h>
#include "MD3.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef UNICODE
#define RemoveDirectoryFromString RemoveDirectoryFromStringW
#define GetDirectoryFromString    GetDirectoryFromStringW
#else /* UNICODE */
#define RemoveDirectoryFromString RemoveDirectoryFromStringA
#define GetDirectoryFromString    GetDirectoryFromStringA
#endif /* UNICODE */

BOOL RemoveDirectoryFromStringA(char szLineOut[], const char szLineIn[]);
BOOL RemoveDirectoryFromStringW(WCHAR szLineOut[], const WCHAR szLineIn[]);

BOOL GetDirectoryFromStringA(char szLineOut[], const char szLineIn[]);
BOOL GetDirectoryFromStringW(WCHAR szLineOut[], const WCHAR szLineIn[]);


BOOL DecodeNormalVector(LPMD3VECTOR lpOut, const LPMD3VERTEX lpVertex);


#define RLERR_INVALIDFILE 0x80000001l
#define RLERR_NOTALINE    0x80000002l

#define RLSUC_READSUCCESS 0x0000000l
#define RLSUC_EOF         0x00000001l
#define RLSUC_FINISHED    0x00000002l

DWORD GetNumLinesInFile(HANDLE hFile);
HRESULT ReadLine(HANDLE hFile, LPSTR szLine);

HRESULT ReadWordFromLine(LPSTR szLineOut, LPSTR szLine, DWORD dwStart, DWORD * dwEnd);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FUNCTIONS_H__ */