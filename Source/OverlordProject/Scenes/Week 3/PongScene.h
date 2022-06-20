#pragma once
class CubePrefab;
class SpherePrefab;

class PongScene final : public GameScene
{
public:
	PongScene() :GameScene(L"PongScene") {}
	~PongScene() override = default;

	PongScene(const PongScene& other) = delete;
	PongScene(PongScene&& other) noexcept = delete;
	PongScene& operator=(const PongScene& other) = delete;
	PongScene& operator=(PongScene&& other) noexcept = delete;

protected:
	void Initialize() override;
	void Update() override;

	void ResetGame();

	enum class InputIds
	{
		P1MoveUp,
		P1MoveDown,
		P2MoveUp,
		P2MoveDown,
		ThrowBall
	};

	SpherePrefab* m_pBall{};
	CubePrefab* m_pLeftPlayer{};
	CubePrefab* m_pRightPlayer{};
	const float m_PlayerMoveSpeed{ 10.f };
	const float m_PlayerMaxPosY{ 9.5f };
	const float m_PlayerMinPosY{ -9.5f };
	const float m_PlayerLength{ 4.f };
	const float m_PlayerHeightWidth{ 1.f };
	const float m_BallThrowImpulse{ 12.f };
	bool m_RoundStarted{};
};

