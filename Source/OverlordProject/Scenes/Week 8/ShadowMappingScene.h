#pragma once
class ShadowMappingScene final : public GameScene
{
public:
	ShadowMappingScene():GameScene(L"ShadowMappingScene"){}
	~ShadowMappingScene() override = default;
	ShadowMappingScene(const ShadowMappingScene& other) = delete;
	ShadowMappingScene(ShadowMappingScene&& other) noexcept = delete;
	ShadowMappingScene& operator=(const ShadowMappingScene& other) = delete;
	ShadowMappingScene& operator=(ShadowMappingScene&& other) noexcept = delete;

protected:
	void Initialize() override;
	void Update() override;
	void PostDraw() override;
	void OnGUI() override;

private:
	bool m_DrawShadowMap{ false };
	float m_ShadowMapScale{0.3f};
};

