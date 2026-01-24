// (c) Beem Media. All rights reserved.

#pragma once

#include "MD3Types.h"

struct md3Skin
{
	std::string MeshName;
	std::string SkinPath;
};

class CMD3SkinConfig
{
public:
	CMD3SkinConfig();
	~CMD3SkinConfig();

	void LoadSkin(const std::filesystem::path& Filename);
	void UnloadSkin();
	void SetSkinRef(const char* Name, md3_uint32 Ref);

protected:
	std::vector<md3Skin> m_Skins; /* The MD3 skins. */
	std::vector<md3_uint32> m_SkinRef; /* Which skin is which reference. */
	md3_bool m_bRefsSet = false; /* Whether or not references are set. */
	md3_uint32 m_NumSkins = 0; /* Number of skins in this file. */
	md3_bool m_bLoaded = false; /* Whether or not a skin file is loaded. */

private:
	void CreateSkinFile(md3_uint32 NumSkins);
	void DeleteSkinFile();

	void ReadSkins(const std::vector<std::string>& SkinLines);
	void ParseLine(md3Skin& Out, const std::string& Line);
};
