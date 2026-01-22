#include <windows.h>
#include <tchar.h>
#include <gl\gl.h>
#include <gl\glu.h>
//#include <gl\glaux.h>

HWND m_hwnd=NULL;
HDC g_hdcOpenGL=NULL;
HGLRC g_hRC=NULL;

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

BOOL InitOpenGL(HWND hwnd)
{	
	PIXELFORMATDESCRIPTOR pfd;
	g_hdcOpenGL=GetDC(hwnd);
	
	memset(&pfd, 0, sizeof(pfd));
	pfd.nSize=sizeof(pfd);
	pfd.nVersion=1;
	pfd.dwFlags=PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER;
	pfd.iPixelType=PFD_TYPE_RGBA;
	pfd.cColorBits=32;
	pfd.cDepthBits=16;
	pfd.iLayerType=PFD_MAIN_PLANE;
	SetPixelFormat(g_hdcOpenGL, ChoosePixelFormat(g_hdcOpenGL, &pfd), &pfd);
	
	g_hRC=wglCreateContext(g_hdcOpenGL);
	wglMakeCurrent(g_hdcOpenGL, g_hRC);
	
	return TRUE;	
}

void ShutdownOpenGL()
{
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(g_hRC);
	ReleaseDC(m_hwnd, g_hdcOpenGL);
}

void GameLoop()
{
	int width=800, height=600;
	GLint nView[4];
	
	glClearColor(0.2f, 0.2f, 1.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer
	glLoadIdentity();
	
	//glViewport(0,0,width,height);						// Reset The Current Viewport
	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix
	
	// Calculate The Aspect Ratio Of The Window
	glGetIntegerv(GL_VIEWPORT, nView);
	gluPerspective(45.0f,(GLfloat)nView[2]/(GLfloat)nView[3],0.1f,100.0f);

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();
	
	glTranslatef(-1.5f,0.0f,-6.0f);	// Move 1.5 Left And 6.0 Into The Screen.
	glBegin(GL_TRIANGLES);		// Drawing Using Triangles
		glVertex3f( 0.0f, 1.0f, 0.0f);		// Top
		glVertex3f(-1.0f,-1.0f, 0.0f);		// Bottom Left
		glVertex3f( 1.0f,-1.0f, 0.0f);		// Bottom Right
	glEnd();					// Finished Drawing

	glTranslatef(3.0f,0.0f,0.0f);			// Move Right
	glBegin(GL_QUADS);				// Draw A Quad
		glVertex3f(-1.0f, 1.0f, 0.0f);		// Top Left
		glVertex3f( 1.0f, 1.0f, 0.0f);		// Top Right
		glVertex3f( 1.0f,-1.0f, 0.0f);		// Bottom Right
		glVertex3f(-1.0f,-1.0f, 0.0f);		// Bottom Left
	glEnd();
    
	SwapBuffers(g_hdcOpenGL);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT nMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(
	HINSTANCE hInst,
	HINSTANCE hPrevInst,
	LPSTR lpCmdLine,
	int nShowCmd)
{
	const TCHAR szAppName[]=_T("OpenGL Demo App");
	MSG msg;
	WNDCLASS wc;
	HWND hwnd;
	
	wc.cbClsExtra=0;
	wc.cbWndExtra=0;
	wc.hbrBackground=(HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.hCursor=LoadCursor(NULL, IDC_ARROW);
	wc.hIcon=LoadIcon(NULL, IDI_APPLICATION);
	wc.hInstance=hInst;
	wc.lpfnWndProc=WndProc;
	wc.lpszClassName=szAppName;
	wc.lpszMenuName=NULL;
	wc.style=CS_OWNDC;
	
	RegisterClass(&wc);
	
	hwnd=CreateWindow(
		szAppName,
		szAppName,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		hInst,
		NULL);
		
	if(!hwnd)
	{
		MessageBoxA(NULL, "This program requires Windows NT!", "OpenGLDemo", MB_OK|MB_ICONERROR);
		return 0;
	}
	ShowWindow(hwnd, nShowCmd);
	//Get the device context for OpenGL	
	InitOpenGL(hwnd);
		
	do
	{
		if(PeekMessage(&msg, NULL, 0, 0, TRUE))
		{
			if(msg.message==WM_QUIT)
				break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			GameLoop();
		}
	}while(TRUE);
	ShutdownOpenGL();
	return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	switch(nMsg)
	{
	case WM_SIZE:
		glViewport(0, 0, LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, nMsg, wParam, lParam);
	}
	return 0l;
}