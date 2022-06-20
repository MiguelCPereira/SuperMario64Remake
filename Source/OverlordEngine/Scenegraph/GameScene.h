#pragma once
#include "Base/Structs.h"
class PostProcessingMaterial;
class BaseMaterial;
class PhysxProxy;
class CameraComponent;
class GameObject;

class GameScene
{
public:
	GameScene(std::wstring sceneName);
	GameScene(const GameScene& other) = delete;
	GameScene(GameScene&& other) noexcept = delete;
	GameScene& operator=(const GameScene& other) = delete;
	GameScene& operator=(GameScene&& other) noexcept = delete;
	virtual ~GameScene();

	template<typename T>
	std::enable_if_t<std::is_base_of_v<GameObject, T>, T*>
		AddChild(T* pObject)
	{
		AddChild_(pObject);
		return pObject;
	}
	void AddChild_(GameObject* pObject);
	void RemoveChild(GameObject* pObject, bool deleteObject = false);

	const SceneContext& GetSceneContext() const { return m_SceneContext; }
	SceneSettings& GetSceneSettings() { return m_SceneContext.settings; }
	CameraComponent* GetActiveCamera() const { return m_pActiveCamera; }

	void AddPostProcessingEffect(PostProcessingMaterial* pMaterial);
	void AddPostProcessingEffect(UINT materialId);
	void RemovePostProcessingEffect(PostProcessingMaterial* pMaterial);
	void RemovePostProcessingEffect(UINT materialId);

	PhysxProxy* GetPhysxProxy() const { return m_pPhysxProxy; }
	void SetActiveCamera(CameraComponent* pCameraComponent);

	virtual void Reset(){};
	virtual void ToggleSoundFadeOut(float) {};

protected:
	virtual void Initialize() = 0;
	virtual void PostInitialize() {};
	virtual void Update() {};
	virtual void Draw() {};
	virtual void PostDraw() {};
	virtual void ShadowDraw() {};

	virtual void OnGUI(){}
	virtual void OnSceneActivated() {}
	virtual void OnSceneDeactivated() {}

	SceneContext m_SceneContext{};
private:
	friend class SceneManager;

	void RootInitialize(const GameContext& /*gameContext*/);
	void RootPostInitialize();
	void RootUpdate();
	void RootDraw();
	void RootOnSceneActivated();
	void RootOnSceneDeactivated();
	void RootOnGUI();
	void RootWindowStateChanged(int state, bool active) const;

	std::vector<GameObject*> m_pChildren{};
	bool m_IsInitialized{};
	std::wstring m_SceneName{};
	CameraComponent* m_pDefaultCamera{}, * m_pActiveCamera{};
	PhysxProxy* m_pPhysxProxy{};

	std::vector<PostProcessingMaterial*> m_PostProcessingMaterials{};
	OverlordGame* m_pGame{};
};
