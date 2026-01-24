// (c) 2025 Beem Media. All rights reserved.

#pragma once

#include <filesystem>

class CDataStream
{
public:
	using ds_byte = std::uint8_t;
	static_assert(sizeof(ds_byte) == 1, "A byte must have a size of 1.");

private:
	std::vector<ds_byte> m_Data;
	std::size_t m_ReadPtr = 0;

public:
	enum MOVE_T
	{
		MOVE_START = 0,
		MOVE_END = 1,
		MOVE_CUR = 2,
	};

public:
	CDataStream() = default;
	CDataStream(const std::filesystem::path& Filename);
	CDataStream(const std::vector<ds_byte>& SourceBytes);
	~CDataStream();

	bool Open(const std::filesystem::path& Filename);
	bool Open(const std::vector<ds_byte>& SourceBytes);
	void Close();
	std::size_t Read(void* pBuffer, std::size_t count);
	std::size_t Read(ds_byte* pBuffer, std::size_t count);
	std::size_t GetSize() const;
	std::size_t Seek(signed long nDistance, MOVE_T nMethod);
	std::size_t SeekFromStart(signed long Distance) { return Seek(Distance, MOVE_START); }
	std::size_t SeekFromEnd(signed long Distance) { return Seek(Distance, MOVE_END); }
	std::size_t SeekFromCurrent(signed long Distance) { return Seek(Distance, MOVE_CUR); }
	std::size_t Tell() const;
};