#pragma once
struct CannonballSpawnInfo;
class MarioCharacter;

struct CannonballDesc
{
	CannonballDesc(
		PxMaterial* pMaterial,
		float radius = 0.864f,
		float height = 0.7296f)
	{
		controller.setToDefault();
		controller.radius = radius;
		controller.height = height;
		controller.material = pMaterial;
	}

	float maxMoveSpeed{ 8.f };
	float maxFallSpeed{ 15.f };

	float moveAccelerationTime{ .5f };
	float fallAccelerationTime{ .3f };

	PxCapsuleControllerDesc controller{};
};

class CannonballCharacter : public GameObject
{
public:
	CannonballCharacter(const CannonballDesc& characterDesc, MarioCharacter* pMario, CannonballSpawnInfo* pSpawnInfo);
	~CannonballCharacter() = default;

	CannonballCharacter(const CannonballCharacter& other) = delete;
	CannonballCharacter(CannonballCharacter&& other) noexcept = delete;
	CannonballCharacter& operator=(const CannonballCharacter& other) = delete;
	CannonballCharacter& operator=(CannonballCharacter&& other) noexcept = delete;

	void Reset();
	void TogglePause(bool paused);
	void ToggleSoundFadeOut(float fadeTime);

protected:
	void Initialize(const SceneContext&) override;
	void Update(const SceneContext&) override;

private:
	ControllerComponent* m_pControllerComponent{};

	CannonballDesc m_CannonballDesc;
	float m_MoveAcceleration{}, m_FallAcceleration{}, m_MoveSpeed{};

	XMFLOAT3 m_TotalVelocity{};


	// Sound
	FMOD::Channel* m_pSFXChannel{};
	FMOD::Sound* m_pMovingSound{};
	const float m_SoundStartDistance{ 60.f };
	const float m_VolumeMultiplier{ 8.f };
	float m_CurrentSFXVol{ 1.f };
	bool m_SFXPlaying{};


	// General Logic
	MarioCharacter* m_pMario{};
	std::vector<XMFLOAT3> m_TargetPositions{};
	int m_CurrentTargetPos{};
	const float m_DamageDistance{ 5.472f };
	const float m_SeekErrorMargin{ 0.8f };
	bool m_InvertedDir{};
	bool m_Paused{};


	// Sound Fade Out Logic
	bool m_SoundFadingOut{};
	float m_SoundTotalFadeTime{};
	float m_SoundFadeOutCounter{};
	void UpdateSoundFadeOut(float elapsedTime);


	CannonballSpawnInfo* m_pSpawnInfo;


	void SeekNextTarget(float elapsedTime);
	bool CheckIfGrounded();
	void ResetPath();
	void Update3DSound();
};

