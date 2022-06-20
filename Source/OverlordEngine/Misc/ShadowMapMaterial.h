#pragma once
class ShadowMapMaterial final: public Material<ShadowMapMaterial>
{
public:
	ShadowMapMaterial();
	~ShadowMapMaterial() override = default;
	ShadowMapMaterial(const ShadowMapMaterial& other) = delete;
	ShadowMapMaterial(ShadowMapMaterial&& other) noexcept = delete;
	ShadowMapMaterial& operator=(const ShadowMapMaterial& other) = delete;
	ShadowMapMaterial& operator=(ShadowMapMaterial&& other) noexcept = delete;

protected:
	void InitializeEffectVariables() override;
};

