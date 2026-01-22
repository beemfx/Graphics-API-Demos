/*
	D3DImage.c - Functionality for D3D images.

	Copyright (c) 2003 Blaine Myers
*/
#include <d3dx9.h>
#include "GFX3D9.h"

/* The image vertex type. */
#define IMGVERTEX_TYPE \
(                      \
	D3DFVF_XYZRHW|      \
	D3DFVF_DIFFUSE|     \
	D3DFVF_TEX1         \
)

/* The image vertex structure. */
typedef struct tagIMGVERTEX{
	float x, y, z, rhw;
	D3DCOLOR Diffuse;
	float tu, tv;
}IMGVERTEX, *LPIMGVERTEX;

/* The image structure. */
typedef struct tagD3DIMAGE{
	LPDIRECT3DTEXTURE9 lpTexture;
	LPDIRECT3DVERTEXBUFFER9 lpVB;
	IMGVERTEX vImage[4];
	DWORD dwWidth;
	DWORD dwHeight;
	BOOL bIsColor;
	LPDIRECT3DDEVICE9 lpDevice;
}D3DIMAGE, *LPD3DIMAGE;

__inline void SetVertices(LPD3DIMAGE lpImage, DWORD dwWidth, DWORD dwHeight){
	/* Set up the initial vertices. */
	lpImage->vImage[0].x=(float)dwWidth;
	lpImage->vImage[0].y=(float)dwHeight;
	lpImage->vImage[0].z=0.0f;
	lpImage->vImage[0].rhw=1.0f;
	lpImage->vImage[0].Diffuse=0xFFFFFFFFl;
	lpImage->vImage[0].tu=1.0f;
	lpImage->vImage[0].tv=1.0f;

	lpImage->vImage[1].x=0.0f;
	lpImage->vImage[1].y=(float)dwHeight;
	lpImage->vImage[1].z=0.0f;
	lpImage->vImage[1].rhw=1.0f;
	lpImage->vImage[1].Diffuse=0xFFFFFFFFl;
	lpImage->vImage[1].tu=0.0f;
	lpImage->vImage[1].tv=1.0f;

	lpImage->vImage[2].x=0.0f;
	lpImage->vImage[2].y=0.0f;
	lpImage->vImage[2].z=0.0f;
	lpImage->vImage[2].rhw=1.0f;
	lpImage->vImage[2].Diffuse=0xFFFFFFFFl;
	lpImage->vImage[2].tu=0.0f;
	lpImage->vImage[2].tv=0.0f;

	lpImage->vImage[3].x=(float)dwWidth;
	lpImage->vImage[3].y=0.0f;
	lpImage->vImage[3].z=0.0f;
	lpImage->vImage[3].rhw=1.0f;
	lpImage->vImage[3].Diffuse=0xFFFFFFFFl;
	lpImage->vImage[3].tu=1.0f;
	lpImage->vImage[3].tv=0.0f;
}

HD3DIMAGE CopyD3DImage(
	HD3DIMAGE lpImageSrc)
{
	if(!lpImageSrc)
		return NULL;

	return CreateD3DImageFromTexture(
		((LPD3DIMAGE)lpImageSrc)->lpDevice, 
		((LPD3DIMAGE)lpImageSrc)->dwWidth, 
		((LPD3DIMAGE)lpImageSrc)->dwHeight, 
		((LPD3DIMAGE)lpImageSrc)->lpTexture);

}

