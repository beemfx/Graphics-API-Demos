// (c) 2025 Beem Media. All rights reserved.

#include "FileSystem.h"
#include "ZipArchive.h"
#include <cassert>
#include <fstream>

std::unique_ptr<CFileSystem> CFileSystem::s_FileSystem;

void CFileSystem::InitFileSystem()
{
	assert(!s_FileSystem);
	s_FileSystem = std::make_unique<CFileSystem>();
}

void CFileSystem::DeinitFileSystem()
{
	assert(s_FileSystem);
	s_FileSystem.reset(nullptr);
}

CFileSystem& CFileSystem::Get()
{
	assert(s_FileSystem);
	return *s_FileSystem;
}

void CFileSystem::AutoMount()
{
	const std::filesystem::path RootDir = ".";

	for (const std::filesystem::directory_entry& Item : std::filesystem::recursive_directory_iterator(RootDir))
	{
		if (Item.is_regular_file())
		{
			const std::filesystem::path Path = Item.path().lexically_relative(RootDir);
			if (Path.extension() == ".pk3")
			{
				MountPK3(Path);
			}
			else
			{
				MountFile(Path);
			}
		}
	}
}

void CFileSystem::MountFile(const std::filesystem::path& Filename)
{
	SMountData& NewMountData = m_MountData[Filename];
	NewMountData.Filename = Filename;
	NewMountData.DiskFile = Filename;
	NewMountData.MountType = mount_type::DiskFile;
	NewMountData.SubIndex = 0;
}

void CFileSystem::MountPK3(const std::filesystem::path& Filename)
{
	const std::filesystem::path Dir = Filename.parent_path();

	CZipArchive A(Filename);

	const std::size_t NumFiles = A.GetNumFiles();

	for (std::size_t ZipRef = 0; ZipRef < NumFiles; ZipRef++)
	{
		SZipFileMetaData MetaData = A.GetFileMetaData(ZipRef);
		if (MetaData.IsValid())
		{
			SMountData& NewMountData = m_MountData[Filename];
			NewMountData.Filename = (Dir / MetaData.Filename).lexically_normal();
			NewMountData.DiskFile = Filename;
			NewMountData.MountType = mount_type::PackageFile;
			NewMountData.SubIndex = ZipRef;
		}
	}
}

std::vector<CFileSystem::fs_byte> CFileSystem::LoadFile(const std::filesystem::path& Filename) const
{
	const std::filesystem::path AdjFilename = Filename.lexically_normal();

	std::vector<CFileSystem::fs_byte> Out;

	if (m_MountData.contains(AdjFilename))
	{
		const SMountData& MountData = m_MountData.at(AdjFilename);
		switch (MountData.MountType)
		{
		case mount_type::DiskFile:
		{
			const bool bIsFile = std::filesystem::is_regular_file(MountData.DiskFile);
			if (bIsFile)
			{
				const std::uintmax_t FileSize = std::filesystem::file_size(MountData.DiskFile);
				Out.resize(static_cast<std::size_t>(FileSize));
				if (Out.size() == FileSize)
				{
					std::ifstream File(MountData.DiskFile, std::ios::binary);
					File.read(reinterpret_cast<char*>(Out.data()), Out.size());
				}
			}
		} break;
		case mount_type::PackageFile:
		{
			CZipArchive A(MountData.DiskFile);
			Out = A.LoadFileData(MountData.SubIndex);
		} break;
		}
	}

	return Out;
}
