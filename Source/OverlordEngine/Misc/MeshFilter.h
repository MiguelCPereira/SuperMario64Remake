#pragma once

#pragma region MeshFilter Helper Structs
struct VertexBufferData
{
	VertexBufferData() :
		pDataStart(nullptr),
		pVertexBuffer(nullptr),
		BufferSize(0),
		VertexStride(0),
		VertexCount(0),
		IndexCount(0),
		InputLayoutID(0) {}

	void* pDataStart;
	ID3D11Buffer* pVertexBuffer;
	UINT BufferSize;
	UINT VertexStride;
	UINT VertexCount;
	UINT IndexCount;
	UINT InputLayoutID;

	void Destroy()
	{
		free(pDataStart);
		SafeRelease(pVertexBuffer);
	}
};

struct AnimationKey
{
	float tick{};
	std::vector<XMFLOAT4X4> boneTransforms{};
};

struct AnimationClip
{
	AnimationClip() = default;

	std::wstring name{};
	float duration{};
	float ticksPerSecond{};
	std::vector<AnimationKey> keys{};
};

struct SubMeshBuffers
{
	std::vector<VertexBufferData> vertexbuffers{};
	ID3D11Buffer* pIndexBuffer{};

	bool HasVertexBuffer(UINT inputLayoutId) const
	{
		return std::ranges::find_if(vertexbuffers, [&inputLayoutId](const VertexBufferData& data) {
			return data.InputLayoutID == inputLayoutId; }) != vertexbuffers.end();
	}

	bool GetVertexBufferData(UINT inputLayoutId, VertexBufferData** pVertexBufferData)
	{
		pVertexBufferData = nullptr;

		const auto it = std::ranges::find_if(vertexbuffers, [&inputLayoutId](const VertexBufferData& data) {
			return data.InputLayoutID == inputLayoutId; });

		if (it == vertexbuffers.end()) return false;

		auto result = &*it;
		pVertexBufferData = &result;
		return true;
	}

	void Release()
	{
		for (auto& vbData : vertexbuffers)
		{
			vbData.Destroy();
		}

		vertexbuffers.clear();
		SafeRelease(pIndexBuffer)
	}
};

struct SubMeshFilter
{
	BYTE id{};
	std::wstring name{};

	UINT vertexCount{};
	UINT indexCount{};
	UINT uvChannelCount{};

	ILSemantic layoutElements{ ILSemantic::NONE };

	std::vector<XMFLOAT3> positions{};
	std::vector<XMFLOAT3> normals{};
	std::vector<XMFLOAT3> tangents{};
	std::vector<XMFLOAT3> binormals{};
	std::vector<XMFLOAT2> texCoords{};
	std::vector<XMFLOAT4> colors{};
	std::vector<XMFLOAT4> blendIndices{};
	std::vector<XMFLOAT4> blendWeights{};

	//INDEX DATA
	std::vector<UINT> indices{};

	//DX Data
	SubMeshBuffers buffers{};

	bool HasElement(ILSemantic element) const
	{
		return isSet(layoutElements, element);
	}

	void Release()
	{
		layoutElements = ILSemantic::NONE;

		positions.clear();
		normals.clear();
		tangents.clear();
		binormals.clear();
		texCoords.clear();
		colors.clear();
		blendIndices.clear();
		blendWeights.clear();
		indices.clear();

		buffers.Release();
	}
};
#pragma endregion

class MeshFilter final
{
public:
	MeshFilter() = default;
	~MeshFilter();
	MeshFilter(const MeshFilter& other) = delete;
	MeshFilter(MeshFilter&& other) noexcept = delete;
	MeshFilter& operator=(const MeshFilter& other) = delete;
	MeshFilter& operator=(MeshFilter&& other) noexcept = delete;

	const std::vector<SubMeshFilter>& GetMeshes() const { return m_Meshes; }
	UINT GetMeshCount() const { return static_cast<UINT>(m_Meshes.size()); }
	const std::vector<AnimationClip>& GetAnimationClips() const { return m_AnimationClips; }
	bool HasAnimations() const { return m_HasAnimations; }

	int GetVertexBufferId(UINT inputLayoutId, UINT8 subMeshId) const;
	void BuildVertexBuffer(const SceneContext& gameContext, UINT inputLayoutID, UINT inputLayoutSize, const std::vector<ILDescription>& inputLayoutDescriptions);
	void BuildVertexBuffer(const SceneContext& sceneContext, BaseMaterial* pMaterial);
	void BuildIndexBuffer(const SceneContext& sceneContext);

	const VertexBufferData& GetVertexBufferData(const SceneContext& sceneContext, BaseMaterial* pMaterial, UINT8 subMeshId = 0);
	const VertexBufferData& GetVertexBufferData(UINT inputLayoutId, UINT8 subMeshId = 0) const;
	ID3D11Buffer* GetIndexBuffer(UINT8 subMeshId = 0) const;

private:
	friend class MeshFilterLoader; //TODO: Resolve Friend Classes
	friend class ModelComponent;
	friend class ModelAnimator;

	void BuildVertexBuffer(const SceneContext& sceneContext, BaseMaterial* pMaterial, UINT8 subMeshId);
	void BuildVertexBuffer(const SceneContext& sceneContext, UINT inputLayoutID, UINT inputLayoutSize, const std::vector<ILDescription>& inputLayoutDescriptions, UINT8 subMeshId);

	std::wstring m_MeshName{};
	std::vector<SubMeshFilter> m_Meshes{};

	std::vector<AnimationClip> m_AnimationClips{};
	bool m_HasAnimations{};
	USHORT m_BoneCount{};

	static XMFLOAT4 m_DefaultColor;
	static XMFLOAT4 m_DefaultFloat4;
	static XMFLOAT3 m_DefaultFloat3;
	static XMFLOAT2 m_DefaultFloat2;
};

