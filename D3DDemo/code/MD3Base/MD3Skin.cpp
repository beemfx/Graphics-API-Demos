#define D3D_MD3
#include <stdio.h>
#include "Defines.h"
#include "Functions.h"
#include "MD3.h"
#include "FileSystem/DataStream.h"

HRESULT CMD3SkinFile::GetTexturePointer(
	DWORD dwRef,
	LPDIRECT3DTEXTURE9* lppTexture)
{
	if ((dwRef < 0) || (dwRef >= m_NumSkins)) {
		*lppTexture = NULL;
		return E_FAIL;
	}

	if (m_bRefsSet)
	{
		*lppTexture = m_Textures[m_SkinRef[dwRef]];
	}
	else
	{
		*lppTexture = m_Textures[dwRef];
	}
	if ((*lppTexture))
		(*lppTexture)->AddRef();
	return S_OK;
}

HRESULT CMD3SkinFile::SetSkin(
	LPDIRECT3DDEVICE9 lpDevice,
	DWORD dwRef)
{
	if ((dwRef < 0) || (dwRef >= m_NumSkins))return E_FAIL;
	if (m_bRefsSet)
		lpDevice->SetTexture(0, m_Textures[m_SkinRef[dwRef]]);
	else {
		lpDevice->SetTexture(0, NULL);
		return S_FALSE;
	}

	if (m_Textures[m_SkinRef[dwRef]] == NULL)
		return S_OK;
	else
		return S_OK;
}


CMD3SkinFile::CMD3SkinFile()
{
	
}

CMD3SkinFile::~CMD3SkinFile()
{
	ClearTextures();
}

void CMD3SkinFile::ClearTextures()
{
	for (auto& Item : m_Textures)
	{
		SAFE_RELEASE(Item);
	}
	m_Textures.resize(0);
	m_Textures.shrink_to_fit();
}

HRESULT CMD3SkinFile::LoadSkin(LPDIRECT3DDEVICE9 lpDevice, const std::filesystem::path& Filename, CGFX3D9TextureDB& TexDB)
{
	CMD3SkinConfig::LoadSkin(Filename);

	ClearTextures();
	m_Textures.resize(m_NumSkins);
	ObtainTextures(lpDevice, Functions::RemoveFilenameFromString(Filename.string()) + "\\", TexDB);

	return S_OK;
}

HRESULT CMD3SkinFile::ObtainTextures(LPDIRECT3DDEVICE9 lpDevice, const std::filesystem::path& TexPath, CGFX3D9TextureDB& TexDB)
{
	//The name and path to the texture.
	char szFilename[MAX_PATH];
	DWORD i = 0;
	size_t dwLen = 0;
	for (i = 0; i < m_NumSkins; i++) {
		strcpy(szFilename, TexPath.string().c_str());
		//Insure that there is a backslash at the end of the texture path.
		dwLen = strlen(szFilename);
		if (szFilename[dwLen - 1] != '\\') {
			szFilename[dwLen] = '\\';
			szFilename[dwLen + 1] = 0;
			MessageBox(0, szFilename, 0, 0);
		}
		//Attach the filename to the texture path.
		strcat(szFilename, m_Skins[i].SkinPath.c_str());

		//If using static texture buffer create and/or obtain texture from
		//the static buffer
		if (TexDB.AddTexture(lpDevice, szFilename))
		{
			m_Textures[i] = TexDB.GetTexture(szFilename);
		}
		else
		{
			m_Textures[i] = NULL;
		}
	}

	return S_OK;
}
