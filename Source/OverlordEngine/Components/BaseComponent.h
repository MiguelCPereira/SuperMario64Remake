#pragma once

class TransformComponent;
class GameObject;

class BaseComponent
{
public:
	BaseComponent();
	virtual ~BaseComponent() = default;
	BaseComponent(const BaseComponent& other) = delete;
	BaseComponent(BaseComponent&& other) noexcept = delete;
	BaseComponent& operator=(const BaseComponent& other) = delete;
	BaseComponent& operator=(BaseComponent&& other) noexcept = delete;

	GameObject* GetGameObject() const { return m_pGameObject; }
	GameScene* GetScene() const { return m_pScene; }
	TransformComponent* GetTransform() const;
	UINT GetComponentId() const { return m_ComponentId; }

protected:

	virtual void Initialize(const SceneContext& sceneContext) = 0;
	virtual void PostInitialize(const SceneContext& /*sceneContext*/){}
	virtual void Update(const SceneContext& /*sceneContext*/){}
	virtual void Draw(const SceneContext& /*sceneContext*/){}
	virtual void ShadowMapDraw(const SceneContext&) {} //update_W9
	virtual void PostDraw(const SceneContext&) {} //update_W9
	virtual void OnOwnerAttach(GameObject* /*pOwner*/) {}
	virtual void OnOwnerDetach(GameObject* /*pPreviousOwner*/) {}
	virtual void OnSceneAttach(GameScene* /*pScene*/) {}
	virtual void OnSceneDetach(GameScene* /*pScene*/) {}

	GameObject* m_pGameObject{};
	GameScene* m_pScene{};
	bool m_IsInitialized{};

	bool m_enablePostDraw{ false }; //update_W9
	bool m_enableShadowMapDraw{ false }; //update_W9

	UINT m_ComponentId{ 0 };

private:
	friend class GameObject;

	void RootInitialize(const SceneContext& sceneContext);
	void RootOnSceneAttach(GameScene*);
	void RootOnSceneDetach(GameScene*);

	static UINT m_ComponentCounter;
};


