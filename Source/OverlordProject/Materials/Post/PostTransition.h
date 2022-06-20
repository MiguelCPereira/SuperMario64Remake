#pragma once
//Resharper Disable All

class PostTransition : public PostProcessingMaterial
{
public:
	PostTransition();
	~PostTransition() override = default;
	PostTransition(const PostTransition& other) = delete;
	PostTransition(PostTransition&& other) noexcept = delete;
	PostTransition& operator=(const PostTransition& other) = delete;
	PostTransition& operator=(PostTransition&& other) noexcept = delete;

	void SetBgColor(const XMFLOAT3& color);
	void SetCircleRadius(float radius);
	void SetCircleCenterPos(const XMFLOAT2& screenPos);
	void SetScreenSize(const XMFLOAT2& screenSize);

protected:
	void Initialize(const GameContext& gameContext) override;

private:
	XMFLOAT3 m_BackgroundColor{ 0.f,0.f,0.f };
	float m_CircleRadius{ 1.0f };
	XMFLOAT2 m_CircleCenterPos{ 0.5f, 0.5f };
};

