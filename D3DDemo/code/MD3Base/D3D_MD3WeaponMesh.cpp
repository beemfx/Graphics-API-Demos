// (c) Beem Media. All rights reserved.

#include "D3D_MD3WeaponMesh.h"
#include <d3dx9.h>
#include <stdio.h>
#include "defines.h"
#include "Library/Functions.h"

CMD3WeaponMesh::CMD3WeaponMesh()
{
	m_lpFlashTex = NULL;
	m_lpWeaponTex = NULL;
	m_lpBarrelTex = NULL;

	m_bBarrel = FALSE;

	m_nTagWeapon = m_nTagBarrel = m_nTagFlash = 0;

	m_bLoaded = FALSE;
}


CMD3WeaponMesh::~CMD3WeaponMesh()
{
	Clear();
}

HRESULT CMD3WeaponMesh::Clear()
{
	DWORD i = 0, j = 0;
	LONG lNumMesh = 0;

	if (!m_bLoaded)
		return S_FALSE;

	m_TexDB.ClearDB();

	//Delete the weapon.
	m_meshWeapon.GetNumMeshes(&lNumMesh);
	for (i = 0; i < (DWORD)lNumMesh; i++) {
		SAFE_RELEASE(m_lpWeaponTex[i]);
	}
	SAFE_FREE(m_lpWeaponTex);
	m_meshWeapon.ClearMD3();

	//Delete the barrel.
	if (m_bBarrel) {
		m_meshBarrel.GetNumMeshes(&lNumMesh);
		for (i = 0; i < (DWORD)lNumMesh; i++) {
			SAFE_RELEASE(m_lpBarrelTex[i]);
		}
		SAFE_FREE(m_lpBarrelTex);
		m_meshBarrel.ClearMD3();
	}

	//Delete the flash.
	m_meshFlash.GetNumMeshes(&lNumMesh);
	for (i = 0; i < (DWORD)lNumMesh; i++) {
		SAFE_RELEASE(m_lpFlashTex[i]);
	}
	SAFE_FREE(m_lpFlashTex);
	m_meshFlash.ClearMD3();

	//Delete the hand.
	m_meshHand.ClearMD3();

	m_bLoaded = FALSE;

	m_bBarrel = FALSE;

	m_nTagWeapon = m_nTagBarrel = m_nTagFlash = 0;

	SAFE_RELEASE(m_lpDevice);
	return S_OK;
}

