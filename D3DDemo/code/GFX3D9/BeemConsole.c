/*
	BeemConsole.c - Functionality for the Beem Console(tm).

	Copyright (c) 2003 Blaine Myers
*/

#define MAX_CHARSPERLINE 100
#define LINE_BUFFER_SIZE 500
#include <d3dx9.h>
#include <stdio.h>
#include "GFX3D9.h"
#include "resource.h"

/* =================================
   Command Parsing Helper functions.
	=================================*/

BOOL BeemParseGetFloat(
	FLOAT * lpValue,
	LPSTR szParams,
	WORD wParam)
{
	char szTemp[100];

	if(!BeemParseGetParam(szTemp, szParams, wParam)){
		*lpValue=0.0f;
		return FALSE;
	}

	*lpValue=(FLOAT)atof(szTemp);
	return TRUE;
}

BOOL BeemParseGetInt(
	LONG * lpValue,
	LPSTR szParams,
	WORD wParam)
{	
	char szTemp[100];

	if(!BeemParseGetParam(szTemp, szParams, wParam)){
		*lpValue=0;
		return FALSE;
	}
	*lpValue=atol(szTemp);
	return TRUE;
}

BOOL BeemParseGetString(
	LPSTR szStringOut,
	LPSTR szString)
{
	DWORD dwLen=0, i=0, j=0;
	BOOL bInQuotes=FALSE;
	BOOL bFoundString=FALSE;

	dwLen=strlen(szString);

	for(i=0, j=0; i<dwLen; i++){
		if(bInQuotes){
			if(szString[i]=='\"'){
				bFoundString=TRUE;
				break;
			}

			szStringOut[j]=szString[i];
			j++;
		}else{
			if(szString[i]=='\"')
				bInQuotes=TRUE;
		}
	}
	szStringOut[j]=0;

	if(bFoundString)
		return TRUE;
	else
		return FALSE;
}

BOOL BeemParseGetParam(
	LPSTR szParamOut, 
	LPSTR szParams, 
	WORD wParam)
{
	DWORD dwLen=0, i=0, j=0;
	BOOL bFoundParam=FALSE;

	if(wParam < 1)
		return FALSE;

	dwLen=strlen(szParams);

	/* Find the space char corresponding with the param. */
	wParam--;

	for(i=0, j=0; i<dwLen; i++){
		if(wParam==0){
			if(szParams[i]==' ')
				break;

			szParamOut[j]=szParams[i];
			j++;
		}else{
			if(szParams[i]==' '){
				wParam--;
			}
		}
	}
	szParamOut[j]=0;

	if(strlen(szParamOut) < 1 )
		return FALSE;
	else
		return TRUE;
}

/* =========================
   Beem Console Private Data
	=========================*/

typedef struct tagBEEMENTRY{
	LPSTR lpstrText;
	struct tagBEEMENTRY * lpNext;
}BEEMENTRY, *LPBEEMENTRY;

typedef struct tagBEEMCONSOLE{
	DWORD dwNumEntries;
	BEEMENTRY * lpActiveEntry;
	BEEMENTRY * lpEntryList;
	LPCOMMAND CommandFunction;
	HD3DFONT ConsoleFont;
	HD3DIMAGE ConsoleBG;
	BOOL bConsoleActive;
	BOOL bDeactivating;
	LONG lPosition;
	DWORD dwScrollPos;
	DWORD dwLastUpdate;
	LPDIRECT3DDEVICE9 lpDevice;
}BEEMCONSOLE, *LPBEEMCONSOLE;



/* Beem Console Private Functions declarations. */
BOOL BeemConsoleRemoveLastEntry(
	HBEEMCONSOLE lpConsole);

BOOL BeemConsoleSimpleParse(
	char szCommand[MAX_CHARSPERLINE], 
	char szParams[MAX_CHARSPERLINE], 
	char szIgnore[128],
	LPSTR szLineIn);

BOOL BeemConsoleEraseInput(
	HBEEMCONSOLE lpConsole);


/* ==============================
	BeemConsole Private Functions.
	==============================*/

