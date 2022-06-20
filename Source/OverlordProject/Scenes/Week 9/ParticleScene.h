#pragma once
class ParticleScene: public GameScene
{
public:
	ParticleScene():GameScene(L"ParticleScene"){}
	~ParticleScene() override = default;
	ParticleScene(const ParticleScene& other) = delete;
	ParticleScene(ParticleScene&& other) noexcept = delete;
	ParticleScene& operator=(const ParticleScene& other) = delete;
	ParticleScene& operator=(ParticleScene&& other) noexcept = delete;

protected:
	void Initialize() override;
	void Update() override;
	void OnGUI() override;

private:
	bool m_AutoMove{ true };

	ParticleEmitterComponent* m_pEmitter{};
	GameObject* m_pTeapot{};
};

