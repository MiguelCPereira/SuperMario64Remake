#pragma once

struct SpawnInfo;
class MarioCharacter;

struct BoBombDesc
{
	BoBombDesc(
		PxMaterial* pMaterial,
		float radius = 1.f,
		float height = 0.6f)
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
	float throwSpeed{ 25.f };

	float wanderAccelerationTime{ .3f };
	float chaseAccelerationTime{ .5f };
	float fallAccelerationTime{ .3f };

	PxCapsuleControllerDesc controller{};
};

class BoBombCharacter : public GameObject
{
public:
	BoBombCharacter(const BoBombDesc& characterDesc, MarioCharacter* pMario, SpawnInfo* pSpawnInfo);
	~BoBombCharacter() = default;

	BoBombCharacter(const BoBombCharacter& other) = delete;
	BoBombCharacter(BoBombCharacter&& other) noexcept = delete;
	BoBombCharacter& operator=(const BoBombCharacter& other) = delete;
	BoBombCharacter& operator=(BoBombCharacter&& other) noexcept = delete;

	void BlowUp();
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

	BoBombDesc m_BoBombDesc;
	float m_WanderAcceleration{}, m_ChaseAcceleration{},
		m_FallAcceleration{}, m_MoveSpeed{};

	XMFLOAT3 m_TotalVelocity{};

	enum BobOmbState
	{
		Wandering,
		Chasing,
		PickedUp,
		Thrown,
		Paused
	};

	BobOmbState m_State{ Wandering };

	// General Movement Logic
	MarioCharacter* m_pMario;
	XMFLOAT3 m_TargetPosition;
	XMFLOAT3 m_TargetDirection;
	float m_Rotated{ 0.f };
	const float m_TurningSpeed{ 1.3f };


	// Wander Logic
	const float m_WanderChangeDirInterval = 2.0f;
	float m_WanderChangeCounter = 0.0f;
	const float m_WanderOffset{ 5.f }; // The larger the offset, the less random the wander is
	const float m_WanderAngleChange{ 180 }; // The larger the angle, the more rotation the wander will cause
	const float m_WanderRadius{ 10 };


	// Lit Fuse Logic
	const float m_LitFuseDistance{ 15 };
	const float m_LitFuseMaxHeightDifference{ 10 };
	const float m_LitFuseTime{ 3.f };
	float m_LitFuseCounter{};


	// Pick Up Logic
	const float m_PickedUpDistance{4.f};
	const float m_PickedUpMarioOffset{ 4.f };
	bool m_StoppedPickUpPunch{};


	// SFX
	FMOD::Channel* m_pChaseChannel{};
	FMOD::Sound* m_pChaseSound{};
	FMOD::Channel* m_pFumeChannel{};
	FMOD::Sound* m_pFumeSound{};
	FMOD::Channel* m_pExplodeChannel{};
	FMOD::Sound* m_pExplodeSound{};
	const float m_SoundStartDistance{ 30.f };
	float m_CurrentChaseVol{ 1.f };
	float m_CurrentFumeVol{ 1.f };
	float m_CurrentExplodeVol{ 1.f };


	// Pause Logic
	BobOmbState m_StateBeforePause{ Wandering };
	bool m_ChasePlayingBeforePause{ false };
	bool m_FumePlayingBeforePause{ false };
	bool m_ExplodePlayingBeforePause{ false };


	// Sound Fade Out Logic
	bool m_SoundFadingOut{};
	float m_SoundTotalFadeTime{};
	float m_SoundFadeOutCounter{};
	void UpdateSoundFadeOut(float elapsedTime);


	// Others
	const float m_BlowUpRadius{ 5.f };
	SpawnInfo* m_pSpawnInfo;
	bool m_ThrownForceXPositive{};
	bool m_ThrownForceZPositive{};
	ParticleEmitterComponent* m_pSmokeEmitter{};
	ParticleEmitterSettings m_pSmokeParticleSettings{};


	void ApplyMovement(float elapsedTime);
	void RotateMesh();
	void StickToMario();
	void DecreaseThrownVelocity(float elapsedTime);
	bool UpdateLitFuseCounter(float elapsedTime);
	bool CheckIfGrounded() const;
	void Update3DSounds();
};



