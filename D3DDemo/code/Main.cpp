// (c) Beem Media. All rights reserved.

#include "D3D_MD3PlayerMesh.h"
#include "D3D_MD3PlayerObject.h"
#include "D3D_MD3WeaponMesh.h"
#include "FileSystem/FileSystem.h"
#include "GFX3D9.h"
#include "resource.h"
#include <d3d9.h>
#include <d3dx9.h>
#include <stdio.h>
#include <tchar.h>
#include <windows.h>

LPDIRECT3DDEVICE9 g_lpDevice=NULL;
D3DPRESENT_PARAMETERS g_SavedPP;

#define WM_USER_ACTIVEAPP (WM_USER+1)

HBEEMCONSOLE BeemConsole=NULL;

#define TESTLIGHT

//#define TESTPRIM
#ifdef TESTPRIM
LPDIRECT3DTEXTURE9 g_lpTestPrimTexture=NULL;
IDirect3DTexture9* g_lpTestPrimLM=NULL;
LPDIRECT3DVERTEXBUFFER9 g_lpTestPrimVB=NULL;
#endif //TESPRIM

#define TESTMD3
#ifdef TESTMD3
CD3D_MD3PlayerMesh g_lpTestHuman;
CD3D_MD3PlayerObject g_lpTestPlayer;

CD3D_MD3WeaponMesh g_lpTestGun;
#endif //TESTMD3

// #define TESTME
#ifdef TESTME
CD3D_MD3ObjectMesh g_lpMeTest;
#endif TESTME

#define TESTFONT
#ifdef TESTFONT
HD3DFONT TestFont=NULL;
#endif //TESTFONT

#define TESTIMAGE
#ifdef TESTIMAGE
HD3DIMAGE TestImage=NULL;
HD3DIMAGE TestBG=NULL;
#endif //TESTIMAGE


typedef unsigned __int64 i64;

static BOOL SyncTime( float* FrameTime )
{
	i64 CurrentTime = 0;

	static i64 LastUpdate = 0;
	static i64 Frequency = 0;

	QueryPerformanceCounter( reinterpret_cast<LARGE_INTEGER*>( &CurrentTime ) );
	if( 0 == Frequency )
	{
		QueryPerformanceFrequency( reinterpret_cast<LARGE_INTEGER*>( &Frequency ) );
		LastUpdate = CurrentTime;
		
	}

	i64 Elapsed = CurrentTime - LastUpdate;
	double ElapsedSeconds = static_cast<double>(Elapsed)/Frequency;
	*FrameTime = static_cast<float>(ElapsedSeconds);
	if( *FrameTime > 1/120.f )
	{
		LastUpdate = CurrentTime;
		return TRUE;
	}

	return FALSE;
}


