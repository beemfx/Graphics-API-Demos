/*
	BaseGFXG7.exe - Example of implimentation of GFXG7

	Copyright (c) 2003, Blaine Myers
*/

#include <ddraw.h>
#include <tchar.h>
#include <windows.h>
#include <windowsx.h>

#include "gfxg7.h"

#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }

//DirectDraw globals
LPDIRECTDRAW7 g_lpDDraw=NULL;
LPDIRECTDRAWSURFACE7 g_lpPrimary=NULL;
LPDIRECTDRAWSURFACE7 g_lpBackBuffer=NULL;

//Necessary globals
BOOL bActiveApp=TRUE;
BOOL g_bWindowed=FALSE;

//Data about the interface
RECT g_rcWindow;
DWORD g_dwTransparentColor=0;
DWORD g_dwWidth=0;
DWORD g_dwHeight=0;
DWORD g_dwColorDepth=0;

//Image declarations
CImage7 TestImage;
CImage7 TestBackground;

/*
	ShutDownApp() should shut down anything that needs
	to be shut down.  In this case all DirectDraw
	objects are released.
*/
void ShutDownApp(){
	//need to release all DirectDraw objects

	//Release the test image.
	TestImage.Release();
	TestBackground.Release();

	//Make sure to release back buffer before primary.
	SAFE_RELEASE(g_lpBackBuffer);
	SAFE_RELEASE(g_lpPrimary);

	//Set cooperative level back to normal.
	g_lpDDraw->SetCooperativeLevel(NULL, DDSCL_NORMAL);
	SAFE_RELEASE(g_lpDDraw);
}


/*
	RestoreGraphics() Should be used any time the surfaces
	are lost.  Generally if the primary surface has been
	lost, all other surfaces are lost.  Note the PageFlip()
	function to see when this function is used.
*/
HRESULT RestoreGraphics(){
	
	//restore all surfaces
	g_lpDDraw->RestoreAllSurfaces();
	//optionally we could restore all surfaces separateley
	/*
	g_lpPrimary->Restore();
	g_lpBackBuffer->Restore();
	TestImage.Restore();
	*/
	
	//now reload images into surfaces
	TestImage.ReloadImageIntoSurface();
	TestBackground.ReloadImageIntoSurface();
	return S_OK;
}

/*
	RenderObjects() is where all rendering should take place.
	The first part of the code clears out the back buffer.
	After that anything that the programmer wants to be drawn
	should be specified.  All drawing should go to the back
	buffer, as drawing to the primary surface will create
	unpleasing results.
*/
HRESULT RenderObjects(){
	//Make sure the buffers exist because if they don't,
	//the blitting won't work!
	if( (!g_lpBackBuffer) || (!g_lpPrimary))return E_FAIL;

	//First thing we'll do is clear the buffer.
	DDBLTFX ddbltfx;
   ZeroMemory(&ddbltfx, sizeof(ddbltfx));
   ddbltfx.dwSize = sizeof(ddbltfx);
   ddbltfx.dwFillColor = 0x00FFFFFFL; //Fill backbuffer as white.
   g_lpBackBuffer->Blt(NULL, NULL, NULL, DDBLT_COLORFILL, &ddbltfx );



	//Now we render our test image.

	//Render test background first.
	TestBackground.DrawImage(g_lpBackBuffer, 0, 0);
	
	//The following code finds out where the mouse is in relation
	//to the window, I do this in order to display the image where
	//the mouse is.
	POINT ps;
	GetCursorPos(&ps);
	ps.x-=g_rcWindow.left+(TestImage.GetWidth()/2);
	ps.y-=g_rcWindow.top+(TestImage.GetHeight()/2);

	//Render to the back buffer, PageFlip() will copy onto the
	//Primary surface.  Most of the time the DrawPrefered()
	//method works the same as DrawClippedImage.  So either
	//can be use.  If DrawImage() is used the image will not
	//clip and it will dissapear if even part of it goes
	//past the edge of the viewport.  For that reason, either
	//DrawPrefered() or DrawClippedImage() should be used.
	TestImage.DrawPrefered(g_lpBackBuffer, ps.x, ps.y);

	return S_OK;
}

/*
	GameLoop() is a demonstration of how the game works.
	In an actual game this function would also call
	object animation functions, call a function to get
	input, etc...

	For this example the function calls RenderObjects()
	then PageFlip().
*/
HRESULT GameLoop(){

	RenderObjects();
	return PageFlip(
		g_lpPrimary, 
		g_lpBackBuffer, 
		g_bWindowed, 
		&g_rcWindow, 
		RestoreGraphics);
}