BOOL BeemConsoleRemoveLastEntry(
	HBEEMCONSOLE lpConsole)
{
	LPBEEMENTRY lpCurrent=NULL;
	LPBEEMENTRY lpPrev=NULL;

	if(!lpConsole)
		return FALSE;

	if(((LPBEEMCONSOLE)lpConsole)->lpEntryList==NULL)
		return FALSE;

	lpCurrent=((LPBEEMCONSOLE)lpConsole)->lpEntryList;

	if(lpCurrent->lpNext==NULL)
		return TRUE;

	while(lpCurrent->lpNext){
		lpPrev=lpCurrent;
		lpCurrent=lpCurrent->lpNext;
	}

	/* Remove the last entry and set it's previous entry as the
		last entry. */
	SAFE_FREE(lpCurrent->lpstrText);
	SAFE_FREE(lpCurrent);
	lpPrev->lpNext=NULL;
	((LPBEEMCONSOLE)lpConsole)->dwNumEntries--;
	return TRUE;
}



BOOL BeemConsoleEraseInput(
	HBEEMCONSOLE lpConsole)
{
	if(lpConsole==NULL)
		return FALSE;

	if(!((LPBEEMCONSOLE)lpConsole)->lpActiveEntry)
		return FALSE;

	((LPBEEMCONSOLE)lpConsole)->lpActiveEntry->lpstrText[0]=0;
	return TRUE;
}

BOOL BeemConsoleSimpleParse(
	char szCommand[MAX_CHARSPERLINE], 
	char szParams[MAX_CHARSPERLINE], 
	char szIgnore[128],
	LPSTR szLineIn)
{
	char szTemp[MAX_CHARSPERLINE];
	DWORD dwLen=0, i=0, j=0, k=0, dwSpaces=0;
	BOOL bFoundChar=FALSE, bIgnore=FALSE;

	DWORD dwIgnoreLen=0;

	dwLen=strlen(szLineIn);
	dwIgnoreLen=strlen(szIgnore);

	/* First thing to do is remove any excess spaces. */
	for(i=0; i<dwLen; i++, bIgnore=FALSE){
		if(!bFoundChar)
			if(szLineIn[i]!=' ')
				bFoundChar=TRUE;

		if(bFoundChar){
			
			for(k=0; k<dwIgnoreLen; k++){
				if(szLineIn[i]==szIgnore[k])
					bIgnore=TRUE;
			}
			if(bIgnore){
				if(dwSpaces==0){
					szTemp[j]=' ';
					j++;
					dwSpaces++;
					continue;
				}else{
					dwSpaces++;
					continue;
				}
			}else{
				dwSpaces=0;
			}
				
			szTemp[j]=szLineIn[i];
			j++;
		}
	}

	szTemp[j]=0;

	dwLen=strlen(szTemp);


	/* Get the command (the first word). */
	for(i=0, j=0; i<dwLen; i++, j++){
		if(szTemp[i]==' ')
			break;
		szCommand[j]=szTemp[i];
	}
	szCommand[j]=0;

	/* Get the parameters. */
	j++;
	i++;
	for(j=0 ; i<dwLen; i++, j++){
		szParams[j]=szTemp[i];
	}
	szParams[j]=0;

	return TRUE;
}

/* ==============================
	Beem Console Public Functions.
	==============================*/

HBEEMCONSOLE CreateBeemConsole(
	LPDIRECT3DDEVICE9 lpDevice,
	HD3DFONT lpFont, 
	HD3DIMAGE lpConsoleBG,
	LPCOMMAND CommandFunction)
{
	LPBEEMCONSOLE lpConsole=NULL;

	HANDLE hBGImage=NULL;
	HRSRC hRsrc=NULL;
	LPVOID lpData=NULL;
	LPDIRECT3DTEXTURE9 lpBGTexture=NULL;
	HINSTANCE hInst=NULL;

	lpConsole=malloc(sizeof(BEEMCONSOLE));
	if(lpConsole==NULL)
		return NULL;
	
	hInst=GetModuleHandleA("GFX3D9.DLL");
	/* If the console background image was NULL we load the default one from memory. */
	if(lpConsoleBG==NULL){
		/* Should load default bg. */

		hRsrc=FindResourceA(
			hInst,
			MAKEINTRESOURCE(IDR_CONSOLEIMG),
			"binary");

		if(hRsrc==NULL)
			return NULL;

		hBGImage=LoadResource(
			hInst, 
			hRsrc);

		if(hBGImage==NULL)
			return NULL;

		lpData=LockResource(hBGImage);

		if(FAILED(D3DXCreateTextureFromFileInMemory(
			lpDevice,
			lpData,
			SizeofResource(hInst, hRsrc),
			&lpBGTexture)))
		{
			FreeResource(hBGImage);
			return NULL;
		}

		FreeResource(hBGImage);

		lpConsole->ConsoleBG=CreateD3DImageFromTexture(
			lpDevice,
			-1,
			-1,
			lpBGTexture);


		if(lpConsole->ConsoleBG==NULL)
		{
			lpBGTexture->lpVtbl->Release(lpBGTexture);
			return NULL;
		}

		lpBGTexture->lpVtbl->Release(lpBGTexture);

	}
	lpConsole->lpActiveEntry=NULL;
	lpConsole->lpEntryList=NULL;
	lpConsole->CommandFunction=CommandFunction;

	/* If the console image was not NULL we copy it. */
	if(lpConsoleBG!=NULL){
		lpConsole->ConsoleBG=CopyD3DImage(lpConsoleBG);
	}

	if(lpFont!=NULL){
		lpConsole->ConsoleFont=CopyD3DFont(lpDevice, lpFont);
	}else{
		lpConsole->ConsoleFont=GetD3DFontDefault(lpDevice);
		SetD3DFontSize(lpConsole->ConsoleFont, 8, 16);
	}
	/* Set some initial values for the console. */
	lpConsole->bConsoleActive=FALSE;
	lpConsole->lPosition=0;
	lpConsole->bDeactivating=FALSE;
	lpConsole->dwNumEntries=0;
	lpConsole->dwScrollPos=0;
	lpConsole->lpDevice=lpDevice;
	lpConsole->lpDevice->lpVtbl->AddRef(lpConsole->lpDevice);
	lpConsole->dwLastUpdate=timeGetTime();

	/* Add a blank entry. */
	AddBeemConsoleEntry((HBEEMCONSOLE)lpConsole, "");

	return (HBEEMCONSOLE)lpConsole;
}

