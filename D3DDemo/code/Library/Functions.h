// (c) Beem Media. All rights reserved.

#pragma once

#include "MD3Types.h"

class CDataStream;

namespace Functions
{
	std::vector<std::string> ReadLines(CDataStream& In);
	std::string ReadWordFromLine(const std::string& Line, std::size_t Start, std::size_t* End);

	std::string RemoveDirectoryFromString(const std::string& In);
	std::string RemoveFilenameFromString(const std::string& In);
}
