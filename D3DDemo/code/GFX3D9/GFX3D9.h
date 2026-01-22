/*
	GFX3D9.h - Header for 3D graphics library

	Copyright (c) 2003, Blaine Myers
*/
#ifndef __GFX3D9_H__
#define __GFX3D9_H__
#include <d3d9.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define GFX3D9_EXPORTS

/* Macros for quicker functionality. */
#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if(p) { delete (p); (p)=NULL; } }
#endif /* SAFE_DELETE */
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if(p) { delete [] (p); (p)=NULL; } }
#endif /* SAFE_DELETE_ARRAY */
#ifndef SAFE_FREE
#define SAFE_FREE(p)         { if(p) { free(p); (p)=NULL; } }
#endif /* SAFE_FREE */
#ifndef SAFE_RELEASE
#ifdef __cplusplus
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }
#else /* __cplusplus */
#define SAFE_RELEASE(p)      { if(p) { (p)->lpVtbl->Release((p)); (p)=NULL; } }
#endif /* __cplusplus */
#endif /* SAFE_RELEASE */


/* The custom vertex type */
#define CUSTOMVERTEX_TYPE	\
(									\
	D3DFVF_XYZ|					\
	D3DFVF_NORMAL|				\
	D3DFVF_DIFFUSE|			\
	D3DFVF_SPECULAR|			\
	D3DFVF_TEX1					\
)


/* The custom vertex structure */
#ifdef __cplusplus
typedef struct GFX3D9_EXPORTS tagCUSTOMVERTEX
#else /* __cplusplus */
typedef struct tagCUSTOMVERTEX
#endif /* __cplusplus */
{
	D3DVECTOR Position;     /* Vertex Position */
	D3DVECTOR Normal;       /* Vertex Normal */
	D3DCOLOR DiffuseColor;  /* Vertex Diffuse Color */
	D3DCOLOR SpecularColor; /* Vertex Specular color */
	float tu, tv;           /* Vertex Texture Coordinates */

	/* C++ adds additional functionality */
	#ifdef __cplusplus
	tagCUSTOMVERTEX();
	tagCUSTOMVERTEX(
		float px, 
		float py, 
		float pz, 
		float nx, 
		float ny, 
		float nz, 
		D3DCOLOR dwDiffuse, 
		D3DCOLOR dwSpecular, 
		float txu, 
		float txv);


	tagCUSTOMVERTEX operator = (const tagCUSTOMVERTEX & ); /* Assignment operator overload */
	int operator == (const tagCUSTOMVERTEX & ); /* Equality operator overload */
	int operator != (const tagCUSTOMVERTEX & ); /* Inequality operator overload */
	#endif /* __cplusplus */
}CUSTOMVERTEX, *LPCUSTOMVERTEX;




/* 2D Image Functionality. */
typedef HANDLE HD3DIMAGE;

#ifdef UNICODE
#define CreateD3DImageFromFile CreateD3DImageFromFileW
#else /* UNICODE */
#define CreateD3DImageFromFile CreateD3DImageFromFileA
#endif /* UNICODE */

GFX3D9_EXPORTS HD3DIMAGE CreateD3DImageFromFileA(
	LPDIRECT3DDEVICE9 lpDevice,
	char szFilename[MAX_PATH],
	DWORD dwWidth,
	DWORD dwHeight,
	D3DCOLOR dwTransparent);

GFX3D9_EXPORTS HD3DIMAGE CreateD3DImageFromFileW(
	LPDIRECT3DDEVICE9 lpDevice,
	WCHAR szFilename[MAX_PATH],
	DWORD dwWidth,
	DWORD dwHeight,
	D3DCOLOR dwTransparent);


GFX3D9_EXPORTS HD3DIMAGE CreateD3DImageFromTexture(
	LPDIRECT3DDEVICE9 lpDevice,
	DWORD dwWidth,
	DWORD dwHeight,
	LPDIRECT3DTEXTURE9 lpTexture);

GFX3D9_EXPORTS HD3DIMAGE CreateD3DImageFromColor(
	LPDIRECT3DDEVICE9 lpDevice,
	DWORD dwWidth,
	DWORD dwHeight,
	D3DCOLOR dwColor);

GFX3D9_EXPORTS BOOL InvalidateD3DImage(
	HD3DIMAGE hImage);

GFX3D9_EXPORTS BOOL ValidateD3DImage(
	HD3DIMAGE hImage);

GFX3D9_EXPORTS BOOL DeleteD3DImage(
	HD3DIMAGE hImage);

GFX3D9_EXPORTS BOOL RenderD3DImage(
	HD3DIMAGE hImage,
	LONG x,
	LONG y);

