#define D3D_MD3
#include <d3dx9.h>
#include <stdio.h>
#include "defines.h"
#include "functions.h"
#include "MD3.h"

CMD3ObjectMesh::CMD3ObjectMesh()
{
	m_lppObjTex=NULL;

	m_bLoaded=FALSE;
}

CMD3ObjectMesh::~CMD3ObjectMesh()
{
	Clear();
}

HRESULT CMD3ObjectMesh::Clear()
{
	LONG lNumMesh=0;
	DWORD i=0;

	if(!m_bLoaded)
		return S_FALSE;

	m_meshObject.GetNumMeshes(&lNumMesh);

	for(i=0; i<(DWORD)lNumMesh; i++){
		SAFE_RELEASE(m_lppObjTex[i]);
	}
	SAFE_FREE(m_lppObjTex);

	m_TexDB.ClearDB();
	m_meshObject.ClearMD3();

	return S_OK;
}

HRESULT CMD3ObjectMesh::TextureExtension(LPDIRECT3DDEVICE9 lpDevice, char szShader[MAX_PATH])
{
	size_t dwLen=0, i=0, j=0;
	char szTemp[MAX_PATH];
	HRESULT hr=0;

	//First attempt to load the name provided.
	hr=m_TexDB.AddTexture(lpDevice, szShader);
	if(SUCCEEDED(hr)){
		RemoveDirectoryFromStringA(szShader, szShader);
		return S_OK;
	}

	dwLen=strlen(szShader);
	for(i=0, j=0; i<dwLen; i++, j++){
		if(szShader[i]=='.'){
			szTemp[j]=szShader[i];
			szTemp[j+1]=0;	
			break;
		}
		szTemp[j]=szShader[i];		
	}

	//Attempt to replace the extension till we successfully load.
	strcpy(szShader, szTemp);

	strcpy(szTemp, szShader);
	strcat(szTemp, "JPG");
	hr=m_TexDB.AddTexture(lpDevice, szTemp);
	if(SUCCEEDED(hr)){
		strcpy(szShader, szTemp);
		RemoveDirectoryFromStringA(szShader, szShader);
		return S_OK;
	}

	
	strcpy(szTemp, szShader);
	strcat(szTemp, "BMP");
	hr=m_TexDB.AddTexture(lpDevice, szTemp);
	if(SUCCEEDED(hr)){
		strcpy(szShader, szTemp);
		RemoveDirectoryFromStringA(szShader, szShader);
		return S_OK;
	}

	strcpy(szTemp, szShader);
	strcat(szTemp, "PNG");
	hr=m_TexDB.AddTexture(lpDevice, szTemp);
	if(SUCCEEDED(hr)){
		strcpy(szShader, szTemp);
		RemoveDirectoryFromStringA(szShader, szShader);
		return S_OK;
	}

	strcpy(szTemp, szShader);
	strcat(szTemp, "DIB");
	hr=m_TexDB.AddTexture(lpDevice, szTemp);
	if(SUCCEEDED(hr)){
		strcpy(szShader, szTemp);
		RemoveDirectoryFromStringA(szShader, szShader);
		return S_OK;
	}

	strcpy(szTemp, szShader);
	strcat(szTemp, "DDS");
	hr=m_TexDB.AddTexture(lpDevice, szTemp);
	if(SUCCEEDED(hr)){
		strcpy(szShader, szTemp);
		RemoveDirectoryFromStringA(szShader, szShader);
		return S_OK;
	}

	strcpy(szTemp, szShader);
	strcat(szTemp, "TGA");
	hr=m_TexDB.AddTexture(lpDevice, szTemp);
	if(SUCCEEDED(hr)){
		strcpy(szShader, szTemp);
		RemoveDirectoryFromStringA(szShader, szShader);
		return S_OK;
	}

	return E_FAIL;
}

HRESULT CMD3ObjectMesh::Load(LPDIRECT3DDEVICE9 lpDevice, char szFile[], MD3DETAIL nDetail)
{
	LONG lNumMesh=0;
	DWORD i=0;
	char szShader[MAX_QPATH];
	char szTexName[MAX_PATH];
	char szPath[MAX_PATH];
	char szExt[7];
	char szFileName[MAX_PATH];
	size_t dwLen=0;

	GetDirectoryFromStringA(szPath, szFile);
	strcpy(szFileName, szFile);

	if(nDetail==DETAIL_LOW)
		strcpy(szExt, "_2.md3");
	else if(nDetail==DETAIL_MEDIUM)
		strcpy(szExt, "_1.md3");
	else
		strcpy(szExt, ".md3");

	dwLen=strlen(szFile);
	for(i=0; i<dwLen; i++){
		if(szFileName[i]=='.')
			break;
	}
	szFileName[i]=0;
	strcat(szFileName, szExt);

	if(FAILED(m_meshObject.LoadMD3(szFileName, NULL, lpDevice, D3DPOOL_DEFAULT))){
		if(nDetail==DETAIL_MEDIUM || nDetail==DETAIL_LOW)
			return Load(lpDevice, szFile, DETAIL_HIGH);
		return E_FAIL;
	}


	m_meshObject.GetNumMeshes(&lNumMesh);

	m_lppObjTex=(LPDIRECT3DTEXTURE9*)malloc(sizeof(LPDIRECT3DTEXTURE9)*lNumMesh);
	if(m_lppObjTex==NULL){
		m_meshObject.ClearMD3();
		return E_FAIL;
	}
	//Get the textures.
	for(i=0; i<(DWORD)lNumMesh; i++){
		m_meshObject.GetShader(i+1, 1, szShader, NULL);
		RemoveDirectoryFromStringA(szShader, szShader);
		sprintf(szTexName, "%s%s", szPath, szShader);
		if(SUCCEEDED(TextureExtension(lpDevice, szTexName))){
			m_TexDB.GetTexture(szTexName, &m_lppObjTex[i]);
		}else{
			m_lppObjTex[i]=NULL;
		}
	}
	m_bLoaded=TRUE;
	return S_OK;
}

HRESULT CMD3ObjectMesh::Invalidate()
{
	if(!m_bLoaded)
		return S_FALSE;

	return m_meshObject.Invalidate();
}

HRESULT CMD3ObjectMesh::Validate(LPDIRECT3DDEVICE9 lpDevice)
{
	if(!m_bLoaded)
		return S_FALSE;

	return m_meshObject.Validate();
}

HRESULT CMD3ObjectMesh::Render(LPDIRECT3DDEVICE9 lpDevice , const D3DMATRIX& SavedWorldMatrix )
{
	LONG lNumMesh=0;
	DWORD i=0;

	D3DXMATRIX WorldMatrix, Orientation, Translation;

	if(!m_bLoaded)
		return S_FALSE;
	D3DXMatrixIdentity(&WorldMatrix);
	D3DXMatrixIdentity(&Orientation);


	D3DXMatrixRotationX(&Translation, 1.5f*D3DX_PI);
	Orientation*=Translation;
	D3DXMatrixRotationY(&Translation, 0.5f*D3DX_PI);
	Orientation*=Translation;

	Orientation*=SavedWorldMatrix;
	WorldMatrix*=Orientation;

	lpDevice->SetTransform(D3DTS_WORLD, &WorldMatrix);

	m_meshObject.GetNumMeshes(&lNumMesh);

	for(i=0; i<(DWORD)lNumMesh; i++){
		m_meshObject.RenderWithTexture(
			m_lppObjTex[i],
			i+1,
			0.0f,
			0,
			0,
			0);
	}

	lpDevice->SetTransform(D3DTS_WORLD, &SavedWorldMatrix);
	return S_OK;
}