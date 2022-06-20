#pragma once
class SceneManager final: public Singleton<SceneManager>
{
public:
	SceneManager(const SceneManager& other) = delete;
	SceneManager(SceneManager&& other) noexcept = delete;
	SceneManager& operator=(const SceneManager& other) = delete;
	SceneManager& operator=(SceneManager&& other) noexcept = delete;

	void AddGameScene(GameScene* pScene);
	void RemoveGameScene(GameScene* pScene, bool deleteObject = false);
	void SetActiveGameScene(const std::wstring& sceneName, bool resetScene = false);
	void NextScene();
	void PreviousScene();
	GameScene* GetActiveScene() const { return m_ActiveScene; }
	const SceneContext& GetActiveSceneContext() const { return m_ActiveScene->GetSceneContext(); }
	SceneSettings& GetActiveSceneSettings() const { return m_ActiveScene->GetSceneSettings(); }

protected:
	void Initialize() override;

private:
	friend class OverlordGame;
	friend class Singleton<SceneManager>;
	SceneManager() = default;
	~SceneManager();

	void PostInitialize() const;
	void WindowStateChanged(int state, bool active) const;
	void Update();
	void Draw() const;
	void OnGUI() const;

	std::vector<GameScene*> m_pScenes{};
	GameScene* m_ActiveScene{}, * m_NewActiveScene{};
};

