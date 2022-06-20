#pragma once
class TorusPrefab final : public GameObject
{

public:
	TorusPrefab(float majorRadius = 1.f, unsigned int majorRadiusSteps = 10, float minorRadius = .2f, unsigned int minorRadiusSteps = 5,
	            XMFLOAT4 color = static_cast<XMFLOAT4>(Colors::Green));
	~TorusPrefab() override = default;
	TorusPrefab(const TorusPrefab& other) = delete;
	TorusPrefab(TorusPrefab&& other) noexcept = delete;
	TorusPrefab& operator=(const TorusPrefab& other) = delete;
	TorusPrefab& operator=(TorusPrefab&& other) noexcept = delete;

protected:
	void Initialize(const SceneContext& sceneContext) override;

private:

	float m_MajorRadius{}, m_MinorRadius{};
	unsigned int m_MajorRadiusSteps{}, m_MinorRadiusSteps{};
	XMFLOAT4 m_Color{};

};