HRESULT CMD3WeaponMesh::Load(LPDIRECT3DDEVICE9 lpDevice, char szDir[], d3d_md3_detail nDetail)
{
	char szPath[MAX_PATH];
	char szWeaponPath[MAX_PATH];
	char szBarrelPath[MAX_PATH];
	char szFlashPath[MAX_PATH];
	char szHandPath[MAX_PATH];
	char szDetailLevel[3];

	char szShaderName[MAX_QPATH];
	char szTexName[MAX_PATH];
	LONG lNumMesh = 0;
	DWORD i = 0;

	size_t dwLen = 0;

	Clear();

	m_lpDevice = lpDevice;
	m_lpDevice->AddRef();

	//Get the weapon's name.
	strcpy(szPath, szDir);
	dwLen = strlen(szPath);
	if (szPath[dwLen - 1] != '\\') {
		szPath[dwLen] = '\\';
		szPath[dwLen + 1] = 0;
	}
	const std::string WeaponName = [&]() -> std::string
		{
			char szWeaponName[MAX_QPATH];
			strcpy(szWeaponName, szPath);
			dwLen = strlen(szWeaponName);
			szWeaponName[dwLen - 1] = 0;
			return Functions::RemoveDirectoryFromString(szWeaponName);
		}();

	//Prepare each of the path names.

	//Set the detail level.
	if (nDetail == d3d_md3_detail::Low)
		strcpy(szDetailLevel, "_2");
	else if (nDetail == d3d_md3_detail::Medium)
		strcpy(szDetailLevel, "_1");
	else
		strcpy(szDetailLevel, "");

	//The weapon path.
	sprintf(szWeaponPath, "%s%s%s.md3", szPath, WeaponName.c_str(), szDetailLevel);
	//The barrel path.
	sprintf(szBarrelPath, "%s%s_barrel%s.md3", szPath, WeaponName.c_str(), szDetailLevel);
	//The flash path.
	sprintf(szFlashPath, "%s%s_flash.md3", szPath, WeaponName.c_str());
	//The hand path.
	sprintf(szHandPath, "%s%s_hand.md3", szPath, WeaponName.c_str());

	//Attempt to load the weapon mesh.
	if (!m_meshWeapon.LoadMD3(szWeaponPath, lpDevice, D3DPOOL_DEFAULT))
	{
		if (nDetail != d3d_md3_detail::High)
		{
			SAFE_RELEASE(m_lpDevice);
			return Load(lpDevice, szDir, d3d_md3_detail::High);
		}
		SAFE_RELEASE(m_lpDevice);
		return E_FAIL;
	}

	//Load the hand and flash meshes.
	const md3_bool bLoadedHandAndFlash 
		= 
		m_meshHand.LoadMD3(szHandPath, lpDevice, D3DPOOL_DEFAULT)
		&&
		m_meshFlash.LoadMD3(szFlashPath, lpDevice, D3DPOOL_DEFAULT);

	if (!bLoadedHandAndFlash)
	{
		m_meshHand.ClearMD3();
		m_meshFlash.ClearMD3();
		m_meshWeapon.ClearMD3();
		SAFE_RELEASE(m_lpDevice);
		return E_FAIL;
	}

	//Load the barrel, if success then we set barrel to true.
	if (m_meshBarrel.LoadMD3(szBarrelPath, lpDevice, D3DPOOL_DEFAULT))
	{
		m_bBarrel = TRUE;
	}
	//Load the textures.
	m_meshWeapon.GetNumMeshes(&lNumMesh);
	m_lpWeaponTex = (LPDIRECT3DTEXTURE9*)malloc(lNumMesh * sizeof(LPDIRECT3DTEXTURE9));
	if (m_lpWeaponTex == NULL) {
		m_meshHand.ClearMD3();
		m_meshFlash.ClearMD3();
		m_meshWeapon.ClearMD3();
		m_meshBarrel.ClearMD3();
		SAFE_RELEASE(m_lpDevice);
		return E_FAIL;
	}
	if (m_bBarrel) {
		m_meshBarrel.GetNumMeshes(&lNumMesh);
		m_lpBarrelTex = (LPDIRECT3DTEXTURE9*)malloc(lNumMesh * sizeof(LPDIRECT3DTEXTURE9));
		if (m_lpBarrelTex == NULL) {
			SAFE_DELETE_ARRAY(m_lpWeaponTex);
			m_meshHand.ClearMD3();
			m_meshFlash.ClearMD3();
			m_meshWeapon.ClearMD3();
			m_meshBarrel.ClearMD3();
			SAFE_RELEASE(m_lpDevice);
			return E_FAIL;
		}
	}
	m_meshFlash.GetNumMeshes(&lNumMesh);
	m_lpFlashTex = (LPDIRECT3DTEXTURE9*)malloc(lNumMesh * sizeof(LPDIRECT3DTEXTURE9));
	if (m_lpFlashTex == NULL) {
		SAFE_DELETE_ARRAY(m_lpWeaponTex);
		if (m_bBarrel) {
			SAFE_DELETE_ARRAY(m_lpBarrelTex);
		}
		m_meshHand.ClearMD3();
		m_meshFlash.ClearMD3();
		m_meshWeapon.ClearMD3();
		m_meshBarrel.ClearMD3();
		SAFE_RELEASE(m_lpDevice);
		return E_FAIL;
	}

	//Get the weapon textures.
	m_meshWeapon.GetNumMeshes(&lNumMesh);
	for (i = 0; i < (DWORD)lNumMesh; i++) {
		m_meshWeapon.GetShader(i + 1, 1, szShaderName, NULL);
		const std::string ShaderName = Functions::RemoveDirectoryFromString(szShaderName);
		sprintf(szTexName, "%s%s", szPath, ShaderName.c_str());
		if (SUCCEEDED(TextureExtension(szTexName))) {
			m_lpWeaponTex[i] = m_TexDB.GetTexture(szTexName);
		}
		else {
			m_lpWeaponTex[i] = NULL;
		}
	}
	//Get the barrel textures if it exists.
	if (m_bBarrel) {
		m_meshBarrel.GetNumMeshes(&lNumMesh);
		for (i = 0; i < (DWORD)lNumMesh; i++) {
			m_meshBarrel.GetShader(i + 1, 1, szShaderName, NULL);
			const std::string ShaderName = Functions::RemoveDirectoryFromString(szShaderName);
			sprintf(szTexName, "%s%s", szPath, ShaderName.c_str());
			if (SUCCEEDED(TextureExtension(szTexName))) {
				m_lpBarrelTex[i] = m_TexDB.GetTexture(szTexName);
			}
			else {
				m_lpBarrelTex[i] = NULL;
			}
		}
	}
	//Get the flash textures.
	m_meshFlash.GetNumMeshes(&lNumMesh);
	for (i = 0; i < (DWORD)lNumMesh; i++) {
		m_meshFlash.GetShader(i + 1, 1, szShaderName, NULL);
		const std::string ShaderName = Functions::RemoveDirectoryFromString(szShaderName);
		sprintf(szTexName, "%s%s", szPath, ShaderName.c_str());
		if (SUCCEEDED(TextureExtension(szTexName))) {
			m_lpFlashTex[i] = m_TexDB.GetTexture(szTexName);
		}
		else {
			m_lpFlashTex[i] = NULL;
		}
	}
	//Get the tags.
	GetLink(&m_meshWeapon, "tag_weapon", &m_nTagWeapon);
	GetLink(&m_meshWeapon, "tag_barrel", &m_nTagBarrel);
	GetLink(&m_meshWeapon, "tag_flash", &m_nTagFlash);

	m_bLoaded = TRUE;

	return S_OK;
}

