#define D3D_MD3
#include <stdio.h>
#include "Defines.h"
#include "Functions.h"
#include "MD3.h"

#define SKINSUC_READSUCCESS 0x00000001l
#define SKINSUC_EOF         0x00000002l
#define SKINSUC_FINISHED    0x00000003l
#define SKINERR_NOTALINE    0x80000001l
#define SKINERR_INVALIDFILE 0x80000002l

CMD3TextureDB CMD3SkinFile::m_md3TexDB;

HRESULT CMD3SkinFile::ClearTexDB()
{
	return m_md3TexDB.ClearDB();
}

HRESULT CMD3SkinFile::SetSkinRef(
	char szName[], 
	DWORD dwRef)
{
	DWORD i=0;

	if((dwRef < 0) || (dwRef >= m_dwNumSkins))return E_FAIL;
	
	for(i=0; i<m_dwNumSkins; i++){
		if(strcmp(szName, m_lpSkins[i].szMeshName)==0){
			m_lpSkinRef[dwRef]=i;
			break;
		}
	}
	m_bRefsSet=TRUE;
	return S_FALSE;
}

HRESULT CMD3SkinFile::GetTexturePointer(
	DWORD dwRef,
	LPDIRECT3DTEXTURE9 * lppTexture)
{
	if((dwRef < 0) || (dwRef >= m_dwNumSkins)){
		*lppTexture=NULL;
		return E_FAIL;
	}

	if(m_bRefsSet)
	{
		*lppTexture=m_lppTextures[m_lpSkinRef[dwRef]];
	}
	else
	{
		*lppTexture=m_lppTextures[dwRef];
	}
	if( (*lppTexture) )
		(*lppTexture)->AddRef();
	return S_OK;
}

HRESULT CMD3SkinFile::SetSkin(
	LPDIRECT3DDEVICE9 lpDevice, 
	DWORD dwRef)
{
	if((dwRef < 0) || (dwRef >= m_dwNumSkins))return E_FAIL;
	if(m_bRefsSet)
		lpDevice->SetTexture(0, m_lppTextures[m_lpSkinRef[dwRef]]);
	else{
		lpDevice->SetTexture(0, NULL);
		return S_FALSE;
	}

	if(m_lppTextures[m_lpSkinRef[dwRef]]==NULL)
		return S_SKINNULL;
	else
		return S_OK;
}




CMD3SkinFile::CMD3SkinFile()
{
	m_bLoaded=FALSE;
	m_lpSkins=NULL;
	m_lpSkinRef=NULL;
	m_bUseStaticDB=TRUE;
	m_bRefsSet=FALSE;
	m_dwNumSkins=0;

	m_lppTextures=NULL;
}

CMD3SkinFile::~CMD3SkinFile()
{
	UnloadSkin();
}


HRESULT CMD3SkinFile::ReadSkins(
	HANDLE hFile, 
	DWORD dwNumSkinsToRead, 
	DWORD * dwNumSkinsRead,
	DWORD dwFlags)
{
	HRESULT hr=0;
	DWORD dwBytes=0;
	DWORD i=0;
	char szLine[MAX_PATH];

	if(hFile==INVALID_HANDLE_VALUE)
		return E_FAIL;

	for(i=0; i<dwNumSkinsToRead; i++)
	{
		hr=ReadLine(hFile, szLine);
		if(FAILED(hr))return hr;
		if(hr!=SKINSUC_FINISHED)
		{
			ParseLine(&(m_lpSkins[i]), szLine);
			if((dwFlags&MD3SKINCREATE_REMOVEDIR)==MD3SKINCREATE_REMOVEDIR)
				RemoveDirectoryFromStringA(m_lpSkins[i].szSkinPath, m_lpSkins[i].szSkinPath);

		}
	}
	return S_OK;
}

HRESULT CMD3SkinFile::LoadSkinA(
	LPDIRECT3DDEVICE9 lpDevice,
	char szFilename[MAX_PATH],
	DWORD dwFlags,
	LPVOID lpTexDB)
{
	HANDLE hFile=NULL;
	DWORD dwNumSkins=0;

	UnloadSkin();

	hFile=CreateFileA(
		szFilename,
		GENERIC_READ,
		FILE_SHARE_READ,
		(LPSECURITY_ATTRIBUTES)NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		(HANDLE)NULL);


	if(hFile==INVALID_HANDLE_VALUE)
		return E_FAIL;

	dwNumSkins=GetNumLinesInFile(hFile);

	CreateSkinFile(dwNumSkins);
	
	ReadSkins(hFile, dwNumSkins, NULL, MD3SKINCREATE_REMOVEDIR);
	
	char szTexPath[MAX_PATH];

	GetDirectoryFromStringA(szTexPath, szFilename);
	
	ObtainTextures(lpDevice, szTexPath, dwFlags, lpTexDB);
	
	return S_OK;
}

