/*
	D3DInit.c - Initialization Functions for GFX3D9

	Copyright (c) 2003, Blaine Myers
*/
#include <d3d9.h>
#include <math.h>
#include <windowsx.h>
#include "GFX3D9.h"

BOOL SetViewPort(
	LPDIRECT3DDEVICE9 lpDevice,
	DWORD dwWidth,
	DWORD dwHeight)
{
	D3DVIEWPORT9 VP;
	ZeroMemory(&VP, sizeof(D3DVIEWPORT9));

	VP.X = 0;
	VP.Y = 0;
	VP.Width = dwWidth;
	VP.Height = dwHeight;
	VP.MinZ = 0.0f;
	VP.MaxZ = 1.0f;

	lpDevice->lpVtbl->SetViewport(lpDevice, &VP);

	return TRUE;
}

BOOL SetProjectionMatrix(
	LPDIRECT3DDEVICE9 lpDevice, 
	DWORD dwWidth, 
	DWORD dwHeight,
	FLOAT zn,
	FLOAT zf)
{
	FLOAT FOV=0.0f;
	FLOAT h=0.0f, w=0.0f;
	D3DMATRIX PM;

	ZeroMemory(&PM, sizeof(D3DMATRIX));

	if(!lpDevice)
		return FALSE;

	FOV = 3.141592654f / 4;

	h=(FLOAT)((DOUBLE)1.0f/tan(FOV/2));
	w=h/((FLOAT)dwWidth/(FLOAT)dwHeight);

	/* Create the projections matrix. */
	PM._11=w; PM._12=0; PM._13=0; PM._14=0;
	PM._21=0; PM._22=h; PM._23=0; PM._24=0;
	PM._31=0; PM._32=0; PM._33=zf/(zf-zn); PM._34=1;
	PM._41=0; PM._42=0; PM._43=((-zn)*zf)/(zf-zn); PM._44=0;
	

	if(SUCCEEDED(lpDevice->lpVtbl->SetTransform(lpDevice, D3DTS_PROJECTION, &PM)))
		return TRUE;
	else
		return FALSE;
}

