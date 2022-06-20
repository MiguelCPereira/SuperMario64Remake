
#include "stdafx.h"

#include "Logger.h"

#include <fcntl.h>
#include <iomanip>
#include <comdef.h>

namespace fs = std::filesystem;

__int64 Logger::m_PerformanceTimerArr[] = { 0 };
double Logger::m_PcFreq = 0.0;
HANDLE Logger::m_ConsoleHandle = nullptr;

Logger::ConsoleLogger* Logger::m_ConsoleLogger = nullptr;
Logger::FileLogger* Logger::m_FileLogger = nullptr;
bool Logger::m_AppendTimestamp = false;

std::map<LogLevel, std::wstring> Logger::m_LevelToStr = {
		{LogLevel::Debug, L"DEBUG"},
		{LogLevel::Info, L"INFO"},
		{LogLevel::Warning, L"WARNING"},
		{LogLevel::Error, L"ERROR"},
		{LogLevel::Todo, L"TODO"},
};
std::map<LogLevel,WORD> Logger::m_LevelToConsoleStyle = {
		{LogLevel::Debug, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE},
		{LogLevel::Info, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE},
		{LogLevel::Warning, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN},
		{LogLevel::Error, FOREGROUND_INTENSITY | FOREGROUND_RED},
		{LogLevel::Todo, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE},
};

void Logger::Initialize()
{
	for (int i = 0; i < MAX_PERFORMANCE_TIMERS; ++i)
		m_PerformanceTimerArr[i] = -1;

	LARGE_INTEGER li;
	if (QueryPerformanceFrequency(&li))
	{
		m_PcFreq = double(li.QuadPart) / 1000.0;
		//LOG ERROR
	}

#if defined(DEBUG) | defined(_DEBUG)
	if (AllocConsole())
	{
		// Redirect the CRT standard input, output, and error handles to the console
		FILE* pCout;
		freopen_s(&pCout, "CONIN$", "r", stdin);
		freopen_s(&pCout, "CONOUT$", "w", stdout);
		freopen_s(&pCout, "CONOUT$", "w", stderr);

		//Clear the error state for each of the C++ standard stream objects. We need to do this, as
		//attempts to access the standard streams before they refer to a valid target will cause the
		//iostream objects to enter an error state. In versions of Visual Studio after 2005, this seems
		//to always occur during startup regardless of whether anything has been read from or written to
		//the console or not.
		std::wcout.clear();
		std::cout.clear();
		std::wcerr.clear();
		std::cerr.clear();
		std::wcin.clear();
		std::cin.clear();
		std::cin.clear();

		//Set ConsoleHandle
		m_ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		m_ConsoleLogger = new ConsoleLogger();

		//Disable Close-Button
		HWND hwnd = GetConsoleWindow();
		if (hwnd != NULL)
		{
			HMENU hMenu = GetSystemMenu(hwnd, FALSE);
			if (hMenu != NULL) DeleteMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);
		}
	}
#endif
}

void Logger::Release()
{
	SafeDelete(m_ConsoleLogger);
	SafeDelete(m_FileLogger);
}

int Logger::StartPerformanceTimer()
{
	int counter = 0;
	while (m_PerformanceTimerArr[counter] != -1)
	{
		++counter;
		if (counter == MAX_PERFORMANCE_TIMERS)
		{
			counter = -1;
			break;
		}
	}

	if (counter >= 0)
	{
		LARGE_INTEGER li;
		QueryPerformanceCounter(&li);
		m_PerformanceTimerArr[counter] = li.QuadPart;
	}

	return counter;
}

double Logger::StopPerformanceTimer(int timerId)
{
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	const double diff = double(li.QuadPart - m_PerformanceTimerArr[timerId]) / m_PcFreq;

	m_PerformanceTimerArr[timerId] = -1;

	return diff;
}

void Logger::ClearConsole()
{
	std::system("cls");
}

void Logger::StartFileLogging(const std::wstring& fileName)
{
	SafeDelete(m_FileLogger);

	m_FileLogger = new FileLogger(fileName);
}

