#include "stdafx.h"
#include "GameObject.h"
#include <algorithm>

GameObject::GameObject():
	m_IsActive(true),
	m_pTransform(new TransformComponent{})
{
	AddComponent(m_pTransform);
}
GameObject::~GameObject()
{
	for(BaseComponent* pComp: m_pComponents)
	{
		SafeDelete(pComp);
	}

	for(GameObject* pChild: m_pChildren)
	{
		SafeDelete(pChild);
	}
}

void GameObject::RootInitialize(const SceneContext& sceneContext)
{
	//We don't want to call init again for performances, but this is a safety net we we do a early exit instead. Could be fixed by separate objects that still need init.
	//assert(!m_IsInitialized); 

	if(m_IsInitialized)
		return;

	//User-Object Initialization
	Initialize(sceneContext);

	
	//Root-Component Initialization
	for(BaseComponent* pComp: m_pComponents)
	{
		pComp->RootInitialize(sceneContext);
	}

	//Root-Object Initialization
	for(GameObject* pChild: m_pChildren)
	{
		pChild->RootInitialize(sceneContext);
	}

	m_IsInitialized = true;
}

void GameObject::RootPostInitialize(const SceneContext& sceneContext)
{
	PostInitialize(sceneContext);

	//Root-Component Initialization
	for (BaseComponent* pComp : m_pComponents)
	{
		pComp->PostInitialize(sceneContext);
	}

	//Root-Object Initialization
	for (GameObject* pChild : m_pChildren)
	{
		pChild->RootPostInitialize(sceneContext);
	}
}

void GameObject::RootUpdate(const SceneContext& sceneContext)
{
	//User-Object Update
	Update(sceneContext);
	
	//Component Update
	for(BaseComponent* pComp: m_pComponents)
	{
		pComp->Update(sceneContext);
	}

	//Root-Object Update
	for(GameObject* pChild: m_pChildren)
	{
		pChild->RootUpdate(sceneContext);
	}
}
void GameObject::RootDraw(const SceneContext& sceneContext)
{
	//User-Object Draw
	Draw(sceneContext);

	//Component Draw
	for(BaseComponent* pComp: m_pComponents)
	{
		pComp->Draw(sceneContext);
	}

	//Root-Object Draw
	for(GameObject* pChild: m_pChildren)
	{
		pChild->RootDraw(sceneContext);
	}
}
void GameObject::RootPostDraw(const SceneContext& sceneContext)
{
	//Post-Draw
	PostDraw(sceneContext);

	//Component Post-Draw
	for (BaseComponent* pComp : m_pComponents)
	{
		if(pComp->m_enablePostDraw)
			pComp->PostDraw(sceneContext);
	}

	//Root-Object Post-Draw
	for (GameObject* pChild : m_pChildren)
	{
		pChild->RootPostDraw(sceneContext);
	}
}

void GameObject::RootShadowMapDraw(const SceneContext& sceneContext) const
{
	//Component Shadow-Draw
	for (BaseComponent* pComp : m_pComponents)
	{
		if(pComp->m_enableShadowMapDraw)
			pComp->ShadowMapDraw(sceneContext);
	}

	//Root-Object Shadow-Draw
	for (GameObject* pChild : m_pChildren)
	{
		pChild->RootShadowMapDraw(sceneContext);
	}
}

void GameObject::RootOnSceneAttach(GameScene* pScene)
{
	ASSERT_NULL(pScene, L"OnSceneAttach called but GameScene is NULL");

	//Set Scene
	m_pParentScene = pScene;
	
	//Initialize Object if not yet initialized
	if(!m_IsInitialized)
	{
		RootInitialize(pScene->GetSceneContext());
	}

	//Call derived OnSceneAttach
	OnSceneAttach(pScene);

	//Signal Components
	for (BaseComponent* pComponent : m_pComponents)
	{
		pComponent->RootOnSceneAttach(pScene);
	}

	//Signal Children
	for(GameObject* pChild:m_pChildren)
	{
		pChild->RootOnSceneAttach(pScene);
	}
}

void GameObject::RootOnSceneDetach(GameScene* pScene)
{
	ASSERT_IF(pScene == nullptr, L"OnSceneDetach called but GameScene is NULL");

	//Signal derived OnSceneDetach
	OnSceneDetach(pScene);

	//Signal Components
	for (BaseComponent* pComponent : m_pComponents)
	{
		pComponent->RootOnSceneDetach(pScene);
	}

	//Signal Children
	for (GameObject* pChild : m_pChildren)
	{
		pChild->RootOnSceneDetach(pScene);
	}

	//Reset Scene
	m_pParentScene = nullptr;
}

