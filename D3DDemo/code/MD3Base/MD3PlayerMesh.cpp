#define D3D_MD3
#include <d3dx9.h>
#include <stdio.h>
#include "defines.h"
#include "md3.h"

CMD3PlayerMesh::CMD3PlayerMesh()
{
	m_nLowerUpperTag=0;
	m_nUpperHeadTag=0;
	m_nUpperWeaponTag=0;

	m_skinUpper=NULL;
	m_skinLower=NULL;
	m_skinHead=NULL;

	m_dwNumSkins=0;
	m_dwDefaultSkin=0;
	m_szSkinName=NULL;

	m_lpDevice=NULL;

	m_bLoaded=FALSE;
}

CMD3PlayerMesh::~CMD3PlayerMesh()
{
	Clear();
}

HRESULT CMD3PlayerMesh::GetSkinRef(DWORD * lpRef, char szSkinName[])
{
	DWORD i=0;

	for(i=0; i<m_dwNumSkins; i++){
		if(_strnicmp(szSkinName, m_szSkinName[i], strlen(m_szSkinName[i]))==0){
			*lpRef=i+1;
			return S_OK;
		}
	}
	return E_FAIL;
}

HRESULT CMD3PlayerMesh::GetAnimation(DWORD dwAnimRef, MD3ANIMATION * lpAnimation)
{
	return m_Animation.GetAnimation(dwAnimRef, lpAnimation, MD3ANIM_ADJUST);
}

HRESULT CMD3PlayerMesh::GetLink(CMD3Mesh * lpFirst, const char szTagName[], WORD * lpTagRef)
{
	LONG i=0;
	LONG lNumTags=0;
	char szTemp[MAX_QPATH];

	lpFirst->GetNumTags(&lNumTags);

	for(i=1; i<=lNumTags; i++){
		lpFirst->GetTagName(i, szTemp);
		if(_strnicmp(szTemp, szTagName, strlen(szTagName))==0){
			*lpTagRef=(WORD)i;
			return S_OK;
		}
	}
	return E_FAIL;
}

HRESULT CMD3PlayerMesh::Invalidate()
{
	if(m_bLoaded){
		m_meshHead.Invalidate();
		m_meshUpper.Invalidate();
		m_meshLower.Invalidate();
	}
	return S_OK;
}
	
HRESULT CMD3PlayerMesh::Validate()
{
	if(m_bLoaded){
		m_meshHead.Validate();
		m_meshUpper.Validate();
		m_meshLower.Validate();
	}
	return S_OK;
}