GFX3D9_EXPORTS BOOL SetTextureFilter(
	LPDIRECT3DDEVICE9 lpDevice,
	DWORD Stage,
	D3DFILTER FilterMode)
{
	D3DCAPS9 Caps;

	if(!lpDevice)return FALSE;

	if(FAILED(IDirect3DDevice9_GetDeviceCaps(lpDevice, &Caps)))
		return FALSE;

	switch(FilterMode)
	{
	case D3DFILTER_POINT:
		IDirect3DDevice9_SetSamplerState(lpDevice, Stage, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
		IDirect3DDevice9_SetSamplerState(lpDevice, Stage, D3DSAMP_MINFILTER, D3DTEXF_POINT);
		IDirect3DDevice9_SetSamplerState(lpDevice, Stage, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
		break;
	case D3DFILTER_BILINEAR: 
		IDirect3DDevice9_SetSamplerState(lpDevice, Stage, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice9_SetSamplerState(lpDevice, Stage, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice9_SetSamplerState(lpDevice, Stage, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
		break;
	case D3DFILTER_TRILINEAR: 
		IDirect3DDevice9_SetSamplerState(lpDevice, Stage, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice9_SetSamplerState(lpDevice, Stage, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice9_SetSamplerState(lpDevice, Stage, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
		break;
	case D3DFILTER_ANISOTROPIC: 
		if((Caps.DevCaps&D3DPRASTERCAPS_ANISOTROPY) == D3DPRASTERCAPS_ANISOTROPY){
			IDirect3DDevice9_SetSamplerState(lpDevice, Stage, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
			IDirect3DDevice9_SetSamplerState(lpDevice, Stage, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
			IDirect3DDevice9_SetSamplerState(lpDevice, Stage, D3DSAMP_MIPFILTER, D3DTEXF_ANISOTROPIC);
			IDirect3DDevice9_SetSamplerState(lpDevice, Stage, D3DSAMP_MAXANISOTROPY, Caps.MaxAnisotropy);
		}else
			return FALSE;
		break;
	default:
		return FALSE;
	};

	return TRUE;
}

BOOL ScreenSwitch(
	DWORD dwWidth,
	DWORD dwHeight,
	BOOL bWindowed,
	D3DFORMAT FullScreenFormat,
	BOOL bVsync,
	LPDIRECT3DDEVICE9 lpDevice,
	LPDIRECT3DSURFACE9 * lppBackSurface,
	D3DPRESENT_PARAMETERS * lpSavedParams,
	POOLFN fpReleasePool,
	POOLFN fpRestorePool)
{

	return TRUE;
}


BOOL ValidateDevice(
	LPDIRECT3DDEVICE9 * lppDevice, 
	LPDIRECT3DSURFACE9 * lppBackSurface,
	D3DPRESENT_PARAMETERS d3dpp,
	POOLFN fpReleasePool,
	POOLFN fpRestorePool)
{
	HRESULT hr=0;
	BOOL bWasBackSurface=FALSE;

	if(lppBackSurface)
		bWasBackSurface=TRUE;

	if(!(lppDevice))return FALSE;
	if(FAILED(hr=IDirect3DDevice9_TestCooperativeLevel((*lppDevice)))){
		if(hr == D3DERR_DEVICELOST)return TRUE;
		if(hr == D3DERR_DEVICENOTRESET){
			if(bWasBackSurface){
				if( *lppBackSurface ){
					IDirect3DSurface9_Release((*lppBackSurface));
					*lppBackSurface=NULL;
				}
			}
			if(fpReleasePool)
				fpReleasePool();
			
			if(FAILED(IDirect3DDevice9_Reset((*lppDevice), &d3dpp))){
				return FALSE;
			}

			if(bWasBackSurface){
				if(FAILED(IDirect3DDevice9_GetBackBuffer(
					(*lppDevice), 
					0, 
					0, 
					D3DBACKBUFFER_TYPE_MONO, 
					(lppBackSurface) )))return FALSE;
			}

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

BOOL InitD3D(
	HWND hWndTarget, 
	DWORD dwWidth, 
	DWORD dwHeight, 
	BOOL bWindowed, 
	BOOL bVsync,
	D3DFORMAT d3dfFullScreenFormat, 
	LPDIRECT3D9 lppD3D, 
	LPDIRECT3DDEVICE9 * lppDevice,
	LPDIRECT3DSURFACE9 * lppBackSurface,
	D3DPRESENT_PARAMETERS * d3dSavedParams)
{
	D3DPRESENT_PARAMETERS d3dpp;
	D3DDISPLAYMODE d3ddm;

	ZeroMemory(&d3ddm, sizeof(d3ddm));
	ZeroMemory(&d3dpp, sizeof(d3dpp));

	if( !(lppD3D) )return FALSE;

	if( (*lppDevice) ){
		IDirect3DDevice9_Release((*lppDevice));
		(*lppDevice)=NULL;
	}



	d3dpp.BackBufferWidth = dwWidth;
	d3dpp.BackBufferHeight = dwHeight;
	d3dpp.BackBufferFormat = bWindowed ? D3DFMT_UNKNOWN : d3dfFullScreenFormat;
	d3dpp.BackBufferCount = 1;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dpp.MultiSampleQuality=0;
	
	/* If vsync */
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;

	d3dpp.hDeviceWindow=hWndTarget;
	d3dpp.Windowed = bWindowed;

	d3dpp.EnableAutoDepthStencil=TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	d3dpp.FullScreen_RefreshRateInHz=D3DPRESENT_RATE_DEFAULT;


	d3dpp.PresentationInterval = 
		(bVsync ? 
			(bWindowed ? 0 : D3DPRESENT_INTERVAL_ONE) : 
			(bWindowed ? 0 : D3DPRESENT_INTERVAL_IMMEDIATE)
		);
	
	d3dpp.Flags = 0;//D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

	if(FAILED(IDirect3D9_CreateDevice(
		(lppD3D),
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		hWndTarget,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING, /* Should be able to specify these flags. */
		&d3dpp,
		lppDevice)))return FALSE;	

	memcpy(d3dSavedParams, &d3dpp, sizeof(D3DPRESENT_PARAMETERS));

	/*
	if(lppBackSurface){
		if(FAILED(IDirect3DDevice9_GetBackBuffer((*lppDevice), 0, 0, D3DBACKBUFFER_TYPE_MONO, lppBackSurface))){
			return FALSE;
		}
	}
	*/
	return TRUE;
}