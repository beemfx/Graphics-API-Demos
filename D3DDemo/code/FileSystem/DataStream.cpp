// (c) 2025 Beem Media. All rights reserved.

#include "DataStream.h"
#include <fstream>

CDataStream::CDataStream(const std::filesystem::path& Filename)
{
	Open(Filename);
}

CDataStream::CDataStream(const std::vector<ds_byte>& SourceBytes)
{
	Open(SourceBytes);
}

CDataStream::~CDataStream()
{
	Close();
}

bool CDataStream::Open(const std::filesystem::path& Filename)
{
	//If a file is already open, close it.
	Close();

	std::filesystem::path FileToOpen = Filename;

	bool bIsFile = std::filesystem::is_regular_file(FileToOpen);

	// We might be running a debug build, in which case, we'll search the data
	// directory for the file.
	if (!bIsFile)
	{
		FileToOpen = std::filesystem::path("..") / "DATA" / Filename;
		bIsFile = std::filesystem::is_regular_file(FileToOpen);
	}

	if (!bIsFile)
	{
		return false;
	}

	const std::uintmax_t FileSize = std::filesystem::file_size(FileToOpen);
	m_Data.resize(static_cast<std::size_t>(FileSize));
	if (m_Data.size() == FileSize)
	{
		std::ifstream File(FileToOpen, std::ios::binary);
		File.read(reinterpret_cast<char*>(m_Data.data()), m_Data.size());
		return true;
	}

	return false;
}

bool CDataStream::Open(const std::vector<ds_byte>& SourceBytes)
{
	m_Data = SourceBytes;
	return m_Data.size() == SourceBytes.size();
}

void CDataStream::Close()
{
	m_Data.resize(0);
	m_ReadPtr = 0;
}

std::size_t CDataStream::Read(void* pBuffer, std::size_t count)
{
	return Read(reinterpret_cast<ds_byte*>(pBuffer), count);
}

std::size_t CDataStream::Read(ds_byte* pBuffer, std::size_t count)
{
	std::size_t SizeToRead = count;

	if ((m_ReadPtr + SizeToRead) > m_Data.size())
	{
		SizeToRead = m_Data.size() - m_ReadPtr;
	}

	std::memcpy(pBuffer, m_Data.data() + m_ReadPtr, SizeToRead);
	m_ReadPtr += SizeToRead;
	return SizeToRead;
}

std::size_t CDataStream::GetSize() const
{
	return m_Data.size();
}

std::size_t CDataStream::Seek(signed long nDistance, MOVE_T nMethod)
{
	switch (nMethod)
	{
	case MOVE_START:
		m_ReadPtr = nDistance;
		break;
	case MOVE_CUR:
		m_ReadPtr += nDistance;
		break;
	case MOVE_END:
		m_ReadPtr = m_Data.size() + nDistance;
		break;
	}

	//Make sure the new data pointer is within the file data.
	m_ReadPtr = std::clamp<std::size_t>(m_ReadPtr, 0, m_Data.size());

	return m_ReadPtr;
}

std::size_t CDataStream::Tell() const
{
	return m_ReadPtr;
}