HD3DIMAGE CreateD3DImageFromFileA(
	LPDIRECT3DDEVICE9 lpDevice,
	char szFilename[MAX_PATH],
	DWORD dwWidth,
	DWORD dwHeight,
	D3DCOLOR dwTransparent)
{
	HRESULT hr=0;
	D3DSURFACE_DESC TexDesc;
	LPD3DIMAGE lpImage=NULL;

	if(!lpDevice)
		return NULL;

	lpImage=malloc(sizeof(D3DIMAGE));
	if(lpImage==NULL)
		return NULL;

	ZeroMemory(&TexDesc, sizeof(D3DSURFACE_DESC));
	lpImage->lpTexture=NULL;
	lpImage->lpVB=NULL;

	/* Load the texture. */
	hr=D3DXCreateTextureFromFileExA(
		lpDevice, 
		szFilename,
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		0,
		D3DFMT_UNKNOWN,
		D3DPOOL_MANAGED,
		D3DX_FILTER_POINT,
		D3DX_FILTER_POINT,
		dwTransparent,
		NULL,
		NULL,
		&lpImage->lpTexture);

	if(FAILED(hr)){
		SAFE_FREE(lpImage);
		return NULL;
	}

	lpImage->lpDevice=lpDevice;
	lpImage->lpDevice->lpVtbl->AddRef(lpImage->lpDevice);

	lpImage->lpTexture->lpVtbl->GetLevelDesc(lpImage->lpTexture, 0, &TexDesc);

	if(dwHeight==-1)
		dwHeight=TexDesc.Height;
	lpImage->dwHeight=dwHeight;
	

	if(dwWidth==-1)
		dwWidth=TexDesc.Width;
	lpImage->dwWidth=dwWidth;
	

	lpImage->bIsColor=FALSE;

	SetVertices(lpImage, dwWidth, dwHeight);

	/* Create the vertex buffer by validate image. */
	if(!ValidateD3DImage(
		(HD3DIMAGE)lpImage))
	{
		SAFE_RELEASE( (lpImage->lpTexture) );
		SAFE_RELEASE( (lpImage->lpDevice) );
		SAFE_FREE(lpImage);
		return NULL;
	}
	
	return (HD3DIMAGE)lpImage;
}

HD3DIMAGE CreateD3DImageFromFileW(
	LPDIRECT3DDEVICE9 lpDevice,
	WCHAR szFilename[MAX_PATH],
	DWORD dwWidth,
	DWORD dwHeight,
	D3DCOLOR dwTransparent)
{
	HRESULT hr=0;
	D3DSURFACE_DESC TexDesc;
	LPD3DIMAGE lpImage=NULL;

	if(!lpDevice)
		return NULL;

	lpImage=malloc(sizeof(D3DIMAGE));
	if(lpImage==NULL)
		return NULL;
	ZeroMemory(&TexDesc, sizeof(D3DSURFACE_DESC));
	lpImage->lpTexture=NULL;
	lpImage->lpVB=NULL;


	/* Load the texture. */
	hr=D3DXCreateTextureFromFileExW(
		lpDevice, 
		szFilename,
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		0,
		D3DFMT_UNKNOWN,
		D3DPOOL_MANAGED,
		D3DX_FILTER_POINT,
		D3DX_FILTER_POINT,
		dwTransparent,
		NULL,
		NULL,
		&lpImage->lpTexture);
	if(FAILED(hr)){
		SAFE_FREE(lpImage);
		return NULL;
	}

	lpImage->lpDevice=lpDevice;
	lpImage->lpDevice->lpVtbl->AddRef(lpImage->lpDevice);

	lpImage->lpTexture->lpVtbl->GetLevelDesc(lpImage->lpTexture, 0, &TexDesc);

	if(dwHeight==-1)
		dwHeight=TexDesc.Height;
	lpImage->dwHeight=dwHeight;
	

	if(dwWidth==-1)
		dwWidth=TexDesc.Width;
	lpImage->dwWidth=dwWidth;
	

	lpImage->bIsColor=FALSE;

	SetVertices(lpImage, dwWidth, dwHeight);

	/* Create the vertex buffer by validate image. */
	if(!ValidateD3DImage(
		(HD3DIMAGE)lpImage))
	{
		SAFE_RELEASE( (lpImage->lpTexture) );
		SAFE_RELEASE( (lpImage->lpDevice) );
		SAFE_FREE(lpImage);
		return NULL;
	}
	
	return (HD3DIMAGE)lpImage;
}


