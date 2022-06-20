#pragma once
class DeathScreen : public GameScene
{
public:
	DeathScreen() : GameScene(L"DeathScreen") {}
	~DeathScreen() override = default;
	DeathScreen(const DeathScreen& other) = delete;
	DeathScreen(DeathScreen&& other) noexcept = delete;
	DeathScreen& operator=(const DeathScreen& other) = delete;
	DeathScreen& operator=(DeathScreen&& other) noexcept = delete;

	void Reset() override;

protected:
	void Initialize() override;
	void Update() override;

private:
	enum InputIds
	{
		SelectLeft,
		SelectRight,
		ConfirmSelection1,
		ConfirmSelection2
	};

	bool m_Active{};
	const float m_TimeToActivate{ 2.f };
	float m_ActivationCounter{};

	FMOD::Channel* m_pSFXChannel{};
	FMOD::Channel* m_pChangeChannel{};
	FMOD::Sound* m_pChangeSound{};
	FMOD::Sound* m_pConfirmSound{};
	FMOD::Sound* m_pStartLevelSound{};

	const float m_MaxSizeIncrease{ 0.04f };

	const float m_TimeForSFX{ 0.8f };
	float m_ChangeCounter{};
	bool m_ChangingScreen{};

	bool m_YesSelected{ true };

	const XMFLOAT4 m_UnselectedColor{ 0.5f, 0.5f, 0.5f, 1.f };
	SpriteComponent* m_pGameOverTitle{};
	SpriteComponent* m_pRestartText{};
	SpriteComponent* m_pBackText{};
	SpriteComponent* m_pYesButton{};
	SpriteComponent* m_pNoButton{};
	GameObject* m_pGameOverTitleGO{};
};

