#pragma once
enum class LogLevel : UINT
{
	Debug = 0x1,
	Info = 0x2,
	Warning = 0x4,
	Error = 0x8,
	Todo = 0x16
};

struct LogString
{
	enum class LogStringType
	{
		Default,
		HResult,
		PhysX,
		Fmod
	};

	LogStringType type{LogStringType::Default};
	HRESULT hresult{};
	FMOD_RESULT fmodResult{};
	PxErrorCode::Enum pxErrorCode{};

	std::wstring_view format{};
	std::source_location location{};

#pragma warning(push)
#pragma warning(disable:26812)
	LogString(PxErrorCode::Enum _pxErrorCode, const std::wstring& _format, const std::source_location& _location = std::source_location::current()) :
		pxErrorCode{ _pxErrorCode }, format{ _format }, location(_location), type{ LogStringType::PhysX }{}
#pragma warning(pop)

	LogString(HRESULT _hresult, const std::wstring& _format, const std::source_location& _location = std::source_location::current()) :
		hresult{ _hresult }, format{_format}, location(_location), type{ LogStringType::HResult }{}

	LogString(HRESULT _hresult, const std::source_location& _location = std::source_location::current()) :
		hresult{ _hresult }, location(_location), type{LogStringType::HResult}{}

	LogString(FMOD_RESULT _fmodResult, const std::source_location& _location = std::source_location::current()) :
		fmodResult{ _fmodResult }, location(_location), type{ LogStringType::Fmod }{}

	LogString(std::wstringstream::_Mystr _format, const std::source_location& _location = std::source_location::current()) :
		format{ _format }, location(_location){}

	LogString(const wchar_t* _format, const std::source_location& _location = std::source_location::current()) :
		format{ _format }, location(_location){}

	LogString(const std::wstring_view& _format, const std::source_location& _location = std::source_location::current()) :
		format{_format}, location(_location){}
};

class Logger final
{
#pragma region BaseLoggers
	//nested classes
	class BaseLogger
	{
	protected:
		std::wostream* m_os = nullptr;
	public:
		BaseLogger() = default;
		virtual ~BaseLogger() = default;

		virtual void Log(const std::wstring& message, bool appendTimestamp = false)
		{
			if(appendTimestamp)
			{
				SYSTEMTIME st;
				GetSystemTime(&st);
				(*m_os) << L"[" << st.wYear << L"-" << st.wMonth << L"-" << st.wDay << L" - ";
				(*m_os) << st.wHour << L":" << st.wMinute << L":" << st.wSecond << L":" << st.wMilliseconds << L"] ";
			}

			(*m_os) << message;
			m_os->flush();
		}
	};

	class FileLogger final: public BaseLogger
	{
		std::wstring m_filename;
	public:
		FileLogger(const FileLogger& other) = delete;
		FileLogger(FileLogger&& other) noexcept = delete;
		FileLogger& operator=(const FileLogger& other) = delete;
		FileLogger& operator=(FileLogger&& other) noexcept = delete;

		explicit FileLogger(const std::wstring& fileName)
			: m_filename(fileName)
		{
			m_os = new std::wofstream(m_filename.c_str());
		}

		~FileLogger() override
		{
			if (m_os)
			{
				std::wofstream* of = static_cast<std::wofstream*>(m_os);
				of->close();
				delete m_os;
			}
		}
	};

	class ConsoleLogger final: public BaseLogger
	{
	public:
		ConsoleLogger()
		{
			m_os = &std::wcout;
		}
	};
#pragma endregion
public:
	~Logger() = delete;
	Logger(const Logger& other) = delete;
	Logger(Logger&& other) noexcept = delete;
	Logger& operator=(const Logger& other) = delete;
	Logger& operator=(Logger&& other) noexcept = delete;

	static void Initialize();
	static void Release();
	static int StartPerformanceTimer();
	static double StopPerformanceTimer(int timerId);

	template <typename ... Args>
	static void LogDebug(const LogString& fmt, Args&&... args);

	template <typename ... Args>
	static void LogWarning(const LogString& fmt, Args&&... args);

	template <typename ... Args>
	static void LogInfo(const LogString& fmt, Args&&... args);

	template <typename ... Args>
	static bool LogError(const LogString& fmt, Args&&... args);

	template <typename ... Args>
	static void LogTodo(const LogString& fmt, Args&&... args);

	static void ClearConsole();
	static void AppendTimestamp(bool append) { m_AppendTimestamp = append; }
	static void StartFileLogging(const std::wstring& fileName);
	static void StopFileLogging();

private:
	Logger() = default;

	static bool ProcessLog(LogLevel level, const LogString& fmt, std::wformat_args args);
	static std::wstring ProcessError(const LogString& fmt, const std::wstring& msg, const std::wstring& filename, const std::wstring& functionName);

	//TIMER 
	static double m_PcFreq;
	static constexpr int MAX_PERFORMANCE_TIMERS{ 10 };
	static __int64 m_PerformanceTimerArr[MAX_PERFORMANCE_TIMERS];
	static bool m_AppendTimestamp;

	static HANDLE m_ConsoleHandle;

	static ConsoleLogger* m_ConsoleLogger;
	static FileLogger* m_FileLogger;

	static std::map<LogLevel, std::wstring> m_LevelToStr;
	static std::map<LogLevel, WORD> m_LevelToConsoleStyle;
};

template <typename ... Args>
void Logger::LogInfo(const LogString& fmt, Args&&... args)
{
	ProcessLog(LogLevel::Info, fmt, std::make_wformat_args(args...));
}

template <typename ... Args>
void Logger::LogDebug(const LogString& fmt, Args&&... args)
{
	ProcessLog(LogLevel::Debug, fmt, std::make_wformat_args(args...));
}

template <typename ... Args>
void Logger::LogWarning(const LogString& fmt, Args&&... args)
{
	ProcessLog(LogLevel::Warning, fmt, std::make_wformat_args(args...));
}

template <typename ... Args>
bool Logger::LogError(const LogString& fmt, Args&&... args)
{
	return ProcessLog(LogLevel::Error, fmt, std::make_wformat_args(args...));
}

template <typename ... Args>
void Logger::LogTodo(const LogString& fmt, Args&&... args)
{
	ProcessLog(LogLevel::Todo, fmt, std::make_wformat_args(args...));
}
