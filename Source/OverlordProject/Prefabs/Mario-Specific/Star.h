#pragma once
class MarioCharacter;

class Star : public GameObject
{
public:
	Star(const XMFLOAT3& position, MarioCharacter* pMario);
	~Star() = default;

	Star(const Star& other) = delete;
	Star(Star&& other) noexcept = delete;
	Star& operator=(const Star& other) = delete;
	Star& operator=(Star&& other) noexcept = delete;

	void Reset();
	void ToggleSoundFadeOut(float fadeTime);

protected:
	void Initialize(const SceneContext&) override;
	void Update(const SceneContext&) override;

private:
	MarioCharacter* m_pMario{};
	const XMFLOAT3 m_OriginalPosition{};
	const float m_TriggerDistance{ 2.f };
	float m_TotalRotation{ 0.f };
	const float m_RotationSpeed{ 135.f };
	const float m_WobbleHeight{ 0.5f };
	const float m_WobbleScale{ 0.08f };
	const float m_OriginalSize{ 0.7f };
	GameObject* m_pModelGO{};


	bool m_Captured{};
	float m_CapturedSize{ 0.5f };
	float m_CapturedPosY{};
	const float m_TimeShrinkingBeforeEnd{ 1.f };
	float m_PostCaptureCounter{};
	const float m_CapturedHeightIncrement{ 2.f };


	// Sound Logic
	FMOD::Sound* m_pIdleSound{};
	FMOD::Channel* m_pIdleChannel{};
	FMOD::Sound* m_pPickedUpSound{};
	FMOD::Channel* m_pPickedUpChannel{};
	const float m_SoundStartDistance{ 25.f };
	const float m_IdleMaxVolume{ 0.1f };
	const float m_PickedUpMaxVolume{ 3.f };
	bool m_IdlePlaying{};
	float m_CurrentIdleVol{ 1.f };
	float m_CurrentPickedUpVol{ 1.f };


	// Sound Fade Out Logic
	bool m_SoundFadingOut{};
	float m_SoundTotalFadeTime{};
	float m_SoundFadeOutCounter{};
	void UpdateSoundFadeOut(float elapsedTime);


	void Update3DSounds();
};

