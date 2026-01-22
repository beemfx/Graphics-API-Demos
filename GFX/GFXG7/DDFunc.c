/*
	DDFunc.c - Functions for setting up and using DirectDraw.

	Copyright (c) 2003 Blaine Myers
*/
#include <windowsx.h>
#include "GFXG7.h"

HRESULT PageFlip(
	LPDIRECTDRAWSURFACE7 lpPrimary, 
	LPDIRECTDRAWSURFACE7 lpBackBuffer, 
	BOOL bWindowed, 
	RECT * rcWindow, 
	HRESULT ( * Restore)())
{
	HRESULT hr=0;
	RECT rcSource;
	DDSURFACEDESC2 desc;

	/* Bail if buffers do not exist as we cannot do anything. */
	if(!lpPrimary || !lpBackBuffer)
		return E_POINTER;

	while(TRUE){
		if(bWindowed){
			desc.dwSize=sizeof(DDSURFACEDESC2);
			desc.dwFlags=DDSD_WIDTH|DDSD_HEIGHT;
			lpBackBuffer->lpVtbl->GetSurfaceDesc(lpBackBuffer, &desc);

			rcSource.top=rcSource.left=0;
			rcSource.bottom=desc.dwHeight;
			rcSource.right=desc.dwWidth;
			
			if(hr = lpPrimary->lpVtbl->Blt(
				lpPrimary,
				rcWindow, 
				lpBackBuffer,
				&rcSource, 
				DDBLT_WAIT, 
				NULL )==DDERR_SURFACELOST)
			{
				return Restore();
			}
		}else{
			if(lpPrimary->lpVtbl->Flip(lpPrimary, NULL,DDFLIP_WAIT)==DDERR_SURFACELOST){
				return Restore();
			}
		}
		if(hr!=DDERR_WASSTILLDRAWING)
			return hr;
	}
	return S_OK;
}

void SetSavedParameters(
	SAVEDPARAMETERS * pSavedParams,
	DWORD dwWidth,
	DWORD dwHeight,
	DWORD dwColorDepth,
	DWORD dwTransparentColor,
	HWND hWnd,
	RECT * rcWindow,
	BOOL bWindowed)
{
	pSavedParams->dwWidth=dwWidth;
	pSavedParams->dwHeight=dwHeight;
	pSavedParams->dwColorDepth=dwColorDepth;
	pSavedParams->dwTransparentColor=dwTransparentColor;
	pSavedParams->hWnd=hWnd;
	if(rcWindow!=NULL)
		memcpy(&(pSavedParams->rcWindow), rcWindow, sizeof(RECT));
	else
		ZeroMemory(&(pSavedParams->rcWindow), sizeof(RECT));
	pSavedParams->bWindowed=bWindowed;
}

HRESULT AdjustWindowSize(
	HWND hWnd, 
	DWORD nWidth, 
	DWORD nHeight, 
	BOOL bWindowed, 
	HMENU hMenu)
{
	if(bWindowed){
		/* Make sure window styles are correct. */
		RECT rcWork;
		RECT rc;
		DWORD dwStyle;

		if(hMenu)
			SetMenu(hWnd, hMenu);

		dwStyle=GetWindowStyle(hWnd);
		dwStyle &= ~WS_POPUP;
		dwStyle |= WS_SYSMENU|WS_OVERLAPPED|WS_CAPTION | WS_DLGFRAME | WS_MINIMIZEBOX; 
		SetWindowLong(hWnd, GWL_STYLE, dwStyle);

		SetRect(&rc, 0, 0, nWidth, nHeight);
		AdjustWindowRectEx(&rc, GetWindowStyle(hWnd), GetMenu(hWnd)!=NULL,
								GetWindowExStyle(hWnd));
								
		SetWindowPos( hWnd, NULL, 0, 0, rc.right-rc.left, rc.bottom-rc.top,
						SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );

		SetWindowPos( hWnd, HWND_NOTOPMOST, 0, 0, 0, 0,
						SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE );
	
		/*  Make sure our window does not hang outside of the work area. */
		SystemParametersInfo( SPI_GETWORKAREA, 0, &rcWork, 0 );
		GetWindowRect( hWnd, &rc );
		if( rc.left < rcWork.left ) rc.left = rcWork.left;
		if( rc.top  < rcWork.top )  rc.top  = rcWork.top;
		SetWindowPos( hWnd, NULL, rc.left, rc.top, 0, 0,
						SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );
		return S_OK;
	}else 
		return S_FALSE;
}

