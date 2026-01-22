#include <windowsx.h>
#include "GFXG8.h"

BOOL ValidateDevice(
	LPDIRECT3DDEVICE9 * lppDevice, 
	LPDIRECT3DSURFACE9 * lppBackSurface,
	D3DPRESENT_PARAMETERS d3dpp,
	POOLFN fpReleasePool,
	POOLFN fpRestorePool)
{
	HRESULT hr=0;

	if(!*lppDevice)return FALSE;
	if(FAILED(hr=IDirect3DDevice9_TestCooperativeLevel((*lppDevice)))){
		if(hr == D3DERR_DEVICELOST)return TRUE;
		if(hr == D3DERR_DEVICENOTRESET){
			if(*lppBackSurface){
				IDirect3DSurface9_Release((*lppBackSurface));
				*lppBackSurface=NULL;
			}
			if(fpReleasePool)
				fpReleasePool();

			if(FAILED(IDirect3DDevice9_Reset((*lppDevice), &d3dpp))){
				return FALSE;
			}

			if(FAILED(IDirect3DDevice9_GetBackBuffer(
				(*lppDevice),
				0,
				0, 
				D3DBACKBUFFER_TYPE_MONO, 
				lppBackSurface )))return FALSE;


			IDirect3DDevice9_Clear(
				(*lppDevice), 
				0, 
				NULL, 
				D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 
				D3DCOLOR_XRGB(0, 0, 200), 
				1.0f, 
				0);

			if(fpRestorePool)
				fpRestorePool();
		}
	}
	
	return TRUE;
}

HRESULT InitD3D(
	HWND hwndTarget,
	DWORD dwWidth,
	DWORD dwHeight,
	BOOL bWindowed,
	D3DFORMAT Format,
	LPDIRECT3D9 lpD3D,
	LPDIRECT3DDEVICE9 * lpDevice,
	D3DPRESENT_PARAMETERS * lpSavedPP,
	LPDIRECT3DSURFACE9 * lpBackBuffer)
{
	D3DPRESENT_PARAMETERS d3dpp;
	D3DDISPLAYMODE d3ddm;
	HRESULT hr=0;

	if((*lpDevice))
		IDirect3DDevice9_Release((*lpDevice));

	ZeroMemory(&d3dpp, sizeof(d3dpp));

	hr=IDirect3D9_GetAdapterDisplayMode(
		lpD3D,
		D3DADAPTER_DEFAULT,
		&d3ddm);

	if(FAILED(hr)){
		return E_FAIL;
	}

	d3dpp.BackBufferWidth=dwWidth;
	d3dpp.BackBufferHeight=dwHeight;
	d3dpp.BackBufferFormat=bWindowed ? d3ddm.Format : Format;
	d3dpp.BackBufferCount=1;
	d3dpp.MultiSampleType=D3DMULTISAMPLE_NONE;
	d3dpp.SwapEffect=D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow=hwndTarget;
	d3dpp.Windowed=bWindowed;
	d3dpp.EnableAutoDepthStencil=TRUE;
	d3dpp.AutoDepthStencilFormat=D3DFMT_D16;
	d3dpp.FullScreen_RefreshRateInHz=0;
	d3dpp.PresentationInterval =
		bWindowed ? 0 : D3DPRESENT_INTERVAL_IMMEDIATE;
	d3dpp.Flags=D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

	hr=IDirect3D9_CreateDevice(
		lpD3D,
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		hwndTarget,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp,
		lpDevice);

	if(FAILED(hr)){
		return E_FAIL;
	}

	*lpSavedPP=d3dpp;

	if(FAILED(IDirect3DDevice9_GetBackBuffer(
		(*lpDevice),
		0,
		0,
		D3DBACKBUFFER_TYPE_MONO,
		lpBackBuffer)))
	{
		return E_FAIL;
	}
	return S_OK;
}

BOOL CorrectWindowSize(
	HWND hWnd, 
	DWORD nWidth, 
	DWORD nHeight, 
	BOOL bWindowed,
	HMENU hMenu){

	if(bWindowed){
		/* Make sure window styles are correct */
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
	
		/* Make sure our window does not hang outside of the work area */
		SystemParametersInfo( SPI_GETWORKAREA, 0, &rcWork, 0 );
		GetWindowRect( hWnd, &rc );
		if( rc.left < rcWork.left ) rc.left = rcWork.left;
		if( rc.top  < rcWork.top )  rc.top  = rcWork.top;
		SetWindowPos( hWnd, NULL, rc.left, rc.top, 0, 0,
						SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );
		return TRUE;
	}else 
		return TRUE;
}

