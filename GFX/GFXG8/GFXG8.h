/*
	GFXG8.h - Header for GraFiX enGine DirectX 8;

	Copyright (c) 2003, Blaine Myers.
*/
#include <d3d8.h>
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define GFXG8_EXPORTS

GFXG8_EXPORTS HRESULT InitD3D(
	HWND hwndTarget,
	DWORD dwWidth,
	DWORD dwHeight,
	BOOL bWindowed,
	D3DFORMAT Format,
	LPDIRECT3D8 lpD3D,
	LPDIRECT3DDEVICE8 * lpDevice,
	D3DPRESENT_PARAMETERS * lpSavedPP,
	LPDIRECT3DSURFACE8 * lpBackBuffer);

/* ValidateDevice validates the device and restores backbuffer.  It
   also calls a RestoreGraphics function if one is specified. */
typedef BOOL ( * POOLFN)();

GFXG8_EXPORTS BOOL ValidateDevice(
	LPDIRECT3DDEVICE8 * lppDevice, 
	LPDIRECT3DSURFACE8 * lppBackSurface,
	D3DPRESENT_PARAMETERS d3dpp,
	POOLFN fpReleasePool,
	POOLFN fpRestorePool);

GFXG8_EXPORTS BOOL CorrectWindowSize(
	HWND hWnd, 
	DWORD nWidth, 
	DWORD nHeight, 
	BOOL bWindowed,
	HMENU hMenu);

GFXG8_EXPORTS HRESULT CopySurfaceToSurface(
	LPDIRECT3DDEVICE8 lpDevice,
	RECT * lpSourceRect, 
	LPDIRECT3DSURFACE8 lpSourceSurf,
	POINT * lpDestPoint,
	LPDIRECT3DSURFACE8 lpDestSurf,
	BOOL bTransparent,
	D3DCOLOR ColorKey);

GFXG8_EXPORTS HRESULT CopySurfaceToSurface32(
	RECT * lpSourceRect, 
	LPDIRECT3DSURFACE8 lpSourceSurf,
	POINT * lpDestPoint,
	LPDIRECT3DSURFACE8 lpDestSurf,
	BOOL bTransparent,
	D3DCOLOR ColorKey);

GFXG8_EXPORTS HRESULT CopySurfaceToSurface16(
	RECT * lpSourceRect, 
	LPDIRECT3DSURFACE8 lpSourceSurf,
	POINT * lpDestPoint,
	LPDIRECT3DSURFACE8 lpDestSurf,
	BOOL bTransparent,
	D3DCOLOR ColorKey);

#ifdef __cplusplus

#ifdef UNICODE
#define CreateImageBM CreateImageBMW
#else /* UNICODE */
#define CreateImageBM CreateImageBMA
#endif

#define CImage CImage8

class GFXG8_EXPORTS CImage8
{
protected:
	LPDIRECT3DSURFACE8 m_lpImage;
	DWORD m_dwWidth;
	DWORD m_dwHeight;


	HRESULT CreateSurface(
		LPVOID lpDevice,
		D3DFORMAT Format, 
		DWORD dwWidth, 
		DWORD dwHeight);

	HRESULT LoadBitmapIntoSurfaceA(
		char szBitmapFileName[],
		DWORD x,
		DWORD y,
		DWORD nSrcWidth,
		DWORD nSrcHeight,
		DWORD nWidth,
		DWORD nHeight);

	HRESULT LoadBitmapIntoSurfaceW(
		WCHAR szBitmapFileName[],
		DWORD x,
		DWORD y,
		DWORD nSrcWidth,
		DWORD nSrcHeight,
		DWORD nWidth,
		DWORD nHeight);

public:
	CImage8();
	~CImage8();

	void ClearSurface();

	HRESULT CreateImageBMA(
		char szBitmapFilename[],
		LPVOID lpDevice,
		D3DFORMAT Format,
		DWORD x,
		DWORD y,
		DWORD nSrcWidth,
		DWORD nSrcHeight,
		DWORD nWidth,
		DWORD nHeight);

	HRESULT CreateImageBMW(
		WCHAR szBitmapFilename[],
		LPVOID lpDevice,
		D3DFORMAT Format,
		DWORD x,
		DWORD y,
		DWORD nSrcWidth,
		DWORD nSrcHeight,
		DWORD nWidth,
		DWORD nHeight);

	HRESULT DrawClippedImage(
		LPVOID lpDevice,
		LPVOID lpBuffer,
		int x,
		int y);

	HRESULT DrawImage(
		LPVOID lpDevice,
		LPVOID lpBuffer,
		int x,
		int y);

	void Release();

	DWORD GetWidth();
	DWORD GetHeight();

};

#endif /* __cplusplus */

#ifdef __cplusplus
}
#endif /* __cplusplus */