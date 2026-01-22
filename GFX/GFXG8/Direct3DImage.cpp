#include <d3dx9.h>
#include "GFXG8.h"

CImage8::CImage8()
{
	m_dwWidth=0;
	m_dwHeight=0;
	m_lpImage=NULL;
}

CImage8::~CImage8()
{
	Release();
}

DWORD CImage8::GetWidth()
{
	return m_dwWidth;
}

DWORD CImage8::GetHeight()
{
	return m_dwHeight;
}

HRESULT CImage8::CreateSurface(
	LPVOID lpDevice,
	D3DFORMAT Format,
	DWORD dwWidth,
	DWORD dwHeight)
{
	if(lpDevice)
		return ((LPDIRECT3DDEVICE9)lpDevice)->CreateOffscreenPlainSurface(
			dwWidth, 
			dwHeight,
			Format, 
			D3DPOOL_DEFAULT,
			&m_lpImage,
			NULL);
	else
		return E_FAIL;
}

HRESULT CImage8::LoadBitmapIntoSurfaceA(
	char szBitmapFileName[],
	DWORD x,
	DWORD y,
	DWORD nSrcWidth,
	DWORD nSrcHeight,
	DWORD nWidth,
	DWORD nHeight)
{
	RECT rcSrc;
	//Prepare the source rectangle.
	rcSrc.top=y;
	rcSrc.bottom=y+nSrcHeight;
	rcSrc.left=x;
	rcSrc.right=x+nSrcWidth;

	return D3DXLoadSurfaceFromFileA(
		m_lpImage,
		NULL,
		NULL,
		szBitmapFileName,
		&rcSrc,
		D3DX_DEFAULT,
		0xFF000000L,
		NULL);
}

HRESULT CImage8::LoadBitmapIntoSurfaceW(
	WCHAR szBitmapFileName[],
	DWORD x,
	DWORD y,
	DWORD nSrcWidth,
	DWORD nSrcHeight,
	DWORD nWidth,
	DWORD nHeight)
{
	RECT rcSrc;
	//Prepare the source rectangle.
	rcSrc.top=y;
	rcSrc.bottom=y+nSrcHeight;
	rcSrc.left=x;
	rcSrc.right=x+nSrcWidth;

	return D3DXLoadSurfaceFromFileW(
		m_lpImage,
		NULL,
		NULL,
		szBitmapFileName,
		&rcSrc,
		D3DX_DEFAULT,
		0xFF000000L,
		NULL);
}

HRESULT CImage8::CreateImageBMA(
	char szBitmapFilename[],
	LPVOID lpDevice,
	D3DFORMAT Format,
	DWORD x,
	DWORD y,
	DWORD nSrcWidth,
	DWORD nSrcHeight,
	DWORD nWidth,
	DWORD nHeight)
{
	if(FAILED(CreateSurface(lpDevice, Format, nWidth, nHeight))){
		return E_FAIL;
	}

	if(FAILED(LoadBitmapIntoSurfaceA(
		szBitmapFilename,
		x,
		y,
		nSrcWidth,
		nSrcHeight,
		nWidth,
		nHeight)))
	{
		return E_FAIL;	
	}

	m_dwWidth=nWidth;
	m_dwHeight=nHeight;
	return S_OK;
}

HRESULT CImage8::CreateImageBMW(
	WCHAR szBitmapFilename[],
	LPVOID lpDevice,
	D3DFORMAT Format,
	DWORD x,
	DWORD y,
	DWORD nSrcWidth,
	DWORD nSrcHeight,
	DWORD nWidth,
	DWORD nHeight)
{
	if(FAILED(CreateSurface(lpDevice, Format, nWidth, nHeight))){
		return E_FAIL;
	}

	if(FAILED(LoadBitmapIntoSurfaceW(
		szBitmapFilename,
		x,
		y,
		nSrcWidth,
		nSrcHeight,
		nWidth,
		nHeight)))
	{
		return E_FAIL;	
	}

	m_dwWidth=nWidth;
	m_dwHeight=nHeight;
	return S_OK;
}

