#pragma once
#include <chrono>
#include <deque>

class GameStats final
{
public:
	static void BeginFrame();
	static void EndFrame();
	static void Reset();
	static const PerfStats& GetStats() { return m_Stats; }

private:
	static bool m_IsMeasuring;
	static bool m_ResetPending;
	static bool m_InterimUpdated;
	static std::chrono::time_point<std::chrono::steady_clock> m_FrameStart;
	static std::chrono::time_point<std::chrono::steady_clock> m_InterimStart;
	static int m_FrameTimingCount;
	static float m_InterimDelay;
	static std::deque<float> m_FrameMsTimings;

	static PerfStats m_Stats;
};

