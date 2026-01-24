// (c) Beem Media. All rights reserved.

#pragma once

#include <filesystem>
#include <map>
#include <string>

struct IDirect3DDevice9;
struct IDirect3DTexture9;

class CGFX3D9TextureDB
{
public:
	CGFX3D9TextureDB();
	~CGFX3D9TextureDB();

	bool AddTexture(IDirect3DDevice9* Dev, const std::filesystem::path& Filename);
	bool HasTexture(const std::filesystem::path& Filename) const;
	IDirect3DTexture9* GetTexture(const std::filesystem::path& Filename) const;
	void ClearDB();

private:
	mutable std::map<std::string,IDirect3DTexture9*> m_Textures;

private:
	static std::string FilenameToID(const std::filesystem::path& Filename);
};