BOOL ConsoleParse(LPSTR szCommand, LPSTR szParams, HBEEMCONSOLE hConsole)
{
	char szOutputMessage[100];

	BEGINCHOICE
	#ifdef TESTMD3
	CHOICE(LOADWEAPON)
		char szFilename[100];
		
		if(!BeemParseGetParam(szFilename, szParams, 1)){
			SendBeemConsoleMessage(hConsole, "Usage: LOADWEAPON <string>");
		}else{
			if(!BeemParseGetString(szFilename, szFilename)){
				SendBeemConsoleMessage(hConsole, "Usage: LOADWEAPON <string>");
			}else{
				if(SUCCEEDED(g_lpTestGun.Load(g_lpDevice, szFilename, d3d_md3_detail::High))){
					g_lpTestPlayer.SetWeapon(&g_lpTestGun);
					sprintf(szOutputMessage, "Successfully loaded \"%s\" weapon.", szFilename);
					SendBeemConsoleMessage(hConsole, szOutputMessage);
				}else
					SendBeemConsoleMessage(hConsole, "Could not load MD3 weapon file!");
			}
		}


	CHOICE(LOADMD3)

		char szFilename[100];

		if(!BeemParseGetParam(szFilename, szParams, 1))
		{
			SendBeemConsoleMessage(hConsole, "Usage: LOADMD3 <string>");
		}
		else
		{
			if(!BeemParseGetString(szFilename, szFilename))
			{
				SendBeemConsoleMessage(hConsole, "Usage: LOADMD3 <string>");
			}
			else
			{
				if(g_lpTestHuman.Load(g_lpDevice, szFilename, d3d_md3_detail::High))
				{
					g_lpTestPlayer.SetPlayerMesh(&g_lpTestHuman);
					g_lpTestPlayer.SetAnimation(TORSO_STAND, MD3SETANIM_WAIT, 2.0f);
					g_lpTestPlayer.SetAnimation(LEGS_IDLE, MD3SETANIM_WAIT, 2.0f);
					sprintf(szOutputMessage, "Successfully loaded \"%s\".", szFilename);
					SendBeemConsoleMessage(hConsole, szOutputMessage);
				}
				else
				{
					SendBeemConsoleMessage(hConsole, "Could not load MD3 file!");
				}
			}
		}
			

	CHOICE(SETSKIN)

		char szSkinName[100];

		if(BeemParseGetParam(szSkinName, szParams, 1)){

				if(!BeemParseGetString(szSkinName, szSkinName)){
					SendBeemConsoleMessage(hConsole, "Usage: SETSKIN <string>");
				}else{

				if(SUCCEEDED(g_lpTestPlayer.SetSkinByName(szSkinName))){
					sprintf(szOutputMessage, "Successfully set skin to \"%s\".", szSkinName);
					SendBeemConsoleMessage(hConsole, szOutputMessage);
				}else{
					SendBeemConsoleMessage(hConsole, "No such skin exists for this model!");
				}
			}
		}else{
			SendBeemConsoleMessage(hConsole, "Usage: SETSKIN <string>");
		}
	#endif //TESTMD3
	CHOICE(QUIT)

		SendBeemConsoleMessage(hConsole, "Quiting...");
		SendMessage(g_SavedPP.hDeviceWindow, WM_CLOSE, 0, 0);

	CHOICE(CLS)

		BeemConsoleClearEntries(hConsole);

	CHOICE(ADD)

		LONG lValue[3];

		if(BeemParseGetInt(&lValue[0], szParams, 1)){
			if(BeemParseGetInt(&lValue[1], szParams, 2)){
				lValue[2]=lValue[0]+lValue[1];
				sprintf(szOutputMessage, "%i", lValue[2]);
				SendBeemConsoleMessage(hConsole, szOutputMessage);
			}else{
				SendBeemConsoleMessage(hConsole, "Usage: ADD <value> <value>");
			}
		}else{
			SendBeemConsoleMessage(hConsole, "Usage: ADD <value> <value>");
		}

	CHOICE(GETTEXMEM)
		
		sprintf(szOutputMessage, "%iMB Texture Memory", g_lpDevice->GetAvailableTextureMem()/1024/1024);
		SendBeemConsoleMessage(hConsole, szOutputMessage);

	CHOICE(CMDLIST)

		SendBeemConsoleMessage(hConsole, "ADD");
		SendBeemConsoleMessage(hConsole, "QUIT");
		SendBeemConsoleMessage(hConsole, "LOADMD3");
		SendBeemConsoleMessage(hConsole, "SETSKIN");
		SendBeemConsoleMessage(hConsole, "CLS");
		SendBeemConsoleMessage(hConsole, "LOADWEAPON");
		SendBeemConsoleMessage(hConsole, "GETTEXMEM");

	INVALIDCHOICE

		sprintf(szOutputMessage, "Unknown command: \"%s\"", szCommand);
		SendBeemConsoleMessage(hConsole, szOutputMessage);

	ENDCHOICE

	return TRUE;
}

#ifdef TESTMD3

