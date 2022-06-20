#pragma once
class DiffuseMaterial_Skinned: public Material<DiffuseMaterial_Skinned>
{
public:
	DiffuseMaterial_Skinned();
	~DiffuseMaterial_Skinned() override = default;

	DiffuseMaterial_Skinned(const DiffuseMaterial_Skinned& other) = delete;
	DiffuseMaterial_Skinned(DiffuseMaterial_Skinned&& other) noexcept = delete;
	DiffuseMaterial_Skinned& operator=(const DiffuseMaterial_Skinned& other) = delete;
	DiffuseMaterial_Skinned& operator=(DiffuseMaterial_Skinned&& other) noexcept = delete;

	void SetDiffuseTexture(const std::wstring& assetFile);

protected:
	void InitializeEffectVariables() override;
	void OnUpdateModelVariables(const SceneContext& /*sceneContext*/, const ModelComponent* /*pModel*/) const;

private:
	TextureData* m_pDiffuseTexture{};
};