void GameObject::AddChild_(GameObject* pObject)
{
#if _DEBUG
	if (pObject->m_pParentObject)
	{
		if (pObject->m_pParentObject == this)
			Logger::LogWarning(L"GameObject::AddChild > GameObject to add is already attached to this parent");
		else
			Logger::LogWarning(L"GameObject::AddChild > GameObject to add is already attached to another GameObject. Detach it from it's current parent before attaching it to another one.");

		return;
	}

	if (pObject->m_pParentScene)
	{
		Logger::LogWarning(L"GameObject::AddChild > GameObject is currently attached to a GameScene. Detach it from it's current parent before attaching it to another one.");
		return;
	}
#endif

	//Set parent & add to Children list
	pObject->m_pParentObject = this;
	m_pChildren.push_back(pObject);

	//Signal object (Attached to parent)
	pObject->OnParentAttach(this); 

	//Signal object & children (Attached to scenegraph)
	if(GameScene* pScene = GetScene()) 
		pObject->RootOnSceneAttach(pScene);
}

void GameObject::RemoveChild(GameObject* obj, bool deleteObject)
{
	const auto it = std::ranges::find(m_pChildren, obj);

#if _DEBUG
	if(it == m_pChildren.end())
	{
		Logger::LogWarning(L"GameObject::RemoveChild > GameObject to remove is not attached to this GameObject!");
		return;
	}
#endif

	m_pChildren.erase(it);

	//Reset object parent pointer
	obj->m_pParentObject = nullptr;

	//Signal object (Parent Detached)
	obj->OnParentDetach(this);

	//Signal object and children if detached from scenegraph (Scene Detached)
	if(GameScene* pScene = GetScene())
		obj->RootOnSceneDetach(pScene);

	if(deleteObject)
	{
		SafeDelete(obj);
	}
}

void GameObject::AddComponent_(BaseComponent* pComponent)
{
#if _DEBUG
	if (typeid(*pComponent) == typeid(TransformComponent) && HasComponent<TransformComponent>())
	{
		Logger::LogWarning(L"GameObject::AddComponent > GameObject can contain only one TransformComponent!");
		return;
	}

	for (auto* pComp : m_pComponents)
	{
		if (pComp == pComponent)
		{
			Logger::LogWarning(L"GameObject::AddComponent > GameObject already contains this component!");
			return;
		}
	}
#endif

	m_pComponents.push_back(pComponent);
	pComponent->m_pGameObject = this;	

	//Signal Component (Attached to GameObject)
	pComponent->OnOwnerAttach(this);

	//If object is already part of scenegraph, signal component about the 'new' scene
	if (GameScene* pScene = GetScene()) 
		pComponent->RootOnSceneAttach(pScene);
}

void GameObject::RemoveComponent(BaseComponent* pComponent, bool deleteObject)
{
	auto it = find(m_pComponents.begin(), m_pComponents.end(), pComponent);

#if _DEBUG
	if(it == m_pComponents.end())
	{
		Logger::LogWarning(L"GameObject::RemoveComponent > Component is not attached to this GameObject!");
		return;
	}

	if(typeid(*pComponent) == typeid(TransformComponent))
	{
		Logger::LogWarning(L"GameObject::RemoveComponent > TransformComponent can't be removed!");
		return;
	}
#endif

	m_pComponents.erase(it);
	pComponent->m_pGameObject = nullptr;

	//Signal about GameObject detach
	pComponent->OnOwnerDetach(this); 

	if(deleteObject)
	{
		SafeDelete(pComponent);
	}
}

void GameObject::OnTrigger(GameObject* pTriggerObject, GameObject* pOtherObject, PxTriggerAction action) const
{
	if(m_OnTriggerCallback)
		m_OnTriggerCallback(pTriggerObject, pOtherObject, action);
}

GameScene* GameObject::GetScene() const
{
	if(!m_pParentScene && m_pParentObject)
	{
		return m_pParentObject->GetScene();
	}

	return m_pParentScene;
}

void GameObject::SetOnTriggerCallBack(PhysicsCallback callback)
{
	m_OnTriggerCallback = callback;
}