HD3DIMAGE CreateD3DImageFromTexture(
	LPDIRECT3DDEVICE9 lpDevice,
	DWORD dwWidth,
	DWORD dwHeight,
	LPDIRECT3DTEXTURE9 lpTexture)
{
	HRESULT hr=0;
	D3DSURFACE_DESC TexDesc;
	LPD3DIMAGE lpImage=NULL;
	if(!lpDevice)
		return NULL;

	lpImage=malloc(sizeof(D3DIMAGE));
	if(lpImage==NULL)
		return NULL;
	ZeroMemory(&TexDesc, sizeof(D3DSURFACE_DESC));
	lpImage->lpTexture=NULL;
	lpImage->lpVB=NULL;

	/* Set the texture. */
	lpTexture->lpVtbl->AddRef(lpTexture);
	lpImage->lpTexture=lpTexture;

	lpImage->lpDevice=lpDevice;
	lpImage->lpDevice->lpVtbl->AddRef(lpImage->lpDevice);

	lpImage->lpTexture->lpVtbl->GetLevelDesc(lpImage->lpTexture, 0, &TexDesc);

	if(dwHeight==-1)
		dwHeight=TexDesc.Height;
	lpImage->dwHeight=dwHeight;
	

	if(dwWidth==-1)
		dwWidth=TexDesc.Width;
	lpImage->dwWidth=dwWidth;
	

	lpImage->bIsColor=FALSE;

	SetVertices(lpImage, dwWidth, dwHeight);

	/* Create the vertex buffer by validate image. */
	if(!ValidateD3DImage(
		(HD3DIMAGE)lpImage))
	{
		SAFE_RELEASE( (lpImage->lpTexture) );
		SAFE_RELEASE( (lpImage->lpDevice) );
		SAFE_FREE(lpImage);
		return NULL;
	}
	
	return (HD3DIMAGE)lpImage;
}

HD3DIMAGE CreateD3DImageFromColor(
	LPDIRECT3DDEVICE9 lpDevice,
	DWORD dwWidth,
	DWORD dwHeight,
	D3DCOLOR dwColor)
{
	HRESULT hr=0;
	LPD3DIMAGE lpImage=NULL;
	if(!lpDevice)
		return NULL;

	lpImage=malloc(sizeof(D3DIMAGE));
	lpImage->lpTexture=NULL;
	lpImage->lpVB=NULL;


	lpImage->dwHeight=dwHeight;
	lpImage->dwWidth=dwWidth;

	lpImage->bIsColor=TRUE;

	SetVertices(lpImage, dwWidth, dwHeight);

	lpImage->lpDevice=lpDevice;
	lpImage->lpDevice->lpVtbl->AddRef(lpImage->lpDevice);

	/* Set the color. */
	lpImage->vImage[0].Diffuse=dwColor;
	lpImage->vImage[1].Diffuse=dwColor;
	lpImage->vImage[2].Diffuse=dwColor;
	lpImage->vImage[3].Diffuse=dwColor;

	/* Create the vertex buffer by validate image. */
	if(!ValidateD3DImage(
		(HD3DIMAGE)lpImage))
	{
		SAFE_RELEASE( (lpImage->lpTexture) );
		SAFE_RELEASE( (lpImage->lpDevice) );
		SAFE_FREE(lpImage);
		return NULL;
	}

	return (HD3DIMAGE)lpImage;
}

BOOL InvalidateD3DImage(
	HD3DIMAGE lpImage)
{
	if(!lpImage)
		return FALSE;
	/* Release the vertex buffer.
		The texture is managed so we don't 
		need to release it. */
	SAFE_RELEASE( (((LPD3DIMAGE)lpImage)->lpVB) );
	return TRUE;
}