BOOL CreateAnimationString(char szAnimationString[100], CD3D_MD3PlayerObject * lpObject)
{
	char szUpper[MAX_QPATH];
	char szLower[MAX_QPATH];
	DWORD dwLower=0, dwUpper=0;
	DWORD dwBoth=0;
	BOOL bBoth=FALSE;

	static DWORD dwLastUpper=0, dwLastLower=0;
	static char szLastString[100];

	lpObject->GetAnimation(&dwUpper, &dwLower);

	if(dwLastUpper==dwUpper && dwLastLower==dwLower){
		strcpy(szAnimationString, szLastString);
		return TRUE;
	}

	if(dwUpper==dwLower){
		bBoth=TRUE;
		dwBoth=dwUpper;
	}

	if(!bBoth){
	switch(dwUpper)
	{
	case TORSO_GESTURE:
		strcpy(szUpper, "TORSO_GESTURE");
		break;
	case TORSO_ATTACK:
		strcpy(szUpper, "TORSO_ATTACK");
		break;
	case TORSO_ATTACK2:
		strcpy(szUpper, "TORSO_ATTACK2");
		break;
	case TORSO_DROP:
		strcpy(szUpper, "TORSO_DROP");
		break;
	case TORSO_RAISE:
		strcpy(szUpper, "TORSO_RAISE");
		break;
	case TORSO_STAND:
		strcpy(szUpper, "TORSO_STAND");
		break;
	case TORSO_STAND2:
		strcpy(szUpper, "TORSO_STAND2");
		break;
	default:
		strcpy(szUpper, "Invalid Torso Animation");
		break;
	};

	switch(dwLower)
	{
	case LEGS_WALKCR:
		strcpy(szLower, "LEGS_WALKCR");
		break;
	case LEGS_WALK:
		strcpy(szLower, "LEGS_WALK");
		break;
	case LEGS_RUN:
		strcpy(szLower, "LEGS_RUN");
		break;
	case LEGS_BACK:
		strcpy(szLower, "LEGS_BACK");
		break;
	case LEGS_SWIM:
		strcpy(szLower, "LEGS_SWIM");
		break;
	case LEGS_JUMP:
		strcpy(szLower, "LEGS_JUMP");
		break;
	case LEGS_LAND:
		strcpy(szLower, "LEGS_LAND");
		break;
	case LEGS_JUMPB:
		strcpy(szLower, "LEGS_JUMPB");
		break;
	case LEGS_LANDB:
		strcpy(szLower, "LEGS_LANDB");
		break;
	case LEGS_IDLE:
		strcpy(szLower, "LEGS_IDLE");
		break;
	case LEGS_IDLECR:
		strcpy(szLower, "LEGS_IDLECR");
		break;
	case LEGS_TURN:
		strcpy(szLower, "LEGS_TURN");
		break;
	default:
		strcpy(szLower, "Invalid Legs Animation");
		break;
	};
	}else{
	switch(dwBoth)
	{
	case BOTH_DEATH1:
		strcpy(szUpper, "BOTH_DEATH1");
		strcpy(szLower, "BOTH_DEATH1");
		break;
	case BOTH_DEAD1:
		strcpy(szUpper, "BOTH_DEAD1");
		strcpy(szLower, "BOTH_DEAD1");
		break;
	case BOTH_DEATH2:
		strcpy(szUpper, "BOTH_DEATH2");
		strcpy(szLower, "BOTH_DEATH2");
		break;
	case BOTH_DEAD2:
		strcpy(szUpper, "BOTH_DEAD2");
		strcpy(szLower, "BOTH_DEAD2");
		break;
	case BOTH_DEATH3:
		strcpy(szUpper, "BOTH_DEATH3");
		strcpy(szLower, "BOTH_DEATH3");
		break;
	case BOTH_DEAD3:
		strcpy(szUpper, "BOTH_DEAD3");
		strcpy(szLower, "BOTH_DEAD3");
		break;
	};
	}

	dwLastUpper=dwUpper;
	dwLastLower=dwLower;

	sprintf(szAnimationString, "Torso: %s Legs: %s", szUpper, szLower);
	strcpy(szLastString, szAnimationString);

	return TRUE;
}

BOOL CycleAnimation(DWORD dwBone, CD3D_MD3PlayerObject * lpObject)
{
	DWORD dwUpper=0, dwLower=0;
	DWORD dwCycleType=MD3SETANIM_FRAME;
	BOOL bWasBoth=FALSE;
	lpObject->GetAnimation(&dwUpper, &dwLower);
	if(dwUpper==dwLower)
		bWasBoth=TRUE;

	switch(dwBone)
	{
	case 1: //Upper
		if(bWasBoth){
			dwUpper=TORSO_STAND;
			dwLower=LEGS_IDLE;
			lpObject->SetAnimation(dwUpper, dwCycleType, 2.0f);
			lpObject->SetAnimation(dwLower, dwCycleType, 2.0f);
		}else{
			dwUpper++;
			if(dwUpper > TORSO_STAND2)
				dwUpper=TORSO_GESTURE;
			lpObject->SetAnimation(dwUpper, dwCycleType, 2.0f);
		}
		
		return TRUE;
	case 2: //Legs
		if(bWasBoth){
			dwUpper=TORSO_STAND;
			dwLower=LEGS_IDLE;
			lpObject->SetAnimation(dwUpper, dwCycleType, 2.0f);
			lpObject->SetAnimation(dwLower, dwCycleType, 2.0f);
		}else{
			dwLower++;
			if(dwLower > LEGS_TURN)
				dwLower=LEGS_WALKCR;
			lpObject->SetAnimation(dwLower, dwCycleType, 2.0f);
		}

		return TRUE;
	case 3: //Both
		if(!bWasBoth){
			dwUpper=BOTH_DEATH1;
			dwLower=BOTH_DEATH1;
			lpObject->SetAnimation(dwUpper, dwCycleType, 1.0f);
		}else{
			dwUpper++;
			dwLower++;
			if(dwUpper > BOTH_DEAD3){
				dwUpper=BOTH_DEATH1;
				dwLower=BOTH_DEATH1;
			}
			lpObject->SetAnimation(dwUpper, dwCycleType, 1.0f);
		}
		
		return TRUE;
	default:
		return FALSE;
	};
}
#endif //TESTMD3




/* ReleasePoolResource() should release any resource in the default pool. This
	function is called during device validation. */
