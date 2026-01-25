// (c) Beem Media. All rights reserved.

#include "ZipArchive.h"
#include "../3rdParty/miniz/miniz.h"

#include <fstream>

class CZipArchive::CInternal
{
public:
	void LoadArchive(const std::filesystem::path& Path)
	{
		UnloadArchive();

		if (!std::filesystem::is_regular_file(Path))
		{
			return;
		}

		const std::size_t FileSize = std::filesystem::file_size(Path);

		std::ifstream InStream(Path, std::ios::binary);
		if (!InStream)
		{
			return;
		}

		m_ArchiveData.resize(FileSize);
		if (m_ArchiveData.size() != FileSize)
		{
			return;
		}

		InStream.read(reinterpret_cast<char*>(m_ArchiveData.data()), m_ArchiveData.size());

		mz_zip_reader_init_mem(&m_Archive, m_ArchiveData.data(), m_ArchiveData.size(), 0);
	}

	void UnloadArchive()
	{
		mz_zip_reader_end(&m_Archive);
		m_Archive = { };
		m_ArchiveData.resize(0);
	}

	std::size_t GetNumFiles() const
	{
		const mz_uint NumFiles = mz_zip_reader_get_num_files(&m_Archive);
		return NumFiles;
	}

	SZipFileMetaData GetFileMetaData(std::size_t Index) const
	{
		SZipFileMetaData Out;

		if (mz_zip_reader_is_file_a_directory(&m_Archive, static_cast<mz_uint>(Index)) == MZ_TRUE)
		{
			// We don't care about directories.
			return Out;
		}

		if (mz_zip_reader_is_file_supported(&m_Archive, static_cast<mz_uint>(Index)) == MZ_FALSE)
		{
			// Not supported so we can't use it.
			return Out;
		}

		mz_zip_archive_file_stat FileStat = { };
		if (mz_zip_reader_file_stat(&m_Archive, static_cast<mz_uint>(Index), &FileStat) == MZ_FALSE)
		{
			// Couldn't read stats so we can't use it.
			return Out;
		}

		Out.Filename = FileStat.m_filename;
		Out.FileSize = FileStat.m_uncomp_size;

		return Out;
	}

	std::vector<za_byte> LoadFileData(std::size_t Index) const
	{
		std::vector<za_byte> Out;

		if (mz_zip_reader_is_file_a_directory(&m_Archive, static_cast<mz_uint>(Index)) == MZ_TRUE)
		{
			// We don't care about directories.
			return Out;
		}

		if (mz_zip_reader_is_file_supported(&m_Archive, static_cast<mz_uint>(Index)) == MZ_FALSE)
		{
			// Not supported so we can't use it.
			return Out;
		}

		mz_zip_archive_file_stat FileStat = { };
		if (mz_zip_reader_file_stat(&m_Archive, static_cast<mz_uint>(Index), &FileStat) == MZ_FALSE)
		{
			// Couldn't read stats so we can't use it.
			return Out;
		}

		Out.resize(FileStat.m_uncomp_size);
		if (Out.size() != FileStat.m_uncomp_size)
		{
			// Couldn't allocate memory.
			return Out;
		}

		const mz_bool ExtractRes = mz_zip_reader_extract_file_to_mem(
			&m_Archive,
			FileStat.m_filename,
			Out.data(),
			Out.size(),
			0);

		if (ExtractRes == MZ_FALSE)
		{
			Out.resize(0);
			Out.shrink_to_fit();
		}

		return Out;
	}

private:
	mutable mz_zip_archive m_Archive = { };
	std::vector<za_byte> m_ArchiveData;
};

CZipArchive::CZipArchive()
{
	m_Internal = std::make_unique<CInternal>();
}

CZipArchive::CZipArchive(const std::filesystem::path& Path)
	: CZipArchive()
{
	m_Internal->LoadArchive(Path);
}

CZipArchive::~CZipArchive()
{
	m_Internal->UnloadArchive();
	m_Internal.reset();
}

std::size_t CZipArchive::GetNumFiles() const
{
	return m_Internal->GetNumFiles();
}

SZipFileMetaData CZipArchive::GetFileMetaData(std::size_t Index) const
{
	return m_Internal->GetFileMetaData(Index);
}

std::vector<CZipArchive::za_byte> CZipArchive::LoadFileData(std::size_t Index) const
{
	return m_Internal->LoadFileData(Index);
}
