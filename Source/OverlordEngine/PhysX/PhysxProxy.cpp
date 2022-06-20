#include "stdafx.h"
#include "PhysxProxy.h"
#include <characterkinematic/PxControllerManager.h>

bool PhysxProxy::m_PhysXFrameStepping{false};
float PhysxProxy::m_PhysXStepTime{0.f};

PhysxProxy::~PhysxProxy()
{
	if (m_pControllerManager != nullptr)
		m_pControllerManager->release();
	if (m_pPhysxScene != nullptr)
		m_pPhysxScene->release();
}

void PhysxProxy::Initialize(GameScene* pParent)
{
	if (m_IsInitialized)
	{
		Logger::LogWarning(L"Multiple Initialization attempts!");
		return;
	}

	m_pPhysxScene = PhysXManager::Get()->CreateScene(pParent);
	ASSERT_IF(!m_pPhysxScene, L"Failed to create physx scene!")

	m_pPhysxScene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
	m_pPhysxScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);
	m_pPhysxScene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LIMITS, 1.0f);
	m_pPhysxScene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LOCAL_FRAMES, 1.0f);
	m_pPhysxScene->setSimulationEventCallback(this);

	if (!m_pPhysxScene) return; //Prevent C6011
	m_pControllerManager = PxCreateControllerManager(*m_pPhysxScene);
	ASSERT_IF(m_pControllerManager == nullptr, L"Failed to create controller manager!")

	m_IsInitialized = true;
}

void PhysxProxy::Update(const SceneContext& sceneContext) const
{
	if (sceneContext.pGameTime->IsRunning() && sceneContext.pGameTime->GetElapsed() > 0)
	{
		if (m_PhysXFrameStepping)
		{
			if (m_PhysXStepTime > 0.f)
			{
				m_pPhysxScene->simulate(m_PhysXStepTime);
				m_pPhysxScene->fetchResults(true);
				m_PhysXStepTime = 0.f;
			}
			else if (m_PhysXStepTime < 0.f)
			{
				m_pPhysxScene->simulate(sceneContext.pGameTime->GetElapsed());
				m_pPhysxScene->fetchResults(true);
			}
		}
		else
		{
			m_pPhysxScene->simulate(sceneContext.pGameTime->GetElapsed());
			m_pPhysxScene->fetchResults(true);
		}
	}

#ifdef _DEBUG
	//Send Camera to PVD
	if (m_pPhysxScene->getScenePvdClient())
	{
		const auto pCameraTransform = sceneContext.pCamera->GetTransform();
		XMFLOAT3 cameraTarget{};
		XMStoreFloat3(&cameraTarget ,XMLoadFloat3(& pCameraTransform->GetWorldPosition()) + (XMLoadFloat3(&pCameraTransform->GetForward()) * 10.f));
		m_pPhysxScene->getScenePvdClient()->updateCamera("SceneCam", PhysxHelper::ToPxVec3(pCameraTransform->GetWorldPosition()), PhysxHelper::ToPxVec3(pCameraTransform->GetUp()), PhysxHelper::ToPxVec3(cameraTarget));
	}
#endif
}

void PhysxProxy::Draw(const SceneContext& sceneContext) const
{
	if (sceneContext.settings.drawPhysXDebug)
		DebugRenderer::DrawPhysX(m_pPhysxScene);
}

void PhysxProxy::onTrigger(PxTriggerPair* pairs, PxU32 count)
{
	for (PxU32 i = 0; i < count; i++)
	{
		// ignore pairs when shapes have been deleted
		if (pairs[i].flags & (PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER | PxTriggerPairFlag::
			eREMOVED_SHAPE_OTHER))
			continue;

		const auto triggerComponent = reinterpret_cast<BaseComponent*>(((pairs[i].triggerShape)->getActor())->userData);
		const auto otherComponent = reinterpret_cast<BaseComponent*>(((pairs[i].otherShape)->getActor())->userData);


		if (triggerComponent != nullptr && otherComponent != nullptr)
		{
			GameObject* trigger = triggerComponent->GetGameObject();
			GameObject* other = otherComponent->GetGameObject();

			if (pairs[i].status & PxPairFlag::eNOTIFY_TOUCH_FOUND)
				trigger->OnTrigger(trigger, other, PxTriggerAction::ENTER);
			else if (pairs[i].status & PxPairFlag::eNOTIFY_TOUCH_LOST)
				trigger->OnTrigger(trigger, other, PxTriggerAction::LEAVE);
		}
	}
}

bool PhysxProxy::Raycast(const PxVec3& origin, const PxVec3& unitDir, const PxReal distance,
                         PxRaycastCallback& hitCall, PxHitFlags hitFlags,
                         const PxQueryFilterData& filterData, PxQueryFilterCallback* filterCall,
                         const PxQueryCache* cache) const
{
	if (m_pPhysxScene != nullptr)
	{
		return m_pPhysxScene->raycast(origin, unitDir, distance, hitCall, hitFlags, filterData, filterCall, cache);
	}

	Logger::LogWarning(L"Raycast failed. Physics scene is a null pointer");
	return false;
}