BOOL InvalidateDeviceResources()
{
	/* Vertex Buffers need to be released. */
	#ifdef TESTPRIM
	SAFE_RELEASE(g_lpTestPrimVB);
	#endif //TESTPRIM
	#ifdef TESTMD3
	g_lpTestHuman.Invalidate();
	g_lpTestGun.Invalidate();
	#endif //TESTMD3

	#ifdef TESTME
	g_lpMeTest.Invalidate();
	#endif //TESTME

	#ifdef TESTIMAGE
	InvalidateD3DImage(TestImage);
	InvalidateD3DImage(TestBG);
	#endif //TESTIMAGE

	#ifdef TESTFONT
	InvalidateD3DFont(TestFont);
	#endif //TESTFONT

	InvalidateBeemConsole(BeemConsole);

	return TRUE;
}


/* RestoreDeviceResources() should reload and reinitialize any resources 
	released in ReleaseDeviceResources.  Addittionally it can be used
	to create resources during app initialization. */
BOOL RestoreDeviceResources()
{
	/* During creation or device validation,
		the viewport, projection matix, view matrix, and other D3D states 
		need to be set. */
	SendBeemConsoleMessage(BeemConsole, "Validating Device Resources...");
	SetViewPort(g_lpDevice, g_SavedPP.BackBufferWidth, g_SavedPP.BackBufferHeight);
	SetProjectionMatrix(g_lpDevice, g_SavedPP.BackBufferWidth, g_SavedPP.BackBufferHeight, 1.0f, 1000.0f);
	D3DXMATRIX ViewMatrix;
	//D3DXMatrixLookAtLH(&ViewMatrix, &D3DXVECTOR3(0.0f, 0.0f, -100.0f), &D3DXVECTOR3(0.0f, 0.0f, 0.0f), &D3DXVECTOR3(0.0f, 1.0f, 0.0f));
	D3DXMatrixIdentity(&ViewMatrix);
	ViewMatrix._43=100.0f;
	#ifdef TESTME
	ViewMatrix._43=250.0f;
	#endif //TESTME
	g_lpDevice->SetTransform(D3DTS_VIEW, &ViewMatrix);

	ValidateBeemConsole(BeemConsole);

	SetTextureFilter(g_lpDevice, 0, D3DFILTER_TRILINEAR);
	SetTextureFilter(g_lpDevice, 1, D3DFILTER_TRILINEAR);
	g_lpDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	g_lpDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

	#ifdef TESTFONT
	ValidateD3DFont(TestFont);
	#endif //TESTFONT
	
	#ifdef TESTLIGHT
	/*
	g_lpDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
	D3DLIGHT9 TestLight;
	ZeroMemory(&TestLight, sizeof(D3DLIGHT9));
	TestLight.Type=D3DLIGHT_DIRECTIONAL;

   TestLight.Diffuse.r=1.0f;
	TestLight.Diffuse.g=1.0f; 
	TestLight.Diffuse.b=1.0f; 
	TestLight.Diffuse.a=1.0f; 

   TestLight.Specular.r=0.0f;
	TestLight.Specular.g=0.0f;
	TestLight.Specular.b=0.0f;
	TestLight.Specular.a=0.0f;

   TestLight.Ambient.r=0.0f;
	TestLight.Ambient.g=0.0f;
	TestLight.Ambient.b=0.0f;
	TestLight.Ambient.a=0.0f;

   TestLight.Position.x=0.0f;
	TestLight.Position.y=0.0f;
	TestLight.Position.z=0.0f;
	
   TestLight.Direction.x=0.0f;
	TestLight.Direction.y=0.0f;
	TestLight.Direction.z=1.0f;

   TestLight.Range=100.0f;           
   TestLight.Falloff=1.0f;  
	        
   TestLight.Attenuation0=0.0f;     
   TestLight.Attenuation1=1.0f;    
   TestLight.Attenuation2=0.0f;

   TestLight.Theta=0.0f; 
   TestLight.Phi=0.0f;

	g_lpDevice->SetLight(0, &TestLight);
	g_lpDevice->LightEnable(0, TRUE);
	

	//g_lpDevice->SetRenderState( D3DRS_AMBIENT, 0xff404040l);
	
	D3DMATERIAL9 mtrl;
	ZeroMemory( &mtrl, sizeof(mtrl) );
	mtrl.Diffuse.r = mtrl.Ambient.r = 1.0f;
	mtrl.Diffuse.g = mtrl.Ambient.g = 1.0f;
	mtrl.Diffuse.b = mtrl.Ambient.b = 1.0f;
	mtrl.Diffuse.a = mtrl.Ambient.a = 1.0f;
	g_lpDevice->SetMaterial( &mtrl );
	*/
	D3DLIGHT9 Light={
		D3DLIGHT_DIRECTIONAL, //Type
		{1.0f, 1.0f, 1.0f, 0.0f},//Diffuse
		{0.0f, 0.0f, 0.0f, 0.0f},//Specular
		{0.0f, 0.0f, 0.0f, 0.0f},//Ambient
		{0.0f, 0.0f, 0.0f},//Position
		{0.0f, 0.0f, 1.0f},//Direction
		0.0f,//Range
		0.0f,//Falloff
		0.0f,//Attenuation0
		0.0f,//Attenuation1
		0.0f,//Attenuation2
		0.0f,//Theta
		0.0f};//Phi
	
	g_lpDevice->SetLight(0, &Light);
	g_lpDevice->LightEnable(0, TRUE);
	D3DMATERIAL9 Material={
	{1.0f, 1.0f, 1.0f, 1.0f},//Diffuse
	{1.0f, 1.0f, 1.0f, 1.0f},//Ambient
	{1.0f, 1.0f, 1.0f, 1.0f},//Specular
	{0.0f, 0.0f, 0.0f, 0.0f},//Emissive
	0.0f}; //Power
	
	g_lpDevice->SetMaterial(&Material);
	
	#endif TESTLIGHT
	

	#ifdef TESTPRIM
	/* All vertices currently in existence should be (re)created. */
	CUSTOMVERTEX vTestPrim[4];

	vTestPrim[0]=CUSTOMVERTEX(
		-40.0f, -40.0f, 30.0f, //Position
		-40.0f, -40.0f, 20.0f,  //Normal
		0xFFFFFFFF,//0xFFFFFFFF, //Diffuse color
		0XFFFFFFFF, //Specular color
		1.0f, 1.0f); //Texture coordinates

	vTestPrim[1]=CUSTOMVERTEX(
		-40.0f, 40.0f, 30.0f,
		-40.0f, 40.0f, 20.0f,
		0xFFFFFFFF,
		0xFFFFFFFF,
		1.0f, 0.0f);

	vTestPrim[2]=CUSTOMVERTEX(
		40.0f, 40.0f, 30.0f,
		40.0f, 40.0f, 20.0f,
		0xFFFFFFFF,
		0xFFFFFFFF,
		0.0f, 0.0f);

	vTestPrim[3]=CUSTOMVERTEX(
		40.0f, -40.0f, 30.0f,
		40.0f, -40.0f, 20.0f,
		0xFFFFFFFF,
		0xFFFFFFFF,
		0.0f,
		1.0f);
	/*
	vTestPrim[0]=CUSTOMVERTEX(
		-40.0f, -40.0f, 30.0f, //Position
		-40.0f, -40.0f, 20.0f,  //Normal
		0xFFFFFFFF,//0xFFFFFFFF, //Diffuse color
		0XFFFFFFFF, //Specular color
		0.0f, 1.0f); //Texture coordinates

	vTestPrim[1]=CUSTOMVERTEX(
		0.0f, 40.0f, 30.0f,
		0.0f, 40.0f, 20.0f,
		0xFFFFFFFF,
		0xFFFFFFFF,
		0.5f, 0.0f);

	vTestPrim[2]=CUSTOMVERTEX(
		40.0f, -40.0f, 30.0f,
		40.0f, -40.0f, 20.0f,
		0xFFFFFFFF,
		0xFFFFFFFF,
		1.0f,
		1.0f);
	*/
	g_lpDevice->CreateVertexBuffer(
		4*sizeof(CUSTOMVERTEX),
		0,
		CUSTOMVERTEX_TYPE,
		D3DPOOL_DEFAULT,
		&g_lpTestPrimVB,
		NULL);

	PVOID pVertices = 0;
	
	if(FAILED(g_lpTestPrimVB->Lock(0, sizeof(CUSTOMVERTEX)*4, &pVertices, 0))){
		return E_FAIL;
	}
	memcpy(pVertices, &vTestPrim, sizeof(CUSTOMVERTEX)*4);
	
	g_lpTestPrimVB->Unlock();

	#endif //TESTPRIM

	#ifdef TESTIMAGE
	ValidateD3DImage(TestImage);
	ValidateD3DImage(TestBG);
	#endif //TESTIMAGE

	#ifdef TESTMD3
	g_lpTestHuman.Validate();
	g_lpTestGun.Validate();
	#endif //TESTMD3
	#ifdef TESTME
	g_lpMeTest.Validate(g_lpDevice);
	#endif //TESTME
	SendBeemConsoleMessage(BeemConsole, "Device Resources Validated.");
	return TRUE;
}




