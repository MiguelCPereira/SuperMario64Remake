#pragma once
class ParticleMaterial: public Material<ParticleMaterial>
{
public:
	ParticleMaterial();
	~ParticleMaterial() override = default;
	ParticleMaterial(const ParticleMaterial& other) = delete;
	ParticleMaterial(ParticleMaterial&& other) noexcept = delete;
	ParticleMaterial& operator=(const ParticleMaterial& other) = delete;
	ParticleMaterial& operator=(ParticleMaterial&& other) noexcept = delete;

protected:
	void InitializeEffectVariables() override;
};

