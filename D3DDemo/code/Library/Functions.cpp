// (c) Beem Media. All rights reserved.

#include "Functions.h"
#include "FileSystem/DataStream.h"

std::vector<std::string> Functions::ReadLines(CDataStream& In)
{
	std::vector<std::string> Out;

	std::vector<md3_char8> CurWord;

	while (!In.IsEOF())
	{
		const md3_char8 c = In.Read<md3_char8>();
		if (c == '\n')
		{
			CurWord.push_back('\0');
			Out.push_back(CurWord.data());
			CurWord.resize(0);
		}
		else if (c == '\r')
		{
			// Ignore return carriage.
		}
		else
		{
			CurWord.push_back(c);
		}
	}

	if (CurWord.size() > 0)
	{
		CurWord.push_back('\0');
		Out.push_back(CurWord.data());
		CurWord.resize(0);
	}

	return Out;
}

std::string Functions::ReadWordFromLine(const std::string& Line, std::size_t Start, std::size_t* End)
{
	std::vector<md3_char8> Out;

	md3_bool bStartedRead = false;

	const std::size_t StrLen = Line.size();
	std::size_t NumRead = 0;

	for (std::size_t i = Start; i < StrLen; i++)
	{
		NumRead++;

		const bool bIsWhitespace = Line[i] == ' ' || Line[i] == '\t';

		if (bIsWhitespace && bStartedRead)
		{
			// If we've read and hit whitespace, we're done.
			break;
		}

		if (!bIsWhitespace)
		{
			bStartedRead = true;
		}

		if (bStartedRead)
		{
			Out.push_back(Line[i]);
		}
	}

	if (End)
	{
		*End = Start + NumRead;
	}

	Out.push_back('\0');
	return Out.data();
}

std::string Functions::RemoveDirectoryFromString(const std::string& In)
{
	const std::filesystem::path Path = In;

	return Path.filename().string();
}

std::string Functions::RemoveFilenameFromString(const std::string& In)
{
	const std::filesystem::path Path = In;

	return Path.parent_path().string();
}