GFX3D9_EXPORTS BOOL RenderD3DImageEx(
	HD3DIMAGE hImage,
	LONG nXDest,
	LONG nYDest,
	LONG nWidthDest,
	LONG nHeightDest,
	LONG nXSrc,
	LONG nYSrc,
	LONG nWidthSrc,
	LONG nHeightSrc);

GFX3D9_EXPORTS BOOL RenderD3DImageRelativeEx(
	HD3DIMAGE hImage,
	float fXDest,
	float fYDest,
	float fWidthDest,
	float fHeightDest,
	LONG nXSrc,
	LONG nYSrc,
	LONG nWidthSrc,
	LONG nHeightSrc);

GFX3D9_EXPORTS HD3DIMAGE CopyD3DImage(
	HD3DIMAGE hImageSrc);

GFX3D9_EXPORTS LONG GetD3DImageWidth(
	HD3DIMAGE hImage);

GFX3D9_EXPORTS LONG GetD3DImageHeight(
	HD3DIMAGE hImage);




/* The font functionality. */
typedef HANDLE HD3DFONT;

#ifdef UNICODE
#define CreateD3DFontFromImage CreateD3DFontFromImageW
#else /* UNICODE */
#define CreateD3DFontFromImage CreateD3DFontFromImageA
#endif /* UNICODE */

GFX3D9_EXPORTS HD3DFONT GetD3DFontDefault(
	LPDIRECT3DDEVICE9 lpDevice);

GFX3D9_EXPORTS BOOL SetD3DFontSize(
	HD3DFONT hFont,
	WORD wWidth,
	WORD wHeight);

GFX3D9_EXPORTS BOOL GetD3DFontDims(
	HD3DFONT hFont,
	WORD * lpWidth,
	WORD * lpHeight);

GFX3D9_EXPORTS HD3DFONT CreateD3DFontFromFont(
	LPDIRECT3DDEVICE9 lpDevice,
	HFONT hFont,
	DWORD dwColor);

GFX3D9_EXPORTS HD3DFONT CreateD3DFontFromD3DImage(
	LPDIRECT3DDEVICE9 lpDevice,
	HD3DIMAGE hImage,
	WORD wCharsPerLine,
	WORD wNumLines);

GFX3D9_EXPORTS HD3DFONT CreateD3DFontFromImageA(
	LPDIRECT3DDEVICE9 lpDevice,
	char szFilename[MAX_PATH],
	WORD wCharsPerLine,
	WORD wNumLines);

GFX3D9_EXPORTS HD3DFONT CreateD3DFontFromImageW(
	LPDIRECT3DDEVICE9 lpDevice,
	WCHAR szFilename[MAX_PATH],
	WORD wCharsPerLine,
	WORD wNumLines);

GFX3D9_EXPORTS HD3DFONT CreateD3DFontFromTexture(
	LPDIRECT3DDEVICE9 lpDevice,
	LPDIRECT3DTEXTURE9 lpTexture,
	WORD wCharsPerLine,
	WORD wNumLines);

GFX3D9_EXPORTS BOOL InvalidateD3DFont(
	HD3DFONT hFont);

GFX3D9_EXPORTS BOOL ValidateD3DFont(
	HD3DFONT hFont);

GFX3D9_EXPORTS BOOL DeleteD3DFont(
	HD3DFONT hFont);

GFX3D9_EXPORTS BOOL RenderD3DFontChar(
	HD3DFONT hFont,
	LONG nXDest,
	LONG nYDest,
	char cChar);

GFX3D9_EXPORTS BOOL RenderD3DFontString(
	HD3DFONT hFont,
	LONG nXDest,
	LONG nYDest,
	char szString[]);

GFX3D9_EXPORTS HD3DFONT CopyD3DFont(
	LPDIRECT3DDEVICE9 lpDevice,
	HD3DFONT hFontSrc);

/* Console functionality. */
typedef HANDLE HBEEMCONSOLE;
typedef BOOL ( * LPCOMMAND)(LPSTR szCommand, LPSTR szParams, HBEEMCONSOLE hConsole);

GFX3D9_EXPORTS BOOL ScrollBeemConsole(
	HBEEMCONSOLE hConsole, 
	LONG lScrollDist);

GFX3D9_EXPORTS BOOL ToggleActivateBeemConsole(
	HBEEMCONSOLE hConsole);

GFX3D9_EXPORTS HBEEMCONSOLE CreateBeemConsole(
	LPDIRECT3DDEVICE9 lpDevice,
	HD3DFONT hFont, 
	HD3DIMAGE hConsoleBG,
	LPCOMMAND CommandFunction);

GFX3D9_EXPORTS BOOL DeleteBeemConsole(
	HBEEMCONSOLE hConsole);

