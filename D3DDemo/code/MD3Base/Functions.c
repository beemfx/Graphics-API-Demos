#include "Functions.h"
#include <math.h>
#include <stdio.h>

BOOL RemoveDirectoryFromStringA(char szLineOut[], const char szLineIn[])
{
	size_t dwStrLen=0;
	size_t i=0, j=0;
	char szFinal[MAX_PATH];

	dwStrLen=strlen(szLineIn);

	for(i=dwStrLen; i>0; i--){
		if((szLineIn[i]=='/') || (szLineIn[i]=='\\')){
			i++;
			break;
		}
	}

	for(j=0; j<dwStrLen; j++, i++){
		szFinal[j]=szLineIn[i];
	}
	szFinal[j]=0;
	strcpy(szLineOut, szFinal);
	return TRUE;
}

BOOL RemoveDirectoryFromStringW(WCHAR szLineOut[], const WCHAR szLineIn[])
{
	size_t dwStrLen=0;
	size_t i=0, j=0;
	WCHAR szFinal[MAX_PATH];

	dwStrLen=wcslen(szLineIn);

	for(i=dwStrLen; i>0; i--){
		if((szLineIn[i]==L'/') || (szLineIn[i]==L'\\')){
			i++;
			break;
		}
	}

	for(j=0; j<dwStrLen; j++, i++){
		szFinal[j]=szLineIn[i];
	}
	szFinal[j]=0;
	wcscpy(szLineOut, szFinal);
	return TRUE;
}




BOOL GetDirectoryFromStringA(char szLineOut[], const char szLineIn[])
{
	size_t dwStrLen=0;
	size_t i=0, j=0;
	char szFinal[MAX_PATH];

	dwStrLen=strlen(szLineIn);
	for(i=dwStrLen; i>0; i--){
		if((szLineIn[i]=='/') || (szLineIn[i]=='\\')){
			break;
		}
	}
	for(j=0; j<=i; j++){
		szFinal[j]=szLineIn[j];
	}
	szFinal[j]=0;
	strcpy(szLineOut, szFinal);

	return TRUE;
}

BOOL GetDirectoryFromStringW(WCHAR szLineOut[], const WCHAR szLineIn[])
{
	size_t dwStrLen=0;
	size_t i=0, j=0;
	WCHAR szFinal[MAX_PATH];

	dwStrLen=wcslen(szLineIn);
	for(i=dwStrLen; i>0; i--){
		if((szLineIn[i]=='/') || (szLineIn[i]=='\\')){
			break;
		}
	}
	for(j=0; j<=i; j++){
		szFinal[j]=szLineIn[j];
	}
	szFinal[j]=0;
	wcscpy(szLineOut, szFinal);

	return TRUE;
}

HRESULT ReadLine(HANDLE hFile, LPSTR szLine)
{
	char cChar=0;
	DWORD dwBytesRead=0;
	DWORD i=0;
	DWORD dwFileSize=0;

	if(!hFile)
		return E_FAIL;

	dwFileSize=GetFileSize(hFile, NULL);

	do{
		if( dwFileSize==SetFilePointer(hFile, 0, NULL, FILE_CURRENT) ){
			szLine[i]=0;
			if(strlen(szLine) < 2)
				return RLSUC_FINISHED;
			else{
				return RLSUC_EOF;
			}
		}

		if( !ReadFile(hFile, &cChar, sizeof(char), &dwBytesRead, NULL) ){
			return RLERR_INVALIDFILE;
		}

		szLine[i]=cChar;
		i++;
	}while ((cChar != '\n') && (cChar != '\r'));

	szLine[i-1]=0;

	if(strlen(szLine) < 2)return ReadLine(hFile, szLine);
	return RLSUC_READSUCCESS;
}

DWORD GetNumLinesInFile(HANDLE hFile)
{
	char szLine[MAX_PATH];
	HRESULT hr=0;
	DWORD i=0;
	DWORD dwFilePointer=0;

	/* Get the file pointer then set it to the beginning of the file. */
	dwFilePointer=SetFilePointer(hFile, 0, 0, FILE_CURRENT);
	SetFilePointer(hFile, 0, 0, FILE_BEGIN);
	while(SUCCEEDED(hr=ReadLine(hFile, szLine)))
	{

		i++;

		if(hr==RLSUC_EOF)
			break;

		if(hr==RLSUC_FINISHED){
			i--;
			break;
		}
	}
	/* Restore the file pointer. */
	SetFilePointer(hFile, dwFilePointer, 0, FILE_BEGIN);
	return i;
}

HRESULT ReadWordFromLine(LPSTR szLineOut, LPSTR szLine, DWORD dwStart, DWORD * dwEnd)
{
	size_t i=0, j=0;
	size_t dwLen=0;

	BOOL bReadChar=FALSE;

	dwLen=strlen(szLine);

	for(i=dwStart,j=0; i<dwLen; i++,j++){
		if(szLine[i]==' ' && bReadChar){
			break;
		}
		szLineOut[j]=szLine[i];

		if(szLine[i]==' ' || szLine[i]=='\t')
			j--;
		else
			bReadChar=TRUE;
	}
	szLineOut[j]=0;
	
	if(dwEnd)
		*dwEnd=(DWORD)(dwStart+j+1);

	return S_OK;
}
