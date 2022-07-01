#pragma once
struct ChainchompSpawnInfo;
class MarioCharacter;

struct ChainchompDesc
{
	ChainchompDesc(
		PxMaterial* pMaterial,
		float radius = 0.864f,
		float height = 4.9f)
	{
		controller.setToDefault();
		controller.radius = radius;
		controller.height = height;
		controller.material = pMaterial;
	}

	float hopSpeed{ 8.f };

	float maxLungeSpeed{ 40.f };
	float maxRecoilSpeed{ 10.f };
	float maxFallSpeed{ 20.f };

	float lungeAccelerationTime{ 0.2f };
	float recoilAccelerationTime{ .5f };
	float fallAccelerationTime{ .3f };

	PxCapsuleControllerDesc controller{};
};

class ChainchompCharacter : public GameObject
{
public:
	ChainchompCharacter(const ChainchompDesc& characterDesc, MarioCharacter* pMario, const XMFLOAT3& spawnPos);
	~ChainchompCharacter() = default;

	ChainchompCharacter(const ChainchompCharacter& other) = delete;
	ChainchompCharacter(ChainchompCharacter&& other) noexcept = delete;
	ChainchompCharacter& operator=(const ChainchompCharacter& other) = delete;
	ChainchompCharacter& operator=(ChainchompCharacter&& other) noexcept = delete;

	void Reset();
	void TogglePause(bool paused);
	void ToggleSoundFadeOut(float fadeTime);

protected:
	void Initialize(const SceneContext&) override;
	void Update(const SceneContext&) override;

private:
	ControllerComponent* m_pControllerComponent{};

	ChainchompDesc m_ChainchompDesc;
	float m_LungeAcceleration{}, m_RecoilAcceleration{}, m_FallAcceleration{}, m_MoveSpeed{};
	XMFLOAT3 m_TotalVelocity{};

	enum ChainchompState
	{
		Idle,
		Rotating,
		Lunging,
		Recoiling,
		Paused
	};

	ChainchompState m_State{ Idle };
	ChainchompState m_StateBeforePause{ Idle };

	// Sound
	FMOD::Channel* m_pChainChannel{};
	FMOD::Channel* m_pLungingChannel{};
	FMOD::Sound* m_pChainSound{};
	FMOD::Sound* m_pLungingSound{};
	const float m_SoundStartDistance{ 60.f };
	const float m_PauseBetweenHopsRot{ 0.6f };
	const float m_PauseBetweenHopsRecoil{ 0.9f };
	float m_PauseBetweenHopsCounter{ 0.f };
	const float m_VolumeMultiplier{ 8.f };
	float m_CurrentVolume{ 1.f };
	bool m_ChainPlayingBeforePause{};
	bool m_LungingPlayingBeforePause{};


	// General Logic
	MarioCharacter* m_pMario{};
	XMFLOAT3 m_TargetDirection{};
	float m_TargetDistance{};
	float m_DistanceLunged{};
	const float m_RecoilMinCenterOffset{ 1.f };
	const float m_LungeTargetErrorMargin{ 0.5f };
	const float m_ActivationDistance{ 25 };
	const float m_DamageDistance{ 5.472f };
	const float m_MaxRotationTime{ 5 };
	float m_RotationTimeCounter{};
	float m_Rotated{ 0.f };
	const float m_TurningSpeed{ 1.2f };


	// Sound Fade Out Logic
	bool m_SoundFadingOut{};
	float m_SoundTotalFadeTime{};
	float m_SoundFadeOutCounter{};
	void UpdateSoundFadeOut(float elapsedTime);


	const XMFLOAT3 m_SpawnPosition;


	void ApplyMovement(float elapsedTime);
	bool CheckIfGrounded();
	void ApplyGravity(float elapsedTime);
	void RotateToFaceTarget(const XMFLOAT3& targetPos);
	void Update3DSound(float marioDistanceLength);
};

