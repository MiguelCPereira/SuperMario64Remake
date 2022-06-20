#pragma once

class UberMaterial final : public Material<UberMaterial>
{
public:
	UberMaterial();
	~UberMaterial() override = default;

	UberMaterial(const UberMaterial& other) = delete;
	UberMaterial(UberMaterial&& other) noexcept = delete;
	UberMaterial& operator=(const UberMaterial& other) = delete;
	UberMaterial& operator=(UberMaterial&& other) noexcept = delete;

	void SetDiffuseTexture(const std::wstring& assetFile, XMFLOAT4 diffuseColor = {1,1,1,1});
	void SetNormalTexture(const std::wstring& assetFile);
	void SetCubemapTexture(const std::wstring& assetFile, float reflectionStrength = 1.0f, float refractionStrength = 1.0f, float fresnelHardness = 0.5f);
	void SetSpecularTexture(const std::wstring& assetFile, bool usePhong = true, bool useBlinn = false);
	void SetOpacityTexture(const std::wstring& assetFile);

protected:
	void InitializeEffectVariables() override;

private:
	TextureData* m_pDiffuseTexture{};
	TextureData* m_pNormalTexture{};
	TextureData* m_pCubemapTexture{};
	TextureData* m_pSpecularTexture{};
	TextureData* m_pOpacityTexture{};
};

