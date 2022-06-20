#include "stdafx.h"
#include "GameStats.h"

#include <numeric>

bool GameStats::m_IsMeasuring = false;
bool GameStats::m_ResetPending = false;
bool GameStats::m_InterimUpdated = true;
std::chrono::time_point<std::chrono::steady_clock> GameStats::m_FrameStart = {};
std::chrono::time_point<std::chrono::steady_clock> GameStats::m_InterimStart = {};
int GameStats::m_FrameTimingCount = 20; 
float GameStats::m_InterimDelay = 1.f;
std::deque<float> GameStats::m_FrameMsTimings = {};
PerfStats GameStats::m_Stats = {};

void GameStats::BeginFrame()
{
	if(m_IsMeasuring)
	{
		Logger::LogWarning(L"GameStats::BeginFrame >> EndFrame not called. Invalid measurement.");
	}

	if(m_ResetPending)
	{
		m_Stats.Reset();
		m_InterimUpdated = true;
		m_FrameMsTimings.clear();
		m_ResetPending = false;
	}

	m_FrameStart = std::chrono::steady_clock::now();
	m_IsMeasuring = true;

	if(m_InterimUpdated)
	{
		m_InterimStart = m_FrameStart;
		m_InterimUpdated = false;
	}

	++m_Stats.frameNr;
}

void GameStats::EndFrame()
{
#pragma warning(disable:4244)
	if(!m_IsMeasuring)
	{
		Logger::LogWarning(L"GameStats::EndFrame >> BeginFrame not called. Invalid measurement.");
		return;
	}

	const auto frameEnd = std::chrono::steady_clock::now();
	const std::chrono::duration<float> elapsedDuration = frameEnd - m_FrameStart;
	const auto elapsedSeconds = elapsedDuration.count();
	const auto elapsedMs = 1000 * elapsedSeconds;

	m_FrameMsTimings.push_back(elapsedMs);

	if(m_FrameMsTimings.size() > m_FrameTimingCount)
	{
		m_FrameMsTimings.pop_front();
	}

	//auto temp = std::accumulate(m_FrameTimings.begin(), m_FrameTimings.end(), 0) / float(m_FrameTimings.size());
	m_Stats.averageMs = std::accumulate(m_FrameMsTimings.begin(), m_FrameMsTimings.end(), 0.f) / float(m_FrameMsTimings.size());
	m_Stats.averageFps = 1000.f / m_Stats.averageMs;
	if (elapsedMs > m_Stats.highMs) m_Stats.highMs = elapsedMs;
	if (elapsedMs < m_Stats.lowMs) m_Stats.lowMs = elapsedMs;

#pragma warning(disable:4189)
	const auto lastUpdate = std::chrono::duration_cast<std::chrono::seconds>(frameEnd - m_InterimStart).count();
	if(lastUpdate > m_InterimDelay)
	{
		m_InterimUpdated = true;
		m_Stats.averageFps_interim = m_Stats.averageFps;
		m_Stats.averageMs_interim = m_Stats.averageMs;
	}

	m_IsMeasuring = false;
}

void GameStats::Reset()
{
	m_ResetPending = true;
}
