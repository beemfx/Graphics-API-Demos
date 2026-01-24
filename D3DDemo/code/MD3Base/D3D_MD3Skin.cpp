// (c) Beem Media. All rights reserved.

#include "D3D_MD3Skin.h"
#include "Library/Functions.h"
#include "GFX3D9/GFX3D9TextureDB.h"

CD3D_MD3Skin::CD3D_MD3Skin()
{
	
}

CD3D_MD3Skin::~CD3D_MD3Skin()
{
	ClearTextures();
}

bool CD3D_MD3Skin::LoadSkin(IDirect3DDevice9* lpDevice, const std::filesystem::path& Filename, CGFX3D9TextureDB& TexDB)
{
	CMD3SkinConfig::LoadSkin(Filename);

	ClearTextures();
	m_Textures.resize(m_NumSkins);
	ObtainTextures(lpDevice, Functions::RemoveFilenameFromString(Filename.string()) + "\\", TexDB);

	return true;
}

IDirect3DTexture9* CD3D_MD3Skin::GetTexturePointer(md3_uint32 Ref)
{
	if ((Ref < 0) || (Ref >= m_NumSkins))
	{
		return nullptr;
	}

	IDirect3DTexture9* Out = nullptr;
	if (m_bRefsSet)
	{
		Out = m_Textures[m_SkinRef[Ref]];
	}
	else
	{
		Out = m_Textures[Ref];
	}

	if (Out)
	{
		Out->AddRef();
	}

	return Out;
}

void CD3D_MD3Skin::ClearTextures()
{
	for (auto& Item : m_Textures)
	{
		if (Item)
		{
			Item->Release();
			Item = nullptr;
		}
	}
	m_Textures.resize(0);
	m_Textures.shrink_to_fit();
}

void CD3D_MD3Skin::ObtainTextures(IDirect3DDevice9* Dev, const std::filesystem::path& TexPath, CGFX3D9TextureDB& TexDB)
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
		if (TexDB.AddTexture(Dev, szFilename))
		{
			m_Textures[i] = TexDB.GetTexture(szFilename);
		}
		else
		{
			m_Textures[i] = NULL;
		}
	}
}
