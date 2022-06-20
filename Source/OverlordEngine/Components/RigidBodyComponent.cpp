#include "stdafx.h"
#include "RigidBodyComponent.h"

RigidBodyComponent::RigidBodyComponent(bool isStatic) :
	m_IsStatic(isStatic),
	m_CollisionGroups(PxFilterData(static_cast<UINT32>(CollisionGroup::Group0), 0, 0, 1))
{
}

RigidBodyComponent::~RigidBodyComponent()
{
	if (m_pActor != nullptr)
	{
		//Only release actor if not part of a pxScene
		if (!m_pActor->getScene()) m_pActor->release();
	}
}

ColliderInfo& RigidBodyComponent::GetCollider(UINT colliderId)
{
	ASSERT_IF(colliderId >= m_Colliders.size(), L"Collider with Id={} cannot be retrieved. (Make sure the RigidBody is initialized before querying a collider.", colliderId);

	return m_Colliders[colliderId];
}

void RigidBodyComponent::Initialize(const SceneContext&)
{
	if(!m_pActor)
		CreateActor();

	//Create delayed colliders
	for(const auto& cci : m_ColliderCreationInfos)
	{
		_AddCollider(*cci.pGeometry, *cci.pMaterial, cci.isTrigger, cci.localPose);
	}
	m_ColliderCreationInfos.clear();
}

void RigidBodyComponent::OnSceneAttach(GameScene* pScene)
{
	if(m_pActor && !m_pActor->getScene())
	{
		pScene->GetPhysxProxy()->GetPhysxScene()->addActor(*m_pActor);
	}
}

void RigidBodyComponent::OnSceneDetach(GameScene* /*pScene*/)
{
	if(m_pActor)
	{
		if (const auto pxScene = m_pActor->getScene())
		{
			pxScene->removeActor(*m_pActor); //Remove actor from pxScene if component is detached from scenegraph
		}
	}
}

void RigidBodyComponent::OnOwnerAttach(GameObject* pOwner)
{
	pOwner->GetTransform()->SetRigidBodyComponent(this);
}

void RigidBodyComponent::OnOwnerDetach(GameObject* pOwner)
{
	pOwner->GetTransform()->SetRigidBodyComponent(nullptr);
}

#pragma region Collider Logic
UINT RigidBodyComponent::_AddCollider(const PxGeometry& geometry, const PxMaterial& material, bool isTrigger, const PxTransform& localPose, UINT colliderId)
{
	ASSERT_NULL_(m_pActor); //This version of AddCollider can only be called if m_pActor exists

	PxShape* pShape = PxRigidActorExt::createExclusiveShape(*m_pActor, geometry, material);
	pShape->setLocalPose(localPose);
	pShape->setSimulationFilterData(m_CollisionGroups);
	pShape->setQueryFilterData(m_CollisionGroups);

	if(colliderId == UINT_MAX)
	{
		colliderId = UINT(m_ColliderCreationInfos.size() + m_Colliders.size());
	}

	m_Colliders.emplace_back(colliderId, this, pShape);
	m_Colliders.back().SetTrigger(isTrigger);

	return colliderId;
}

void RigidBodyComponent::RemoveCollider(const ColliderInfo& colliderInfo)
{
	if(colliderInfo.GetOwner() != this)
	{
		Logger::LogWarning(L"[RigidBodyComponent] Failed to remove the given collider. (Incorrect owner)");
		return;
	}

	auto it = std::find(m_Colliders.begin(), m_Colliders.end(), colliderInfo);
	if(it == m_Colliders.end())
	{
		Logger::LogWarning(L"[RigidBodyComponent] Failed to remove the given collider. (Collider not found on owner)");
		return;
	}

	PxShape* pShape = colliderInfo.GetShape();
	m_pActor->detachShape(*pShape);
	pShape->release();
	m_Colliders.erase(it);
}

void RigidBodyComponent::RemoveColliders()
{
	for(auto& colliderInfo:m_Colliders)
	{
		PxShape* pShape = colliderInfo.GetShape();
		m_pActor->detachShape(*pShape);
		pShape->release();
	}

	m_Colliders.clear();
}
#pragma endregion