BOOL ValidateD3DImage(
	HD3DIMAGE lpImage)
{
	HRESULT hr=0;

	if(!lpImage)
		return FALSE;

	if( (((LPD3DIMAGE)lpImage)->lpVB) )
		return FALSE;


	/* Create the vertex buffer. */
	hr=((LPD3DIMAGE)lpImage)->lpDevice->lpVtbl->CreateVertexBuffer(
		((LPD3DIMAGE)lpImage)->lpDevice,
		sizeof(IMGVERTEX)*4,
		0,
		IMGVERTEX_TYPE,
		D3DPOOL_DEFAULT,
		&((LPD3DIMAGE)lpImage)->lpVB,
		NULL);

		if(FAILED(hr))
			return FALSE;

	return TRUE;
}

BOOL DeleteD3DImage(
	HD3DIMAGE lpImage)
{
	if(!lpImage)
		return FALSE;
	/* Release the texture and VB. */
	SAFE_RELEASE( (((LPD3DIMAGE)lpImage)->lpTexture) );
	SAFE_RELEASE( (((LPD3DIMAGE)lpImage)->lpVB) );
	SAFE_RELEASE( (((LPD3DIMAGE)lpImage)->lpDevice) );
	SAFE_FREE(lpImage);
	return TRUE;
}