HRESULT CMD3WeaponMesh::TextureExtension(char szShader[MAX_PATH])
{
	size_t dwLen = 0, i = 0, j = 0;
	char szTemp[MAX_PATH];

	//First attempt to load the name provided.
	if (m_TexDB.AddTexture(m_lpDevice, szShader)) {
		return S_OK;
	}

	dwLen = strlen(szShader);
	for (i = 0, j = 0; i < dwLen; i++, j++) {
		if (szShader[i] == '.') {
			szTemp[j] = szShader[i];
			szTemp[j + 1] = 0;
			break;
		}
		szTemp[j] = szShader[i];
	}

	//Attempt to replace the extension till we successfully load.
	strcpy(szShader, szTemp);

	strcpy(szTemp, szShader);
	strcat(szTemp, "JPG");
	if (m_TexDB.AddTexture(m_lpDevice, szTemp)) {
		strcpy(szShader, szTemp);
		return S_OK;
	}


	strcpy(szTemp, szShader);
	strcat(szTemp, "BMP");
	if (m_TexDB.AddTexture(m_lpDevice, szTemp)) {
		strcpy(szShader, szTemp);
		return S_OK;
	}

	strcpy(szTemp, szShader);
	strcat(szTemp, "PNG");
	if (m_TexDB.AddTexture(m_lpDevice, szTemp)) {
		strcpy(szShader, szTemp);
		return S_OK;
	}

	strcpy(szTemp, szShader);
	strcat(szTemp, "DIB");
	if (m_TexDB.AddTexture(m_lpDevice, szTemp)) {
		strcpy(szShader, szTemp);
		return S_OK;
	}

	strcpy(szTemp, szShader);
	strcat(szTemp, "DDS");
	if (m_TexDB.AddTexture(m_lpDevice, szTemp)) {
		strcpy(szShader, szTemp);
		return S_OK;
	}

	strcpy(szTemp, szShader);
	strcat(szTemp, "TGA");
	if (m_TexDB.AddTexture(m_lpDevice, szTemp)) {
		strcpy(szShader, szTemp);
		return S_OK;
	}

	return E_FAIL;
}

HRESULT CMD3WeaponMesh::GetLink(CD3D_MD3Mesh* lpFirst, const char szTagName[], WORD* lpTagRef)
{
	LONG i = 0;
	LONG lNumTags = 0;
	char szTemp[MAX_QPATH];

	lpFirst->GetNumTags(&lNumTags);

	for (i = 1; i <= lNumTags; i++) {
		lpFirst->GetTagName(i, szTemp);
		if (_strnicmp(szTemp, szTagName, strlen(szTagName)) == 0) {
			*lpTagRef = (WORD)i;
			return S_OK;
		}
	}
	return E_FAIL;
}

