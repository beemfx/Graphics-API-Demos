// (c) Beem Media. All rights reserved.

#include "MD3SkinConfig.h"
#include "Library/Functions.h"
#include "FileSystem/DataStream.h"

CMD3SkinConfig::CMD3SkinConfig()
{

}

CMD3SkinConfig::~CMD3SkinConfig()
{
	UnloadSkin();
}

void CMD3SkinConfig::SetSkinRef(const char* Name, md3_uint32 Ref)
{
	if ((Ref < 0) || (Ref >= m_NumSkins))
	{
		return;
	}

	for (md3_uint32 i = 0; i < m_NumSkins; i++)
	{
		if (m_Skins[i].MeshName == Name)
		{
			m_SkinRef[Ref] = i;
			break;
		}
	}
	m_bRefsSet = true;
}


void CMD3SkinConfig::ReadSkins(const std::vector<std::string>& SkinLines)
{
	assert(SkinLines.size() == m_NumSkins);

	for (md3_uint32 i = 0; i < m_NumSkins; i++)
	{
		ParseLine(m_Skins[i], SkinLines[i]);
		m_Skins[i].SkinPath = Functions::RemoveDirectoryFromString(m_Skins[i].SkinPath);
	}
}

void CMD3SkinConfig::LoadSkin(const std::filesystem::path& Filename)
{
	UnloadSkin();

	CDataStream SkinData(Filename);

	const std::vector<std::string> SkinLines = Functions::ReadLines(SkinData);

	CreateSkinFile(static_cast<md3_uint32>(SkinLines.size()));

	ReadSkins(SkinLines);
}

void CMD3SkinConfig::UnloadSkin()
{
	DeleteSkinFile();
}

void CMD3SkinConfig::CreateSkinFile(md3_uint32 NumSkins)
{
	m_NumSkins = NumSkins;

	m_Skins.resize(NumSkins);

	m_SkinRef.resize(NumSkins);
	for (auto& Item : m_SkinRef)
	{
		Item = 0;
	}
}

void CMD3SkinConfig::DeleteSkinFile()
{
	m_Skins.resize(0);
	m_Skins.shrink_to_fit();
	m_SkinRef.resize(0);
	m_SkinRef.shrink_to_fit();
}

void CMD3SkinConfig::ParseLine(md3Skin& Out, const std::string& Line)
{
	size_t dwLineLen = 0;
	bool bSecondPart = false;
	std::size_t nStringPos = 0;
	const std::size_t LineLen = Line.size();

	std::vector<md3_char8> MeshName;
	std::vector<md3_char8> SkinPath;

	for (std::size_t i = 0; i < LineLen; i++, nStringPos++)
	{
		if (Line[i] == ',')
		{
			i++;
			nStringPos = 0;
			bSecondPart = true;
		}

		if (!bSecondPart)
		{
			MeshName.push_back(Line[i]);
		}
		else
		{
			SkinPath.push_back(Line[i]);
		}
	}

	MeshName.push_back('\0');
	SkinPath.push_back('\0');

	Out.MeshName = MeshName.data();
	Out.SkinPath = SkinPath.data();
}