void Logger::StopFileLogging()
{
	SafeDelete(m_FileLogger);
}

bool Logger::ProcessLog(LogLevel level, const LogString& fmt, std::wformat_args args)
{
	//Validate True Error
	if(level == LogLevel::Error) //Skip if non-error
	{
		if (fmt.type == LogString::LogStringType::Fmod && fmt.fmodResult == FMOD_OK) return false;
		if (fmt.type == LogString::LogStringType::HResult && SUCCEEDED(fmt.hresult)) return false;
	}

	//Skip Debug/T0d0 message in release build
#ifdef NDEBUG
	if (level == LogLevel::Debug || level == LogLevel::Todo) return false;
#endif

	//Gather source intel
	const auto& levelStr = m_LevelToStr[level];
	const auto filename = fs::path{ fmt.location.file_name() }.filename().wstring();
	const auto functionName = StringUtil::utf8_decode(fmt.location.function_name());

	//Generate Message
	std::wstring logMsg{ std::vformat(fmt.format, args)}; //DEFAULT FORMATTING
	if (level == LogLevel::Error) logMsg = ProcessError(fmt, logMsg, filename, functionName);

	const auto full_log = std::format(L"[{}] @ {}::{} (line {})\n **{}\n\n", levelStr, filename, functionName, fmt.location.line(), logMsg);


	//Console Log
	if (m_ConsoleLogger)
	{
		SetConsoleTextAttribute(m_ConsoleHandle, m_LevelToConsoleStyle[level]);
		m_ConsoleLogger->Log(full_log);
	}

	//File Log
	if(m_FileLogger)
	{
		m_FileLogger->Log(full_log, true);
	}

	//Show MessageBox
	if (level == LogLevel::Error)
	{
		MessageBox(0, logMsg.c_str(), L"ERROR", MB_OK | MB_ICONERROR);
		
#ifdef NDEBUG
		//Shutdown if Release Build
		exit(-1);
#endif
	}

	return true;
}

std::wstring Logger::ProcessError(const LogString& fmt, const std::wstring& msg, const std::wstring& filename, const std::wstring& functionName)
{
	std::wstringstream ss{};

	switch(fmt.type)
	{
	case LogString::LogStringType::HResult:
	{
		_com_error err{ fmt.hresult };
		ss << "A HResult error was reported!\n\n";
		ss << std::left << std::setw(15) << "File:" << filename << std::endl;
		ss << std::left << std::setw(15) << "Function:" << functionName << " (line " << fmt.location.line() << ")" << std::endl;
		ss << std::left << std::setw(15) << "Error Code:" << "0x" << std::hex << fmt.hresult << std::endl;
		ss << std::left << std::setw(15) << "Description:" << err.ErrorMessage() << std::endl;

		if(!msg.empty())
		{
			ss << std::left << std::setw(20) << "\nInfo: " << msg;
		}
	}
		break;
	case LogString::LogStringType::Fmod:
	{
		ss << "A FMOD error was reported!\n\n";
		ss << std::left << std::setw(15) << "File:" << filename << std::endl;
		ss << std::left << std::setw(15) << "Function:" << functionName << " (line " << fmt.location.line() << ")" << std::endl;
		ss << std::left << std::setw(15) << "Error Code:" << fmt.fmodResult << std::endl;
		ss << std::left << std::setw(15) << "Description:" << FMOD_ErrorString(fmt.fmodResult);
	}
		break;
	case LogString::LogStringType::PhysX:
	{
		ss << "A PHYSX error was reported!\n\n";
		ss << msg;
	}
		break;
	default:
	{
		ss << "An error was reported!\n\n";
		ss << std::left << std::setw(15) << L"File:" << filename << std::endl;
		ss << std::left << std::setw(15) << L"Function:" << functionName << L" (line " << fmt.location.line() << L")" << std::endl;
		ss << std::left << std::setw(15) << L"Error Msg:" << msg;
	}
		break;
	}

	return ss.str();
}