#pragma once
struct ChainchompSpawnInfo;
class MarioCharacter;

struct ChainchompDesc
{
	ChainchompDesc(
		PxMaterial* pMaterial,
		float radius = 0.864f,
		float height = 0.7296f)
	{
		controller.setToDefault();
		controller.radius = radius;
		controller.height = height;
		controller.material = pMaterial;
	}

	float maxMoveSpeed{ 12.f };
	float maxRotationSpeed{ 8.f };
	float maxFallSpeed{ 15.f };

	float rotationAccelerationTime{ .5f };
	float moveAccelerationTime{ .8f };
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
	float m_MoveAcceleration{}, m_RotationAcceleration{}, m_FallAcceleration{}, m_MoveSpeed{};

	XMFLOAT3 m_TotalVelocity{};
	XMFLOAT3 m_CurrentDirection{};

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
	const float m_ChainPauseBetweenLoopRot{ 2.f };
	const float m_ChainPauseBetweenLoopRecoil{ 1.f };
	float m_ChainPauseCounter{ 0.f };
	const float m_VolumeMultiplier{ 8.f };
	float m_CurrentVolume{ 1.f };
	bool m_ChainPlayingBeforePause{};
	bool m_LungingPlayingBeforePause{};


	// General Logic
	MarioCharacter* m_pMario{};
	XMFLOAT3 m_TargetPosition{};
	int m_CurrentTargetPos{};
	const float m_ActivationDistance{ 15 };
	const float m_DamageDistance{ 5.472f };
	const float m_MaxRotationTime{ 5 };
	float m_RotationTimeCounter{};


	// Sound Fade Out Logic
	bool m_SoundFadingOut{};
	float m_SoundTotalFadeTime{};
	float m_SoundFadeOutCounter{};
	void UpdateSoundFadeOut(float elapsedTime);


	const XMFLOAT3 m_SpawnPosition;


	bool CheckIfGrounded();
	void Update3DSound(float marioDistanceLength);
};

