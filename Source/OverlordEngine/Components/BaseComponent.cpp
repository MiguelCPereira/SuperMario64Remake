#include "stdafx.h"

UINT BaseComponent::m_ComponentCounter{};

BaseComponent::BaseComponent()
{
	++m_ComponentCounter;
	m_ComponentId = m_ComponentCounter;
}

void BaseComponent::RootInitialize(const SceneContext& sceneContext)
{
	//assert(!m_IsInitialized); //Shouldn't be called more than once...
	if (m_IsInitialized) return;
	Initialize(sceneContext);
	m_IsInitialized = true;
}

void BaseComponent::RootOnSceneAttach(GameScene* pScene)
{
	ASSERT_NULL(pScene, L"GameScene is NULL");

	m_pScene = pScene;

	if(!m_IsInitialized)
	{
		RootInitialize(pScene->GetSceneContext());
	}

	//Signal Derived
	OnSceneAttach(pScene);
}

void BaseComponent::RootOnSceneDetach(GameScene* pScene)
{
	ASSERT_NULL(pScene, L"GameScene is NULL");

	m_pScene = pScene;

	//Signal Derived
	OnSceneDetach(pScene);
}

TransformComponent* BaseComponent::GetTransform() const
{
#if _DEBUG
	if(m_pGameObject == nullptr)
	{
		Logger::LogWarning(L"Failed to retrieve the TransformComponent. GameObject is NULL.");
		return nullptr;
	}
#endif

	return m_pGameObject->GetTransform();
}