void RigidBodyComponent::SetCollisionIgnoreGroups(CollisionGroup ignoreGroups)
{
	m_CollisionGroups.word1 = static_cast<UINT32>(ignoreGroups);
	for(auto& colliderInfo : m_Colliders)
	{
		colliderInfo.GetShape()->setSimulationFilterData(m_CollisionGroups);
	}
}

void RigidBodyComponent::SetCollisionGroup(CollisionGroup group)
{
	m_CollisionGroups.word0 = static_cast<UINT32>(group);
	for (auto& colliderInfo : m_Colliders)
	{
		colliderInfo.GetShape()->setSimulationFilterData(m_CollisionGroups);
		colliderInfo.GetShape()->setQueryFilterData(m_CollisionGroups);
	}
}

void RigidBodyComponent::SetKinematic(bool isKinematic)
{
	if (m_IsStatic && isKinematic) {
		Logger::LogWarning(L"A static RigidBody can never be kinematic");
		return;
	}

	m_IsKinematic = isKinematic;

	if (m_pActor != nullptr && !m_IsStatic)
	{
		m_pActor->is<PxRigidDynamic>()->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, m_IsKinematic);
	}
}

void RigidBodyComponent::CreateActor()
{
	ASSERT_IF(m_pActor != nullptr, L"CreateActor cannot be called multiple times")

	const auto pPhysX = PhysXManager::Get()->GetPhysics();
	const auto pPhysxScene = GetGameObject()->GetScene()->GetPhysxProxy()->GetPhysxScene();
	const auto pTransform = GetTransform();

	if (m_IsStatic)
		m_pActor = pPhysX->createRigidStatic(PxTransform(PhysxHelper::ToPxVec3(pTransform->GetPosition()), PhysxHelper::ToPxQuat(pTransform->GetRotation())));
	else
	{
		m_pActor = pPhysX->createRigidDynamic(PxTransform(PhysxHelper::ToPxVec3(pTransform->GetPosition()), PhysxHelper::ToPxQuat(pTransform->GetRotation())));
		m_pActor->is<PxRigidDynamic>()->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, m_IsKinematic);

		if (m_InitialConstraints != RigidBodyConstraint::None)
		{
			SetConstraint(RigidBodyConstraint::All, true);
			SetConstraint(m_InitialConstraints, false);
		}
	}

	pPhysxScene->addActor(*m_pActor);
	m_pActor->userData = this;
}

void RigidBodyComponent::SetDensity(float density) const
{
	if (m_pActor && m_pActor->is<PxRigidDynamic>())
	{
		PxRigidBodyExt::updateMassAndInertia(*m_pActor->is<PxRigidDynamic>(), density);
	}
}

void RigidBodyComponent::Translate(const XMFLOAT3& position) const
{
	ASSERT_NULL_(m_pActor)
	const PxTransform localPose = PxTransform(PhysxHelper::ToPxVec3(position), PhysxHelper::ToPxQuat(GetRotation()));

	if (!m_IsKinematic)
	{
		m_pActor->setGlobalPose(localPose);

		if (!m_IsStatic)
			m_pActor->is<PxRigidDynamic>()->setLinearVelocity(PxVec3{ PxZero }); //Non-Kinematic Only
	}
	else
		m_pActor->is<PxRigidDynamic>()->setKinematicTarget(localPose);
}

void RigidBodyComponent::Rotate(const XMFLOAT4& rotation) const
{
	ASSERT_NULL_(m_pActor)
	const PxTransform localPose = PxTransform(PhysxHelper::ToPxVec3(GetPosition()), PhysxHelper::ToPxQuat(rotation));

	if (!m_IsKinematic) {
		m_pActor->setGlobalPose(localPose);

		if(!m_IsStatic)
			m_pActor->is<PxRigidDynamic>()->setAngularVelocity(PxVec3{ PxZero }); //Non-Kinematic Only!
	}
	else
		m_pActor->is<PxRigidDynamic>()->setKinematicTarget(localPose);

}

