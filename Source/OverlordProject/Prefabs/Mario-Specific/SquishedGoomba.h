#pragma once
class SquishedGoomba : public GameObject
{
public:
	SquishedGoomba(const XMFLOAT3& position);
	~SquishedGoomba() override = default;

	SquishedGoomba(const SquishedGoomba& other) = delete;
	SquishedGoomba(SquishedGoomba&& other) noexcept = delete;
	SquishedGoomba& operator=(const SquishedGoomba& other) = delete;
	SquishedGoomba& operator=(SquishedGoomba&& other) noexcept = delete;

protected:
	void Initialize(const SceneContext&) override;
	void Update(const SceneContext&) override;

private:
	const XMFLOAT3 m_Position{};
	const float m_LifeTime{ 1.f };
	const float m_Size{ 2.f };
	float m_TimeCounter{};
	ModelComponent* m_pModelComponent{};
};