HRESULT CopySurfaceToSurface16(
	RECT * lpSourceRect, 
	LPDIRECT3DSURFACE9 lpSourceSurf,
	POINT * lpDestPoint,
	LPDIRECT3DSURFACE9 lpDestSurf,
	BOOL bTransparent,
	D3DCOLOR ColorKey)
{

	HRESULT r=0;

	D3DLOCKED_RECT LockedSource;
	D3DLOCKED_RECT LockedDest;
	RECT SourceRect;
	POINT DestPoint;
	D3DSURFACE_DESC d3dsdSource;
	D3DSURFACE_DESC d3dsdDest;

	WORD * lpSourceData=NULL;
	WORD * lpDestData=NULL;

	int SourceOffset=0, DestOffset=0;
	int y=0, x=0;

	/* Make sure surfaces exist. */
	if(!lpSourceSurf || !lpDestSurf)return E_POINTER;

	/* Prepare destination and source values. */

	lpSourceSurf->lpVtbl->GetDesc(lpSourceSurf, &d3dsdSource);

	lpDestSurf->lpVtbl->GetDesc(lpDestSurf, &d3dsdDest);

	if(lpSourceRect)
		SourceRect = *lpSourceRect;
	else
		SetRect(&SourceRect, 0, 0, d3dsdSource.Width, d3dsdSource.Height);

	if(lpDestPoint)
		DestPoint = *lpDestPoint;

	else{
		DestPoint.x=DestPoint.y=0;
	}

	/* Do a series of checks to make sure the parameters were valid. */
	if(DestPoint.x<0)return E_FAIL;
	if(DestPoint.x>(int)d3dsdDest.Width)return E_FAIL;
	if(DestPoint.y<0)return E_FAIL;
	if(DestPoint.y>(int)d3dsdDest.Height)return E_FAIL;

	if(SourceRect.top<0)return E_FAIL;
	if(SourceRect.left<0)return E_FAIL;
	if(SourceRect.bottom>(int)d3dsdSource.Height)return E_FAIL;
	if(SourceRect.right>(int)d3dsdSource.Width)return E_FAIL;

	if((DestPoint.x+(SourceRect.right-SourceRect.left))>(int)d3dsdDest.Width)return E_FAIL;
	if((DestPoint.y+(SourceRect.bottom-SourceRect.top))>(int)d3dsdDest.Height)return E_FAIL;
	if((DestPoint.x+(SourceRect.left))<0)return E_FAIL;
	if((DestPoint.y+(SourceRect.top))<0)return E_FAIL;

	/* Lock the surfaces. */
	r=lpSourceSurf->lpVtbl->LockRect(lpSourceSurf, &LockedSource, 0, D3DLOCK_READONLY);
	if(FAILED(r))return E_FAIL;
	
	r=lpDestSurf->lpVtbl->LockRect(lpDestSurf, &LockedDest, 0, 0);
	if(FAILED(r)){
		lpSourceSurf->lpVtbl->UnlockRect(lpSourceSurf);
		return E_FAIL;
	}

	/* Adjust pitch to match a 4 bytes value for each pixel. */
	LockedSource.Pitch /=2;
	LockedDest.Pitch /= 2;

	/* Obtain 16 bit pointer to data. */
	lpSourceData = (WORD*)LockedSource.pBits;
	lpDestData = (WORD*)LockedDest.pBits;


	/* Prepare the appropriate offset. */
	SourceOffset = SourceRect.left+LockedSource.Pitch*SourceRect.top;
	DestOffset = DestPoint.x+LockedDest.Pitch*DestPoint.y;

	/* Write the data pixel by pixel. */
	for(y=0; y<(SourceRect.bottom-SourceRect.top); y++){
		for(x=0; x<(SourceRect.right-SourceRect.left); x++){
			if(bTransparent){
				if(lpSourceData[SourceOffset]!=ColorKey){
					lpDestData[DestOffset]=lpSourceData[SourceOffset];
				}

			}
			else{
				lpDestData[DestOffset]=lpSourceData[SourceOffset];
			}

			DestOffset++;
			SourceOffset++;
		}
		SourceOffset += LockedSource.Pitch-(SourceRect.right-SourceRect.left);
		DestOffset += LockedDest.Pitch-(SourceRect.right-SourceRect.left);
	}
	

	/* Unlock the rectangles now that were through. */
	lpSourceSurf->lpVtbl->UnlockRect(lpSourceSurf);
	lpDestSurf->lpVtbl->UnlockRect(lpDestSurf);
	return S_OK;
}

