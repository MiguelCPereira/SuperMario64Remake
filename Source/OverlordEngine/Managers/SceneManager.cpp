#include "stdafx.h"
#include "SceneManager.h"
#include <algorithm>


void SceneManager::Initialize()
{
	for (GameScene* pScene : m_pScenes)
	{
		pScene->RootInitialize(m_GameContext);
	}
} 

SceneManager::~SceneManager()
{
	for (GameScene* scene : m_pScenes)
	{
		SafeDelete(scene);
	}
}

void SceneManager::AddGameScene(GameScene* pScene)
{
	const auto it = std::ranges::find(m_pScenes, pScene);

	if (it == m_pScenes.end())
	{
		m_pScenes.push_back(pScene);

		if (m_IsInitialized) {
			pScene->RootInitialize(m_GameContext);
			pScene->RootPostInitialize();
		}

		if (m_ActiveScene == nullptr && m_NewActiveScene == nullptr)
			m_NewActiveScene = pScene;
	}
}

void SceneManager::RemoveGameScene(GameScene* pScene, bool deleteObject)
{
	const auto it = std::ranges::find(m_pScenes, pScene);

	if (it != m_pScenes.end())
	{
		m_pScenes.erase(it);
		if (deleteObject)
		{
			SafeDelete(pScene);
		}
	}
}

void SceneManager::NextScene()
{
	for (unsigned int i = 0; i < m_pScenes.size(); ++i)
	{
		if (m_pScenes[i] == m_ActiveScene)
		{
			const auto nextScene = ++i % m_pScenes.size();
			m_NewActiveScene = m_pScenes[nextScene];
			break;
		}
	}
}

void SceneManager::PreviousScene()
{
	const UINT numScenes = static_cast<UINT>(m_pScenes.size());
	for (UINT i = 0; i < numScenes; ++i)
	{
		if (m_pScenes[i] == m_ActiveScene)
		{
			if (i == 0) i = numScenes;
			--i;
			m_NewActiveScene = m_pScenes[i];
			break;
		}
	}
}

void SceneManager::SetActiveGameScene(const std::wstring& sceneName, bool resetScene)
{
	const auto it = std::ranges::find_if(m_pScenes, [sceneName](const GameScene* pScene)
	{
		return wcscmp(pScene->m_SceneName.c_str(), sceneName.c_str()) == 0;
	});

	if (it != m_pScenes.end())
	{
		m_NewActiveScene = *it;

		if(resetScene)
			m_NewActiveScene->Reset();
	}
}

void SceneManager::WindowStateChanged(int state, bool active) const
{
	if (state == 0 && m_ActiveScene)
	{
		m_ActiveScene->RootWindowStateChanged(state, active);
	}
}

void SceneManager::PostInitialize() const
{
	for (GameScene* pScene : m_pScenes)
	{
		pScene->RootPostInitialize();
	}
}

void SceneManager::Update()
{
	if (m_NewActiveScene != nullptr)
	{
		if (m_NewActiveScene != m_ActiveScene)
		{
			//Deactivate the current active scene
			if (m_ActiveScene != nullptr)
				m_ActiveScene->RootOnSceneDeactivated();

			//Set New Scene
			m_ActiveScene = m_NewActiveScene;

			//Active the new scene and reset SceneTimer
			m_ActiveScene->RootOnSceneActivated();
			GameStats::Reset(); //Reset performance stats
		}

		m_NewActiveScene = nullptr;
	}

	if (m_ActiveScene != nullptr)
	{
		DebugRenderer::BeginFrame(m_ActiveScene->m_SceneContext.settings);
		m_ActiveScene->RootUpdate();
		return;
	}

	ASSERT_IF(m_ActiveScene == nullptr, L"No Active Scene Set!")
}

void SceneManager::Draw() const
{
	if (m_ActiveScene != nullptr)
		m_ActiveScene->RootDraw();
}

void SceneManager::OnGUI() const
{
	if (m_ActiveScene != nullptr)
		m_ActiveScene->RootOnGUI();
}

