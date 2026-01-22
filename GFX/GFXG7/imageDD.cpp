/*
main.cpp Entry point for GFXG7 Dynamic-Link Library

Copyright (c) 2002, 2003 Blaine Myers
*/
/*
GFXG is a libray with functions for manipulating graphics
in a 2D environment.  It features both a Direct3D8 and
DirectDraw7 interfaces.  The functions and classes provided
are intended for easy implementation.

GFXG was originally built for ScrollGin technology.
*/
/*
	ImageDD.cpp - the DirectDraw image class

	Copyright (c) 2002, 2003 Blaine Myers
*/


#include <stdio.h>
#include "defines.h"
#include "gfxg7.h"

CImage7::CImage7(){
	m_lpImage=NULL;
	m_hBitmap=NULL;
}

CImage7::~CImage7(){
}


HRESULT CImage7::CreateSurface(DWORD nWidth, DWORD nHeight, LPVOID lpObject, DWORD dwTransparent){
	//Make sure DirectDraw exists
	if(!lpObject)return E_FAIL;
	//if the surface already exist we clear it out
	ClearSurface();

	//Generate surface descriptors and create surface
	DDSURFACEDESC2 ddsd;
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
   ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
   ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
   ddsd.dwWidth = nWidth;
   ddsd.dwHeight = nHeight;
	//create the surface
	if(((LPDIRECTDRAW7)lpObject)->CreateSurface(&ddsd, &m_lpImage, NULL) != DD_OK)
		return E_FAIL;

	//Set transparent color 
	DDCOLORKEY ddck;
	ddck.dwColorSpaceLowValue=ddck.dwColorSpaceHighValue=dwTransparent;
	m_lpImage->SetColorKey(DDCKEY_SRCBLT, &ddck);

	if(m_lpImage)return S_OK;
	else return E_FAIL;
	return S_OK;
}

HRESULT CImage7::LoadColorIntoSurface(DWORD dwColor){
	//check to make sure surface exists
	if(!m_lpImage)return E_FAIL;

	DDBLTFX ddbltfx;
   ZeroMemory(&ddbltfx, sizeof(ddbltfx));
   ddbltfx.dwSize = sizeof(ddbltfx);
   ddbltfx.dwFillColor = dwColor;

   if(SUCCEEDED(m_lpImage->Blt( NULL, NULL, NULL, DDBLT_COLORFILL, &ddbltfx ))){
		return S_OK;
	}else return E_FAIL;
}

DWORD CImage7::GetWidth(){
	return m_sImageData.nWidth;
}

DWORD CImage7::GetHeight(){
	return m_sImageData.nHeight;
}


HRESULT CImage7::DrawPrefered(LPVOID lpBuffer, int x, int y){
	//clipped image is always prefered
	return DrawClippedImage(lpBuffer, x, y);
	//return DrawImage(lpBuffer, x, y);
}

HRESULT CImage7::DrawClippedImage(LPVOID lpBuffer, int x, int y){
	if(!lpBuffer || !m_lpImage)return E_FAIL;
	
	RECT rcSrc, rcDest;
	
	DDSURFACEDESC2 ddBufferDesc;
	ZeroMemory(&ddBufferDesc, sizeof(ddBufferDesc));
	ddBufferDesc.dwSize=sizeof(DDSURFACEDESC2);
	if(FAILED(((LPDIRECTDRAWSURFACE7)lpBuffer)->GetSurfaceDesc(&ddBufferDesc)))return E_FAIL;
	
	DWORD nBufferWidth=ddBufferDesc.dwWidth;
	DWORD nBufferHeight=ddBufferDesc.dwHeight;
	
	//if we don't need to clip we do a regular blt
	if(
		//withing the width
		(x>=0) &&
		((m_sImageData.nWidth+x)<nBufferWidth) &&
		//within the height
		(y>=0) &&
		((m_sImageData.nHeight+y)<nBufferHeight)
	)return DrawImage(lpBuffer, x, y);

	//if the image is off screen we do no blt
	if(x>(int)nBufferWidth)return S_FALSE;
	if(y>(int)nBufferHeight)return S_FALSE;
	if((x+m_sImageData.nWidth)<=0)return S_FALSE;
	if((y+m_sImageData.nHeight)<=0)return S_FALSE;

	//if it has been determined that we need to do a clipped blt lets prepare the rectangles

	//prepare destination rectangle
	if(x>=0)
		rcDest.left=x;
	else
		rcDest.left=0;

	if((x+m_sImageData.nWidth)<nBufferWidth)
		rcDest.right=x+m_sImageData.nWidth;
	else
		rcDest.right=nBufferWidth;

	if(y>=0)
		rcDest.top=y;
	else
		rcDest.top=0;

	if((y+m_sImageData.nHeight)<nBufferHeight)
		rcDest.bottom=y+m_sImageData.nHeight;
	else
		rcDest.bottom=nBufferHeight;

	
	//prepare src rectangle
	if(x>=0)
		rcSrc.left=0;
	else
		rcSrc.left=0-x;

	if((x+m_sImageData.nWidth)<nBufferWidth)
		rcSrc.right=m_sImageData.nWidth;
	else
		rcSrc.right=nBufferWidth-x;

	if(y>=0)
		rcSrc.top=0;
	else
		rcSrc.top=0-y;

	if((y+m_sImageData.nHeight)<nBufferHeight)
		rcSrc.bottom=m_sImageData.nHeight;
	else
		rcSrc.bottom=nBufferHeight-y;


	if(SUCCEEDED(((LPDIRECTDRAWSURFACE7)lpBuffer)->Blt(&rcDest, m_lpImage, &rcSrc, DDBLT_KEYSRC|DDBLT_WAIT, NULL))){
		return S_OK;
	}
	return E_FAIL;
}

