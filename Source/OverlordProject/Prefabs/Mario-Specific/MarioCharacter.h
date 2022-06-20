#pragma once
class PostBlur;
class PostColor;
class PostTransition;

struct MarioDesc
{
	MarioDesc(
		PxMaterial* pMaterial,
		float radius = .5f,
		float height = 2.f)
	{
		controller.setToDefault();
		controller.radius = radius;
		controller.height = height;
		controller.material = pMaterial;
	}

	float maxMoveSpeed{ 15.f };
	float maxFallSpeed{ 15.f };
	float maxCrouchSpeed{ 5.f };
	float maxGroundPoundSpeed{ 60.f };

	float DamagedThrowSpeed{ 35.f };
	float NormalJumpSpeed{ 15.f };
	float SecondJumpSpeed{ 23.f };
	float ThirdJumpSpeed{ 30.f };
	float BackFlipSpeedUp{ 27.f };
	float BackFlipSpeedBack{ 15.f };
	XMFLOAT3 CameraOffset{ 0.f, 12.f, 10.f };

	float moveAccelerationTime{ .3f };
	float fallAccelerationTime{ .3f };
	float crouchAccelerationTime{ .3f };
	float groundPoundAccelerationTime{ .15f };

	PxCapsuleControllerDesc controller{};

	float gamepadRotationSpeed{ 120.f };
	float mouseRotationSpeed{ 20.f };

	int actionId_MoveLeft{ -1 };
	int actionId_MoveRight{ -1 };
	int actionId_MoveForward{ -1 };
	int actionId_MoveBackward{ -1 };
	int actionId_Jump{ -1 };
	int actionId_Crouch{ -1 };
	int actionId_Punch{ -1 };
	int actionId_TurnCameraLeft{ -1 };
	int actionId_TurnCameraRight{ -1 };
};

enum MarioState
{
	Idle,
	CrouchIdle,
	Running,
	CrouchMoving,
	MidAir,
	GroundSlamming,
	Punching,
	MidDamaged,
	Dead,
	Paused,
	Spawning
};

class MarioCharacter : public GameObject
{
public:
	MarioCharacter(const MarioDesc& characterDesc);
	~MarioCharacter() = default;

	MarioCharacter(const MarioCharacter& other) = delete;
	MarioCharacter(MarioCharacter&& other) noexcept = delete;
	MarioCharacter& operator=(const MarioCharacter& other) = delete;
	MarioCharacter& operator=(MarioCharacter&& other) noexcept = delete;

	void GetDamaged(int dmgAmount, XMFLOAT3 dmgOrigin);
	void GetLunged(XMFLOAT3 lungeVector);
	MarioState GetState() const { return m_State; }
	void Reset();
	void TogglePause(bool paused);

	float m_TotalPitch{}, m_TotalYaw{};				//Total camera Pitch(X) and Yaw(Y) rotation
	bool m_GrabbingBobOmb{};

protected:
	void Initialize(const SceneContext&) override;
	void Update(const SceneContext&) override;

private:
	// Movement Logic
	ControllerComponent* m_pControllerComponent{};
	MarioDesc m_CharacterDesc;
	float m_MoveAcceleration{}, m_FallAcceleration{}, m_CrouchAcceleration{},
		m_GroundPoundAcceleration{}, m_MoveSpeed{};
	XMFLOAT3 m_TotalVelocity{};
	XMFLOAT3 m_CurrentDirection{};
	float m_LastMousePosX{}, m_LastMousePosY{};


	// Camera Logic
	CameraComponent* m_pCameraComponent{};
	const float m_PitchMax{ 35.f }, m_PitchMin{ 0.f };


	// Falling Logic
	const float m_MaxAirTimeBeforeStateSwitch{ 0.2f };
	float m_AirTimeCounter{};


	// Jump Logic
	int m_JumpCounter{};
	float m_BetweenJumpsTimer{};
	const float m_BetweenJumpsMaxTime{0.5f};
	void Jump(bool crouched);
	void UpdateJumpTimer(float elapsedTime);


	// Punch Logic
	float m_PunchTimer{};
	const float m_PunchMaxTime{ 0.5f };