HRESULT CMD3SkinFile::LoadSkinW(
	LPDIRECT3DDEVICE9 lpDevice,
	WCHAR szFilename[MAX_PATH],
	DWORD dwFlags,
	LPVOID lpTexDB)
{
	HANDLE hFile=NULL;
	DWORD dwNumSkins=0;

	UnloadSkin();

	hFile=CreateFileW(
		szFilename,
		GENERIC_READ,
		FILE_SHARE_READ,
		(LPSECURITY_ATTRIBUTES)NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		(HANDLE)NULL);

	if(hFile==INVALID_HANDLE_VALUE)
		return E_FAIL;

	dwNumSkins=GetNumLinesInFile(hFile);

	CreateSkinFile(dwNumSkins);

	ReadSkins(hFile, dwNumSkins, NULL, MD3SKINCREATE_REMOVEDIR);

	WCHAR szTemp[MAX_PATH];
	char szTexPath[MAX_PATH];

	GetDirectoryFromStringW(szTemp, szFilename);

	WideCharToMultiByte(CP_ACP, 0, szTemp, -1, szTexPath, MAX_PATH, NULL, NULL);

	ObtainTextures(lpDevice, szTexPath, dwFlags, lpTexDB);

	return S_OK;
}

HRESULT CMD3SkinFile::ObtainTextures(
	LPDIRECT3DDEVICE9 lpDevice,
	char szTexPath[], 
	DWORD dwFlags, 
	LPVOID lpTexDB)
{
	//The name and path to the texture.
	char szFilename[MAX_PATH];
	DWORD i=0;
	size_t dwLen=0;
	for(i=0; i<m_dwNumSkins; i++){
		strcpy(szFilename, szTexPath);
		//Insure that there is a backslash at the end of the texture path.
		dwLen=strlen(szFilename);
		if(szFilename[dwLen-1] != '\\'){
			szFilename[dwLen]='\\';
			szFilename[dwLen+1]=0;
			MessageBox(0, szFilename, 0, 0);
		}
		//Attach the filename to the texture path.
		strcat(szFilename, m_lpSkins[i].szSkinPath);
		
		//If using static texture buffer create and/or obtain texture from
		//the static buffer
		if( (dwFlags&MD3SKINCREATE_STATICTEXDB)==MD3SKINCREATE_STATICTEXDB){
			if(SUCCEEDED(m_md3TexDB.AddTexture(lpDevice, szFilename))){
				if(SUCCEEDED(m_md3TexDB.GetTexture(m_lpSkins[i].szSkinPath, &m_lppTextures[i]))){

				}else{
					m_lppTextures[i]=NULL;
				}
			}else{
				m_lppTextures[i]=NULL;
			}
		//If using dynamic buffer add texture to the parameter as database.
		}else if( (dwFlags&MD3SKINCREATE_DYNAMICTEXDB)==MD3SKINCREATE_DYNAMICTEXDB){
			if(SUCCEEDED(((CMD3TextureDB*)lpTexDB)->AddTexture(lpDevice, szFilename))){
				if(SUCCEEDED(((CMD3TextureDB*)lpTexDB)->GetTexture(m_lpSkins[i].szSkinPath, &m_lppTextures[i]))){
				}else{
					m_lppTextures[i]=NULL;
				}
			}else{
				m_lppTextures[i]=NULL;
			}
		//If invalid parameter apply null to the texture.
		}else{
			m_lppTextures[i]=NULL;
		}
	}

	if( (dwFlags&MD3SKINCREATE_STATICTEXDB)==MD3SKINCREATE_STATICTEXDB){
		m_bUseStaticDB=TRUE;
	}
	else{
		m_bUseStaticDB=FALSE;
	}
	return S_OK;
}


HRESULT CMD3SkinFile::UnloadSkin()
{
	if(SUCCEEDED(DeleteSkinFile())){
		m_dwNumSkins=0;
		return S_OK;
	}
	else
		return E_FAIL;
}

HRESULT CMD3SkinFile::CreateSkinFile(DWORD dwNumSkins)
{
	DWORD i=0;
	m_dwNumSkins=dwNumSkins;
	m_lpSkins=(MD3SKIN*)malloc(sizeof(MD3SKIN)*dwNumSkins);
	m_lpSkinRef=(DWORD*)malloc(sizeof(DWORD)*dwNumSkins);
	for(i=0; i<dwNumSkins; i++){
		m_lpSkinRef[i]=0;
	}
	m_lppTextures=(LPDIRECT3DTEXTURE9*)malloc(sizeof(LPDIRECT3DTEXTURE9)*dwNumSkins);
	for(i=0; i<dwNumSkins; i++){
		m_lppTextures[i]=NULL;
	}
	return S_OK;
}

HRESULT CMD3SkinFile::DeleteSkinFile()
{
	DWORD i=0;

	SAFE_FREE(m_lpSkins);
	SAFE_FREE(m_lpSkinRef);
	for(i=0; i<m_dwNumSkins; i++){
		SAFE_RELEASE(m_lppTextures[i]);
	}
	SAFE_FREE(m_lppTextures);

	return S_OK;
}


BOOL CMD3SkinFile::ParseLine(MD3SKIN * skinout, LPSTR szLineIn)
{
	size_t dwLineLen=0;
	BOOL bSecondPart=FALSE;
	DWORD i=0;
	DWORD nStringPos=0;
	dwLineLen=strlen(szLineIn);
	for(i=0; i<dwLineLen; i++, nStringPos++){
		if( szLineIn[i] == ',' ){
			skinout->szMeshName[nStringPos]=0;
			i++;
			nStringPos=0;
			bSecondPart=TRUE;
		}
		if(!bSecondPart)
			skinout->szMeshName[nStringPos]=szLineIn[i];
		else
			skinout->szSkinPath[nStringPos]=szLineIn[i];
	}
	skinout->szSkinPath[nStringPos]=0;
	return TRUE;
}

