#pragma once

enum class LightType : uint32_t
{
	Point = 0,
	Spot = 1
};

//__declspec(align(16)) struct Light //(with 24B padding)
struct Light
{
	XMFLOAT4 direction; //16B
	XMFLOAT4 position; //16B
	XMFLOAT4 color; //16B
	float intensity; //4B
	float range; //4B
	float spotLightAngle; //4B
	LightType type; //4B
	//4 * 16B (To Shader)

	bool isEnabled; //4B (Discard)

	Light() :
		direction({ 0.f, 1.f, 0.f, 1.f }),
		position({ 0.f, 0.f, 0.f, 0.f }),
		color({ 1.f, 1.f, 1.f, 1.f }),
		intensity(1.f),
		range(50.f),
		spotLightAngle(45.f),
		type(LightType::Point),
		isEnabled(true) {}
};

class LightManager
{
public:
	LightManager();
	~LightManager();
	LightManager(const LightManager& other) = delete;
	LightManager(LightManager&& other) noexcept = delete;
	LightManager& operator=(const LightManager& other) = delete;
	LightManager& operator=(LightManager&& other) noexcept = delete;

	//For Deferred Rendering
	UINT AddLight(const Light& l) { m_Lights.push_back(l); return static_cast<UINT>(m_Lights.size() - 1); }
	Light& GetLight(int id) { return m_Lights[id]; }
	const std::vector<Light>& GetLights() const { return m_Lights; }

	Light& GetDirectionalLight() { return m_DirectionalLight; }
	void SetDirectionalLight(const XMFLOAT3& position, const XMFLOAT3& direction);

private:

	std::vector<Light> m_Lights{};
	Light m_DirectionalLight{};
};

