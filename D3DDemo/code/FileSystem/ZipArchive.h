// (c) Beem Media. All rights reserved.

#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

struct SZipFileMetaData
{
	std::filesystem::path Filename;
	std::size_t FileSize = 0;

	bool IsValid() const { return Filename.has_filename(); }
};

class CZipArchive
{
public:
	using za_byte = std::uint8_t;
	static_assert(sizeof(za_byte) == 1, "Read size must be one byte.");

public:
	CZipArchive();
	CZipArchive(const std::filesystem::path& Path);
	~CZipArchive();

	std::size_t GetNumFiles() const;
	SZipFileMetaData GetFileMetaData(std::size_t Index) const;
	std::vector<za_byte> LoadFileData(std::size_t Index) const;

private:
	class CInternal;

private:
	std::unique_ptr<CInternal> m_Internal;
};
