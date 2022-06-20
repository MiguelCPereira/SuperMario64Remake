#include "stdafx.h"
#include "PhysxErrorCallback.h"

#pragma warning(push)
#pragma warning(disable: 26812)
void PhysxErrorCallback::reportError(PxErrorCode::Enum code, const char* message, const char* file, int line)
{
	std::wstringstream ss;
	ss << L"PHYSX CALLBACK REPORT:" << std::endl;
	ss << "Error Code: " << code << std::endl;
	ss << "Message: " << message << std::endl;
	ss << "File: " << file << " (line: " << line << ")";

	switch (code)
	{
	case PxErrorCode::eDEBUG_INFO:
		Logger::LogDebug(L"PHYSX Debug Info @ {} (line {})\n\t{}", StringUtil::utf8_decode(file), line, StringUtil::utf8_decode(message));
		break;
	case PxErrorCode::ePERF_WARNING:
	case PxErrorCode::eDEBUG_WARNING:
		Logger::LogWarning(L"PHYSX Warning @ {} (line {})\n\t{}", StringUtil::utf8_decode(file), line, StringUtil::utf8_decode(message));
		break;
	default:
		HANDLE_ERROR(LogString(code, L"File: {} (line {})\n\n{}"), StringUtil::utf8_decode(file), line, StringUtil::utf8_decode(message));
		break;
	}
}
#pragma warning(pop)