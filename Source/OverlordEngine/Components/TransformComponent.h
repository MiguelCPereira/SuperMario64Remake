#pragma once

class ControllerComponent;
class RigidBodyComponent;

class TransformComponent : public BaseComponent
{
public:
	TransformComponent();
	~TransformComponent() override = default;

	TransformComponent(const TransformComponent& other) = delete;
	TransformComponent(TransformComponent&& other) noexcept = delete;
	TransformComponent& operator=(const TransformComponent& other) = delete;
	TransformComponent& operator=(TransformComponent&& other) noexcept = delete;


	void Translate(float x, float y, float z);
	void Translate(const XMFLOAT3& position);
	void Translate(const XMVECTOR& position);

	void Rotate(float x, float y, float z, bool degrees = true);
	void Rotate(const XMFLOAT3& rotation, bool degrees = true);
	void Rotate(const XMVECTOR& rotation, bool isQuaternion = true);

	void Scale(float x, float y, float z);
	void Scale(float s);
	void Scale(const XMFLOAT3& scale);

	const XMFLOAT3& GetPosition() const { return m_Position; }
	const XMFLOAT3& GetWorldPosition() const { return m_WorldPosition; }
	const XMFLOAT3& GetScale() const { return m_Scale; }
	const XMFLOAT3& GetWorldScale() const { return m_WorldScale; }
	const XMFLOAT4& GetRotation() const { return m_Rotation; }
	const XMFLOAT4& GetWorldRotation() const { return m_WorldRotation; }
	const XMFLOAT4X4& GetWorld() const { return m_World; }

	const XMFLOAT3& GetForward() const { return m_Forward; }
	const XMFLOAT3& GetUp() const { return m_Up; }
	const XMFLOAT3& GetRight() const { return m_Right; }

	bool IsDirty() const { return m_IsDirty; }
	void SetRigidBodyComponent(RigidBodyComponent* pRigidBody) { m_pRigidBodyComponent = pRigidBody; }
	void SetControllerComponent(ControllerComponent* pController) { m_pControllerComponent = pController; }

protected:

	void Initialize(const SceneContext& sceneContext) override;
	void Update(const SceneContext& sceneContext) override;

	void UpdateTransforms();
	bool CheckConstraints() const;

	bool CheckIfDirty();
	bool m_IsDirty{};

private:

	XMFLOAT3 m_Position{}, m_WorldPosition{};
	XMFLOAT3 m_Scale{}, m_WorldScale{};
	XMFLOAT3 m_Forward{}, m_Up{}, m_Right{};
	XMFLOAT4 m_Rotation{}, m_WorldRotation{};
	XMFLOAT4X4 m_World{};
	TransformChanged m_IsTransformChanged{};

	RigidBodyComponent* m_pRigidBodyComponent{};
	ControllerComponent* m_pControllerComponent{};
};
