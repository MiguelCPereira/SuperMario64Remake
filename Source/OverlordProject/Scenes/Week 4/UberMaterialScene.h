#pragma once
class UberMaterial;

class UberMaterialScene : public GameScene
{
public:
	UberMaterialScene() :GameScene(L"UberMaterialScene") {}
	~UberMaterialScene() override = default;

	UberMaterialScene(const UberMaterialScene& other) = delete;
	UberMaterialScene(UberMaterialScene&& other) noexcept = delete;
	UberMaterialScene& operator=(const UberMaterialScene& other) = delete;
	UberMaterialScene& operator=(UberMaterialScene&& other) noexcept = delete;

protected:
	void Initialize() override;
	void Update() override;
	void OnGUI() override;

	GameObject* m_pSphere{};
	UberMaterial* m_pSphereMaterial{};
	float m_CurrentRotY{};

private:
	const float m_RotationPerSec{ 20.f };
};