HRESULT CopySurfaceToSurface32(
	RECT * lpSourceRect, 
	LPDIRECT3DSURFACE9 lpSourceSurf,
	POINT * lpDestPoint,
	LPDIRECT3DSURFACE9 lpDestSurf,
	BOOL bTransparent,
	D3DCOLOR ColorKey)
{

	HRESULT r=0;

	D3DLOCKED_RECT LockedSource;
	D3DLOCKED_RECT LockedDest;
	RECT SourceRect;
	POINT DestPoint;
	D3DSURFACE_DESC d3dsdSource;
	D3DSURFACE_DESC d3dsdDest;

	DWORD * lpSourceData=NULL;
	DWORD * lpDestData=NULL;

	int SourceOffset=0, DestOffset=0;
	int y=0, x=0;

	/* Make sure surfaces exist. */
	if(!lpSourceSurf || !lpDestSurf)return E_POINTER;

	/* Prepare destination and source values. */

	lpSourceSurf->lpVtbl->GetDesc(lpSourceSurf, &d3dsdSource);

	lpDestSurf->lpVtbl->GetDesc(lpDestSurf, &d3dsdDest);

	if(lpSourceRect)
		SourceRect = *lpSourceRect;
	else
		SetRect(&SourceRect, 0, 0, d3dsdSource.Width, d3dsdSource.Height);

	if(lpDestPoint)
		DestPoint = *lpDestPoint;

	else{
		DestPoint.x=DestPoint.y=0;
	}

	/* Do a series of checks to make sure the parameters were valid. */
	if(DestPoint.x<0)return E_FAIL;
	if(DestPoint.x>(int)d3dsdDest.Width)return E_FAIL;
	if(DestPoint.y<0)return E_FAIL;
	if(DestPoint.y>(int)d3dsdDest.Height)return E_FAIL;

	if(SourceRect.top<0)return E_FAIL;
	if(SourceRect.left<0)return E_FAIL;
	if(SourceRect.bottom>(int)d3dsdSource.Height)return E_FAIL;
	if(SourceRect.right>(int)d3dsdSource.Width)return E_FAIL;

	if((DestPoint.x+(SourceRect.right-SourceRect.left))>(int)d3dsdDest.Width)return E_FAIL;
	if((DestPoint.y+(SourceRect.bottom-SourceRect.top))>(int)d3dsdDest.Height)return E_FAIL;
	if((DestPoint.x+(SourceRect.left))<0)return E_FAIL;
	if((DestPoint.y+(SourceRect.top))<0)return E_FAIL;

	/* Lock the surfaces. */
	r=lpSourceSurf->lpVtbl->LockRect(lpSourceSurf, &LockedSource, 0, D3DLOCK_READONLY);
	if(FAILED(r))return E_FAIL;
	
	r=lpDestSurf->lpVtbl->LockRect(lpDestSurf, &LockedDest, 0, 0);
	if(FAILED(r)){
		lpSourceSurf->lpVtbl->UnlockRect(lpSourceSurf);
		return E_FAIL;
	}

	/* Adjust pitch to match a 4 bytes value for each pixel. */
	LockedSource.Pitch /=4;
	LockedDest.Pitch /= 4;

	/* Obtain 32 bit pointer to data. */
	lpSourceData = (DWORD*)LockedSource.pBits;
	lpDestData = (DWORD*)LockedDest.pBits;


	/* Prepare the appropriate offset. */
	SourceOffset = SourceRect.left+LockedSource.Pitch*SourceRect.top;
	DestOffset = DestPoint.x+LockedDest.Pitch*DestPoint.y;

	/* Write the data pixel by pixel. */
	for(y=0; y<(SourceRect.bottom-SourceRect.top); y++){
		for(x=0; x<(SourceRect.right-SourceRect.left); x++){
			if(bTransparent){
				if(lpSourceData[SourceOffset]!=ColorKey){
					lpDestData[DestOffset]=lpSourceData[SourceOffset];
				}

			}
			else{
				lpDestData[DestOffset]=lpSourceData[SourceOffset];
			}

			DestOffset++;
			SourceOffset++;
		}
		SourceOffset += LockedSource.Pitch-(SourceRect.right-SourceRect.left);
		DestOffset += LockedDest.Pitch-(SourceRect.right-SourceRect.left);
	}
	

	/* Unlock the rectangles now that were through. */
	lpSourceSurf->lpVtbl->UnlockRect(lpSourceSurf);
	lpDestSurf->lpVtbl->UnlockRect(lpDestSurf);
	return S_OK;
}


HRESULT CopySurfaceToSurface(
	LPDIRECT3DDEVICE9 lpDevice,
	RECT * lpSourceRect, 
	LPDIRECT3DSURFACE9 lpSourceSurf,
	POINT * lpDestPoint,
	LPDIRECT3DSURFACE9 lpDestSurf,
	BOOL bTransparent,
	D3DCOLOR ColorKey)
{
	D3DSURFACE_DESC descSrc, descDest;
	IDirect3DSurface9_GetDesc(
		lpSourceSurf,
		&descSrc);
	IDirect3DSurface9_GetDesc(
		lpDestSurf,
		&descDest);

	if(descDest.Format != descSrc.Format)
		return E_FAIL;

	if(bTransparent){
		if(descDest.Format==D3DFMT_R5G6B5)
			return CopySurfaceToSurface16(
				lpSourceRect,
				lpSourceSurf,
				lpDestPoint,
				lpDestSurf,
				bTransparent,
				ColorKey);
		else
			return CopySurfaceToSurface32(
				lpSourceRect, 
				lpSourceSurf, 
				lpDestPoint, 
				lpDestSurf, 
				bTransparent, 
				ColorKey);
	}else{
		if (descDest.Format == D3DFMT_R5G6B5)
			return CopySurfaceToSurface16(
				lpSourceRect,
				lpSourceSurf,
				lpDestPoint,
				lpDestSurf,
				FALSE,
				0);
		else
			return CopySurfaceToSurface32(
				lpSourceRect,
				lpSourceSurf,
				lpDestPoint,
				lpDestSurf,
				FALSE,
				0);
	}
}