HRESULT CMD3PlayerMesh::Render(
	LONG lUpperFirstFrame, 
	LONG lUpperSecondFrame, 
	FLOAT fUpperTime,
	LONG lLowerFirstFrame,
	LONG lLowerSecondFrame,
	FLOAT fLowerTime,
	DWORD dwSkinRef,
	CMD3WeaponMesh * lpWeapon,
	const D3DMATRIX& SavedWorldMatrix )
{
	if(!m_bLoaded)
		return 0;

	D3DXMATRIX WorldMatrix, Translation, Temp;

	DWORD dwRenderFlags=0;//MD3RENDER_WIREFRAME;

	//In order to render in the propper location it is necessary
	//to save the current World Matrix.  Then create a rotation
	//matrix to get the mesh properly oriented, then apply the
	//world matrix that was previously saved.

	if(!m_bLoaded)
		return S_FALSE;


	if(dwSkinRef==SKIN_DEFAULT)
		dwSkinRef=m_dwDefaultSkin;

	if((dwSkinRef<1) || (dwSkinRef>m_dwNumSkins))
		return E_FAIL;

	if(fUpperTime>1.0f)
		fUpperTime-=(LONG)fUpperTime;

	if(fLowerTime>1.0f)
		fLowerTime-=(LONG)fLowerTime;


	D3DXMatrixIdentity(&WorldMatrix);

	//Orient the model in the right direction.
	D3DXMatrixRotationX(&Translation, 1.5f*D3DX_PI);
	WorldMatrix*=Translation;
	D3DXMatrixRotationY(&Translation, 0.5f*D3DX_PI);
	WorldMatrix*=Translation;
	WorldMatrix*=SavedWorldMatrix;
	m_lpDevice->SetTransform(D3DTS_WORLD, &WorldMatrix);
	

	//Render the lower mesh.
	m_meshLower.Render(
		&m_skinLower[dwSkinRef-1], 
		fLowerTime, 
		lLowerFirstFrame, 
		lLowerSecondFrame,
		dwRenderFlags);
	
	//Get and set the translation for the lower mesh.
	m_meshLower.GetTagTranslation(
		m_nLowerUpperTag, 
		fLowerTime, 
		lLowerFirstFrame, 
		lLowerSecondFrame, 
		&Translation);
	WorldMatrix=Translation*WorldMatrix;
	m_lpDevice->SetTransform(D3DTS_WORLD, &WorldMatrix);
	D3DMATRIX TaggedMatrix = WorldMatrix;

	//Render the upper mesh.
	m_meshUpper.Render( 
		&m_skinUpper[dwSkinRef-1], 
		fUpperTime, 
		lUpperFirstFrame, 
		lUpperSecondFrame,
		dwRenderFlags);

	//Render a weapon if one is set.
	if(lpWeapon)
	{	
		Temp=TaggedMatrix;

		m_meshUpper.GetTagTranslation(
			m_nUpperWeaponTag,
			fUpperTime,
			lUpperFirstFrame,
			lUpperSecondFrame,
			&Translation);

		WorldMatrix=Translation*WorldMatrix;

		//We must derotate because the object will properly orient itself.
		D3DXMatrixRotationX(&Translation, 0.5f*D3DX_PI);
		WorldMatrix=Translation*WorldMatrix;
		D3DXMatrixRotationY(&Translation, 1.5f*D3DX_PI);
		WorldMatrix=Translation*WorldMatrix;

		m_lpDevice->SetTransform(D3DTS_WORLD, &WorldMatrix);

		//Find out if the weapon should perform the flash.
		LONG lShoot=0, lGauntlet=0;
		MD3ANIMATION Animation;
		m_Animation.GetAnimation(TORSO_ATTACK, &Animation, MD3ANIM_ADJUST);
		lShoot=Animation.lFirstFrame;
		m_Animation.GetAnimation(TORSO_ATTACK2, &Animation, MD3ANIM_ADJUST);
		lGauntlet=Animation.lFirstFrame;

		if(lUpperSecondFrame==(lShoot+1) || lUpperSecondFrame==(lGauntlet+2) || lUpperSecondFrame==(lGauntlet+3))
			lpWeapon->Render(TRUE, WorldMatrix);
		else
			lpWeapon->Render(FALSE, WorldMatrix);

		WorldMatrix=Temp;
	}
	
	//Get and set translation for head mesh.
	m_meshUpper.GetTagTranslation(
		m_nUpperHeadTag, 
		fUpperTime, 
		lUpperFirstFrame, 
		lUpperSecondFrame, 
		&Translation);
	WorldMatrix=Translation*WorldMatrix;
	m_lpDevice->SetTransform(D3DTS_WORLD, &WorldMatrix);

	//Render the head mesh (time, and frame must be set to zero).
	m_meshHead.Render(
		&m_skinHead[dwSkinRef-1], 
		0.0f, 
		0, 
		0,
		dwRenderFlags);
	
	
	//Restore the original world matrix.
	m_lpDevice->SetTransform(D3DTS_WORLD, &SavedWorldMatrix);
	return S_OK;
}

