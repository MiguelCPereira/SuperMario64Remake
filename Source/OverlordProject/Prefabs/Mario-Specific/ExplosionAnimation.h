#pragma once
class ExplosionAnimation : public GameObject
{
public:
	ExplosionAnimation(const XMFLOAT3& position);
	~ExplosionAnimation() override = default;

	ExplosionAnimation(const ExplosionAnimation& other) = delete;
	ExplosionAnimation(ExplosionAnimation&& other) noexcept = delete;
	ExplosionAnimation& operator=(const ExplosionAnimation& other) = delete;
	ExplosionAnimation& operator=(ExplosionAnimation&& other) noexcept = delete;

protected:
	void Initialize(const SceneContext&) override;
	void Update(const SceneContext&) override;

private:
	const XMFLOAT3 m_Position{};
	const int m_FrameNumber{ 5 };
	const float m_LifeTime{ 0.2f };
	const float m_Size{ 2.f };
	float m_TimeCounter{};
	int m_SpriteCounter{};
	ParticleEmitterComponent* m_pEmitter{};
};

