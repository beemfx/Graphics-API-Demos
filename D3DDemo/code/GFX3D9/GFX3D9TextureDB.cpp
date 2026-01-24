// (c) Beem Media. All rights reserved.

#include "GFX3D9TextureDB.h"
#include "FileSystem/DataStream.h"
#include "Library/Functions.h"
#include <d3d9.h>
#include <d3dx9.h>

CGFX3D9TextureDB::CGFX3D9TextureDB()
{

}

CGFX3D9TextureDB::~CGFX3D9TextureDB()
{
	ClearDB();
}

bool CGFX3D9TextureDB::AddTexture(IDirect3DDevice9* Dev, const std::filesystem::path& Filename)
{
	if (IDirect3DTexture9* ExistingTex = GetTexture(Filename))
	{
		// Already added, nothing more to do.
		ExistingTex->Release();
		return true;
	}

	const std::string TexName = FilenameToID(Filename);

	CDataStream Data(Filename);
	if (Data.GetSize() == 0)
	{
		return false;
	}

	IDirect3DTexture9* NewTexture = nullptr;
	const HRESULT Res = D3DXCreateTextureFromFileInMemory(Dev, Data.GetData(), static_cast<UINT>(Data.GetDataSize()), &NewTexture);
	if (SUCCEEDED(Res) && NewTexture)
	{
		m_Textures.emplace(TexName, NewTexture);
		return true;
	}

	return false;
}

bool CGFX3D9TextureDB::HasTexture(const std::filesystem::path& Filename) const
{
	const std::string TexName = FilenameToID(Filename);
	return m_Textures.contains(TexName);
}

IDirect3DTexture9* CGFX3D9TextureDB::GetTexture(const std::filesystem::path& Filename) const
{
	const std::string TexName = FilenameToID(Filename);
	if (m_Textures.contains(TexName))
	{
		IDirect3DTexture9* Out = m_Textures[TexName];
		if (Out)
		{
			Out->AddRef();
		}
		return Out;
	}

	return nullptr;
}

void CGFX3D9TextureDB::ClearDB()
{
	for (auto& Item : m_Textures)
	{
		if (Item.second)
		{
			Item.second->Release();
		}
	}
	m_Textures.clear();
}

std::string CGFX3D9TextureDB::FilenameToID(const std::filesystem::path& Filename)
{
	// TODO: Set case to lower.
	return Functions::RemoveDirectoryFromString(Filename.string());
}
