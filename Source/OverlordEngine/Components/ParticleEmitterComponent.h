#pragma once
class ParticleMaterial;

struct ParticleEmitterSettings
{
	float minSize{ .1f }; //The minimum size each particle can be at the time when it is spawned
	float maxSize{ 2.f }; //The maximum size each particle can be at the time when it is spawned

	float minEnergy{ 1.f }; //The minimum lifetime of each particle, measured in seconds
	float maxEnergy{ 2.f }; //The maximum lifetime of each particle, measured in seconds

	float minEmitterRadius{ 9.f }; //The minimum radius that the particles are spawned in
	float maxEmitterRadius{ 10.f }; //The maximum radius that the particles are spawned in

	float minScale{ 1.f }; //The percentual minimum change in size/scale during the particle's lifetime
	float maxScale{ 1.f }; //The percentual maximum change in size/scale during the particle's lifetime

	XMFLOAT3 velocity{}; //The initial speed & (relative) direction of particles along X, Y and Z
	XMFLOAT4 color{ XMFLOAT4{Colors::White } }; //The color of a particle
};

struct Particle
{
	VertexParticle vertexInfo{};

	bool isActive{ false };

	float totalEnergy{};
	float currentEnergy{};

	float initialSize{};
	float sizeChange{};

	XMFLOAT3 velocity{};
};

class ParticleEmitterComponent : public BaseComponent
{
public:
	ParticleEmitterComponent(const std::wstring& assetFile, const ParticleEmitterSettings& emitterSettings = {},
		UINT particleCount = 50, bool startSpawning = true, bool inCircle = false, float initialCircleRadius = 0.f);
	~ParticleEmitterComponent() override;
	ParticleEmitterComponent(const ParticleEmitterComponent& other) = delete;
	ParticleEmitterComponent(ParticleEmitterComponent&& other) noexcept = delete;
	ParticleEmitterComponent& operator=(const ParticleEmitterComponent& other) = delete;
	ParticleEmitterComponent& operator=(ParticleEmitterComponent&& other) noexcept = delete;

	ParticleEmitterSettings& GetSettings() { return m_EmitterSettings; }; //EmitterSettings Getter (by reference) > allows settings changes
	void DrawImGui();
	void SwitchTexture(const std::wstring& assetFile) { m_AssetFile = assetFile; m_pParticleTexture = ContentManager::Load<TextureData>(assetFile); }

	void TogglePause(bool paused);
	void ToggleSpawning(bool spawning);

protected:
	void Initialize(const SceneContext&) override;
	void Update(const SceneContext&) override;
	void PostDraw(const SceneContext&) override;

private:
	void CreateVertexBuffer(const SceneContext& sceneContext); //Method to create the vertex buffer
	void UpdateParticle(Particle& p, float elapsedTime) const;
	void SpawnParticle(Particle& p, XMFLOAT3 velocity, XMFLOAT3 position);

	TextureData* m_pParticleTexture{};
	static ParticleMaterial* m_pParticleMaterial; //Material used to render the particles (static >> shared by all emitters)
	ParticleEmitterSettings m_EmitterSettings{}; //The settings for this particle system

	ID3D11Buffer* m_pVertexBuffer{}; //The vertex buffer, containing ParticleVertex information for each Particle

	Particle* m_ParticlesArray; //Array of particle objects
	UINT m_ParticleCount{}; //The total amount of particles
	UINT m_MaxParticles{};
	UINT m_ActiveParticles{}; //The active particles for the current frame
	float m_LastParticleSpawn{}; //Total seconds since the last created particle
	std::wstring m_AssetFile{};

	bool m_DrawImGui{ false };

	bool m_Paused{ false };
	bool m_Spawning{ true };

	bool m_InCircle{};
	float m_InitialCircleRadius{};
};

