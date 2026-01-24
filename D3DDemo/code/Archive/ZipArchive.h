// (c) Beem Media. All rights reserved.

#pragma once

#include <memory>
#include <string>
#include <vector>
#include <filesystem>

class CZipArchive
{
public:
	CZipArchive();
	CZipArchive(const std::filesystem::path& Path);
	~CZipArchive();

private:
	class CInternal;

private:
	std::unique_ptr<CInternal> m_Internal;
};