BOOL RenderD3DImage(
	HD3DIMAGE lpImage,
	LONG x,
	LONG y)
{
	HRESULT hr=0;
	IMGVERTEX vFinalImage[4];
	DWORD dwPrevFVF=0;
	DWORD dwOldCullMode=0;
	LPVOID lpVertices=NULL;
	LPDIRECT3DTEXTURE9 lpOldTexture=NULL;
	LPDIRECT3DVERTEXSHADER9 lpOldVS=NULL;

	DWORD dwOldAlphaEnable=0, dwOldSrcBlend=0, dwOldDestBlend=0, dwOldAlphaA=0;

	DWORD dwZEnable=0;

	if(!lpImage)
		return FALSE;

	if(!((LPD3DIMAGE)lpImage)->lpVB)
		return FALSE;


	/* Copy over the final image. */
	vFinalImage[0]=((LPD3DIMAGE)lpImage)->vImage[0];
	vFinalImage[1]=((LPD3DIMAGE)lpImage)->vImage[1];
	vFinalImage[2]=((LPD3DIMAGE)lpImage)->vImage[2];
	vFinalImage[3]=((LPD3DIMAGE)lpImage)->vImage[3];

	/* Apply the position. */
	vFinalImage[0].x+=x;
	vFinalImage[0].y+=y;

	vFinalImage[1].x+=x;
	vFinalImage[1].y+=y;

	vFinalImage[2].x+=x;
	vFinalImage[2].y+=y;

	vFinalImage[3].x+=x;
	vFinalImage[3].y+=y;

	/* Copy over the transformed vertices. */
	hr=((LPD3DIMAGE)lpImage)->lpVB->lpVtbl->Lock(((LPD3DIMAGE)lpImage)->lpVB, 0, sizeof(IMGVERTEX)*4, &lpVertices, 0);

	if(FAILED(hr))
		return FALSE;

	memcpy(lpVertices, &vFinalImage, sizeof(IMGVERTEX)*4);

	hr=((LPD3DIMAGE)lpImage)->lpVB->lpVtbl->Unlock(((LPD3DIMAGE)lpImage)->lpVB);

	/* Set the texture. */
	((LPD3DIMAGE)lpImage)->lpDevice->lpVtbl->GetTexture(((LPD3DIMAGE)lpImage)->lpDevice, 0, (LPDIRECT3DBASETEXTURE9*)&lpOldTexture);
	((LPD3DIMAGE)lpImage)->lpDevice->lpVtbl->SetTexture(((LPD3DIMAGE)lpImage)->lpDevice, 0, (LPDIRECT3DBASETEXTURE9)((LPD3DIMAGE)lpImage)->lpTexture);
	/* Get and set FVF. */
	((LPD3DIMAGE)lpImage)->lpDevice->lpVtbl->GetFVF(((LPD3DIMAGE)lpImage)->lpDevice, &dwPrevFVF);
	((LPD3DIMAGE)lpImage)->lpDevice->lpVtbl->SetFVF(((LPD3DIMAGE)lpImage)->lpDevice, IMGVERTEX_TYPE);
	/* Get and set vertex shader. */
	((LPD3DIMAGE)lpImage)->lpDevice->lpVtbl->GetVertexShader(((LPD3DIMAGE)lpImage)->lpDevice, &lpOldVS);
	((LPD3DIMAGE)lpImage)->lpDevice->lpVtbl->SetVertexShader(((LPD3DIMAGE)lpImage)->lpDevice, NULL);
	/* Get and set cull mode. */
	((LPD3DIMAGE)lpImage)->lpDevice->lpVtbl->GetRenderState(((LPD3DIMAGE)lpImage)->lpDevice, D3DRS_CULLMODE, &dwOldCullMode);
	((LPD3DIMAGE)lpImage)->lpDevice->lpVtbl->SetRenderState(((LPD3DIMAGE)lpImage)->lpDevice, D3DRS_CULLMODE, D3DCULL_CCW);

	/* Set alpha blending. */	
	((LPD3DIMAGE)lpImage)->lpDevice->lpVtbl->GetRenderState(((LPD3DIMAGE)lpImage)->lpDevice, D3DRS_ALPHABLENDENABLE, &dwOldAlphaEnable);
	((LPD3DIMAGE)lpImage)->lpDevice->lpVtbl->SetRenderState(((LPD3DIMAGE)lpImage)->lpDevice, D3DRS_ALPHABLENDENABLE, TRUE);	

	((LPD3DIMAGE)lpImage)->lpDevice->lpVtbl->GetRenderState(((LPD3DIMAGE)lpImage)->lpDevice, D3DRS_SRCBLEND, &dwOldSrcBlend);
	((LPD3DIMAGE)lpImage)->lpDevice->lpVtbl->SetRenderState(((LPD3DIMAGE)lpImage)->lpDevice, D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		
	((LPD3DIMAGE)lpImage)->lpDevice->lpVtbl->GetRenderState(((LPD3DIMAGE)lpImage)->lpDevice, D3DRS_DESTBLEND, &dwOldDestBlend);
	((LPD3DIMAGE)lpImage)->lpDevice->lpVtbl->SetRenderState(((LPD3DIMAGE)lpImage)->lpDevice, D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		
	((LPD3DIMAGE)lpImage)->lpDevice->lpVtbl->GetTextureStageState(((LPD3DIMAGE)lpImage)->lpDevice, 0, D3DTSS_ALPHAARG1, &dwOldAlphaA);
	((LPD3DIMAGE)lpImage)->lpDevice->lpVtbl->SetTextureStageState(((LPD3DIMAGE)lpImage)->lpDevice, 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

	/* Get and set z-buffer status. */
	((LPD3DIMAGE)lpImage)->lpDevice->lpVtbl->GetRenderState(((LPD3DIMAGE)lpImage)->lpDevice, D3DRS_ZENABLE, &dwZEnable);
	((LPD3DIMAGE)lpImage)->lpDevice->lpVtbl->SetRenderState(((LPD3DIMAGE)lpImage)->lpDevice, D3DRS_ZENABLE, FALSE);


	/* Render the image. */
	((LPD3DIMAGE)lpImage)->lpDevice->lpVtbl->SetStreamSource(((LPD3DIMAGE)lpImage)->lpDevice, 0, ((LPD3DIMAGE)lpImage)->lpVB, 0, sizeof(IMGVERTEX));
	((LPD3DIMAGE)lpImage)->lpDevice->lpVtbl->DrawPrimitive(((LPD3DIMAGE)lpImage)->lpDevice, D3DPT_TRIANGLEFAN, 0, 2);

	/* Restore all saved values. */
	((LPD3DIMAGE)lpImage)->lpDevice->lpVtbl->SetTexture(((LPD3DIMAGE)lpImage)->lpDevice, 0, (LPDIRECT3DBASETEXTURE9)lpOldTexture);
	SAFE_RELEASE(lpOldTexture);
	((LPD3DIMAGE)lpImage)->lpDevice->lpVtbl->SetFVF(((LPD3DIMAGE)lpImage)->lpDevice, dwPrevFVF);
	((LPD3DIMAGE)lpImage)->lpDevice->lpVtbl->SetVertexShader(((LPD3DIMAGE)lpImage)->lpDevice, lpOldVS);
	((LPD3DIMAGE)lpImage)->lpDevice->lpVtbl->SetRenderState(((LPD3DIMAGE)lpImage)->lpDevice, D3DRS_CULLMODE, dwOldCullMode);

	/* Restore alpha blending state. */
	((LPD3DIMAGE)lpImage)->lpDevice->lpVtbl->SetRenderState(((LPD3DIMAGE)lpImage)->lpDevice, D3DRS_ALPHABLENDENABLE, dwOldAlphaEnable);	
	((LPD3DIMAGE)lpImage)->lpDevice->lpVtbl->SetRenderState(((LPD3DIMAGE)lpImage)->lpDevice, D3DRS_SRCBLEND, dwOldSrcBlend);	
	((LPD3DIMAGE)lpImage)->lpDevice->lpVtbl->SetRenderState(((LPD3DIMAGE)lpImage)->lpDevice, D3DRS_DESTBLEND, dwOldDestBlend);	
	((LPD3DIMAGE)lpImage)->lpDevice->lpVtbl->SetTextureStageState(((LPD3DIMAGE)lpImage)->lpDevice, 0, D3DTSS_ALPHAARG1, dwOldAlphaA);

	/* Restore Z-Buffering status. */
	((LPD3DIMAGE)lpImage)->lpDevice->lpVtbl->SetRenderState(((LPD3DIMAGE)lpImage)->lpDevice, D3DRS_ZENABLE, dwZEnable);
	
	return TRUE;
}

BOOL RenderD3DImageEx(
	HD3DIMAGE lpImage,
	LONG nXDest,
	LONG nYDest,
	LONG nWidthDest,
	LONG nHeightDest,
	LONG nXSrc,
	LONG nYSrc,
	LONG nWidthSrc,
	LONG nHeightSrc)
{
	D3DIMAGE FinalImage;

	if(!lpImage)
		return FALSE;

	ZeroMemory(&FinalImage, sizeof(D3DIMAGE));

	FinalImage=*(LPD3DIMAGE)lpImage;

	FinalImage.dwWidth=nWidthDest;
	FinalImage.dwHeight=nHeightDest;

	FinalImage.vImage[0].x=(float)nWidthDest;
	FinalImage.vImage[0].y=(float)nHeightDest;
	FinalImage.vImage[1].y=(float)nHeightDest;
	FinalImage.vImage[3].x=(float)nWidthDest;

	/* Discover the proper texture coords. */
	FinalImage.vImage[0].tu=(float)(nWidthSrc+nXSrc)/(float)GetD3DImageWidth(lpImage);
	FinalImage.vImage[0].tv=(float)(nHeightSrc+nYSrc)/(float)GetD3DImageHeight(lpImage);

	FinalImage.vImage[1].tu=(float)nXSrc/(float)GetD3DImageWidth(lpImage);
	FinalImage.vImage[1].tv=(float)(nHeightSrc+nYSrc)/(float)GetD3DImageHeight(lpImage);

	FinalImage.vImage[2].tu=(float)nXSrc/(float)GetD3DImageWidth(lpImage);
	FinalImage.vImage[2].tv=(float)nYSrc/(float)GetD3DImageHeight(lpImage);

	FinalImage.vImage[3].tu=(float)(nWidthSrc+nXSrc)/GetD3DImageWidth(lpImage);
	FinalImage.vImage[3].tv=(float)nYSrc/(float)GetD3DImageHeight(lpImage);

	return RenderD3DImage((HD3DIMAGE)&FinalImage, nXDest, nYDest);
}

BOOL RenderD3DImageRelativeEx(
	HD3DIMAGE lpImage,
	float fXDest,
	float fYDest,
	float fWidthDest,
	float fHeightDest,
	LONG nXSrc,
	LONG nYSrc,
	LONG nWidthSrc,
	LONG nHeightSrc)
{
	D3DIMAGE FinalImage;
	LONG dwScreenWidth=0, dwScreenHeight=0;
	D3DVIEWPORT9 ViewPort;

	if(!lpImage)
		return FALSE;

	ZeroMemory(&FinalImage, sizeof(D3DIMAGE));
	ZeroMemory(&ViewPort, sizeof(D3DVIEWPORT9));

	((LPD3DIMAGE)lpImage)->lpDevice->lpVtbl->GetViewport(((LPD3DIMAGE)lpImage)->lpDevice, &ViewPort);
	dwScreenWidth=ViewPort.Width;
	dwScreenHeight=ViewPort.Height;

	FinalImage=*(LPD3DIMAGE)lpImage;

	
	/* Scale down the dest values. */
	fWidthDest/=100.0f;
	fHeightDest/=100.0f;
	fXDest/=100.0f;
	fYDest/=100.0f;

	FinalImage.dwWidth=(LONG)(fWidthDest*(float)dwScreenWidth);
	FinalImage.dwHeight=(LONG)(fHeightDest*(float)dwScreenHeight);

	FinalImage.vImage[0].x=(float)FinalImage.dwWidth;
	FinalImage.vImage[0].y=(float)FinalImage.dwHeight;
	FinalImage.vImage[1].y=(float)FinalImage.dwHeight;
	FinalImage.vImage[3].x=(float)FinalImage.dwWidth;

	

	/* Discover the proper texture coords. */
	FinalImage.vImage[0].tu=(float)(nWidthSrc+nXSrc)/(float)GetD3DImageWidth(lpImage);
	FinalImage.vImage[0].tv=(float)(nHeightSrc+nYSrc)/(float)GetD3DImageHeight(lpImage);

	FinalImage.vImage[1].tu=(float)nXSrc/(float)GetD3DImageWidth(lpImage);
	FinalImage.vImage[1].tv=(float)(nHeightSrc+nYSrc)/(float)GetD3DImageHeight(lpImage);

	FinalImage.vImage[2].tu=(float)nXSrc/(float)GetD3DImageWidth(lpImage);
	FinalImage.vImage[2].tv=(float)nYSrc/(float)GetD3DImageHeight(lpImage);

	FinalImage.vImage[3].tu=(float)(nWidthSrc+nXSrc)/(float)GetD3DImageWidth(lpImage);
	FinalImage.vImage[3].tv=(float)nYSrc/(float)GetD3DImageHeight(lpImage);
	
	return RenderD3DImage((HD3DIMAGE)&FinalImage, (LONG)(fXDest*dwScreenWidth), (LONG)(fYDest*dwScreenHeight));
}

LONG GetD3DImageWidth(HD3DIMAGE lpImage)
{
	if(!lpImage)
		return 0;

	return ((LPD3DIMAGE)lpImage)->dwWidth;
}

LONG GetD3DImageHeight(HD3DIMAGE lpImage)
{
	if(!lpImage)
		return 0;
	return ((LPD3DIMAGE)lpImage)->dwHeight;
}