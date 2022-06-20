#pragma once

class BoneObject;

class SoftwareSkinningScene_3 final : public GameScene
{
public:

	SoftwareSkinningScene_3() : GameScene(L"SoftwareSkinningScene_3") {}
	~SoftwareSkinningScene_3() override = default;

	SoftwareSkinningScene_3(const SoftwareSkinningScene_3& other) = delete;
	SoftwareSkinningScene_3(SoftwareSkinningScene_3&& other) noexcept = delete;
	SoftwareSkinningScene_3& operator=(const SoftwareSkinningScene_3& other) = delete;
	SoftwareSkinningScene_3& operator=(SoftwareSkinningScene_3&& other) noexcept = delete;

protected:
	void Initialize() override;
	void Update() override;
	void OnGUI() override;

private:
	BoneObject* m_pBone0{}, * m_pBone1{};

	const float m_BoneLength{ 15.f };
	const float m_BoneRotationPerSec{ 45 };
	const float m_BoneMaxRotation{ 45 };
	float m_BoneAutoRotation{};
	XMFLOAT3 m_Bone1Rotation{};
	XMFLOAT3 m_Bone2Rotation{};
	int m_RotationSign{ 1 };
	bool m_AutoRotate{ true };

	struct VertexSoftwareSkinned
	{
		VertexSoftwareSkinned(XMFLOAT3 position, XMFLOAT3 normal, XMFLOAT4 color, float weight0, float weight1)
			: transformedVertex{ position, normal, color }
			, originalVertex{ position, normal, color }
			, blendWeight0{ weight0 }
			, blendWeight1{ weight1 }
		{}

		VertexPosNormCol transformedVertex{};
		VertexPosNormCol originalVertex{};
		float blendWeight0{};
		float blendWeight1{};
	};

	void InitializeVertices(float length);

	MeshDrawComponent* m_pMeshDrawer{};
	std::vector<VertexSoftwareSkinned> m_SkinnedVertices{};
};

