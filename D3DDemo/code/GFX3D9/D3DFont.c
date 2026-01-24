/*
	D3DFont.c - Functionality for font objects.

	Copyright (c) 2003 Blaine Myers
*/
#include <d3dx9.h>
#include "GFX3D9.h"
#include "resource.h"

typedef struct tagD3DFONT{
	HD3DIMAGE FontImage;
	WORD wCharsPerLine;
	WORD wNumLines;
	float fCharWidth;
	float fCharHeight;
	WORD wFontHeight;
	WORD wFontWidth;
	BOOL bUseD3DFont;
	LPD3DXFONT lpD3DXFont;
	HFONT hD3DFont;
	DWORD dwD3DColor;
	LPDIRECT3DDEVICE9 lpDevice;
}D3DFONT, *LPD3DFONT;

HD3DFONT CopyD3DFont(
	LPDIRECT3DDEVICE9 lpDevice,
	HD3DFONT lpFontSrc)
{
	HD3DFONT tempDest=NULL;

	if(!lpFontSrc)
		return NULL;
	
	if(!((LPD3DFONT)lpFontSrc)->bUseD3DFont){
		tempDest=CreateD3DFontFromD3DImage(lpDevice, ((LPD3DFONT)lpFontSrc)->FontImage, ((LPD3DFONT)lpFontSrc)->wCharsPerLine, ((LPD3DFONT)lpFontSrc)->wNumLines);
		((LPD3DFONT)tempDest)->wFontHeight=((LPD3DFONT)lpFontSrc)->wFontHeight;
		((LPD3DFONT)tempDest)->wFontWidth=((LPD3DFONT)lpFontSrc)->wFontWidth;
		return tempDest;
	}else{
		return CreateD3DFontFromFont(lpDevice, ((LPD3DFONT)lpFontSrc)->hD3DFont, ((LPD3DFONT)lpFontSrc)->dwD3DColor);
	}

	return (HD3DFONT)tempDest;
}

HD3DFONT GetD3DFontDefault(
	LPDIRECT3DDEVICE9 lpDevice)
{
	HANDLE hFontRes=NULL;
	HRSRC hRsrc=NULL;
	LPBYTE lpData=NULL;
	HD3DFONT hD3DFont=NULL;
	LPDIRECT3DTEXTURE9 lpFontTex=NULL;
	HINSTANCE hInst=NULL;

	hInst=GetModuleHandleA("GFX3D9.DLL");

	hRsrc=FindResourceA(hInst, MAKEINTRESOURCE(IDB_DEFFONT), RT_BITMAP);

	if(hRsrc==NULL)
		return NULL;

	hFontRes=LoadResource(hInst, hRsrc);

	if(hFontRes==NULL)
		return NULL;

	lpData=LockResource(hFontRes);

	if(lpData==NULL){
		DeleteObject(hFontRes);
		return NULL;
	}

	if(FAILED(D3DXCreateTextureFromFileInMemoryEx(
		lpDevice,
		lpData,
		SizeofResource(hInst, hRsrc),
		0,
		0,
		0,
		0,
		D3DFMT_UNKNOWN,
		D3DPOOL_MANAGED,
		D3DX_FILTER_POINT,
		D3DX_FILTER_POINT,
		0xFFFF00FF,
		NULL,
		NULL,
		&lpFontTex)))
	{
		DeleteObject(hFontRes);
		return FALSE;
	}
	DeleteObject(hFontRes);

	hD3DFont=CreateD3DFontFromTexture(
		lpDevice,
		lpFontTex,
		8,
		16);

	SetD3DFontSize(hD3DFont, 16, 16);

	lpFontTex->lpVtbl->Release(lpFontTex);

	return hD3DFont;
}

