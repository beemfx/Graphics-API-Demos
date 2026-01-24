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

	bool LoadSkin(IDirect3DDevice9* lpDevice, const std::filesystem::path& Filename, CGFX3D9TextureDB& TexDB);
	IDirect3DTexture9* GetTexturePointer(md3_uint32 Ref);

private:
	void ClearTextures();

	void ObtainTextures(IDirect3DDevice9* Dev, const std::filesystem::path& TexPath, CGFX3D9TextureDB& TexDB);

private:
	mutable std::vector<IDirect3DTexture9*> m_Textures;
};
