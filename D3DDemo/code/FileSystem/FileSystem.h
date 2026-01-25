// (c) 2025 Beem Media. All rights reserved.

#pragma once

#include <cstdint>
#include <filesystem>
#include <map>
#include <memory>

class CFileSystem
{
public:
	using fs_byte = std::uint8_t;
	static_assert(sizeof(fs_byte) == 1, "Wrong size.");

public:
	static void InitFileSystem();
	static void DeinitFileSystem();
	static CFileSystem& Get();

private:
	static std::unique_ptr<CFileSystem> s_FileSystem;

public:
	void AutoMount();
	void MountFile(const std::filesystem::path& Filename);
	void MountPK3(const std::filesystem::path& Filename);
	std::vector<fs_byte> LoadFile(const std::filesystem::path& Filename) const;

private:
	enum class mount_type
	{
		DiskFile,
		PackageFile,
	};

	struct SMountData
	{
		std::filesystem::path Filename;
		std::filesystem::path DiskFile;
		mount_type MountType = mount_type::DiskFile;
		std::size_t SubIndex = 0;
	};

	std::map<std::filesystem::path, SMountData> m_MountData;

private:
	static std::filesystem::path GetAdjustedFilename(const std::filesystem::path& In);
};
