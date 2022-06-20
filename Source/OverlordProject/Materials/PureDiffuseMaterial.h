#pragma once
class PureDiffuseMaterial final : public Material<PureDiffuseMaterial>
{
public:
	PureDiffuseMaterial();
	~PureDiffuseMaterial() override = default;

	PureDiffuseMaterial(const PureDiffuseMaterial& other) = delete;
	PureDiffuseMaterial(PureDiffuseMaterial&& other) noexcept = delete;
	PureDiffuseMaterial& operator=(const PureDiffuseMaterial& other) = delete;
	PureDiffuseMaterial& operator=(PureDiffuseMaterial&& other) noexcept = delete;

	void SetDiffuseTexture(const std::wstring& assetFile);

protected:
	void InitializeEffectVariables() override;

private:
	TextureData* m_pDiffuseTexture{};

};
