/*
	GFXGin.h - header for GFXG7 Dynamic-Link Library

	Copyright (c) 2002, 2003 Blaine Myers
*/

#ifndef __GFXG7_H__
#define __GFXG7_H__

#include <ddraw.h>
#include <windows.h>

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */


#define GFXG7_EXPORTS


typedef struct tagSAVEDPARAMETERS{
	DWORD dwWidth;
	DWORD dwHeight;
	DWORD dwColorDepth;
	DWORD dwTransparentColor;
	HWND hWnd;
	RECT rcWindow;
	BOOL bWindowed;
}SAVEDPARAMETERS;



typedef enum tagIMAGETYPE{
	ET_BITMAP=0, 
	ET_COLOR
}IMAGETYPE;

typedef struct tagIMAGEDATA3{
	IMAGETYPE nType; //type of image bitmap or color

	DWORD nWidth; //width of image
	DWORD nHeight; //height of image

	DWORD dwColor; //color of image
}IMAGEDATA3;


#define RV_LEFTRIGHT 0x01000000l
#define RV_UPDOWN    0x02000000l

#define CImage CImage7

#ifdef UNICODE
#define CreateImageBM CreateImageBMW
#else //unicode
#define CreateImageBM CreateImageBMA
#endif //unicode

GFXG7_EXPORTS HRESULT InitDirectDraw(
	LPDIRECTDRAW7 *lppDDraw,
	LPDIRECTDRAWSURFACE7 *lppPrimary,
	LPDIRECTDRAWSURFACE7 *lppBack,
	HWND hwnd,
	BOOL bWindowed,
	DWORD dwWidth,
	DWORD dwHeight,
	DWORD dwBitDepth,
	RECT *rcWindow,
	DWORD *dwTransparentColor);

GFXG7_EXPORTS HRESULT InitDirectDrawEx(
	LPDIRECTDRAW7 * lppDDraw,
	LPDIRECTDRAWSURFACE7 * lppPrimary,
	LPDIRECTDRAWSURFACE7 * lppBack,
	SAVEDPARAMETERS * pSavedParams);

GFXG7_EXPORTS HRESULT InitFullScreenDirectDraw(
	LPDIRECTDRAW7 *lppDDraw, 
	LPDIRECTDRAWSURFACE7 *lppBackBuffer, 
	LPDIRECTDRAWSURFACE7 *lppPrimary, 
	HWND hWnd, 
	DWORD dwWidth,	
	DWORD dwHeight, 
	DWORD dwBitDepth, 
	RECT *rcWindow, 
	DWORD *nTransparentColor);

GFXG7_EXPORTS HRESULT InitWindowedDirectDraw(
	LPDIRECTDRAW7 *lppDDraw, 
	LPDIRECTDRAWSURFACE7 *lppBackBuffer, 
	LPDIRECTDRAWSURFACE7 *lppPrimary, 
	HWND hWnd, DWORD dwWidth, 
	DWORD dwHeight, 
	RECT * rcWindow, 
	DWORD * lpTransparentColor);


GFXG7_EXPORTS void SetSavedParameters(
	SAVEDPARAMETERS * pSavedParams,
	DWORD dwWidth,
	DWORD dwHeight,
	DWORD dwColorDepth,
	DWORD dwTransparentColor,
	HWND hWnd,
	RECT * rcWindow,
	BOOL bWindowed);

GFXG7_EXPORTS DWORD WindowsColorToDirectDraw(
	COLORREF rgb, 
	LPDIRECTDRAWSURFACE7 surface);

GFXG7_EXPORTS HRESULT UpdateBounds(
	BOOL bWindowed, 
	HWND hWnd, 
	RECT * rcWindow);

GFXG7_EXPORTS HRESULT AdjustWindowSize(
	HWND hWnd, 
	DWORD nWidth, 
	DWORD nHeight, 
	BOOL bWindowed, 
	HMENU hMenu);


GFXG7_EXPORTS HRESULT PageFlip(
	LPDIRECTDRAWSURFACE7 lpPrimary, 
	LPDIRECTDRAWSURFACE7 lpBackBuffer, 
	BOOL bWindowed, 
	RECT * rcWindow, 
	HRESULT ( * Restore)());

#ifdef __cplusplus
class GFXG7_EXPORTS CImage7{
private:
	LPDIRECTDRAWSURFACE7 m_lpImage; //Surface for image
	HBITMAP m_hBitmap; //Bitmap compatible version of image
	IMAGEDATA3 m_sImageData; //data about image

	HRESULT CreateSurface(DWORD nWidth, DWORD nHeight, LPVOID lpObject, DWORD dwTransparent); //create surface for image
	
	HRESULT LoadBitmapInMemoryIntoSurface(
		HBITMAP hBitmap,
		DWORD nWidth,
		DWORD nHeight); //loads an hbitmap into m_lpImage, both should have same dimensions

	HRESULT LoadColorIntoSurface(DWORD dwColor); //loads color into surface
public:
	CImage7(); //constructor
	~CImage7(); //destructor

	DWORD GetWidth();  //Get Image Width
	DWORD GetHeight(); //Get Image Height

	HRESULT Restore(); //restore images surface
	HRESULT ReloadImageIntoSurface(); //reloads image into surface (for after a restore)
	HRESULT ClearSurface(); //clears the surface to write new image to
	void Release(); //releases the surface

	LPDIRECTDRAWSURFACE7* GetPointerToSurface();


	
	HRESULT CreateImageBMA(
		LPVOID lpDevice,
		DWORD dwTransparent,
		char szFilename[MAX_PATH], 
		int nX, 
		int nY, 
		int nSrcWidth, 
		int nSrcHeight, 
		DWORD nWidth, 
		DWORD nHeight, 
		DWORD dwReverseFlags); //creates and image in the surface from a bitmap file

	HRESULT CreateImageBMW(
		LPVOID lpDevice,
		DWORD dwTransparent,
		WCHAR szFilename[MAX_PATH], 
		int nX, 
		int nY, 
		int nSrcWidth, 
		int nSrcHeight, 
		DWORD nWidth, 
		DWORD nHeight, 
		DWORD dwReverseFlags); //creates and image in the surface from a bitmap file

	HRESULT CreateImageBMInMemory(
		LPVOID lpObject,
		DWORD dwTransparent,
		HBITMAP hBitmap,
		int nX,
		int nY,
		int nSrcWidth,
		int nSrcHeight,
		DWORD nWidth,
		DWORD nHeight,
		DWORD dwReverseFlags);
	
	HRESULT CreateImageColor(
		LPVOID lpDevice,
		DWORD dwTransparent,
		DWORD dwColor, 
		DWORD nWidth, 
		DWORD nHeight); //creates image from color

	//The following draws (and clips if necessary an image) the surface being blted to does
	//not require a clipper for clipping to take place
	HRESULT DrawClippedImage(
		LPVOID lpBuffer, 
		int x, 
		int y);

	HRESULT DrawImage(
		LPVOID lpBuffer, 
		int x, 
		int y); //draws image at specified coords to specified buffer

	HRESULT DrawPrefered(
		LPVOID lpBuffer, 
		int x, 
		int y); //clips if in windowed mode
};
#endif /* __cplusplus */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GFXG7_H__ */