/*
	BaseGFXG8.exe - Example of implimentation of GFXG8

	Copyright (c) 2003, Blaine Myers
*/

#include <d3d9.h>
#include <tchar.h>
#include <windows.h>
#include <windowsx.h>
#include "gfxg8.h"
#include "resource.h"


#pragma comment(lib, "d3d9.lib")
#if defined(_DEBUG)
#pragma comment(lib, "d3dx9d.lib")
#else
#pragma comment(lib, "d3dx9.lib")
#endif

#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }

//DirectDraw globals
LPDIRECT3D9 g_lpD3D=NULL;
LPDIRECT3DDEVICE9 g_lpDevice=NULL;
LPDIRECT3DSURFACE9 g_lpBackBuffer=NULL;
D3DPRESENT_PARAMETERS g_SavedPP;

//Necessary globals
BOOL bActiveApp=TRUE;
BOOL g_bWindowed=TRUE;

//Data about the interface
DWORD g_dwWidth=0;
DWORD g_dwHeight=0;

//Image declarations
CImage8 TestImage;
CImage8 TestBackground;


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
	if(!g_lpDevice)
		return E_FAIL;

	if(!ValidateDevice(
		&g_lpDevice,
		&g_lpBackBuffer,
		g_SavedPP,
		NULL,//Release
		NULL))return S_FALSE;

	//Clear the buffer.
	g_lpDevice->Clear(
		NULL,
		NULL,
		D3DCLEAR_TARGET,
		0xFF0000FFl,
		0.0f,
		0);
	//Render test background first.
	TestBackground.DrawClippedImage(g_lpDevice, g_lpBackBuffer, 0, 0);
	
	//The following code finds out where the mouse is in relation
	//to the window, I do this in order to display the image where
	//the mouse is.
	POINT ps;
	GetCursorPos(&ps);
	ScreenToClient(g_SavedPP.hDeviceWindow, &ps);
	ps.x-=(TestImage.GetWidth()/2);
	ps.y-=(TestImage.GetHeight()/2);

	//Draw the image.
	TestImage.DrawClippedImage(g_lpDevice, g_lpBackBuffer, ps.x, ps.y);
	
	//Present the back buffer.
	g_lpDevice->Present(0, 0, 0, 0);
		
	return S_OK;
}

/*
	GameLoop() is a demonstration of how the game works.
	In an actual game this function would also call
	object animation functions, call a function to get
	input, etc...

	For this example the function calls RenderObjects().
*/
HRESULT GameLoop(){
	RenderObjects();

	return S_OK;
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

//Initialize the application.
BOOL GameInit(HWND hwnd, BOOL bWindowed)
{
	//We create Direct3D then create the device
	//and obtain the back buffer with the function:
	//InitD3D.
	g_lpD3D=Direct3DCreate9(D3D_SDK_VERSION);

	if(!g_lpD3D)
		return FALSE;

	//Initialize all interface data
	g_bWindowed=bWindowed;
	g_dwWidth=640;
	g_dwHeight=480;

	if(FAILED(InitD3D(
		hwnd,
		g_dwWidth,
		g_dwHeight,
		g_bWindowed,
		D3DFMT_X8R8G8B8,
		g_lpD3D,
		&g_lpDevice,
		&g_SavedPP,
		&g_lpBackBuffer))){
		SAFE_RELEASE(g_lpD3D);
		return FALSE;
	}

	//Correct the window size.
	CorrectWindowSize(
		hwnd,
		g_dwWidth,
		g_dwHeight,
		g_bWindowed,
		NULL);

	//At this point if in FullScreen mode we will hide the cursor
	//Note that if you activate mouse support using DirectInput
	//the mouse will not be shown, and you do not need to hide
	//the cursor yourself.
	if(!g_bWindowed)ShowCursor(FALSE);

	//This is the method to create an image.  Any subsequent calls
	//of this function (or any other CDirectDrawImage::CreateImageXXXXX)
	//will erase the current image and put the new image in it's place.
	//This method only supports Direct3D compatible imag types.
	TestBackground.CreateImageBM(
		TEXT("bg.bmp"),
		g_lpDevice,
		g_SavedPP.BackBufferFormat,
		0,
		0,
		256,
		256,
		g_dwWidth,
		g_dwHeight);

	TestImage.CreateImageBM(
		TEXT("TestImage.bmp"),
		g_lpDevice,
		g_SavedPP.BackBufferFormat, 
		0, 
		0, 
		100, 
		100, 
		100, 
		100);

	return TRUE;
}

BOOL GameShutdown()
{
	//need to release all Direct3D objects and the images.
	TestImage.Release();
	TestBackground.Release();

	SAFE_RELEASE(g_lpBackBuffer);
	SAFE_RELEASE(g_lpDevice);
	SAFE_RELEASE(g_lpD3D);

	return TRUE;
}

/*
	WinMain() is the entry point for this application.
	In this case I first create and register a windows
	class.  Then I Initialize DirectDraw.  After that
	I demonstrate house a CDirectDrawImage (of GFXG7)
	should be created.  Then go into the main message
	loop.
*/
int WINAPI WinMain(
	HINSTANCE hInstance, 
	HINSTANCE hPrevInstance, 
	LPSTR lpCmdLine, 
	int nShowCmd)
{
	MSG msg;
	HWND hWnd=NULL;
	WNDCLASSEX wc;
	HACCEL hAccel=NULL;

	static TCHAR szAppName[] = TEXT("GFXG8 Demo");

	wc.cbSize=sizeof(wc);
	wc.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
	wc.cbClsExtra=0;
	wc.cbWndExtra=0;
	wc.lpfnWndProc=MainWndProc;
	wc.hInstance=hInstance;
	wc.hbrBackground=(HBRUSH)GetStockObject(DKGRAY_BRUSH);
	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
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

	if(hWnd==NULL)
		return 0;

	ShowWindow(hWnd, nShowCmd);
	SetFocus(hWnd);

	GameInit(hWnd, TRUE);


	while(TRUE){
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
			if(msg.message==WM_QUIT)
				break;

			TranslateMessage(&msg); 
			DispatchMessage(&msg);
		}else{
			if(bActiveApp)
				GameLoop();
			else 
				WaitMessage();
		}
	}
	GameShutdown();
	return msg.wParam;


}