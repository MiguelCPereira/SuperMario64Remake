#pragma once
class StartScreen : public GameScene
{
public:
	StartScreen() :GameScene(L"StartScreen") {}
	~StartScreen() = default;
	StartScreen(const StartScreen& other) = delete;
	StartScreen(StartScreen&& other) noexcept = delete;
	StartScreen& operator=(const StartScreen& other) = delete;
	StartScreen& operator=(StartScreen&& other) noexcept = delete;

	void Reset() override;

protected:
	void Initialize() override;
	void Update() override;

private:
	enum InputIds
	{
		SelectLeft,
		SelectRight,
		SelectUp,
		SelectDown,
		ConfirmSelection1,
		ConfirmSelection2
	};

	enum Button
	{
		Start = 0,
		Controls = 1,
		Quit = 2
	};

	bool m_Active{};
	const float m_TimeToActivate{ 0.5f };
	float m_ActivationCounter{};
	float m_TitleRegularScale{ 1.7f };

	FMOD::Channel* m_pTrackChannel{};
	FMOD::Sound* m_pTrackSound{};
	FMOD::Sound* m_pReadySound{};
	FMOD::Channel* m_pSFXChannel{};
	FMOD::Sound* m_pChangeSound{};
	FMOD::Channel* m_pChangeChannel{};
	FMOD::Sound* m_pConfirmSound{};
	FMOD::Sound* m_pStartLevelSound{};
	bool m_SoundPlayed{};

	const float m_TimeForSFX{ 0.8f };
	float m_ChangeCounter{};
	bool m_ChangingScreen{};
	bool m_ControlsScreenOpen{};

	Button m_SelectedButton{ Start }; // 0 Start | 1 Controls | 2 Quit

	const XMFLOAT4 m_UnselectedColor{ 0.5f, 0.5f, 0.5f, 1.f };
	GameObject* m_pTitleGO{};
	SpriteComponent* m_pBanner{};
	SpriteComponent* m_pStartButton{};
	SpriteComponent* m_pControlsButton{};
	SpriteComponent* m_pQuitButton{};
	SpriteComponent* m_pControlsScreen{};
};

