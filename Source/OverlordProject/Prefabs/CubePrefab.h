#pragma once
class CubePrefab final : public GameObject
{
public:
	CubePrefab(float width = 1.f, float height = 1.f, float depth = 1.f, const XMFLOAT4& color = static_cast<XMFLOAT4>(Colors::Red));
	CubePrefab(const XMFLOAT3& dimensions, const XMFLOAT4& color = static_cast<XMFLOAT4>(Colors::Red));
	~CubePrefab() override = default;

	CubePrefab(const CubePrefab& other) = delete;
	CubePrefab(CubePrefab&& other) noexcept = delete;
	CubePrefab& operator=(const CubePrefab& other) = delete;
	CubePrefab& operator=(CubePrefab&& other) noexcept = delete;

protected:
	void Initialize(const SceneContext& sceneContext) override;

private:
	float m_Width{}, m_Height{}, m_Depth{};
	XMFLOAT4 m_Color{};
};