BOOL DeleteBeemConsole(
	HBEEMCONSOLE lpConsole)
{
	LPBEEMENTRY lpCurrent=NULL, lpNext=NULL;
	/* Delete the list of entrys. */
	if(!lpConsole)
		return FALSE;

	lpCurrent=((LPBEEMCONSOLE)lpConsole)->lpEntryList;
	while(lpCurrent!=NULL){
		lpNext=lpCurrent->lpNext;
		/* Clear the text string. */
		SAFE_FREE(lpCurrent->lpstrText);
		/* Clear the entry. */
		SAFE_FREE(lpCurrent);
		lpCurrent=lpNext;
	}
	((LPBEEMCONSOLE)lpConsole)->lpActiveEntry=NULL;
	/* Delete the bg, and font. */
	DeleteD3DImage(((LPBEEMCONSOLE)lpConsole)->ConsoleBG);
	DeleteD3DFont(((LPBEEMCONSOLE)lpConsole)->ConsoleFont);

	((LPBEEMCONSOLE)lpConsole)->CommandFunction=NULL;
	((LPBEEMCONSOLE)lpConsole)->lpDevice->lpVtbl->Release(((LPBEEMCONSOLE)lpConsole)->lpDevice);

	SAFE_FREE(lpConsole);

	return TRUE;
}

BOOL ScrollBeemConsole(
	HBEEMCONSOLE lpConsole, 
	LONG lScrollDist)
{
	#define MIN_ENTRIES_PER_SCREEN 2

	if(lpConsole==NULL)
		return FALSE;

	if(!((LPBEEMCONSOLE)lpConsole)->bConsoleActive)
		return TRUE;


	if(((LPBEEMCONSOLE)lpConsole)->dwNumEntries <= (MIN_ENTRIES_PER_SCREEN-1)){
		((LPBEEMCONSOLE)lpConsole)->dwScrollPos=0;
		return TRUE;
	}

	if( (LONG)(((LPBEEMCONSOLE)lpConsole)->dwScrollPos+lScrollDist) > (LONG)(((LPBEEMCONSOLE)lpConsole)->dwNumEntries-(MIN_ENTRIES_PER_SCREEN))){
		((LPBEEMCONSOLE)lpConsole)->dwScrollPos=((LPBEEMCONSOLE)lpConsole)->dwNumEntries-(MIN_ENTRIES_PER_SCREEN);
		return TRUE;
	}

	if( (LONG)(((LPBEEMCONSOLE)lpConsole)->dwScrollPos+lScrollDist) < 0){
		((LPBEEMCONSOLE)lpConsole)->dwScrollPos=0;
		return TRUE;
	}

	((LPBEEMCONSOLE)lpConsole)->dwScrollPos+=lScrollDist;

	return TRUE;
}