HRESULT CMD3PlayerMesh::GetSkinsA(char szDir[])
{
	WIN32_FIND_DATA FindData;
	HANDLE hFind=NULL;
	DWORD dwNumSkins=0;
	HRESULT hr=0;

	DWORD i=0, j=0, k=0;
	size_t dwLen=0;
	BOOL bFoundName=FALSE;

	char szHeadSkin[MAX_PATH];
	char szUpperSkin[MAX_PATH];
	char szLowerSkin[MAX_PATH];

	char szFindString[MAX_PATH];
	char szTemp[MAX_PATH];
	

	//Basically my intent is to find out how many skins there
	//are.  This can be done by finding out how many files with names
	//skin file begining with upper_ exist.  We could use any bone
	//of the body, but upper will do nicely.
	ZeroMemory(&FindData, sizeof(WIN32_FIND_DATA));
	strcpy(szFindString, szDir);
	strcat(szFindString, "upper_*.skin");
	hFind=FindFirstFile(
		szFindString,
		&FindData);

	if(hFind==INVALID_HANDLE_VALUE)
		return E_FAIL;

	do{
		dwNumSkins++;
	}while(FindNextFile(hFind, &FindData));

	FindClose(hFind);

	m_dwNumSkins=dwNumSkins;

	//Allocate memory to all of the skin files.
	m_skinHead=new CMD3SkinFile[m_dwNumSkins];
	if(m_skinHead==NULL)
		return E_FAIL;
	m_skinUpper=new CMD3SkinFile[m_dwNumSkins];
	if(m_skinUpper==NULL){
		SAFE_DELETE_ARRAY(m_skinHead);
		return E_FAIL;
	}
	m_skinLower=new CMD3SkinFile[m_dwNumSkins];
	if(m_skinLower==NULL){
		SAFE_DELETE_ARRAY(m_skinHead);
		SAFE_DELETE_ARRAY(m_skinUpper);
		return E_FAIL;
	}

	//Allocate memory for the string names.
	m_szSkinName=(char**)malloc(sizeof(char*)*m_dwNumSkins);

	if(m_szSkinName==NULL){
		SAFE_DELETE_ARRAY(m_skinHead);
		SAFE_DELETE_ARRAY(m_skinLower);
		SAFE_DELETE_ARRAY(m_skinUpper);
		return E_FAIL;
	}

	for(i=0; i<m_dwNumSkins; i++){
		m_szSkinName[i]=(char*)malloc(MAX_PATH*sizeof(char));
		if(m_szSkinName[i]==NULL){
			SAFE_DELETE_ARRAY(m_skinHead);
			SAFE_DELETE_ARRAY(m_skinLower);
			SAFE_DELETE_ARRAY(m_skinUpper);

			for(j=0; j<i; j++){
				SAFE_DELETE_ARRAY(m_szSkinName[i]);
			}

			SAFE_DELETE_ARRAY(m_szSkinName);

			return E_FAIL;
		}
	}
	
	//Obtain the names of all the skins.
	hFind=FindFirstFile(szFindString, &FindData);

	if(hFind==INVALID_HANDLE_VALUE){
		SAFE_DELETE_ARRAY(m_skinHead);
		SAFE_DELETE_ARRAY(m_skinLower);
		SAFE_DELETE_ARRAY(m_skinUpper);

		for(i=0; i<m_dwNumSkins; i++){
			SAFE_DELETE_ARRAY(m_szSkinName[i]);
		}
		SAFE_DELETE_ARRAY(m_szSkinName);
		return E_FAIL;
	}

	for(i=0; i<m_dwNumSkins; i++){
		strcpy(szTemp, FindData.cFileName);

		dwLen=strlen(szTemp);
		bFoundName=FALSE;
		//Find the first occurence of '_'.
		for(j=0, k=0; j<dwLen; j++){
			if(szTemp[j]=='.'){
				m_szSkinName[i][k]=0;
				if(strncmp(m_szSkinName[i], "default", 7)==0){
					m_dwDefaultSkin=i+1;
				}
				break;
			}
			if(bFoundName){
				m_szSkinName[i][k]=szTemp[j];
				k++;
			}
			if(szTemp[j]=='_')
				bFoundName=TRUE;
		}
		
		FindNextFile(hFind, &FindData);
	}

	//We'll now go ahead and load all the skins.
	for(i=0; i<m_dwNumSkins; i++){
		sprintf(szHeadSkin, "%s%s%s%s", szDir, "head_", m_szSkinName[i], ".skin");
		sprintf(szUpperSkin, "%s%s%s%s", szDir, "upper_", m_szSkinName[i], ".skin");
		sprintf(szLowerSkin, "%s%s%s%s", szDir, "lower_", m_szSkinName[i], ".skin");

		hr=m_skinHead[i].LoadSkinA(m_lpDevice, szHeadSkin, MD3SKINCREATE_DYNAMICTEXDB, &m_TexDB);
		hr|=m_skinUpper[i].LoadSkinA(m_lpDevice, szUpperSkin, MD3SKINCREATE_DYNAMICTEXDB, &m_TexDB);
		hr|=m_skinLower[i].LoadSkinA(m_lpDevice, szLowerSkin, MD3SKINCREATE_DYNAMICTEXDB, &m_TexDB);

		if(FAILED(hr)){
			for(j=0; j<i; j++){
				m_skinHead[j].UnloadSkin();
				m_skinUpper[j].UnloadSkin();
				m_skinLower[j].UnloadSkin();
			}
			for(j=0; j<m_dwNumSkins; j++){
				SAFE_DELETE_ARRAY(m_szSkinName[i]);
			}
			SAFE_DELETE_ARRAY(m_szSkinName);
			
			return E_FAIL;
		}
	}

	
	//Set all the skin references for the model.
	for(i=0; i<m_dwNumSkins; i++){
		m_meshHead.SetSkinRefs(&m_skinHead[i]);
		m_meshUpper.SetSkinRefs(&m_skinUpper[i]);
		m_meshLower.SetSkinRefs(&m_skinLower[i]);
	}

	return S_OK;
}

HRESULT CMD3PlayerMesh::GetSkinsW(WCHAR szDir[])
{
	return E_FAIL;
}