XMFLOAT3 RigidBodyComponent::GetPosition() const
{
	ASSERT_NULL_(m_pActor);

	PxTransform pose;
	if (!m_IsKinematic || !m_pActor->is<PxRigidDynamic>()->getKinematicTarget(pose))
		pose = m_pActor->getGlobalPose();

	return { pose.p.x, pose.p.y, pose.p.z };
}

XMFLOAT4 RigidBodyComponent::GetRotation() const
{
	ASSERT_NULL_(m_pActor);
	PxTransform pose;
	if (!m_IsKinematic || !reinterpret_cast<PxRigidDynamic*>(m_pActor)->getKinematicTarget(pose))
		pose = m_pActor->getGlobalPose();

	return { pose.q.x, pose.q.y, pose.q.z, pose.q.w };
}

void RigidBodyComponent::PutToSleep() const
{
	if (m_pActor != nullptr && !m_IsStatic)
		m_pActor->is<PxRigidDynamic>()->putToSleep();
	else
		Logger::LogWarning(L"Cannot put a static actor to sleep");
}

void RigidBodyComponent::WakeUp() const
{
	if (m_pActor != nullptr && !m_IsStatic)
		m_pActor->is<PxRigidDynamic>()->wakeUp();
	else
		Logger::LogWarning(L"Cannot wake-up a static actor");
}

#pragma warning(push)
#pragma warning(disable: 26812)
void RigidBodyComponent::AddForce(const XMFLOAT3& force, PxForceMode::Enum mode, bool autowake) const
{
	if (m_pActor != nullptr && !m_IsStatic && !m_IsKinematic)
		m_pActor->is<PxRigidDynamic>()->addForce(PhysxHelper::ToPxVec3(force), mode, autowake);
	else
		Logger::LogWarning(L"Cannot apply a force on a static or kinematic actor");
}
#pragma warning(pop)

void RigidBodyComponent::AddTorque(const XMFLOAT3& torque, PxForceMode::Enum mode, bool autowake) const
{
	if (m_pActor != nullptr && !m_IsStatic && !m_IsKinematic)
		m_pActor->is<PxRigidDynamic>()->addTorque(PhysxHelper::ToPxVec3(torque), mode, autowake);
	else
		Logger::LogWarning(L"Cannot apply torque on a static or kinematic actor");
}

void RigidBodyComponent::ClearForce(PxForceMode::Enum mode) const
{
	if (m_pActor != nullptr && !m_IsStatic && !m_IsKinematic)
		m_pActor->is<PxRigidDynamic>()->clearForce(mode);
	else
		Logger::LogWarning(L"Cannot clear force on a static or kinematic actor");
}

void RigidBodyComponent::ClearTorque(PxForceMode::Enum mode) const
{
	if (m_pActor != nullptr && !m_IsStatic && !m_IsKinematic)
		m_pActor->is<PxRigidDynamic>()->clearTorque(mode);
	else
		Logger::LogWarning(L"Cannot clear torque on a static or kinematic actor");
}

void RigidBodyComponent::SetConstraint(RigidBodyConstraint flag, bool enable)
{
	if (m_IsStatic || m_IsKinematic)
	{
		Logger::LogWarning(L"Can't constrain a static or kinematic actor!");
		return;
	}

	if(m_pActor)
	{
		//TODO: Refactor!!
		const auto pRigidBody = m_pActor->is<PxRigidDynamic>();
		if (isSet(flag, RigidBodyConstraint::RotX))
		{
			pRigidBody->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, !enable);
		}
		if (isSet(flag, RigidBodyConstraint::RotY))
		{
			pRigidBody->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, !enable);
		}
		if (isSet(flag, RigidBodyConstraint::RotZ))
		{
			pRigidBody->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, !enable);
		}
		if (isSet(flag, RigidBodyConstraint::TransX))
		{
			pRigidBody->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_LINEAR_X, !enable);
		}
		if (isSet(flag, RigidBodyConstraint::TransY))
		{
			pRigidBody->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_LINEAR_Y, !enable);
		}
		if (isSet(flag, RigidBodyConstraint::TransZ))
		{
			pRigidBody->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_LINEAR_Z, !enable);
		}
	}
	else
	{
		if (!enable) m_InitialConstraints |= flag;
		else m_InitialConstraints &= ~flag;
	}
}
