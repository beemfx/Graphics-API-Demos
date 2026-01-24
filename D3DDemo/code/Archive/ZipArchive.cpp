// (c) Beem Media. All rights reserved.

#include "ZipArchive.h"
#include "../3rdParty/miniz/miniz.h"

#include <fstream>

class CZipArchive::CInternal
{
private:
	using za_byte = std::uint8_t;
	static_assert(sizeof(za_byte) == 1, "Read size must be one byte.");

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

		const mz_uint NumFiles = mz_zip_reader_get_num_files(&m_Archive);
		for (mz_uint i = 0; i < NumFiles; i++)
		{
			mz_zip_archive_file_stat FileStat = { };
			if (!mz_zip_reader_file_stat(&m_Archive, i, &FileStat))
			{
				printf("mz_zip_reader_file_stat() failed!\n");
				return;
			}

			if (mz_zip_reader_is_file_a_directory(&m_Archive, i) == MZ_FALSE)
			{
				if (mz_zip_reader_is_file_supported(&m_Archive, i) == MZ_TRUE)
				{
					printf("The filename is: %s", FileStat.m_filename);
				}

				std::vector<za_byte> FileBuffer;
				FileBuffer.resize(FileStat.m_uncomp_size);
				if (FileBuffer.size() == FileStat.m_uncomp_size)
				{
					const mz_bool ExtractRes = mz_zip_reader_extract_file_to_mem(
						&m_Archive,
						FileStat.m_filename,
						FileBuffer.data(),
						FileBuffer.size(),
						0);
					if (ExtractRes == MZ_TRUE)
					{
						printf("File extracted.");
					}
					else
					{
						printf("Could not extract file.");
					}
				}
			}
		}
	}

	void UnloadArchive()
	{
		mz_zip_reader_end(&m_Archive);
		m_Archive = { };
		m_ArchiveData.resize(0);
	}
private:
	mz_zip_archive m_Archive = { };
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