/*
	MainWndProc() is the procedures specifed for the application window
	any programmer with knowledge of windows programming should
	understand this.
*/
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	switch(uMsg)
	{
	case WM_CREATE:
		break;
	case WM_ACTIVATEAPP: 
		bActiveApp=wParam;
		break;
	case WM_KEYDOWN:
		//If escape is pressed we exit.
		if(((int)wParam==27))
			SendMessage(hwnd, WM_CLOSE, 0, 0);
		break;
	case WM_MOVE:
		//When the window is moved we need to update
		//its rectangle.  GFXG7 provides this function.
		//This will ensure that the Backbuffer is
		//properly display in windows mode.
		UpdateBounds(g_bWindowed, hwnd, &g_rcWindow);
		break;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0l;
}

/*
	WinMain() is the entry point for this application.
	In this case I first create and register a windows
	class.  Then I Initialize DirectDraw.  After that
	I demonstrate house a CDirectDrawImage (of GFXG7)
	should be created.  Then go into the main message
	loop.
*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	MSG msg;
	HWND hWnd=NULL;
	WNDCLASSEX wc;
	HACCEL hAccel=NULL;

	static TCHAR szAppName[] = TEXT("BaseGFXG7");

	wc.cbSize=sizeof(wc);
	wc.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
	wc.cbClsExtra=0;
	wc.cbWndExtra=0;
	wc.lpfnWndProc=MainWndProc;
	wc.hInstance=hInstance;
	wc.hbrBackground=(HBRUSH)GetStockObject(DKGRAY_BRUSH);
	wc.hIcon = NULL;
	wc.hIconSm = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszMenuName=NULL;
	wc.lpszClassName=szAppName;

	if(!RegisterClassEx(&wc)){
		MessageBox(
			NULL, 
			TEXT("This program requires Windows NT!"), 
			TEXT("Notice"), 
			MB_OK|MB_ICONERROR);
		return 0;
	}

	hWnd = CreateWindowEx(
		WS_EX_TOPMOST, 
		szAppName,
		szAppName,
		WS_POPUP|WS_SYSMENU|WS_VISIBLE,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		512,
		512,
		NULL,
		NULL,
		hInstance,
		NULL);

	//Initialize all interface data
	g_bWindowed=TRUE;
	g_dwTransparentColor=RGB(255, 0, 255);
	g_dwWidth=640;
	g_dwHeight=480;
	g_dwColorDepth=16;

	//Startup DirectDraw.
	//This function will initiate the DirectDraw object,
	//Create primary and secondary surfaces, convert
	//the transparent color to DirectDraw compatible
	//color, and adjust the window rect.  It also
	//gets a pointer of the DirectDraw object, and
	//transparent color for use with the CDirectDrawImage
	//class (See documentation).
	if(FAILED(InitDirectDraw(
		&g_lpDDraw, 
		&g_lpPrimary, 
		&g_lpBackBuffer,
		hWnd,
		g_bWindowed,
		g_dwWidth,
		g_dwHeight,
		g_dwColorDepth,
		&g_rcWindow,
		&g_dwTransparentColor)))return E_FAIL;

	//At this point if in FullScreen mode we will hide the cursor
	//Note that if you activate mouse support using DirectInput
	//the mouse will not be shown, and you do not need to hide
	//the cursor yourself.
	if(!g_bWindowed)ShowCursor(FALSE);

	ShowWindow(hWnd, nShowCmd);
	SetFocus(hWnd);

	AdjustWindowSize(hWnd, g_dwWidth, g_dwHeight, g_bWindowed, NULL);

	//This is the method to create an image.  Any subsequent calls
	//of this function (or any other CDirectDrawImage::CreateImageXXXXX)
	//will erase the current image and put the new image in it's place.
	//This method only supports Device Independant Bitmaps (*.DIB, *.BMP)
	//that is standard bitmaps.
	TestImage.CreateImageBM(g_lpDDraw, g_dwTransparentColor, TEXT("TestImage.bmp"), 0, 0, 100, 100, 100, 100, NULL);

	//This is another method for loading an image.
	HBITMAP hBitmap=0;
	hBitmap=(HBITMAP)LoadImage(NULL, TEXT("bg.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	TestBackground.CreateImageBMInMemory(g_lpDDraw, g_dwTransparentColor, hBitmap, 0, 0, 256, 256, 640, 480, NULL);
	DeleteObject(hBitmap);

	if(hWnd==NULL)return 0;

	while(TRUE){
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
			if(msg.message==WM_QUIT)
				break;
			TranslateMessage(&msg); 
			DispatchMessage(&msg);
		}else{
			if(bActiveApp)GameLoop();else WaitMessage();
		}
	}
	ShutDownApp();
	//If we were in FullScreen mode we now need to show the cursor.
	if(!g_bWindowed)ShowCursor(TRUE);
	return msg.wParam;

}