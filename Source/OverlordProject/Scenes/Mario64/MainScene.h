#pragma once
class Tube;
class Star;
class CannonballCharacter;
class MarioCharacter;
class GoombaCharacter;
class BoBombCharacter;

struct SpawnInfo
{
	bool alive = true;
	XMFLOAT3 spawnPosition = XMFLOAT3(0,0,0);
};

struct CannonballSpawnInfo
{
	std::vector<XMFLOAT3> targetPositions;
	bool resetAfterAllTargets;
};

class MainScene : public GameScene
{
public:
	MainScene() :GameScene(L"MainScene") {}
	~MainScene() override;
	MainScene(const MainScene& other) = delete;
	MainScene(MainScene&& other) noexcept = delete;
	MainScene& operator=(const MainScene& other) = delete;
	MainScene& operator=(MainScene&& other) noexcept = delete;

	void Reset() override;
	void ToggleSoundFadeOut(float fadeTime) override;

protected:
	void Initialize() override;
	void Update() override;

private:
	enum InputIds
	{
		CharacterMoveLeft,
		CharacterMoveRight,
		CharacterMoveForward,
		CharacterMoveBackward,
		CharacterJump,
		CharacterCrouch,
		CharacterPunch,
		CharacterTurnCameraLeft,
		CharacterTurnCameraRight,

		Pause1,
		Pause2,
		PauseSelectUp,
		PauseSelectDown
	};

	MarioCharacter* m_pCharacter{};
	XMFLOAT3 m_MarioSpawnPosition{ 112.936f, 1.9f, 110.05f };
	Star* m_pStar{};
	std::vector <std::pair<GoombaCharacter*, SpawnInfo*>> m_pGoombas{};
	std::vector<std::pair<BoBombCharacter*, SpawnInfo*>> m_pBobOmbs{};
	std::vector<std::pair<CannonballCharacter*, CannonballSpawnInfo*>> m_pCannonballs{};
	GameObject* m_pGoombaSpawnerGO{};
	GameObject* m_pBobOmbSpawnerGO{};
	GameObject* m_pCannonballsSpawnerGO{};

	Tube* m_pTube{};
	const XMFLOAT3 m_TubeSpawnPos = XMFLOAT3(112.936f, 1.f, 110.05f);
	const XMFLOAT3 m_TubeFinalPos = XMFLOAT3(112.936f, -3.f, 110.05f);
	const float m_TubeTimeToHide = 2.f;

	FMOD::Channel* m_pTrackStartChannel{};
	FMOD::Sound* m_pTrackStartSound{};
	FMOD::Channel* m_pTrackLoopChannel{};
	FMOD::Sound* m_pTrackLoopSound{};
	const float m_TrackStartTime{ 73.2f };
	float m_TrackCounter{};
	bool m_MusicStarted{};
	bool m_LoopingTrack{};

	void InitializeLevel(const PxMaterial* pDefaultMaterial);
	void SpawnGoombas(PxMaterial* pDefaultMaterial);
	void SpawnBobOmbs(PxMaterial* pDefaultMaterial);
	void SpawnCannonballs(PxMaterial* pDefaultMaterial);

	// Pause Logic
	enum PauseButton
	{
		Resume = 0,
		Back = 1,
		Quit = 2
	};
	bool m_Paused{};
	bool m_ChangingScreen{};
	const float m_TimeForSFX{ 0.8f };
	float m_ChangeCounter{};
	PauseButton m_PauseSelectedButton{ Resume };
	const XMFLOAT4 m_UnselectedColor{ 0.5f, 0.5f, 0.5f, 1.f };
	SpriteComponent* m_pPauseBackground{};
	SpriteComponent* m_pPauseTitle{};
	SpriteComponent* m_pPauseResumeButton{};
	SpriteComponent* m_pPauseBackButton{};
	SpriteComponent* m_pPauseQuitButton{};
	FMOD::Sound* m_pPauseConfirmSound{};
	FMOD::Sound* m_pPauseChangeSound{};
	FMOD::Channel* m_pPauseChangeChannel{};


	// Track Fade Out Logic
	bool m_SoundFadingOut{};
	float m_SoundTotalFadeTime{};
	float m_SoundFadeOutCounter{};
	void UpdateSoundFadeOut();


	void TogglePause(bool paused);
	void HandlePauseScreenInput();
};