HRESULT CMD3WeaponMesh::Invalidate()
{
	if (!m_bLoaded)
		return S_FALSE;

	m_meshFlash.Invalidate();
	m_meshHand.Invalidate();
	m_meshWeapon.Invalidate();
	if (m_bBarrel) {
		m_meshBarrel.Invalidate();
	}
	return S_OK;
}

HRESULT CMD3WeaponMesh::Validate()
{
	if (!m_bLoaded)
		return S_FALSE;

	m_meshFlash.Validate();
	m_meshHand.Validate();
	m_meshWeapon.Validate();
	if (m_bBarrel) {
		m_meshBarrel.Validate();
	}
	return S_OK;
}

HRESULT CMD3WeaponMesh::Render(BOOL bFlash, const D3DMATRIX& SavedWorldMatrix)
{
	DWORD i = 0, j = 0;
	LONG lNumMesh = 0;
	D3DXMATRIX WorldMatrix, Translation, Orientation, Temp;


	if (!m_bLoaded)
		return S_FALSE;

	D3DXMatrixIdentity(&WorldMatrix);
	D3DXMatrixIdentity(&Orientation);


	D3DXMatrixRotationX(&Translation, 1.5f * D3DX_PI);
	Orientation *= Translation;
	D3DXMatrixRotationY(&Translation, 0.5f * D3DX_PI);
	Orientation *= Translation;

	Orientation *= SavedWorldMatrix;
	WorldMatrix *= Orientation;

	m_lpDevice->SetTransform(D3DTS_WORLD, &WorldMatrix);

	//Render the weapon first.
	Temp = WorldMatrix;
	m_meshWeapon.GetTagTranslation(m_nTagWeapon, 0.0f, 0, 0, &Translation);
	WorldMatrix = Translation * WorldMatrix;
	m_lpDevice->SetTransform(D3DTS_WORLD, &WorldMatrix);
	m_meshWeapon.GetNumMeshes(&lNumMesh);
	for (i = 0; i < (DWORD)lNumMesh; i++) {
		m_meshWeapon.RenderWithTexture(
			m_lpWeaponTex[i],
			i + 1,
			0.0f,
			0,
			0,
			0);
	}
	WorldMatrix = Temp;

	//Render the barrel if there is one.
	if (m_bBarrel) {
		Temp = WorldMatrix;
		m_meshWeapon.GetTagTranslation(m_nTagBarrel, 0.0f, 0, 0, &Translation);
		WorldMatrix = Translation * WorldMatrix;
		m_lpDevice->SetTransform(D3DTS_WORLD, &WorldMatrix);
		m_meshBarrel.GetNumMeshes(&lNumMesh);
		for (i = 0; i < (DWORD)lNumMesh; i++) {
			m_meshBarrel.RenderWithTexture(
				m_lpBarrelTex[i],
				i + 1,
				0.0f,
				0,
				0,
				0);
		}
		WorldMatrix = Temp;
	}

	if (bFlash) {
		Temp = WorldMatrix;
		m_meshWeapon.GetTagTranslation(m_nTagFlash, 0.0f, 0, 0, &Translation);
		WorldMatrix = Translation * WorldMatrix;
		m_lpDevice->SetTransform(D3DTS_WORLD, &WorldMatrix);
		m_meshFlash.GetNumMeshes(&lNumMesh);
		//////////////////////////////////////////
		//NOTICE: Should enable alpha blending.///
		//////////////////////////////////////////
		for (i = 0; i < (DWORD)lNumMesh; i++) {
			m_meshFlash.RenderWithTexture(
				m_lpFlashTex[i],
				i + 1,
				0.0f,
				0,
				0,
				CD3D_MD3Mesh::MD3TEXRENDER_NOCULL);
		}
		//Should restore alpha blending values.
		WorldMatrix = Temp;
	}

	m_lpDevice->SetTransform(D3DTS_WORLD, &SavedWorldMatrix);

	return S_OK;
}