HRESULT CImage7::DrawImage(LPVOID lpBuffer, int x, int y){
	if(!lpBuffer || !m_lpImage)return E_FAIL;

	RECT rcSrc, rcDest;

	rcDest.left=x;
	rcDest.right=x+m_sImageData.nWidth;
	rcDest.top=y;
	rcDest.bottom=y+m_sImageData.nHeight;

	rcSrc.top=rcSrc.left=0;
	rcSrc.bottom=m_sImageData.nHeight;
	rcSrc.right=m_sImageData.nWidth;

	if(SUCCEEDED(((LPDIRECTDRAWSURFACE7)lpBuffer)->Blt(&rcDest, m_lpImage, &rcSrc, DDBLT_KEYSRC|DDBLT_WAIT, NULL))){
			return S_OK;
	}
	return E_FAIL;
}

HRESULT CImage7::Restore(){
	m_lpImage->Restore();
	return S_OK;
}

HRESULT CImage7::ReloadImageIntoSurface(){
	switch(m_sImageData.nType){
	case ET_BITMAP:
		if(SUCCEEDED(LoadBitmapInMemoryIntoSurface(
			m_hBitmap, 
			m_sImageData.nWidth, 
			m_sImageData.nHeight)))return S_OK;
		else return E_FAIL;
		break;
	case ET_COLOR:
		if(SUCCEEDED(LoadColorIntoSurface(m_sImageData.dwColor)))return S_OK;
		else return E_FAIL;
		break;
	default: return E_FAIL;
	}
}


HRESULT CImage7::ClearSurface(){
	Release();
	return S_OK;
}

LPDIRECTDRAWSURFACE7* CImage7::GetPointerToSurface(){
	return &m_lpImage;
}

void CImage7::Release(){
	SAFE_RELEASE(m_lpImage);
	DeleteObject(m_hBitmap);
}

HRESULT CImage7::LoadBitmapInMemoryIntoSurface(
		HBITMAP hBitmap,
		DWORD nWidth,
		DWORD nHeight)
{
	HDC hdcSurface=0, hdcImage=0;

	m_lpImage->GetDC(&hdcSurface);
	hdcImage=CreateCompatibleDC(hdcSurface);
	
	SelectObject(hdcImage, hBitmap);

	SetMapMode(hdcImage, GetMapMode(hdcSurface));
	SetStretchBltMode(hdcImage, COLORONCOLOR);

	StretchBlt(
		hdcSurface,
		0,
		0,
		nWidth,
		nHeight,
		hdcImage,
		0,
		0,
		nWidth,
		nHeight,
		SRCCOPY);

	DeleteDC(hdcImage);
	m_lpImage->ReleaseDC(hdcSurface);

	return S_OK;
}

