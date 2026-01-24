// (c) Beem Media. All rights reserved.

#pragma once

#include "MD3SkinConfig.h"
#include <d3d9.h>

class CGFX3D9TextureDB;

class CMD3SkinFile : public CMD3SkinConfig
{
public:
	CMD3SkinFile();
	~CMD3SkinFile();

	HRESULT LoadSkin(LPDIRECT3DDEVICE9 lpDevice, const std::filesystem::path& Filename, CGFX3D9TextureDB& TexDB);
	HRESULT GetTexturePointer(DWORD dwRef, LPDIRECT3DTEXTURE9* lppTexture);
	HRESULT SetRenderTexture(DWORD dwRef, LPDIRECT3DDEVICE9 lpDevice);
	HRESULT SetSkin(LPDIRECT3DDEVICE9 lpDevice, DWORD dwRef);

private:
	void ClearTextures();

	HRESULT ObtainTextures(LPDIRECT3DDEVICE9 lpDevice, const std::filesystem::path& TexPath, CGFX3D9TextureDB& TexDB);

private:
	std::vector<IDirect3DTexture9*> m_Textures;
};
