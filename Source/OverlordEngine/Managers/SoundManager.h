#pragma once
class SoundManager final: public Singleton<SoundManager>
{
public:
	SoundManager(const SoundManager& other) = delete;
	SoundManager(SoundManager&& other) noexcept = delete;
	SoundManager& operator=(const SoundManager& other) = delete;
	SoundManager& operator=(SoundManager&& other) noexcept = delete;

	FMOD::System* GetSystem() const { return m_pFmodSystem; }

protected:
	void Initialize() override;

private:
	friend class Singleton<SoundManager>;
	SoundManager();
	~SoundManager();

	FMOD::System* m_pFmodSystem = nullptr;
};