void CImage8::ClearSurface()
{
	m_dwWidth=0;
	m_dwHeight=0;

	Release();
}

void CImage8::Release()
{
	if(m_lpImage){
		m_lpImage->Release();
		m_lpImage=NULL;
	}
}

HRESULT CImage8::DrawImage(
	LPVOID lpDevice,
	LPVOID lpBuffer,
	int x,
	int y)
{
	RECT rcSrc;
	POINT pDest;

	//Prepare the source rectangle.
	rcSrc.top=rcSrc.left=0;
	rcSrc.bottom=m_dwHeight;
	rcSrc.right=m_dwWidth;

	//Prepare the destianion point.
	pDest.x=x;
	pDest.y=y;

	
	return CopySurfaceToSurface(
		(LPDIRECT3DDEVICE9)lpDevice,
		&rcSrc,
		m_lpImage,
		&pDest,
		(LPDIRECT3DSURFACE9)lpBuffer,
		TRUE,
		0x00FF00FF);
	
	//return ((LPDIRECT3DDEVICE8)lpDevice)->CopyRects(
	//	m_lpImage,                                     
	//	&rcSrc,                                        
	//	1,                                             
	//	((LPDIRECT3DSURFACE9)lpBuffer),                
	//	&pDest);
}

HRESULT CImage8::DrawClippedImage(
	LPVOID lpDevice,
	LPVOID lpBuffer,
	int x,
	int y)
{
	if(!lpBuffer || !m_lpImage || !lpDevice)return E_FAIL;
	
	RECT rcSrc;
	POINT psDest;
	//we get a description for the buffer
	D3DSURFACE_DESC d3dsDesc;
	((LPDIRECT3DSURFACE9)lpBuffer)->GetDesc(&d3dsDesc);
	
	//store buffer dimensions
	int nBufferWidth=d3dsDesc.Width;
	int nBufferHeight=d3dsDesc.Height;
	
	//if we don't need to clip we do a regular blt
	if(
		//withing the width
		(x>=0) &&
		((int)(m_dwWidth+x)<nBufferWidth) &&
		//within the height
		(y>=0) &&
		((int)(m_dwHeight+y)<nBufferHeight)
	)return DrawImage(lpDevice, lpBuffer, x, y);

	//if the image is off screen we do no blt
	if(x>nBufferWidth)return S_FALSE;
	if(y>nBufferHeight)return S_FALSE;
	if((x+m_dwWidth)<=0)return S_FALSE;
	if((y+m_dwHeight)<=0)return S_FALSE;

	//if it has been determined that we need to do a clipped blt lets prepare the rectangles

	//prepare destination rectangle (with D3D we only determine x, y value
	if(x>=0)
		psDest.x=x;
	else
		psDest.x=0;

	if(y>=0)
		psDest.y=y;
	else
		psDest.y=0;
	
	//prepare src rectangle this algorithm is appropriate for the matter.
	if(x>=0)
		rcSrc.left=0;
	else
		rcSrc.left=0-x;

	if((int)(x+m_dwWidth)<nBufferWidth)
		rcSrc.right=m_dwWidth;
	else
		rcSrc.right=nBufferWidth-x;

	if(y>=0)
		rcSrc.top=0;
	else
		rcSrc.top=0-y;

	if((int)(y+m_dwHeight)<nBufferHeight)
		rcSrc.bottom=m_dwHeight;
	else
		rcSrc.bottom=nBufferHeight-y;

	//Copy the rects.
	return CopySurfaceToSurface(
		(LPDIRECT3DDEVICE9)lpDevice,
		&rcSrc, 
		m_lpImage, 
		&psDest, 
		(LPDIRECT3DSURFACE9)lpBuffer, 
		TRUE, 
		0x00FF00FF);
}
