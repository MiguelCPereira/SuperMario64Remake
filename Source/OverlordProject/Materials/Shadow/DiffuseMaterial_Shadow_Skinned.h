#pragma once
class DiffuseMaterial_Shadow_Skinned: public Material<DiffuseMaterial_Shadow_Skinned>
{
public:
	DiffuseMaterial_Shadow_Skinned();
	~DiffuseMaterial_Shadow_Skinned() override = default;

	DiffuseMaterial_Shadow_Skinned(const DiffuseMaterial_Shadow_Skinned& other) = delete;
	DiffuseMaterial_Shadow_Skinned(DiffuseMaterial_Shadow_Skinned&& other) noexcept = delete;
	DiffuseMaterial_Shadow_Skinned& operator=(const DiffuseMaterial_Shadow_Skinned& other) = delete;
	DiffuseMaterial_Shadow_Skinned& operator=(DiffuseMaterial_Shadow_Skinned&& other) noexcept = delete;

	void SetDiffuseTexture(const std::wstring& assetFile);

protected:
	void InitializeEffectVariables() override;
	void OnUpdateModelVariables(const SceneContext& /*sceneContext*/, const ModelComponent* /*pModel*/) const override;

private:
	TextureData* m_pDiffuseTexture{};
};

