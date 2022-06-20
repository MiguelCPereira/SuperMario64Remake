#pragma once

#pragma region ColliderInfo
class ColliderInfo
{
public:
	ColliderInfo(UINT Id, RigidBodyComponent* pOwner, PxShape* pShape) :
		m_ColliderId{Id},
		m_pOwner{pOwner},
		m_pShape(pShape){}

	PxShape* GetShape() const { return m_pShape; }
	bool IsTrigger() const { return m_IsTrigger; }
	const std::wstring& GetTag() const { return m_Tag; }
	RigidBodyComponent* GetOwner() const { return m_pOwner; }
	UINT GetColliderId() const { return m_ColliderId; }

	void SetTag(const std::wstring& tag) { m_Tag = tag; }
	void SetTrigger(bool isTrigger)
	{
		if(m_pShape && m_IsTrigger != isTrigger)
		{
			m_IsTrigger = isTrigger;
			m_pShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, !m_IsTrigger);
			m_pShape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, m_IsTrigger);
		}
	}

	static ColliderInfo INVALID;

private:
	RigidBodyComponent* m_pOwner{};
	PxShape* m_pShape{};
	bool m_IsTrigger{};
	std::wstring m_Tag{};
	UINT m_ColliderId{ UINT_MAX };
};
inline bool operator==(const ColliderInfo& lhs, const ColliderInfo& rhs)
{
	return lhs.GetOwner() == rhs.GetOwner() && lhs.GetShape() == rhs.GetShape();
}
#pragma endregion

class RigidBodyComponent final : public BaseComponent
{
public:
	RigidBodyComponent(bool isStatic = false);
	~RigidBodyComponent() override;

	RigidBodyComponent(const RigidBodyComponent& other) = delete;
	RigidBodyComponent(RigidBodyComponent&& other) noexcept = delete;
	RigidBodyComponent& operator=(const RigidBodyComponent& other) = delete;
	RigidBodyComponent& operator=(RigidBodyComponent&& other) noexcept = delete;


	bool IsStatic()  const { return m_IsStatic; }
	bool IsDynamic()  const { return !m_IsStatic; }
	bool IsKinematic() const { return m_IsKinematic; }
	void SetKinematic(bool isKinematic);
	void PutToSleep() const;
	void WakeUp() const;

	PxRigidActor* GetPxRigidActor() const { return m_pActor; };
	void AddForce(const XMFLOAT3& force, PxForceMode::Enum mode = PxForceMode::eFORCE, bool autowake = true) const;
	void AddTorque(const XMFLOAT3& torque, PxForceMode::Enum mode = PxForceMode::eFORCE, bool autowake = true) const;
	void ClearForce(PxForceMode::Enum mode = PxForceMode::eFORCE) const;
	void ClearTorque(PxForceMode::Enum mode = PxForceMode::eFORCE) const;

	void SetConstraint(RigidBodyConstraint flag, bool enable);
	void SetCollisionGroup(CollisionGroup group);
	void SetCollisionIgnoreGroups(CollisionGroup ignoreGroups);
	void SetDensity(float density) const;

	//Internal Use (use Transform Component for Transformations)
	void Translate(const XMFLOAT3& position) const;
	void Rotate(const XMFLOAT4& rotation) const;
	XMFLOAT3 GetPosition() const;
	XMFLOAT4 GetRotation() const;

	//Shapes (Colliders)
	template<typename T>
	UINT AddCollider(const T& geometry, const PxMaterial& material, bool isTrigger=false, const PxTransform& localPose = PxTransform(PxIdentity));
	void RemoveCollider(const ColliderInfo& colliderInfo);
	void RemoveColliders();
	const std::vector<ColliderInfo>& GetColliders() const { return m_Colliders; }
	ColliderInfo& GetCollider(UINT colliderId);

protected:

	void Initialize(const SceneContext& /*sceneContext*/) override;
	void OnSceneAttach(GameScene* pScene) override;
	void OnSceneDetach(GameScene* pScene) override;
	void OnOwnerAttach(GameObject*) override;
	void OnOwnerDetach(GameObject*) override;

private:
	//Temporary Storage data
	struct ColliderCreationInfo
	{
		UINT colliderId{};
		const std::shared_ptr<PxGeometry> pGeometry{};
		const PxMaterial* pMaterial{};
		PxTransform localPose{};
		bool isTrigger{};
	};
	std::vector<ColliderCreationInfo> m_ColliderCreationInfos{};

	PxRigidActor* m_pActor{};
	std::vector<ColliderInfo> m_Colliders{};

	bool m_IsStatic{};
	bool m_IsKinematic{};

	PxFilterData m_CollisionGroups{};

	RigidBodyConstraint m_InitialConstraints{};

	void CreateActor();
	UINT _AddCollider(const PxGeometry& geometry, const PxMaterial& material, bool isTrigger = false, const PxTransform& localPose = PxTransform(PxIdentity), UINT colliderId = UINT_MAX);
};

template <typename T>
UINT RigidBodyComponent::AddCollider(const T& geometry, const PxMaterial& material, bool isTrigger, const PxTransform& localPose)
{
	if (!m_pActor)
	{
		const UINT colliderId = UINT(m_ColliderCreationInfos.size());
		const std::shared_ptr<PxGeometry> pGeom(new T{ geometry }); //Prevent PxGeometry Object Slicing...

		m_ColliderCreationInfos.push_back({ colliderId, pGeom, &material, localPose, isTrigger});
		return colliderId;
	}

	return _AddCollider(geometry, material, isTrigger, localPose);
}
