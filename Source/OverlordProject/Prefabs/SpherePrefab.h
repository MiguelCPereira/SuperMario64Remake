#pragma once
class SpherePrefab final : public GameObject
{
public:
	SpherePrefab(float radius = 1.f, int steps = 10, XMFLOAT4 color = static_cast<XMFLOAT4>(Colors::Blue));
	~SpherePrefab() override = default;

	SpherePrefab(const SpherePrefab& other) = delete;
	SpherePrefab(SpherePrefab&& other) noexcept = delete;
	SpherePrefab& operator=(const SpherePrefab& other) = delete;
	SpherePrefab& operator=(SpherePrefab&& other) noexcept = delete;

protected:
	void Initialize(const SceneContext& sceneContext) override;

private:

	float m_Radius{};
	int m_Steps{};
	XMFLOAT4 m_Color{};
};