HD3DFONT CreateD3DFontFromFont(
	LPDIRECT3DDEVICE9 lpDevice,
	HFONT hFont,
	DWORD dwColor)
{
	LPD3DFONT lpFont=NULL;
	LOGFONT TempData;

	if(!hFont)
		return NULL;
	
	lpFont=malloc(sizeof(D3DFONT));
	if(lpFont==NULL)
		return NULL;

	ZeroMemory(&TempData, sizeof(LOGFONT));

	lpFont->FontImage=NULL;
	lpFont->wCharsPerLine=0;
	lpFont->wNumLines=0;
	lpFont->fCharHeight=0.0f;
	lpFont->fCharWidth=0.0f;
	lpFont->bUseD3DFont=TRUE;
	lpFont->dwD3DColor=dwColor;

	lpFont->hD3DFont=hFont;
	lpFont->lpD3DXFont=NULL;

	GetObject(hFont, sizeof(LOGFONT), &TempData);
	lpFont->wFontHeight=(WORD)TempData.lfHeight;
	lpFont->wFontWidth=(WORD)TempData.lfWidth;

	lpFont->lpDevice=lpDevice;
	lpFont->lpDevice->lpVtbl->AddRef(lpFont->lpDevice);
	
	/* Create the font. */
	if(!ValidateD3DFont((HD3DFONT)lpFont)){
		SAFE_FREE(lpFont);
		lpFont->lpDevice->lpVtbl->Release(lpFont->lpDevice);
		return NULL;
	}

	return (HD3DFONT)lpFont;
}

HD3DFONT CreateD3DFontFromD3DImage(
	LPDIRECT3DDEVICE9 lpDevice,
	HD3DIMAGE hImage,
	WORD wCharsPerLine,
	WORD wNumLines)
{
	LPD3DFONT lpFont=NULL;

	lpFont=malloc(sizeof(D3DFONT));
	if(lpFont==NULL)
		return NULL;

	lpFont->FontImage=CopyD3DImage(hImage);

	lpFont->wCharsPerLine=wCharsPerLine;
	lpFont->wNumLines=wNumLines;

	lpFont->fCharWidth=(float)GetD3DImageWidth(lpFont->FontImage)/(float)wCharsPerLine;
	lpFont->fCharHeight=(float)GetD3DImageHeight(lpFont->FontImage)/(float)wNumLines;

	lpFont->wFontWidth=16;
	lpFont->wFontHeight=32;

	lpFont->bUseD3DFont=FALSE;
	lpFont->hD3DFont=NULL;
	lpFont->lpD3DXFont=NULL;
	lpFont->dwD3DColor=0;

	lpFont->lpDevice=lpDevice;
	lpFont->lpDevice->lpVtbl->AddRef(lpFont->lpDevice);


	return (HD3DFONT)lpFont;
}

HD3DFONT CreateD3DFontFromImageA(
	LPDIRECT3DDEVICE9 lpDevice,
	char szFilename[MAX_PATH],
	WORD wCharsPerLine,
	WORD wNumLines)
{
	LPD3DFONT lpFont=NULL;

	lpFont=malloc(sizeof(D3DFONT));
	if(lpFont==NULL)
		return NULL;

	/* Attempt to load the font's image. */
	lpFont->FontImage=CreateD3DImageFromFileA(
		lpDevice,
		szFilename,
		-1,
		-1,
		0xFFFF00FF);
	if(lpFont->FontImage==NULL)
	{
		SAFE_FREE(lpFont);
		return NULL;
	}

	lpFont->wCharsPerLine=wCharsPerLine;
	lpFont->wNumLines=wNumLines;

	lpFont->fCharWidth=(float)GetD3DImageWidth(lpFont->FontImage)/(float)wCharsPerLine;
	lpFont->fCharHeight=(float)GetD3DImageHeight(lpFont->FontImage)/(float)wNumLines;

	lpFont->wFontWidth=16;
	lpFont->wFontHeight=32;

	lpFont->bUseD3DFont=FALSE;
	lpFont->hD3DFont=NULL;
	lpFont->lpD3DXFont=NULL;
	lpFont->dwD3DColor=0;

	lpFont->lpDevice=lpDevice;
	lpFont->lpDevice->lpVtbl->AddRef(lpFont->lpDevice);

	return (HD3DFONT)lpFont;
}