BOOL BeemConsoleClearEntries(
	HBEEMCONSOLE lpConsole)
{
	LPBEEMENTRY lpCurrent=NULL;
	LPBEEMENTRY lpNext=NULL;

	if(!lpConsole)
		return FALSE;

	lpCurrent=((LPBEEMCONSOLE)lpConsole)->lpEntryList;
	while(lpCurrent!=NULL){
		lpNext=lpCurrent->lpNext;
		/* Clear the text string. */
		SAFE_FREE(lpCurrent->lpstrText);
		/* Clear the entry. */
		SAFE_FREE(lpCurrent);
		lpCurrent=lpNext;
	}
	((LPBEEMCONSOLE)lpConsole)->lpEntryList=NULL;
	((LPBEEMCONSOLE)lpConsole)->lpActiveEntry=NULL;
	((LPBEEMCONSOLE)lpConsole)->dwNumEntries=0;
	((LPBEEMCONSOLE)lpConsole)->dwScrollPos=0;
	AddBeemConsoleEntry(lpConsole, "");
	return TRUE;
}

BOOL SendBeemConsoleMessage(
	HBEEMCONSOLE lpConsole,
	LPSTR szMessage)
{
	char szTemp[MAX_CHARSPERLINE];
	if(!lpConsole)
		return FALSE;
	/* Save the current line. */
	strcpy(szTemp, ((LPBEEMCONSOLE)lpConsole)->lpActiveEntry->lpstrText);
	/* Copy in the new line. */
	strcpy(((LPBEEMCONSOLE)lpConsole)->lpActiveEntry->lpstrText, szMessage);
	/* Re-Add the old line. */
	return AddBeemConsoleEntry(
		lpConsole,
		szTemp);
}

BOOL SendBeemConsoleCommand(
	HBEEMCONSOLE hConsole,
	LPSTR szCommandLine)
{
	char szParams[MAX_CHARSPERLINE];
	char szCommand[MAX_CHARSPERLINE];
	char szParseIgnore[]=" ,()";
	DWORD dwLen=0;

	dwLen=strlen(szCommandLine);

	if(dwLen>0){
		/* First thing to do is a simple parse on the command. */
		if(!BeemConsoleSimpleParse(szCommand, szParams, szParseIgnore, szCommandLine)){
			SendBeemConsoleMessage(hConsole, "ERROR: COULD NOT PERFORM SIMPLE PARSE!");
		}else{
			if(((LPBEEMCONSOLE)hConsole)->CommandFunction==NULL){
				SendBeemConsoleMessage(hConsole, "ERROR: NO COMMAND PARSER LOADED!");
			}else{
				if(!((LPBEEMCONSOLE)hConsole)->CommandFunction(szCommand, szParams, hConsole))
					SendBeemConsoleMessage(hConsole, "--- ERROR: COULD NOT PARSE COMMAND ---");				
			}
		}
	}else{
		SendBeemConsoleMessage(hConsole, "");
	}

	return TRUE;
}


BOOL AddBeemConsoleEntry(
	HBEEMCONSOLE lpConsole,
	LPSTR szEntry)
{
	LPBEEMENTRY lpCurrent=NULL;

	if(!lpConsole)
		return FALSE;
	
	lpCurrent=((LPBEEMCONSOLE)lpConsole)->lpEntryList;
	((LPBEEMCONSOLE)lpConsole)->lpEntryList=malloc(sizeof(BEEMENTRY));
	if(((LPBEEMCONSOLE)lpConsole)->lpEntryList==NULL){
		((LPBEEMCONSOLE)lpConsole)->lpEntryList=lpCurrent;
		return FALSE;
	}
	((LPBEEMCONSOLE)lpConsole)->lpEntryList->lpstrText=malloc(sizeof(char)*MAX_CHARSPERLINE);
	if(((LPBEEMCONSOLE)lpConsole)->lpEntryList->lpstrText==NULL){
		SAFE_FREE(((LPBEEMCONSOLE)lpConsole)->lpEntryList);
		((LPBEEMCONSOLE)lpConsole)->lpEntryList=lpCurrent;
	}
	((LPBEEMCONSOLE)lpConsole)->lpEntryList->lpNext=lpCurrent;
	((LPBEEMCONSOLE)lpConsole)->lpActiveEntry=((LPBEEMCONSOLE)lpConsole)->lpEntryList;
	strcpy(((LPBEEMCONSOLE)lpConsole)->lpEntryList->lpstrText, szEntry);

	((LPBEEMCONSOLE)lpConsole)->dwNumEntries++;
	if(((LPBEEMCONSOLE)lpConsole)->dwNumEntries > LINE_BUFFER_SIZE)
		BeemConsoleRemoveLastEntry(lpConsole);

	return TRUE;
}

