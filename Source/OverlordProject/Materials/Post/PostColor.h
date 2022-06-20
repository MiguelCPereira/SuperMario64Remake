#pragma once
//Resharper Disable All

class PostColor : public PostProcessingMaterial
{
public:
	PostColor();
	~PostColor() override = default;
	PostColor(const PostColor& other) = delete;
	PostColor(PostColor&& other) noexcept = delete;
	PostColor& operator=(const PostColor& other) = delete;
	PostColor& operator=(PostColor&& other) noexcept = delete;

	void SetColor(const XMFLOAT3& color);
	void SetIntensity(float intensity);

protected:
	void Initialize(const GameContext& /*gameContext*/) override {}

private:
	XMFLOAT3 m_Color{0.f,0.f,0.f};
	float m_Intensity{ 0.5f };
};
