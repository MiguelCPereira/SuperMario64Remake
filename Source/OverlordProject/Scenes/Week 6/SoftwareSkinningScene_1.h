#pragma once

class BoneObject;

class SoftwareSkinningScene_1 final : public GameScene
{
public:

	SoftwareSkinningScene_1() : GameScene(L"SoftwareSkinningScene_1"){}
	~SoftwareSkinningScene_1() override = default;

	SoftwareSkinningScene_1(const SoftwareSkinningScene_1& other) = delete;
	SoftwareSkinningScene_1(SoftwareSkinningScene_1&& other) noexcept = delete;
	SoftwareSkinningScene_1& operator=(const SoftwareSkinningScene_1& other) = delete;
	SoftwareSkinningScene_1& operator=(SoftwareSkinningScene_1&& other) noexcept = delete;

protected:
	void Initialize() override;
	void Update() override;
	void OnGUI() override;

private:
	BoneObject* m_pBone0{}, * m_pBone1{};

	const float m_BoneRotationPerSec{ 45 };
	const float m_BoneMaxRotation{ 45 };
	float m_BoneAutoRotation{};
	XMFLOAT3 m_Bone1Rotation{};
	XMFLOAT3 m_Bone2Rotation{};
	int m_RotationSign{ 1 };
	bool m_AutoRotate{ true };
};