BOOL GameInit(HWND hwnd, BOOL bWindowed, HINSTANCE hInstance)
{
	LPDIRECT3D9 lpD3D=NULL;
	DWORD dwDeviceWidth=0, dwDeviceHeight=0;
	BOOL bResult=TRUE;
	char szTemp[200];
	/* We start by creating Direct3D */
	
	lpD3D=Direct3DCreate9(D3D_SDK_VERSION);

	if(!lpD3D)
	{
		MessageBox(hwnd, "This requires DirectX 9.0c", "D3DDemo", MB_OK);
		return FALSE;
	}
	
	/*
	CoInitialize(NULL);
	CoCreateInstance(CLSID_Direct3D9, NULL, CLSCTX_ALL, IID_IDirect3D9, &g_lpD3D);
	*/
	if(lpD3D==NULL)
		return FALSE;

	if(bWindowed){
		dwDeviceWidth=800;
		dwDeviceHeight=600;
	}else{
		dwDeviceWidth=1024;
		dwDeviceHeight=768;
	}

	/* We Initialize the Direct3D Device */
	if(!InitD3D(
		hwnd,
		dwDeviceWidth,
		dwDeviceHeight,
		bWindowed,
		FALSE,
		//D3DFMT_R5G6B5, 
		D3DFMT_X8R8G8B8,
		lpD3D,
		&g_lpDevice,
		NULL,
		&g_SavedPP))
	{
		lpD3D->Release();
		return FALSE;
	}
	
	lpD3D->Release();

	CorrectWindowSize(hwnd, dwDeviceWidth, dwDeviceHeight, bWindowed, NULL);


	g_lpDevice->Clear(
		0,
		0, 
		D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 
		D3DCOLOR_XRGB(0, 0, 200), 
		1.0f, 0);

	//#define CSYSTEMFONT
	#ifdef CSYSTEMFONT
	HD3DFONT Font=NULL;
	Font=CreateD3DFontFromFont(
		g_lpDevice,
		(HFONT)GetStockObject(SYSTEM_FONT),
		0xFFFFFF00);
	BeemConsole=CreateBeemConsole(g_lpDevice, Font, NULL, ConsoleParse);
	DeleteD3DFont(Font);
	#else //CSYSTEMFONT
	BeemConsole=CreateBeemConsole(g_lpDevice, NULL, NULL, ConsoleParse);
	#endif //CSYSTEMFONT
	SendBeemConsoleMessage(BeemConsole, "Direct3DCreate9()");
	SendBeemConsoleMessage(BeemConsole, "InitD3D()");
	sprintf(szTemp, "Display Mode Set To: %ix%i", dwDeviceWidth, dwDeviceHeight);
	SendBeemConsoleMessage(BeemConsole, szTemp);
	SendBeemConsoleMessage(BeemConsole, "CorrectWindowSize()");
	SendBeemConsoleMessage(BeemConsole, "IDirect3DDevice9::Clear()");
	SendBeemConsoleMessage(BeemConsole, "CreateBeemConsole()");

	#ifdef TESTMD3
	SendBeemConsoleCommand(BeemConsole, "loadmd3 \"laracroft\"");
	SendBeemConsoleCommand(BeemConsole, "loadweapon \"machinegun\"");
	#endif //TESTMD3

	#ifdef TESTME
	g_lpMeTest.Load(g_lpDevice, "me\\me_md3", DETAIL_HIGH);
	#endif //TESTME

	#ifdef TESTPRIM
	/* Load the texture from the file. */
	D3DXCreateTextureFromFile(g_lpDevice, "test\\testtex.jpg", &g_lpTestPrimTexture);
	D3DXCreateTextureFromFile(g_lpDevice, "test\\testtexlm.jpg", &g_lpTestPrimLM);
	SendBeemConsoleMessage(BeemConsole, "D3DXCreateTextureFromFile(): texture.bmp");
	#endif //TESTPRIM

	#ifdef TESTIMAGE
	TestImage=CreateD3DImageFromFile(g_lpDevice, "TestImage2.bmp", 100, 100, 0xFFFF00FF);
	SendBeemConsoleMessage(BeemConsole, "CreateD3DImageFromeFile(): TestImage2.bmp");
	TestBG=CreateD3DImageFromFile(g_lpDevice, "bg.bmp", 256, 256, 0xFFFF00FF);
	SendBeemConsoleMessage(BeemConsole, "CreateD3DImageFromFile(): bg.bmp");
	#endif //TESTIMAGE

	#ifdef TESTFONT
	TestFont=GetD3DFontDefault(g_lpDevice);
	SendBeemConsoleMessage(BeemConsole, "GetD3DFontDefault()");
	#endif //TESTFONT

	RestoreDeviceResources();
	SendBeemConsoleMessage(BeemConsole, "---");

	return TRUE;
}
void GameShutdown()
{
	/* Any COM interfaces that were created
	   should be released.
	*/
	LPDIRECT3D9 lpD3D=NULL;
	SendBeemConsoleMessage(BeemConsole, "Shutting Down Game...");
	g_lpDevice->GetDirect3D(&lpD3D);

	#ifdef TESTPRIM
	SAFE_RELEASE(g_lpTestPrimVB);
	SAFE_RELEASE(g_lpTestPrimTexture);
	SAFE_RELEASE(g_lpTestPrimLM);
	#endif //TESTPRIM

	#ifdef TESTMD3
	g_lpTestHuman.Clear();
	g_lpTestGun.Clear();
	//CMD3SkinFile::ClearTexDB();
	#endif //TESTMD3

	#ifdef TESTME
	g_lpMeTest.Clear();
	#endif //TESTME

	#ifdef TESTIMAGE
	DeleteD3DImage(TestImage);
	DeleteD3DImage(TestBG);
	#endif //TESTIMAGE

	#ifdef TESTFONT
	DeleteD3DFont(TestFont);
	#endif //TESTFONT

	DeleteBeemConsole(BeemConsole);

	//#define DEBUG_D3DSHUTDOWN
	#ifdef DEBUG_D3DSHUTDOWN
	LONG lDeviceLeft=0;
	char szDeviceLeft[100];
	lDeviceLeft=g_lpDevice->Release();
	
	sprintf(szDeviceLeft, "%i devices left", lDeviceLeft);
	MessageBox(0, szDeviceLeft, 0, 0);
	lDeviceLeft=lpD3D->Release();
	sprintf(szDeviceLeft, "%i D3D's left", lDeviceLeft);
	MessageBox(0, szDeviceLeft, 0, 0);
	#else //DEBUG_D3DSHUTDOWN
	SAFE_RELEASE(g_lpDevice);
	SAFE_RELEASE(lpD3D);
	#endif //DEBUG_D3DSHUTDOWN

	//CoUninitialize();
}


