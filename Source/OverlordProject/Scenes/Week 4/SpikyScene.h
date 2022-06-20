#pragma once
class SpikyMaterial;

class SpikyScene : public GameScene
{
public:
	SpikyScene() :GameScene(L"SpikyScene") {}
	~SpikyScene() override = default;

	SpikyScene(const SpikyScene& other) = delete;
	SpikyScene(SpikyScene&& other) noexcept = delete;
	SpikyScene& operator=(const SpikyScene& other) = delete;
	SpikyScene& operator=(SpikyScene&& other) noexcept = delete;

protected:
	void Initialize() override;
	void OnGUI() override;
	void Update() override;

	GameObject* m_pSphere{};
	SpikyMaterial* m_pSphereMaterial{};
	float m_CurrentRotY{};

private:
	const float m_RotationPerSec{ 20.f };
};




