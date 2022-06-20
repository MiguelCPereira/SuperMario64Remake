#pragma once

class DiffuseMaterial_Shadow;

class Tube : public GameObject
{
public:
	Tube(const XMFLOAT3& initialPosition, const XMFLOAT3& finalPosition, float moveTime);
	~Tube() = default;

	Tube(const Tube& other) = delete;
	Tube(Tube&& other) noexcept = delete;
	Tube& operator=(const Tube& other) = delete;
	Tube& operator=(Tube&& other) noexcept = delete;

	void Reset();

protected:
	void Initialize(const SceneContext&) override;
	void Update(const SceneContext&) override;

private:
	ModelComponent* m_pModelComponent{};
	DiffuseMaterial_Shadow* m_pTubeMaterial{};
	const XMFLOAT3 m_OriginalPosition{};
	const XMFLOAT3 m_FinalPosition{};
	const float m_TimeToHide{};
	const float m_TriggerDistance{ 2.f };
	float m_Counter{ 0.f };
	bool m_ComponentDeleted{};
};