HRESULT UpdateBounds(BOOL bWindowed, HWND hWnd, RECT * rcWindow)
{
    if( bWindowed )
    {
        GetClientRect(hWnd, rcWindow );
        ClientToScreen(hWnd, (POINT*)rcWindow );
        ClientToScreen(hWnd, (POINT*)rcWindow+1 );
    }
    else
    {
        SetRect(rcWindow, 0, 0, GetSystemMetrics(SM_CXSCREEN),
                 GetSystemMetrics(SM_CYSCREEN) );
    }

    return S_OK;
}

HRESULT InitDirectDrawEx(
	LPDIRECTDRAW7 * lppDDraw,
	LPDIRECTDRAWSURFACE7 * lppPrimary,
	LPDIRECTDRAWSURFACE7 * lppBack,
	SAVEDPARAMETERS * pSavedParams)
{
	return InitDirectDraw(
		lppDDraw,
		lppPrimary,
		lppBack,
		pSavedParams->hWnd,
		pSavedParams->bWindowed,
		pSavedParams->dwWidth,
		pSavedParams->dwHeight,
		pSavedParams->dwColorDepth,
		&(pSavedParams->rcWindow),
		&(pSavedParams->dwTransparentColor));
}

HRESULT InitDirectDraw(
	LPDIRECTDRAW7 *lppDDraw,
	LPDIRECTDRAWSURFACE7 *lppPrimary,
	LPDIRECTDRAWSURFACE7 *lppBack,
	HWND hwnd,
	BOOL bWindowed,
	DWORD dwWidth,
	DWORD dwHeight,
	DWORD dwBitDepth,
	RECT *rcWindow,
	DWORD *dwTransparentColor)
{
	HRESULT hr=0;

	if(bWindowed){
		hr=InitWindowedDirectDraw(
			lppDDraw,
			lppBack,
			lppPrimary,
			hwnd,
			dwWidth,
			dwHeight,
			rcWindow,
			dwTransparentColor);
	}else{
		hr=InitFullScreenDirectDraw(
			lppDDraw,
			lppBack,
			lppPrimary,
			hwnd,
			dwWidth,
			dwHeight,
			dwBitDepth,
			rcWindow,
			dwTransparentColor);
	}
	return hr;
}

HRESULT InitWindowedDirectDraw(
		LPDIRECTDRAW7 * lppDDraw, 
		LPDIRECTDRAWSURFACE7 * lppBackBuffer, 
		LPDIRECTDRAWSURFACE7 * lppPrimary, 
		HWND hWnd, DWORD dwWidth, 
		DWORD dwHeight, 
		RECT * rcWindow,
		DWORD * lpTransparentColor)
{
	HRESULT hr=0;
	DDSURFACEDESC2 ddsd;
	LPDIRECTDRAWCLIPPER pcClipper=NULL;

	ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
	
	/* Create Direcdraw and set cooperative level. */
	if(FAILED(hr=DirectDrawCreateEx(
		NULL, 
		(VOID**)&*lppDDraw,
		&IID_IDirectDraw7, 
		NULL)))return E_FAIL;

	hr=(*lppDDraw)->lpVtbl->SetCooperativeLevel((*lppDDraw), hWnd, DDSCL_NORMAL);
	if(FAILED(hr))return E_FAIL;

	/* Create the primary surface. */
   ZeroMemory( &ddsd, sizeof( ddsd ) );
   ddsd.dwSize         = sizeof( ddsd );
   ddsd.dwFlags        = DDSD_CAPS;
   ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;


	if( FAILED( (*lppDDraw)->lpVtbl->CreateSurface((*lppDDraw), &ddsd, &*lppPrimary, NULL )))
        return E_FAIL;

	if(FAILED((*lppDDraw)->lpVtbl->CreateClipper((*lppDDraw), 0, &pcClipper, NULL)))return E_FAIL;
	if( FAILED( hr = pcClipper->lpVtbl->SetHWnd(pcClipper, 0, hWnd)))
    {
        pcClipper->lpVtbl->Release(pcClipper);
        return E_FAIL;
    }

	

    if( FAILED( hr = (*lppPrimary)->lpVtbl->SetClipper((*lppPrimary), pcClipper)))
    {
        pcClipper->lpVtbl->Release(pcClipper);
        return E_FAIL;
    }
	 	 
	/* Done with clipper. */
   pcClipper->lpVtbl->Release(pcClipper);

	/* Create the backbuffer surface. */
   ddsd.dwFlags        = DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT;    
   ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN| DDSCAPS_3DDEVICE;
   ddsd.dwWidth        = dwWidth;
   ddsd.dwHeight       = dwHeight;

   if( FAILED( hr = (*lppDDraw)->lpVtbl->CreateSurface((*lppDDraw), &ddsd, &*lppBackBuffer, NULL)))
        return E_FAIL;

	*lpTransparentColor=WindowsColorToDirectDraw(*lpTransparentColor, *lppPrimary);
	UpdateBounds(TRUE, hWnd, rcWindow);

	return S_OK;
}


