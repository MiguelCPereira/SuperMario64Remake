#include "stdafx.h"
#include "GameTime.h"

GameTime::GameTime():
		m_TotalGameTime(0.0f),
		m_ElapsedGameTime(0.0f),
		m_SecondsPerCount(0.0f),
		m_ForceElapsedUpperBound(false),
		m_ElapsedUpperBound(0.03f),
		m_BaseTime(0),
		m_PausedTime(0),
		m_StopTime(0),
		m_PrevTime(0),
		m_CurrTime(0),
		m_IsStopped(true),
		m_FPS(0),
		m_FpsTimer(0.0f),
		m_FpsCount(0)
{
	__int64 countsPerSecond{};
	QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&countsPerSecond));
	m_SecondsPerCount = 1.0f / static_cast<float>(countsPerSecond);
}

void GameTime::Reset()
{
	__int64 currTime{};
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currTime));

	m_BaseTime = currTime;
	m_PrevTime = currTime;
	m_StopTime = 0;
	m_FpsTimer = 0.0f;
	m_FpsCount = 0;
	m_IsStopped = false;
}

void GameTime::Update()
{
	if(m_IsStopped)
	{
		m_FPS = 0;
		m_ElapsedGameTime = 0.0f;
		m_TotalGameTime = static_cast<float>((m_StopTime - m_PausedTime - m_BaseTime) * m_BaseTime);
		return;
	}

	__int64 currTime{};
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currTime));
	m_CurrTime = currTime;

	m_ElapsedGameTime = static_cast<float>((m_CurrTime - m_PrevTime) * m_SecondsPerCount);
	m_PrevTime = m_CurrTime;

	if(m_ElapsedGameTime < 0.0f)
		m_ElapsedGameTime = 0.0f;

	if(m_ForceElapsedUpperBound && m_ElapsedGameTime > m_ElapsedUpperBound)
	{
		m_ElapsedGameTime = m_ElapsedUpperBound;
	}

	m_TotalGameTime = static_cast<float>(((m_CurrTime - m_PausedTime) - m_BaseTime) * m_SecondsPerCount);

	//FPS LOGIC
	m_FpsTimer += m_ElapsedGameTime;
	++m_FpsCount;
	if(m_FpsTimer >= 1.0f)
	{
		m_FPS = m_FpsCount;
		m_FpsCount = 0;
		m_FpsTimer -= 1.0f;
	}
}

void GameTime::Start()
{
	if(m_IsStopped)
	{
		__int64 startTime{};
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&startTime));

		m_PausedTime += (startTime - m_StopTime);

		m_PrevTime = startTime;
		m_StopTime = 0;
		m_IsStopped = false;
	}
}

void GameTime::Stop()
{
	if(!m_IsStopped)
	{
		__int64 currTime{};
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currTime));

		m_StopTime = currTime;
		m_IsStopped = true;
	}
}