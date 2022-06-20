#pragma once
class DiffuseMaterial_Shadow final : public Material<DiffuseMaterial_Shadow>
{
public:
	DiffuseMaterial_Shadow();
	~DiffuseMaterial_Shadow() override = default;

	DiffuseMaterial_Shadow(const DiffuseMaterial_Shadow& other) = delete;
	DiffuseMaterial_Shadow(DiffuseMaterial_Shadow&& other) noexcept = delete;
	DiffuseMaterial_Shadow& operator=(const DiffuseMaterial_Shadow& other) = delete;
	DiffuseMaterial_Shadow& operator=(DiffuseMaterial_Shadow&& other) noexcept = delete;

	void SetDiffuseTexture(const std::wstring& assetFile);

protected:
	void InitializeEffectVariables() override;
	void OnUpdateModelVariables(const SceneContext&, const ModelComponent*) const override;

private:
	TextureData* m_pDiffuseTexture{};
};