BOOL InvalidateBeemConsole(
	HBEEMCONSOLE lpConsole)
{
	if(!lpConsole)
		return FALSE;

	InvalidateD3DImage(((LPBEEMCONSOLE)lpConsole)->ConsoleBG);
	InvalidateD3DFont(((LPBEEMCONSOLE)lpConsole)->ConsoleFont);
	return TRUE;
}

BOOL ValidateBeemConsole(
	HBEEMCONSOLE lpConsole)
{
	if(!lpConsole)
		return FALSE;

	ValidateD3DImage(((LPBEEMCONSOLE)lpConsole)->ConsoleBG);
	ValidateD3DFont(((LPBEEMCONSOLE)lpConsole)->ConsoleFont);
	return TRUE;
}

BOOL ToggleActivateBeemConsole(
	HBEEMCONSOLE lpConsole)
{
	if(!lpConsole)
		return FALSE;
	
	/* If we weren't active, then we set to active. */
	if(!((LPBEEMCONSOLE)lpConsole)->bConsoleActive){
		((LPBEEMCONSOLE)lpConsole)->bDeactivating=FALSE;
		((LPBEEMCONSOLE)lpConsole)->bConsoleActive=TRUE;
	}else{
		if(((LPBEEMCONSOLE)lpConsole)->bDeactivating)
			((LPBEEMCONSOLE)lpConsole)->bDeactivating=FALSE;
		else
			((LPBEEMCONSOLE)lpConsole)->bDeactivating=TRUE;
	}
	return TRUE;
}

BOOL RenderBeemConsole(
	HBEEMCONSOLE lpConsole)
{
	LONG lYOffset=0;
	LONG lStringPos=0;
	WORD wFontWidth=0, wFontHeight=0;
	DWORD i=0;
	BOOL bFoundEntry=FALSE;
	DWORD dwScrollDist=0, dwTimeElapsed=0;
	D3DVIEWPORT9 ViewPort;

	LPBEEMENTRY lpCurrent=NULL;

	char szActiveString[MAX_CHARSPERLINE+1];

	if(!lpConsole)
		return FALSE;

	if(!((LPBEEMCONSOLE)lpConsole)->bConsoleActive){
		((LPBEEMCONSOLE)lpConsole)->dwLastUpdate=timeGetTime();
		return TRUE;
	}

	((LPBEEMCONSOLE)lpConsole)->lpDevice->lpVtbl->GetViewport(((LPBEEMCONSOLE)lpConsole)->lpDevice, &ViewPort);

	/* Figure out the scroll distance. */
	/* It should take approximately 1/2 sec. to scroll down completely. */
	dwTimeElapsed=timeGetTime()-((LPBEEMCONSOLE)lpConsole)->dwLastUpdate;
	if(dwTimeElapsed)
		((LPBEEMCONSOLE)lpConsole)->dwLastUpdate=timeGetTime();

	#define CONSOLE_SCROLL_SPEED 500
	dwScrollDist=(DWORD)(dwTimeElapsed * ( (FLOAT)(ViewPort.Height/2)/(FLOAT)CONSOLE_SCROLL_SPEED ));

	if(((LPBEEMCONSOLE)lpConsole)->bDeactivating){
		if(((LPBEEMCONSOLE)lpConsole)->lPosition<=0){
			((LPBEEMCONSOLE)lpConsole)->lPosition=0;
			((LPBEEMCONSOLE)lpConsole)->bConsoleActive=FALSE;
			return TRUE;
		}
			((LPBEEMCONSOLE)lpConsole)->lPosition-=dwScrollDist;
	}else{
		((LPBEEMCONSOLE)lpConsole)->lPosition+=dwScrollDist;
		if(((LPBEEMCONSOLE)lpConsole)->lPosition>(LONG)(ViewPort.Height/2))
			((LPBEEMCONSOLE)lpConsole)->lPosition=ViewPort.Height/2;

	}
	

	GetD3DFontDims(((LPBEEMCONSOLE)lpConsole)->ConsoleFont, &wFontWidth, &wFontHeight);
	/* We first set the string position to the bottom of the console. */
	lStringPos=(ViewPort.Height/2)-wFontHeight;
	lYOffset=ViewPort.Height/2-((LPBEEMCONSOLE)lpConsole)->lPosition;
	

	/* Render the console position. */
	RenderD3DImageEx(
		((LPBEEMCONSOLE)lpConsole)->ConsoleBG, 
		0, 
		0-lYOffset, 
		ViewPort.Width, 
		ViewPort.Height/2, 
		0, 
		0, 
		GetD3DImageWidth(((LPBEEMCONSOLE)lpConsole)->ConsoleBG), 
		GetD3DImageHeight(((LPBEEMCONSOLE)lpConsole)->ConsoleBG));

	/* Render each line. */
	/* Render active entry first. */

	RenderD3DFontChar(
		((LPBEEMCONSOLE)lpConsole)->ConsoleFont,
		0,
		lStringPos-lYOffset,
		']');

	/* If there is no entry we return. */
	if(!((LPBEEMCONSOLE)lpConsole)->lpActiveEntry)
		return TRUE;

	/* Create and render the active string. */
	sprintf(szActiveString, "%s_", ((LPBEEMCONSOLE)lpConsole)->lpActiveEntry->lpstrText);

	RenderD3DFontString(
		((LPBEEMCONSOLE)lpConsole)->ConsoleFont,
		wFontWidth,
		lStringPos-lYOffset,
		szActiveString);

	if(((LPBEEMCONSOLE)lpConsole)->dwScrollPos>0){
		lStringPos-=wFontHeight;

		for(i=0; i<ViewPort.Width; i+=wFontWidth*5){
			RenderD3DFontChar(
				((LPBEEMCONSOLE)lpConsole)->ConsoleFont,
				i,
				lStringPos-lYOffset,
				'^');
		}

	}
	

	lStringPos-=wFontHeight;

	

	/* Get the second line and render until no strings are left,
		or no space is left on the screen. */
	lpCurrent=((LPBEEMCONSOLE)lpConsole)->lpEntryList;
	lpCurrent=lpCurrent->lpNext;
	if( lpCurrent!=NULL && ((LPBEEMCONSOLE)lpConsole)->dwScrollPos > 0)
		lpCurrent=lpCurrent->lpNext;

	i=0;
	bFoundEntry=FALSE;

	while(lpCurrent){
		if(bFoundEntry){
			RenderD3DFontString(
				((LPBEEMCONSOLE)lpConsole)->ConsoleFont,
				wFontWidth,
				lStringPos-lYOffset,
				lpCurrent->lpstrText);

			lStringPos-=wFontHeight;

			/* Break if no rendering room left. */
			if((lStringPos-lYOffset)<(0-wFontHeight))
				break;
		}else{
			if(i>=((LPBEEMCONSOLE)lpConsole)->dwScrollPos){
				bFoundEntry=TRUE;
				continue;
			}		
			i++;
		}
		lpCurrent=lpCurrent->lpNext;
	}
	return TRUE;
}

