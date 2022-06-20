#include "stdafx.h"
#include "BinaryReader.h"

BinaryReader::~BinaryReader(void)
{
	Close();
}

std::wstring BinaryReader::ReadLongString()
{
	ASSERT_IF(m_pReader == nullptr, L"BinaryReader doesn't exist!\nUnable to read binary data...");
	const auto stringLength = Read<UINT>();
	
 std::wstringstream ss;
	for(UINT i=0; i<stringLength; ++i)
	{
		ss<<Read<wchar_t>();
	}

	return (std::wstring)ss.str();
}

std::wstring BinaryReader::ReadNullString()
{
	ASSERT_IF(!m_pReader, L"BinaryReader doesn't exist!\nUnable to read binary data...");

	if (m_pReader) {
		std::string buff{};
		getline(*m_pReader, buff, '\0');

		return std::wstring(buff.begin(), buff.end());
	}//Prevent C6011

	return L""; 
}


std::wstring BinaryReader::ReadString()
{
	ASSERT_IF(m_pReader == nullptr, L"BinaryReader doesn't exist!\nUnable to read binary data...");
	const int stringLength = (int)Read<char>();
	
 std::wstringstream ss;
	for(int i=0; i<stringLength; ++i)
	{
		ss<<Read<char>();
	}

	return (std::wstring)ss.str();
}

void BinaryReader::Open(const std::wstring& binaryFile)
{
	Close();

	auto temp = new std::ifstream();
	temp->open(binaryFile, std::ios::in| std::ios::binary);
	if(temp->is_open())
	{
		m_pReader = temp;
		m_Exists = true;
	}
	else
	{
		Logger::LogWarning(L"Failed to open the file!\n\nFilepath: {}", binaryFile);
		Close();
	}
}

void BinaryReader::Open(char* s, UINT32 size)
{
	Close();

	std::string data(s, size);
	m_pReader = new std::istringstream(data);
	m_Exists = true;
}

void BinaryReader::Close()
{
	SafeDelete(m_pReader);
	m_Exists = false;
}

int BinaryReader::GetBufferPosition() const
{
	if(m_pReader)
	{
		return int(m_pReader->tellg());
	}

	Logger::LogWarning(L"m_pReader doesn't exist");
	return -1;
}

bool BinaryReader::SetBufferPosition(int pos)
{
	if(m_pReader)
	{
		m_pReader->seekg(pos);
		return true;
	}

	Logger::LogWarning(L"m_pReader doesn't exist");
	return false;
}

bool BinaryReader::MoveBufferPosition(int move)
{
	const auto currPos = GetBufferPosition();
	if(currPos>0)
	{
		return SetBufferPosition(currPos + move);
	}

	return false;
}