HD3DFONT CreateD3DFontFromImageW(
	LPDIRECT3DDEVICE9 lpDevice,
	WCHAR szFilename[MAX_PATH],
	WORD wCharsPerLine,
	WORD wNumLines)
{
	LPD3DFONT lpFont=NULL;

	lpFont=malloc(sizeof(D3DFONT));
	if(lpFont==NULL)
		return NULL;

	/* Attempt to load the font's image. */
	lpFont->FontImage=CreateD3DImageFromFileW(
		lpDevice,
		szFilename,
		-1,
		-1,
		0xFFFF00FF);
	if(lpFont->FontImage==NULL)
	{
		SAFE_FREE(lpFont);
		return NULL;
	}

	lpFont->wCharsPerLine=wCharsPerLine;
	lpFont->wNumLines=wNumLines;

	lpFont->fCharWidth=(float)GetD3DImageWidth(lpFont->FontImage)/(float)wCharsPerLine;
	lpFont->fCharHeight=(float)GetD3DImageHeight(lpFont->FontImage)/(float)wNumLines;

	lpFont->wFontWidth=16;
	lpFont->wFontHeight=32;

	lpFont->bUseD3DFont=FALSE;
	lpFont->hD3DFont=NULL;
	lpFont->lpD3DXFont=NULL;
	lpFont->dwD3DColor=0;

	lpFont->lpDevice=lpDevice;
	lpFont->lpDevice->lpVtbl->AddRef(lpFont->lpDevice);

	return (HD3DFONT)lpFont;
}

HD3DFONT CreateD3DFontFromTexture(
	LPDIRECT3DDEVICE9 lpDevice,
	LPDIRECT3DTEXTURE9 lpTexture,
	WORD wCharsPerLine,
	WORD wNumLines)
{
	LPD3DFONT lpFont=NULL;
	
	lpFont=malloc(sizeof(D3DFONT));
	if(lpFont==NULL)
		return NULL;

	/* Attempt to load the font's image. */
	lpFont->FontImage=CreateD3DImageFromTexture(
		lpDevice,
		-1,
		-1,
		lpTexture);
	if(lpFont->FontImage==NULL)
	{
		SAFE_FREE(lpFont);
		return NULL;
	}

	lpFont->wCharsPerLine=wCharsPerLine;
	lpFont->wNumLines=wNumLines;

	lpFont->fCharWidth=(float)GetD3DImageWidth(lpFont->FontImage)/(float)wCharsPerLine;
	lpFont->fCharHeight=(float)GetD3DImageHeight(lpFont->FontImage)/(float)wNumLines;

	lpFont->wFontWidth=16;
	lpFont->wFontHeight=32;

	lpFont->bUseD3DFont=FALSE;
	lpFont->hD3DFont=NULL;
	lpFont->lpD3DXFont=NULL;
	lpFont->dwD3DColor=0;

	lpFont->lpDevice=lpDevice;
	lpFont->lpDevice->lpVtbl->AddRef(lpFont->lpDevice);

	return (HD3DFONT)lpFont;
}

BOOL InvalidateD3DFont(
	HD3DFONT lpFont)
{
	if(!lpFont)
		return FALSE;
	if(!((LPD3DFONT)lpFont)->bUseD3DFont)
		return InvalidateD3DImage(((LPD3DFONT)lpFont)->FontImage);
	else{
		/* Release D3DXFont. */
		SAFE_RELEASE((((LPD3DFONT)lpFont)->lpD3DXFont));
		return TRUE;
	}
}