HRESULT CImage7::CreateImageBMInMemory(
	LPVOID lpObject,
	DWORD dwTransparent,
	HBITMAP hBitmap,
	int nX,
	int nY,
	int nSrcWidth,
	int nSrcHeight,
	DWORD nWidth,
	DWORD nHeight,
	DWORD dwReverseFlags)
{
	if(!hBitmap)return E_FAIL;

	if(FAILED(CreateSurface(nWidth, nHeight, (LPDIRECTDRAW7)lpObject, dwTransparent)))return E_FAIL;

	HDC hdcSurface=0, hdcImage=0, hdcImageDest=0;

	int nXFinal=nX, nYFinal=nY;
	int nWidthFinal=nSrcWidth, nHeightFinal=nSrcHeight;

	if( RV_LEFTRIGHT == (dwReverseFlags&RV_LEFTRIGHT)){
		nXFinal=nX+nSrcWidth-1;
		nWidthFinal=-nSrcWidth;
	}
	if( RV_UPDOWN == (dwReverseFlags&RV_UPDOWN)){
		nYFinal=nY+nSrcHeight-1;
		nHeightFinal=-nSrcHeight;
	}
	
	m_lpImage->GetDC(&hdcSurface);

	hdcImage=CreateCompatibleDC(hdcSurface);
	hdcImageDest=CreateCompatibleDC(hdcSurface);
	m_hBitmap=CreateCompatibleBitmap(hdcSurface, nWidth, nHeight);

	SelectObject(hdcImage, hBitmap);
	SelectObject(hdcImageDest, m_hBitmap);

	SetMapMode(hdcImage, GetMapMode(hdcSurface));
	SetStretchBltMode(hdcImage, COLORONCOLOR);

	StretchBlt(
		hdcImageDest,
		0,
		0,
		nWidth,
		nHeight,
		hdcImage,
		nXFinal,
		nYFinal,
		nWidthFinal,
		nHeightFinal,
		SRCCOPY);


	DeleteDC(hdcImage);
	DeleteDC(hdcImageDest);
	m_lpImage->ReleaseDC(hdcSurface);

	LoadBitmapInMemoryIntoSurface(m_hBitmap, nWidth, nHeight);
	m_sImageData.nWidth=nWidth;
	m_sImageData.nHeight=nHeight;
	m_sImageData.nType=ET_BITMAP;

	return S_OK;
}

HRESULT CImage7::CreateImageBMA(
	LPVOID lpDevice,
	DWORD dwTransparent,
	char szBitmapFilename[MAX_PATH],
	int nX,
	int nY,
	int nSrcWidth,
	int nSrcHeight,
	DWORD nWidth,
	DWORD nHeight,
	DWORD dwReverseFlags)
{
	HBITMAP hBitmap=0;
	hBitmap=(HBITMAP)LoadImageA(NULL, szBitmapFilename, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	HRESULT hr=0;
	hr=CreateImageBMInMemory(lpDevice, dwTransparent, hBitmap, nX, nY, nSrcWidth, nSrcHeight, nWidth, nHeight, dwReverseFlags);
	DeleteObject(hBitmap);

	return hr;
}

HRESULT CImage7::CreateImageBMW(
	LPVOID lpDevice,
	DWORD dwTransparent,
	WCHAR szBitmapFilename[MAX_PATH],
	int nX,
	int nY,
	int nSrcWidth,
	int nSrcHeight,
	DWORD nWidth,
	DWORD nHeight,
	DWORD dwReverseFlags)
{
	HBITMAP hBitmap=0;
	hBitmap=(HBITMAP)LoadImageW(NULL, szBitmapFilename, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	HRESULT hr=0;
	hr=CreateImageBMInMemory(lpDevice, dwTransparent, hBitmap, nX, nY, nSrcWidth, nSrcHeight, nWidth, nHeight, dwReverseFlags);
	DeleteObject(hBitmap);

	return hr;
}

HRESULT CImage7::CreateImageColor(
	LPVOID lpDevice,
	DWORD dwTransparent,
	DWORD dwColor, 
	DWORD nWidth, 
	DWORD nHeight)
{
	if(FAILED(CreateSurface(nWidth, nHeight, (LPDIRECTDRAW7)lpDevice, dwTransparent)))return E_FAIL;
	if(FAILED(LoadColorIntoSurface(dwColor)))return E_FAIL;

	m_sImageData.dwColor=dwColor;
	m_sImageData.nType=ET_COLOR;
	return S_OK;
}