DWORD WindowsColorToDirectDraw(COLORREF rgb, LPDIRECTDRAWSURFACE7 surface){
	DWORD dw=CLR_INVALID;
	COLORREF rgbT;
	HDC hDc;
	DDSURFACEDESC2 ddsd;
	HRESULT hres;

	if(rgb!=CLR_INVALID&&SUCCEEDED(surface->lpVtbl->GetDC(surface, &hDc))){
		rgbT=GetPixel(hDc, 0, 0);
		SetPixel(hDc, 0, 0, rgb);
		surface->lpVtbl->ReleaseDC(surface, hDc);
	}

	ddsd.dwSize=sizeof(ddsd);
	while((hres=surface->lpVtbl->Lock(surface, NULL, &ddsd, 0, NULL))==DDERR_WASSTILLDRAWING);
	if(SUCCEEDED(hres)){
		dw=*(DWORD*)ddsd.lpSurface;
		if(ddsd.ddpfPixelFormat.dwRGBBitCount!=32)
			dw&=(1<<ddsd.ddpfPixelFormat.dwRGBBitCount)-1;
		surface->lpVtbl->Unlock(surface, NULL);
	}
	
	if(rgb!=CLR_INVALID&&SUCCEEDED(surface->lpVtbl->GetDC(surface, &hDc))){
		SetPixel(hDc, 0, 0, rgbT);
		surface->lpVtbl->ReleaseDC(surface, hDc);
	}
	return dw;
}

HRESULT InitFullScreenDirectDraw(
		LPDIRECTDRAW7 *lppDDraw, 
		LPDIRECTDRAWSURFACE7 *lppBackBuffer, 
		LPDIRECTDRAWSURFACE7 *lppPrimary, 
		HWND hWnd, DWORD dwWidth,	
		DWORD dwHeight, 
		DWORD dwBitDepth, 
		RECT *rcWindow,
		DWORD *nTransparentColor)
{
	HRESULT hr=0;
	DDSURFACEDESC2 ddsd;
	DDSCAPS2 ddscaps;
	DWORD dwStyle=0;

	SetMenu(hWnd, NULL);
	if(FAILED(hr=DirectDrawCreateEx(
		NULL, 
		(VOID**)&*lppDDraw,
		&IID_IDirectDraw7, 
		NULL)))return E_FAIL;

	hr=(*lppDDraw)->lpVtbl->SetCooperativeLevel((*lppDDraw), hWnd, DDSCL_EXCLUSIVE|DDSCL_FULLSCREEN);
	if(FAILED(hr))return E_FAIL;

	if(FAILED((*lppDDraw)->lpVtbl->SetDisplayMode((*lppDDraw), dwWidth, dwHeight, dwBitDepth, 0, 0)))
		return E_FAIL;
	/* Create the primary surface. */
   ZeroMemory(&ddsd,sizeof( ddsd ) );
   ddsd.dwSize = sizeof( ddsd );
   ddsd.dwFlags = DDSD_CAPS|DDSD_BACKBUFFERCOUNT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE|DDSCAPS_FLIP|
                          DDSCAPS_COMPLEX|DDSCAPS_3DDEVICE;
   ddsd.dwBackBufferCount = 1;

	if(FAILED(hr=(*lppDDraw)->lpVtbl->CreateSurface((*lppDDraw), &ddsd, &*lppPrimary, NULL)))
        return E_FAIL;

	/* Get a pointer to the back buffer. */
   ZeroMemory(&ddscaps, sizeof(ddscaps));
   ddscaps.dwCaps=DDSCAPS_BACKBUFFER;

   if(FAILED(hr =(*lppPrimary)->lpVtbl->GetAttachedSurface((*lppPrimary), &ddscaps, &*lppBackBuffer)))
		return E_FAIL;
	
   dwStyle=WS_POPUP;
   SetWindowLong( hWnd, GWL_STYLE, dwStyle );
	
	*nTransparentColor=WindowsColorToDirectDraw(*nTransparentColor, *lppPrimary);
	UpdateBounds(FALSE, hWnd, rcWindow);
	return hr;
}