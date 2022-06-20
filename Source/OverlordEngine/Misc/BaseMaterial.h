#pragma once
class BaseMaterial
{
public:
	BaseMaterial() = default;
	virtual ~BaseMaterial();

	BaseMaterial(const BaseMaterial& other) = delete;
	BaseMaterial(BaseMaterial&& other) noexcept = delete;
	BaseMaterial& operator=(const BaseMaterial& other) = delete;
	BaseMaterial& operator=(BaseMaterial&& other) noexcept = delete;

	virtual void Initialize(const D3D11Context& d3d11Context, UINT materialId) = 0;

	const MaterialTechniqueContext& GetTechniqueContext() const { return m_TechniqueContext; }
	const MaterialTechniqueContext& GetTechniqueContext(int index) const;

	UINT GetMaterialId() const { return m_MaterialId; }
	bool HasValidMaterialId() const { return m_MaterialId != UINT_MAX; }
	bool IsValid() const { return HasValidMaterialId() && m_IsInitialized; }

	ID3DX11EffectVariable* GetVariable(const std::wstring& varName) const;

	void SetVariable_Scalar(const std::wstring& varName, float scalar) const;
	void SetVariable_Scalar(const std::wstring& varName, bool scalar) const;
	void SetVariable_Scalar(const std::wstring& varName, int scalar) const;
	void SetVariable_Matrix(const std::wstring& varName, XMFLOAT4X4 matrix) const;
	void SetVariable_Matrix(const std::wstring& varName, const float* pData) const;
	void SetVariable_MatrixArray(const std::wstring& varName, const float* pData, UINT count) const;
	void SetVariable_Vector(const std::wstring& varName, const float* pData) const;
	void SetVariable_Vector(const std::wstring& varName, XMFLOAT3 vector) const;
	void SetVariable_Vector(const std::wstring& varName, XMFLOAT4 vector) const;
	void SetVariable_VectorArray(const std::wstring& varName, const float* pData, UINT count) const;
	void SetVariable_Texture(const std::wstring& varName, const TextureData* pTexture) const;
	void SetVariable_Texture(const std::wstring& varName, ID3D11ShaderResourceView* pSRV) const;

	void SetTechnique(const std::wstring& techName);
	void SetTechnique(int index);
	void ResetMaterialId() { m_MaterialId = UINT_MAX; }

	void SetMaterialName(const std::wstring& name) { m_MaterialName = name; m_MaterialNameUtf8 = StringUtil::utf8_encode(name); }

	void UpdateEffectVariables(const SceneContext& sceneContext, const ModelComponent* pModelComponent);
	void UpdateEffectVariables(const SceneContext& sceneContext, const RenderTarget* pSourceTarget);

	void DrawImGui();

protected:
#pragma region BaseVariables (preset variable by semantic)
	enum class eRootVariable
	{
		WORLD,
		VIEW,
		VIEW_INVERSE,
		WORLD_VIEW_PROJECTION,

		COUNT //@End
	};

	static std::map<std::string, eRootVariable> m_RootVariableSemanticLUT;
	ID3DX11EffectVariable* m_RootVariableLUT[static_cast<UINT>(eRootVariable::COUNT)]{};
#pragma endregion

	virtual const std::map<size_t, UINT>& GetVariableIndexLUT() const = 0;
	virtual int GetRootVariableIndex(eRootVariable rootVariable) const = 0;
	virtual const std::map<size_t, MaterialTechniqueContext>& GetTechniques() const = 0;
	virtual const std::wstring& GetEffectName() const = 0;

	//Internal Material Implementation
	void _baseInitialize(ID3DX11Effect* pRootEffect, UINT materialId);

	virtual void InitializeEffectVariables() = 0;
	virtual void OnUpdateModelVariables(const SceneContext& /*sceneContext*/, const ModelComponent* /*pModel*/) const {};

	MaterialTechniqueContext m_TechniqueContext{};
	UINT m_numTechniques{};
	std::wstring m_MaterialName;
	std::string m_MaterialNameUtf8;
	
private:
	UINT m_MaterialId{UINT_MAX};
	bool m_IsInitialized{};
	bool m_DrawImGui{};

	ID3DX11Effect* m_pEffect{};

	bool NeedsUpdate(UINT frame, UINT id) const;
	UINT m_LastUpdateFrame{};
	UINT m_LastUpdateID{};
};