BOOL BeemConsoleOnChar(
	HBEEMCONSOLE lpConsole,
	char cChar)
{
	DWORD dwLen=0;

	if(lpConsole==NULL)
		return FALSE;

	/* If console is not active bail. */
	if(!((LPBEEMCONSOLE)lpConsole)->bConsoleActive)
		return TRUE;
	/* If no active entry, then add an entry. */
	if(((LPBEEMCONSOLE)lpConsole)->lpActiveEntry==NULL)
		AddBeemConsoleEntry(lpConsole, "");

	dwLen=strlen(((LPBEEMCONSOLE)lpConsole)->lpActiveEntry->lpstrText);
	/* Do any necessary actions depending on cChar. */
	switch(cChar)
	{
	case VK_RETURN:
		SendBeemConsoleCommand(lpConsole, ((LPBEEMCONSOLE)lpConsole)->lpActiveEntry->lpstrText);
		BeemConsoleEraseInput(lpConsole);
		break;
	case VK_BACK:
		if(dwLen>0){
			/* Remove last char, and add null-termination. */
			((LPBEEMCONSOLE)lpConsole)->lpActiveEntry->lpstrText[dwLen-1]=0;
		}
		break;
	default:
		/* The funcion made it here if a standard key was pressed. */
		/* Break if line has reached it's max limit. */
		if(dwLen >= (MAX_CHARSPERLINE-1))
			return FALSE;
		
		/* Add the char into the string and a null-termination. */
		if((cChar >= ' ') && (cChar <= '~')){
			((LPBEEMCONSOLE)lpConsole)->lpActiveEntry->lpstrText[dwLen]=cChar;
			((LPBEEMCONSOLE)lpConsole)->lpActiveEntry->lpstrText[dwLen+1]=0;
		}
		break;
	};


	return TRUE;
}