BOOL ValidateD3DFont(
	HD3DFONT hFont)
{
	LPD3DFONT lpFont=hFont;
	if(!lpFont)
		return FALSE;

	/* If we're not using a D3DXFONT we validate the image. Otherwize we create the font. */
	if(!((LPD3DFONT)lpFont)->bUseD3DFont)
		return ValidateD3DImage(((LPD3DFONT)lpFont)->FontImage);
	else{
		/* Create D3D font here. */
		if(((LPD3DFONT)lpFont)->lpD3DXFont)
			return TRUE;

		if(FAILED(D3DXCreateFont(
			((LPD3DFONT)lpFont)->lpDevice,
			(INT)lpFont->fCharHeight,
			(UINT)lpFont->fCharWidth,
			1,
			1,
			FALSE,
			0,
			0,
			0,
			0,
			"Courier New",
			//((LPD3DFONT)lpFont)->hD3DFont, 
			&((LPD3DFONT)lpFont)->lpD3DXFont)))
			return FALSE;
		return TRUE;
	}
}

BOOL DeleteD3DFont(
	HD3DFONT lpFont)
{
	if(!lpFont)
		return FALSE;

	if(!((LPD3DFONT)lpFont)->bUseD3DFont){
		DeleteD3DImage(((LPD3DFONT)lpFont)->FontImage);
	}else{
		/* Release D3DX Font. */
		SAFE_RELEASE((((LPD3DFONT)lpFont)->lpD3DXFont));
	}
	SAFE_RELEASE( (((LPD3DFONT)lpFont)->lpDevice) );
	SAFE_FREE(lpFont);
	return TRUE;
}

BOOL GetD3DFontDims(HD3DFONT lpFont, WORD * lpWidth, WORD * lpHeight)
{
	if(!lpFont)
		return FALSE;

	if(lpWidth)
		*lpWidth=((LPD3DFONT)lpFont)->wFontWidth;

	if(lpHeight)
		*lpHeight=((LPD3DFONT)lpFont)->wFontHeight;

	return TRUE;
}