GFX3D9_EXPORTS BOOL InvalidateBeemConsole(
	HBEEMCONSOLE hConsole);

GFX3D9_EXPORTS BOOL ValidateBeemConsole(
	HBEEMCONSOLE hConsole);


GFX3D9_EXPORTS BOOL BeemConsoleClearEntries(
	HBEEMCONSOLE hConsole);

GFX3D9_EXPORTS BOOL AddBeemConsoleEntry(
	HBEEMCONSOLE hConsole,
	LPSTR szEntry);

GFX3D9_EXPORTS BOOL SendBeemConsoleCommand(
	HBEEMCONSOLE hConsole,
	LPSTR szCommandLine);

GFX3D9_EXPORTS BOOL SendBeemConsoleMessage(
	HBEEMCONSOLE hConsole,
	LPSTR szMessage);

GFX3D9_EXPORTS BOOL BeemConsoleOnChar(
	HBEEMCONSOLE hConsole,
	char cChar);

GFX3D9_EXPORTS BOOL RenderBeemConsole(
	HBEEMCONSOLE hConsole);

GFX3D9_EXPORTS BOOL BeemParseGetParam(
	LPSTR szParamOut, 
	LPSTR szParams, 
	WORD wParam);

GFX3D9_EXPORTS BOOL BeemParseGetString(
	LPSTR szStringOut,
	LPSTR szString);

GFX3D9_EXPORTS BOOL BeemParseGetFloat(
	FLOAT * lpValue,
	LPSTR szParams,
	WORD wParam);

GFX3D9_EXPORTS BOOL BeemParseGetInt(
	LONG * lpValue,
	LPSTR szParams,
	WORD wParam);

#define BEGINCHOICE if(0){
#define CHOICE(s) }else if( _stricmp(#s, szCommand)==0){
#define INVALIDCHOICE }else{
#define ENDCHOICE }


/* InitD3D Initializes the device and backbuffer. */
GFX3D9_EXPORTS BOOL InitD3D(
	HWND hWndTarget, 
	DWORD dwWidth, 
	DWORD dwHeight, 
	BOOL bWindowed, 
	BOOL bVsync,
	D3DFORMAT d3dfFullScreenFormat, 
	LPDIRECT3D9 lppD3D, 
	LPDIRECT3DDEVICE9 * lppDevice,
	LPDIRECT3DSURFACE9 * lppBackSurface,
	D3DPRESENT_PARAMETERS * d3dSavedParams);



/* CorrectWindowSize adjusts the client area of a window to specified size. */
GFX3D9_EXPORTS BOOL CorrectWindowSize(
	HWND hWnd, 
	DWORD nWidth, 
	DWORD nHeight, 
	BOOL bWindowed,
	HMENU hMenu);

GFX3D9_EXPORTS BOOL SetProjectionMatrix(
	LPDIRECT3DDEVICE9 lpDevice, 
	DWORD dwWidth, 
	DWORD dwHeight,
	FLOAT zn,
	FLOAT zf);

GFX3D9_EXPORTS BOOL SetViewPort(
	LPDIRECT3DDEVICE9 lpDevice,
	DWORD dwWidth,
	DWORD dwHeight);

typedef enum tagD3DFILTER{
	D3DFILTER_POINT=0x00000000l,
	D3DFILTER_BILINEAR,
	D3DFILTER_TRILINEAR,
	D3DFILTER_ANISOTROPIC
}D3DFILTER;

GFX3D9_EXPORTS BOOL SetTextureFilter(
	LPDIRECT3DDEVICE9 lpDevice,
	DWORD Stage,
	D3DFILTER FilterMode);


/* ValidateDevice validates the device and restores backbuffer.  It
   also calls a RestoreGraphics function if one is specified. */
typedef BOOL ( * POOLFN)();

GFX3D9_EXPORTS BOOL ValidateDevice(
	LPDIRECT3DDEVICE9 * lppDevice, 
	LPDIRECT3DSURFACE9 * lppBackSurface,
	D3DPRESENT_PARAMETERS d3dpp,
	POOLFN fpReleasePool,
	POOLFN fpRestorePool);

GFX3D9_EXPORTS BOOL ScreenSwitch(
	DWORD dwWidth,
	DWORD dwHeight,
	BOOL bWindowed,
	D3DFORMAT FullScreenFormat,
	BOOL bVsync,
	LPDIRECT3DDEVICE9 lpDevice,
	LPDIRECT3DSURFACE9 * lppBackSurface,
	D3DPRESENT_PARAMETERS * lpSavedParams,
	POOLFN fpReleasePool,
	POOLFN fpRestorePool);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GFX3D9_H__ */