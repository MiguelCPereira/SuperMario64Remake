#pragma once
class ShadowMapMaterial;

class ShadowMapRenderer: public Singleton<ShadowMapRenderer>
{
public:
	ShadowMapRenderer(const ShadowMapRenderer& other) = delete;
	ShadowMapRenderer(ShadowMapRenderer&& other) noexcept = delete;
	ShadowMapRenderer& operator=(const ShadowMapRenderer& other) = delete;
	ShadowMapRenderer& operator=(ShadowMapRenderer&& other) noexcept = delete;

	void UpdateMeshFilter(const SceneContext& sceneContext, MeshFilter* pMeshFilter) const;

	void Begin(const SceneContext&);
	void DrawMesh(const SceneContext& sceneContext, MeshFilter* pMeshFilter, const XMFLOAT4X4& meshWorld, const std::vector<XMFLOAT4X4>& meshBones = {});
	void End(const SceneContext&) const;

	ID3D11ShaderResourceView* GetShadowMap() const;
	const XMFLOAT4X4& GetLightVP() const { return m_LightVP; }

	void Debug_DrawDepthSRV(const XMFLOAT2& position = { 0.f,0.f }, const XMFLOAT2& scale = { 1.f,1.f }, const XMFLOAT2& pivot = {0.f,0.f}) const;

protected:
	void Initialize() override;

private:
	friend class Singleton<ShadowMapRenderer>;
	ShadowMapRenderer() = default;
	~ShadowMapRenderer();

	//Rendertarget to render the 'shadowmap' to (depth-only)
	//Contains depth information for all rendered shadow-casting meshes from a light's perspective (usual the main directional light)
	RenderTarget* m_pShadowRenderTarget{ nullptr };

	//Light ViewProjection (perspective used to render ShadowMap)
	XMFLOAT4X4 m_LightVP{};

	//Shadow Generator is responsible of drawing all shadow casting meshes to the ShadowMap
	//There are two techniques, one for static (non-skinned) meshes, and another for skinned meshes (with bones, blendIndices, blendWeights)
	enum class ShadowGeneratorType
	{
		Static,
		Skinned
	};

	ShadowMapMaterial* m_pShadowMapGenerator{ nullptr };

	//Information about each technique (static/skinned) is stored in a MaterialTechniqueContext structure
	//This information is automatically create by the Material class, we only store it in a local array for fast retrieval 
	static int const NUM_TYPES{ 2 };
	MaterialTechniqueContext m_GeneratorTechniqueContexts[NUM_TYPES];
};

