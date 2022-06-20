#pragma once

class MeshIndexedDrawComponent final: public BaseComponent
{
public:

	MeshIndexedDrawComponent(UINT vertexCapacity = 50, UINT indexCapacity = 50);
	~MeshIndexedDrawComponent() override;

	MeshIndexedDrawComponent(const MeshIndexedDrawComponent& other) = delete;
	MeshIndexedDrawComponent(MeshIndexedDrawComponent&& other) noexcept = delete;
	MeshIndexedDrawComponent& operator=(const MeshIndexedDrawComponent& other) = delete;
	MeshIndexedDrawComponent& operator=(MeshIndexedDrawComponent&& other) noexcept = delete;

	void AddVertex(VertexPosNormCol vertex, bool updateBuffer = false);
	void AddIndex(unsigned int index, bool updateBuffer = false);

	void ClearVertexList();
	void ClearIndexList();

	void UpdateVertexBuffer();
	void UpdateIndexBuffer();
	void GenerateNormals();

protected:

	void Draw(const SceneContext& sceneContext) override;
	void Initialize(const SceneContext& sceneContext) override;

private:
	static ID3DX11EffectMatrixVariable* m_pWorldVar, * m_pWvpVar;

	std::vector<VertexPosNormCol> m_vecVertices{};
	std::vector<UINT> m_vecIndices{};

	ID3D11Buffer* m_pVertexBuffer{}, * m_pIndexBuffer{};
	unsigned int m_VertexCapacity{}, m_IndexCapacity{};

	ID3DX11Effect* m_pEffect{};
	ID3DX11EffectTechnique* m_pTechnique{};
	ID3D11InputLayout* m_pInputLayout{};

	void LoadEffect(const SceneContext& sceneContext);
	void InitializeVertexBuffer(const SceneContext& sceneContext);
	void InitializeIndexBuffer(const SceneContext& sceneContext);
};