	// HP Logic
	const int m_MaxHealth{ 8 };
	int m_CurrentHealth{ 8 };
	bool m_Recovering{};
	bool m_DmgBackToIdleOnLand{};
	bool m_DamagedForTooLong{};
	const float m_RecoveryTime{ 1.f };
	float m_RecoveryCounter{};
	const float m_MinFreezeDamageTime{ 0.2f };
	const float m_MaxFreezeDamageTime{ 0.6f };
	bool m_LungeForceXPositive{};
	bool m_LungeForceZPositive{};
	void UpdateRecovery(float elapsedTime);


	// HP HUD Logic
	SpriteComponent* m_LivesHUD{};
	bool m_ShowLivesHUD{};
	const float m_HUDShowTime{ 2.f };
	const float m_HUDFadeInTime{ 0.2f };
	const float m_HUDFadeOutTime{ 1.f };
	float m_HUDCounter{};


	// SFX Logic
	FMOD::Channel* m_pWalkChannel{};
	FMOD::Channel* m_pCrawlChannel{};
	FMOD::Sound* m_pSpawnSound{};
	FMOD::Sound* m_pDamagedSound{};
	FMOD::Sound* m_pJump1Sound{};
	FMOD::Sound* m_pJump2Sound{};
	FMOD::Sound* m_pJump3Sound{};
	FMOD::Sound* m_pBackFlipSound{};
	FMOD::Sound* m_pPunchSound{};
	FMOD::Sound* m_pGroundSlamSound{};
	FMOD::Channel* m_pCameraSpinChannel{};
	FMOD::Sound* m_pCameraSpinSound{};
	FMOD::Channel* m_pSFXChannel{};
	bool m_CameraSoundPlaying{};


	// Post Effects Logic
	PostBlur* m_pPostBlur{};
	PostColor* m_pPostRed{};
	PostTransition* m_pPostTransition{};
	const float m_MaxRedIntensity{ 0.2f };
	const float m_MaxBlurIntensity{ 1.f };
	const float m_HurtPostFullIntensityTime{ 0.5f };
	const float m_HurtPostFadeOutTime{ 1.f };
	const XMFLOAT2 m_TransitionCenter{ 0.5f, 0.5f };
	const float m_TransitionMaxRadius{ 1.f };
	const float m_TransitionMaxTime{ 2.5f };
	float m_TransitionCounter{};


	// Particle Logic
	ParticleEmitterComponent* m_pWalkDustEmitter{};
	ParticleEmitterComponent* m_pSlamStarEmitter{};
	ParticleEmitterComponent* m_pSlamDustEmitter{};
	const float m_ParticleOffset{ 1.3f };
	const float m_GroundSlamParticleLifeTime{ 0.5f };
	const float m_GroundSlamParticleSpawnRadius{ 0.7f };
	float m_GroundSlamParticleCounter{};
	bool m_JustGroundSlammed{};


	// Others
	GameObject* m_pModelGO{};
	ModelAnimator* m_pAnimator{};


	void HandleCamera(const SceneContext& sceneContext);
	void DecreaseSpeed(float elapsedTime);
	void DecreaseLungedVelocity(float elapsedTime);
	void MoveSideways(const SceneContext& sceneContext, bool crouched);
	void MoveSidewaysBackflip(const SceneContext& sceneContext);
	void ApplyGravity(const SceneContext& sceneContext, bool groundSlamming);
	void ApplyMovement(float elapsedTime);
	void UpdateUI(float elapsedTime);

	bool CheckIfGrounded();
	bool CheckIfAnyMovementInput(const SceneContext& sceneContext) const;

	MarioState m_State{MarioState::Spawning};
	bool m_SpawnSoundPlayed = false;

	// Pause Logic
	MarioState m_StateBeforePause{ MarioState::Spawning };
	float m_AnimatorSpeedBeforePause{ 1.f };
	bool m_WalkPlayingBeforePause{false};
	bool m_CrawlPlayingBeforePause{ false };
	bool m_CameraSpinPlayingBeforePause{ false };
	bool m_SFXPlayingBeforePause{ false };
};

