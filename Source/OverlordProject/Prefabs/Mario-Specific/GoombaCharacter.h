#pragma once

struct SpawnInfo;
class MarioCharacter;

struct GoombaDesc
{
	GoombaDesc(
		PxMaterial* pMaterial,
		float radius = 1.f,
		float height = 0.3f)
	{
		controller.setToDefault();
		controller.radius = radius;
		controller.height = height;
		controller.material = pMaterial;
	}

	float jumpSpeed{ 8.f };
	float maxWanderSpeed{ 1.f };
	float maxChaseSpeed{ 6.f };
	float maxFallSpeed{ 15.f };

	float wanderAccelerationTime{ .3f };
	float chaseAccelerationTime{ .5f };
	float fallAccelerationTime{ .3f };

	float damagedThrowSpeed{ 15.f };

	PxCapsuleControllerDesc controller{};
};

class GoombaCharacter : public GameObject
{
public:
	GoombaCharacter(const GoombaDesc& characterDesc, MarioCharacter* pMario, SpawnInfo* pSpawnInfo);
	~GoombaCharacter() = default;

	GoombaCharacter(const GoombaCharacter& other) = delete;
	GoombaCharacter(GoombaCharacter&& other) noexcept = delete;
	GoombaCharacter& operator=(const GoombaCharacter& other) = delete;
	GoombaCharacter& operator=(GoombaCharacter&& other) noexcept = delete;

	void GetSquished();
	void Reset();
	void TogglePause(bool paused);
	void ToggleSoundFadeOut(float fadeTime);

protected:
	void Initialize(const SceneContext&) override;
	void Update(const SceneContext&) override;

private:
	GameObject* m_pModelParentGO{};
	GameObject* m_pModelGO{};
	ControllerComponent* m_pControllerComponent{};

	GoombaDesc m_GoombaDesc;
	float m_WanderAcceleration{}, m_ChaseAcceleration{},
		m_FallAcceleration{},m_MoveSpeed{};

	XMFLOAT3 m_TotalVelocity{};

	enum GoombaState
	{
		Wandering,
		AlertJump,
		Chasing,
		Punched,
		Paused
	};

	GoombaState m_State{ Wandering };

	// Sound Logic
	FMOD::Channel* m_pChaseChannel{};
	FMOD::Sound* m_pChaseSound{};
	FMOD::Channel* m_pSFXChannel{};
	FMOD::Sound* m_pJumpedOnSound{};
	FMOD::Sound* m_pPunchedSound{};
	FMOD::Sound* m_pAlertSound{};
	const float m_SoundStartDistance{ 30.f };
	float m_CurrentChaseVol{ 1.f };
	float m_CurrentSFXVol{ 1.f };

	// General Movement Logic
	MarioCharacter* m_pMario;
	XMFLOAT3 m_TargetPosition;
	XMFLOAT3 m_TargetDirection;
	float m_Rotated{ 0.f };
	const float m_TurningSpeed{ 1.2f };
	const float m_OriginalSize{ 3.5f };


	// Wander Logic
	const float m_WanderChangeDirInterval = 2.0f;
	float m_WanderChangeCounter = 0.0f;
	const float m_WanderOffset{ 5.f }; // The larger the offset, the less random the wander is
	const float m_WanderAngleChange{ 180 }; // The larger the angle, the more rotation the wander will cause
	const float m_WanderRadius{ 10 };


	// Chase/Alert Logic
	const float m_ChaseDistance{ 15 };
	const float m_ChaseMaxHeightDifference{ 10 };
	const float m_TimeToLeaveFloor{ 0.2f };
	float m_JumpCounter{};


	// Damage Logic
	const float m_DamageDistance{ 4.f };
	const float m_PunchDistance{ 5.f };
	const float m_MinDamageTime{0.2f};
	const float m_MaxDamageTime{0.8f};
	float m_DamagedCounter{};

	// Pause Logic
	GoombaState m_StateBeforePause{ Wandering };
	bool m_ChasePlayingBeforePause{ false };
	bool m_SFXPlayingBeforePause{ false };

	// Sound Fade Out Logic
	bool m_SoundFadingOut{};
	float m_SoundTotalFadeTime{};
	float m_SoundFadeOutCounter{};
	void UpdateSoundFadeOut(float elapsedTime);

	SpawnInfo* m_pSpawnInfo;


	void ApplyMovement(float elapsedTime);
	void RotateMesh();
	bool CheckIfGrounded() const;
	void Update3DSounds();
	void ApplyGravity(float elapsedTime);
};

