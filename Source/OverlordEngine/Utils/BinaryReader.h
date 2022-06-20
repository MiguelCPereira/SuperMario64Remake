#pragma once
#include "Base/Logger.h"

class BinaryReader final
{
public:
	BinaryReader() = default;
	~BinaryReader();
	BinaryReader(const BinaryReader& other) = delete;
	BinaryReader(BinaryReader&& other) noexcept = delete;
	BinaryReader& operator=(const BinaryReader& other) = delete;
	BinaryReader& operator=(BinaryReader&& other) noexcept = delete;


	template<class T>
	T Read()
	{
		ASSERT_IF(m_pReader == nullptr, L"BinaryReader doesn't exist!\nUnable to read binary data...");

		T value;
		m_pReader->read((char*)&value, sizeof(T));
		return value;
	}

 std::wstring ReadString();
 std::wstring ReadLongString();
 std::wstring ReadNullString();

	int GetBufferPosition() const;
	bool SetBufferPosition(int pos);
	bool MoveBufferPosition(int move);
	bool Exists() const { return m_Exists; }

	void Open(const std::wstring& binaryFile);
	void Open(char* s, UINT32 size);
	void Close();

private: 

	bool m_Exists{};
	std::istream* m_pReader{nullptr};
};