BOOL Render()
{
	float DeltaTime = 0;
	BOOL Continue = SyncTime( &DeltaTime );

	if( !Continue )
	{
		return TRUE;
	}

	if(!ValidateDevice(
		&g_lpDevice, 
		NULL,//&g_lpBackSurface, 
		g_SavedPP,
		InvalidateDeviceResources,
		RestoreDeviceResources))return FALSE;

	if(!g_lpDevice)return FALSE;

	/* Clear the buffer */
	
	g_lpDevice->Clear(
		0, 
		0, 
		D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 
		D3DCOLOR_XRGB(0, 0, 255), 
		1.0f, 
		0);

	g_lpDevice->BeginScene();
	//All 3D Rendering goes here.

	#ifdef TESTIMAGE
	//RenderD3DImage(&TestBG, 0, 0, g_lpDevice);
	RenderD3DImageRelativeEx(TestBG, 0.0f, 0.0f, 100.0f, 100.0f, 0, 0, GetD3DImageWidth(TestBG), GetD3DImageHeight(TestBG));
	#endif //TESTIMAGE

	#define TESTROTATE
	#ifdef TESTROTATE
	/* The following rotates the world around, just to show that the enviro is 3D */

	static float Seconds = 0.f;
	Seconds += DeltaTime;
	if( Seconds > 5.f )
	{
		Seconds = 0.f;
	}
	D3DXMATRIX WorldMatrix;
	D3DXMatrixRotationY(&WorldMatrix, (Seconds/5.f)*D3DX_PI*2.f);

	g_lpDevice->SetTransform(D3DTS_WORLD, &WorldMatrix);
	#endif //TESTROTATE
	#ifdef TESTPRIM

	//D3DXMatrixIdentity(&WorldMatrix);
	g_lpDevice->SetVertexShader(NULL);
	g_lpDevice->SetFVF(CUSTOMVERTEX_TYPE);
	#define MULTIPASS
	#ifdef MULTIPASS
	g_lpDevice->SetTexture(0, g_lpTestPrimTexture);
	g_lpDevice->SetStreamSource(0, g_lpTestPrimVB, 0, sizeof(CUSTOMVERTEX));
	g_lpDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	g_lpDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	g_lpDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
	g_lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	g_lpDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
	g_lpDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
	g_lpDevice->SetTexture(0, g_lpTestPrimLM);
	g_lpDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
	g_lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	g_lpDevice->SetTexture(0, NULL);
	#else //MULTIPASS
	g_lpDevice->SetStreamSource(0, g_lpTestPrimVB, 0, sizeof(CUSTOMVERTEX));
	g_lpDevice->SetTexture(0, g_lpTestPrimTexture);
	g_lpDevice->SetTexture(1, g_lpTestPrimLM);
	g_lpDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
	g_lpDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	g_lpDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	g_lpDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0);
	g_lpDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	g_lpDevice->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
	g_lpDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
	g_lpDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
	g_lpDevice->SetTexture(1, NULL);
	g_lpDevice->SetTexture(0, NULL);
	#endif //MULTIPASS
	#endif
	#ifdef TESTMD3
	g_lpTestPlayer.Render( WorldMatrix );
	g_lpDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	#ifdef TESTFONT
	char szAnimationString[100];
	CreateAnimationString(szAnimationString, &g_lpTestPlayer);
	RenderD3DFontString(TestFont, 0, 0, szAnimationString);
	#endif //TESTFONT
	#endif //TESTMD3

	#ifdef TESTME
	g_lpMeTest.Render(g_lpDevice);
	#endif //TESTME

	#ifdef TESTIMAGE
	POINT ps;
	GetCursorPos(&ps);
	ScreenToClient(g_SavedPP.hDeviceWindow, &ps);
	ps.x-=(GetD3DImageWidth(TestImage)/2);
	ps.y-=(GetD3DImageHeight(TestImage)/2);

	RenderD3DImage(TestImage, ps.x, ps.y);
	#endif //TESTIMAGE

	RenderBeemConsole(BeemConsole);

	g_lpDevice->EndScene();

	//Present the BackBuffer
	g_lpDevice->Present(NULL, NULL, NULL, NULL);


	return TRUE;
}