BOOL RenderD3DFontChar(
	HD3DFONT lpFont,
	LONG nXDest,
	LONG nYDest,
	char cChar)
{
	WORD nColumn=0, nRow=0;
	RECT rcDest;
	DWORD dwSavedMin=0, dwSavedMag=0, dwSavedMip=0;
	BOOL bResult=TRUE;

	if(!lpFont)
		return FALSE;


	if(!((LPD3DFONT)lpFont)->bUseD3DFont){
		if(cChar<0x0020 || cChar>0x007E){
			/* Change to space for nonexistant characters. */
			cChar=0;
		}else{
			cChar-=32;
		}

		if(cChar==0){
			nRow=0;
			nColumn=0;
		}else{
			nRow=cChar/((LPD3DFONT)lpFont)->wCharsPerLine;
			nColumn=cChar%((LPD3DFONT)lpFont)->wCharsPerLine;
		}

		/* Set the texture filter to point filtering. */
		((LPD3DFONT)lpFont)->lpDevice->lpVtbl->GetSamplerState(((LPD3DFONT)lpFont)->lpDevice, 0, D3DSAMP_MAGFILTER, &dwSavedMag);
		((LPD3DFONT)lpFont)->lpDevice->lpVtbl->GetSamplerState(((LPD3DFONT)lpFont)->lpDevice, 0, D3DSAMP_MINFILTER, &dwSavedMin);
		((LPD3DFONT)lpFont)->lpDevice->lpVtbl->GetSamplerState(((LPD3DFONT)lpFont)->lpDevice, 0, D3DSAMP_MIPFILTER, &dwSavedMip);

		((LPD3DFONT)lpFont)->lpDevice->lpVtbl->SetSamplerState(((LPD3DFONT)lpFont)->lpDevice, 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
		((LPD3DFONT)lpFont)->lpDevice->lpVtbl->SetSamplerState(((LPD3DFONT)lpFont)->lpDevice, 0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
		((LPD3DFONT)lpFont)->lpDevice->lpVtbl->SetSamplerState(((LPD3DFONT)lpFont)->lpDevice, 0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
	
		bResult=RenderD3DImageEx(
			((LPD3DFONT)lpFont)->FontImage,
			nXDest,
			nYDest,
			((LPD3DFONT)lpFont)->wFontWidth,
			((LPD3DFONT)lpFont)->wFontHeight,
			(LONG)(nColumn*((LPD3DFONT)lpFont)->fCharWidth),
			(LONG)(nRow*((LPD3DFONT)lpFont)->fCharHeight),
			(LONG)((LPD3DFONT)lpFont)->fCharWidth,
			(LONG)((LPD3DFONT)lpFont)->fCharHeight);

		/* Restore old filter format. */
		((LPD3DFONT)lpFont)->lpDevice->lpVtbl->SetSamplerState(((LPD3DFONT)lpFont)->lpDevice, 0, D3DSAMP_MAGFILTER, dwSavedMag);
		((LPD3DFONT)lpFont)->lpDevice->lpVtbl->SetSamplerState(((LPD3DFONT)lpFont)->lpDevice, 0, D3DSAMP_MINFILTER, dwSavedMin);
		((LPD3DFONT)lpFont)->lpDevice->lpVtbl->SetSamplerState(((LPD3DFONT)lpFont)->lpDevice, 0, D3DSAMP_MIPFILTER, dwSavedMip);

		return bResult;
	}else{
		/* Render D3DX Font string. */
		if(((LPD3DFONT)lpFont)->lpD3DXFont==NULL)
			return FALSE;
		nXDest++;
		nYDest++;

//		((LPD3DFONT)lpFont)->lpD3DXFont->lpVtbl->Begin(((LPD3DFONT)lpFont)->lpD3DXFont);

		SetRect(&rcDest, nXDest, nYDest, 0, 0);

		((LPD3DFONT)lpFont)->lpD3DXFont->lpVtbl->DrawTextA(((LPD3DFONT)lpFont)->lpD3DXFont, NULL, (char *)&cChar, 1, &rcDest, 0, ((LPD3DFONT)lpFont)->dwD3DColor);

//		((LPD3DFONT)lpFont)->lpD3DXFont->lpVtbl->End(((LPD3DFONT)lpFont)->lpD3DXFont);

		return TRUE;
	}
		
}

BOOL SetD3DFontSize(
	HD3DFONT lpFont,
	WORD wWidth,
	WORD wHeight)
{
	if(!lpFont)
		return FALSE;

	((LPD3DFONT)lpFont)->wFontHeight=wHeight;
	((LPD3DFONT)lpFont)->wFontWidth=wWidth;
	return TRUE;
}

BOOL RenderD3DFontString(
	HD3DFONT lpFont,
	LONG nXDest,
	LONG nYDest,
	char szString[])
{
	DWORD i=0, dwXOffset=0;
	LONG y=nYDest;

	RECT rcDest;

	if(!lpFont)
		return FALSE;

	if(((LPD3DFONT)lpFont)->bUseD3DFont){
		SetRect(&rcDest, nXDest, nYDest, 0, 0);

		nXDest++;
		nYDest++;

//		((LPD3DFONT)lpFont)->lpD3DXFont->lpVtbl->Begin(((LPD3DFONT)lpFont)->lpD3DXFont);

		((LPD3DFONT)lpFont)->lpD3DXFont->lpVtbl->DrawTextA(((LPD3DFONT)lpFont)->lpD3DXFont, NULL, szString, -1, &rcDest, DT_NOPREFIX, 0xFFFFFF00);

//		((LPD3DFONT)lpFont)->lpD3DXFont->lpVtbl->End(((LPD3DFONT)lpFont)->lpD3DXFont);

	}else{
		while(szString[i]!=0){
			if(szString[i]=='\n'){
				dwXOffset=0;
				y+=((LPD3DFONT)lpFont)->wFontHeight;
				i++;
				continue;
			}	
	

			RenderD3DFontChar(
				lpFont,
				nXDest+dwXOffset*((LPD3DFONT)lpFont)->wFontWidth,
				y,
				szString[i]);

			i++;
			dwXOffset++;
		}
	}
	return TRUE;
}