HRESULT CMD3PlayerMesh::LoadA(LPDIRECT3DDEVICE9 lpDevice, char szDir[], MD3DETAIL nDetail)
{
	size_t dwLen=0;
	HRESULT hr=0;
	char szDirectory[MAX_PATH];
	char szHead[MAX_PATH];
	char szUpper[MAX_PATH];
	char szLower[MAX_PATH];
	char szAnimation[MAX_PATH];

	if(!lpDevice)
		return E_FAIL;
	
	Clear();

	m_lpDevice=lpDevice;
	m_lpDevice->AddRef();


	//First thing to do is attempt to acquire the actual md3's.
	strcpy(szDirectory, szDir);
	dwLen=strlen(szDirectory);

	//Insure that there is a backslash at the end of the directory.
	if(szDirectory[dwLen-1]!='\\'){
		szDirectory[dwLen]='\\';
		szDirectory[dwLen+1]=0;
		dwLen++;
	}

	switch(nDetail)
	{
	case DETAIL_HIGH:
		strcpy(szHead, szDirectory);
		strcpy(szUpper, szDirectory);
		strcpy(szLower, szDirectory);
		strcat(szHead, "head.md3");
		strcat(szUpper, "upper.md3");
		strcat(szLower, "lower.md3");
		break;
	case DETAIL_MEDIUM:
		strcpy(szHead, szDirectory);
		strcpy(szUpper, szDirectory);
		strcpy(szLower, szDirectory);
		strcat(szHead, "head_1.md3");
		strcat(szUpper, "upper_1.md3");
		strcat(szLower, "lower_1.md3");
		break;
	case DETAIL_LOW:
		strcpy(szHead, szDirectory);
		strcpy(szUpper, szDirectory);
		strcpy(szLower, szDirectory);
		strcat(szHead, "head_2.md3");
		strcat(szUpper, "upper_2.md3");
		strcat(szLower, "lower_2.md3");
		break;
	};

	hr=m_meshHead.LoadMD3A(szHead, NULL, lpDevice, D3DPOOL_DEFAULT);
	hr|=m_meshUpper.LoadMD3A(szUpper, NULL, lpDevice, D3DPOOL_DEFAULT);
	hr|=m_meshLower.LoadMD3A(szLower, NULL, lpDevice, D3DPOOL_DEFAULT);

	if(FAILED(hr)){
		m_meshHead.ClearMD3();
		m_meshUpper.ClearMD3();
		m_meshLower.ClearMD3();

		if( (nDetail==DETAIL_MEDIUM) || (nDetail==DETAIL_LOW))
		{
			//Lower details may not exist of this model so
			//attempt to load at high detail.
			SAFE_RELEASE(m_lpDevice);
			return LoadA(lpDevice, szDir, DETAIL_HIGH);
		}
		SAFE_RELEASE(m_lpDevice);
		return hr;
	}


	//Attempt to load the animation.
	strcpy(szAnimation, szDirectory);
	strcat(szAnimation, "animation.cfg");
	hr=m_Animation.LoadAnimationA(szAnimation);
	if(FAILED(hr)){
		m_meshHead.ClearMD3();
		m_meshUpper.ClearMD3();
		m_meshLower.ClearMD3();
		SAFE_RELEASE(m_lpDevice);
		return E_FAIL;
	}
	
	hr=GetSkinsA(szDirectory);


	if(FAILED(hr)){
		m_dwNumSkins=0;

		m_meshHead.ClearMD3();
		m_meshUpper.ClearMD3();
		m_meshLower.ClearMD3();
		SAFE_RELEASE(m_lpDevice);
		return hr;
	}

	//Get the link reference for all the md3 mesh's.
	GetLink(&m_meshLower, "tag_torso", &m_nLowerUpperTag);
	GetLink(&m_meshUpper, "tag_head", &m_nUpperHeadTag);
	GetLink(&m_meshUpper, "tag_weapon", &m_nUpperWeaponTag);

	m_bLoaded=TRUE;

	return S_OK;
}

HRESULT CMD3PlayerMesh::LoadW(LPDIRECT3DDEVICE9 lpDevice, WCHAR szDir[], MD3DETAIL nDetail)
{
	return E_FAIL;
}

HRESULT CMD3PlayerMesh::Clear()
{
	DWORD i=0;

	if(!m_bLoaded)return S_FALSE;

	m_meshHead.ClearMD3();
	m_meshUpper.ClearMD3();
	m_meshLower.ClearMD3();
	m_TexDB.ClearDB();
	for(i=0; i<m_dwNumSkins; i++){
		m_skinHead[i].UnloadSkin();
		m_skinUpper[i].UnloadSkin();
		m_skinLower[i].UnloadSkin();

		SAFE_FREE(m_szSkinName[i]);
	}
	SAFE_FREE(m_szSkinName);

	SAFE_DELETE_ARRAY(m_skinHead);
	SAFE_DELETE_ARRAY(m_skinLower);
	SAFE_DELETE_ARRAY(m_skinUpper);

	SAFE_RELEASE(m_lpDevice);

	m_dwNumSkins=0;
	m_dwDefaultSkin=0;
	m_bLoaded=FALSE;
	return S_OK;
}