BOOL GameLoop()
{
	Render();

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_CREATE:
		 break;
	case WM_ACTIVATEAPP: 
		if(wParam){
			PostMessage(hwnd, WM_USER_ACTIVEAPP, TRUE, 0);
		}else{
			PostMessage(hwnd, WM_USER_ACTIVEAPP, FALSE, 0);
		}
		break;
	case WM_LBUTTONDOWN:
		#ifdef TESTMD3
		CycleAnimation(1, &g_lpTestPlayer);
		#endif //TESTMD3
		break;
	case WM_RBUTTONDOWN:
		#ifdef TESTMD3
		CycleAnimation(2, &g_lpTestPlayer);
		#endif //TESTMD3
		break;
	case WM_MBUTTONDOWN:
		#ifdef TESTMD3
		CycleAnimation(3, &g_lpTestPlayer);
		#endif
		break;
	case WM_CHAR:
		BeemConsoleOnChar(BeemConsole, static_cast<char>(wParam));
		break;
	case WM_KEYDOWN:
	{
		switch(wParam)
		{
		case VK_ESCAPE:
			SendMessage(hwnd, WM_CLOSE, 0, 0);
			break;
		case VK_F11:
			ToggleActivateBeemConsole(BeemConsole);
			break;
		case VK_NEXT:
			ScrollBeemConsole(BeemConsole, -1);
			break;
		case VK_PRIOR:
			ScrollBeemConsole(BeemConsole, 1);
			break;
		default:
			break;
		}		
		break;
	}
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

int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nShowCmd)
{
	HWND hwnd=NULL;
	MSG msg;
	WNDCLASSEX wc;
	
	BOOL bActiveApp=TRUE;
	/* Create the window's class */
	static TCHAR szAppName[] = TEXT("D3DDemo");
   
	wc.cbSize=sizeof(wc);
	wc.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
	wc.cbClsExtra=0;
	wc.cbWndExtra=0;
	wc.lpfnWndProc=WndProc;
	wc.hInstance=hInstance;
	wc.hbrBackground=(HBRUSH)GetStockObject(DKGRAY_BRUSH);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DXICON));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszMenuName=NULL;
	wc.lpszClassName=szAppName;

	if(!RegisterClassEx(&wc)){
		MessageBox(
			NULL, 
			TEXT("This program requires Windows NT!"), 
			szAppName, 
			MB_OK|MB_ICONERROR);
		return 0;
	}
	
	/* Create the window */
	hwnd = CreateWindowEx(
		0, 
		szAppName,
		szAppName,
		WS_CAPTION|WS_SYSMENU|WS_VISIBLE|WS_MINIMIZEBOX,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		640,
		480,
		NULL,
		NULL,
		hInstance,
		NULL);

	if(hwnd==NULL)
		return -1;

	CFileSystem::InitFileSystem();

	SetCurrentDirectory( TEXT( "data" ) );

	CFileSystem::Get().AutoMount();

	ShowWindow(hwnd, nShowCmd);
	UpdateWindow(hwnd);

	if(!GameInit(hwnd, TRUE, hInstance))
		DestroyWindow(hwnd);

	while(TRUE){
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
			if(msg.message==WM_QUIT)
				break;

			if(msg.message==WM_USER_ACTIVEAPP)
				bActiveApp=static_cast<BOOL>(msg.wParam);

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

	CFileSystem::DeinitFileSystem();

	return 0;
}