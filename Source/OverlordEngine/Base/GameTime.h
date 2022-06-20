#pragma once
class GameTime
{
public:
	GameTime(const GameTime& other) = delete;
	GameTime(GameTime&& other) noexcept = delete;
	GameTime& operator=(const GameTime& other) = delete;
	GameTime& operator=(GameTime&& other) noexcept = delete;

	GameTime();
	~GameTime() = default;

	void Reset();
	void Update();

	float GetElapsed() const { return m_ElapsedGameTime; }
	float GetTotal() const { return m_TotalGameTime; }
	int GetFPS() const { return m_FPS; }
	void ForceElapsedUpperbound(bool force, float upperBound = 0.03f){m_ForceElapsedUpperBound = force; m_ElapsedUpperBound = upperBound;}
	bool IsRunning() const { return !m_IsStopped; }

	void Start();
	void Stop();

private:

	float m_TotalGameTime;
	float m_ElapsedGameTime;
	float m_SecondsPerCount;

	bool m_ForceElapsedUpperBound;
	float m_ElapsedUpperBound;

	__int64 m_BaseTime;
	__int64 m_PausedTime;
	__int64 m_StopTime;
	__int64 m_PrevTime;
	__int64 m_CurrTime;

	bool m_IsStopped;

	int m_FPS;
	float m_FpsTimer;
	int m_FpsCount;
};

