#pragma once
class VictoryScreen : public GameScene
{
public:
	VictoryScreen() :GameScene(L"VictoryScreen") {}
	~VictoryScreen() override = default;
	VictoryScreen(const VictoryScreen& other) = delete;
	VictoryScreen(VictoryScreen&& other) noexcept = delete;
	VictoryScreen& operator=(const VictoryScreen& other) = delete;
	VictoryScreen& operator=(VictoryScreen&& other) noexcept = delete;

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

	FMOD::Sound* m_pTrackSound{};
	FMOD::Channel* m_pTrackChannel{};
	FMOD::Channel* m_pSFXChannel{};
	FMOD::Sound* m_pChangeSound{};
	FMOD::Channel* m_pChangeChannel{};
	FMOD::Sound* m_pConfirmSound{};
	FMOD::Sound* m_pStartLevelSound{};
	bool m_MusicStarted{};

	const float m_MaxWobbleDistScreenPerc{ 0.02f };

	const float m_TimeForSFX{ 0.8f };
	float m_ChangeCounter{};
	bool m_ChangingScreen{};

	bool m_RetrySelected{ true };

	const XMFLOAT4 m_UnselectedColor{ 0.5f, 0.5f, 0.5f, 1.f };
	SpriteComponent* m_pRetryButton{};
	SpriteComponent* m_pBackButton{};
	GameObject* m_pVictoryTitle{};
};

