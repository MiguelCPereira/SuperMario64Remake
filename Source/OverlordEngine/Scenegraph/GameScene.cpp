#include "stdafx.h"
#include "GameScene.h"

GameScene::GameScene(std::wstring sceneName):
	m_SceneName(std::move(sceneName))
{
}

GameScene::~GameScene()
{
	SafeDelete(m_SceneContext.pGameTime);
	SafeDelete(m_SceneContext.pInput);
	SafeDelete(m_SceneContext.pLights);

	for (auto pChild : m_pChildren)
	{
		SafeDelete(pChild);
	}

	SafeDelete(m_pPhysxProxy);
}

void GameScene::AddChild_(GameObject* pObject)
{
#if _DEBUG
	if (pObject->m_pParentScene)
	{
		if (pObject->m_pParentScene == this)
			Logger::LogWarning(L"GameObject is already attached to this GameScene");
		else
			Logger::LogWarning(L"GameObject is already attached to another GameScene.Detach it from it's current scene before attaching it to another one.");

		return;
	}

	if (pObject->m_pParentObject)
	{
		Logger::LogWarning(
			L"GameObject is currently attached to a GameObject. Detach it from it's current parent before attaching it to another one.");
		return;
	}
#endif

	m_pChildren.push_back(pObject);
	pObject->RootOnSceneAttach(this);
}

void GameScene::RemoveChild(GameObject* pObject, bool deleteObject)
{
	const auto it = std::ranges::find(m_pChildren, pObject);

#if _DEBUG
	if (it == m_pChildren.end())
	{
		Logger::LogWarning(L"GameObject to remove is not attached to this GameScene!");
		return;
	}
#endif

	m_pChildren.erase(it);
	pObject->m_pParentScene = nullptr;
	pObject->RootOnSceneDetach(this);

	if (deleteObject)
	{
		SafeDelete(pObject);
	}		
}

void GameScene::RootInitialize(const GameContext& gameContext)
{
	if (m_IsInitialized)
		return;

	//SET Reference to OverlordGame
	m_pGame = gameContext.pGame;

	//SET SceneContext
	m_SceneContext.windowWidth = static_cast<float>(gameContext.windowWidth);
	m_SceneContext.windowHeight = static_cast<float>(gameContext.windowHeight);
	m_SceneContext.aspectRatio = m_SceneContext.windowWidth / m_SceneContext.windowHeight;

	m_SceneContext.pGameTime = new GameTime();
	m_SceneContext.pGameTime->Reset();
	m_SceneContext.pGameTime->Stop();

	m_SceneContext.pInput = new InputManager();
	m_SceneContext.pLights = new LightManager();

	m_SceneContext.d3dContext = gameContext.d3dContext;

	// Initialize Physx
	m_pPhysxProxy = new PhysxProxy();
	m_pPhysxProxy->Initialize(this);

	//Create DefaultCamera
	const auto pFreeCamera = new FreeCamera();
	pFreeCamera->SetRotation(30, 0);
	pFreeCamera->GetTransform()->Translate(0, 50, -80);
	AddChild(pFreeCamera);

	m_pDefaultCamera = pFreeCamera->GetComponent<CameraComponent>();
	SetActiveCamera(m_pDefaultCamera); //Also sets pCamera in SceneContext

	//User-Scene Initialize
	Initialize();

	//Root-Scene Initialize
	for (const auto pChild : m_pChildren)
	{
		pChild->RootInitialize(m_SceneContext);
	}

	m_IsInitialized = true;
}

void GameScene::RootPostInitialize()
{
	//Root-Scene Initialize
	for (const auto pChild : m_pChildren)
	{
		pChild->RootPostInitialize(m_SceneContext);
	}

	PostInitialize();
}

void GameScene::RootUpdate()
{
	m_SceneContext.pGameTime->Update();
	m_SceneContext.pInput->Update();
	m_SceneContext.pCamera = m_pActiveCamera;
	m_SceneContext.frameNumber = static_cast<UINT>(GameStats::GetStats().frameNr);

#pragma warning(push)
#pragma warning(disable : 26812)
	SoundManager::Get()->GetSystem()->update();
#pragma warning(pop)

	//User-Scene Update
	Update();

	//Root-Scene Update
	for (const auto pChild : m_pChildren)
	{
		pChild->RootUpdate(m_SceneContext);
	}

	m_pPhysxProxy->Update(m_SceneContext);
}

void GameScene::RootDraw()
{
#pragma region SHADOW PASS
	//SHADOW_PASS
	//+++++++++++
	//TODO_W8(L"Implement Shadow Pass")

	//1. BEGIN > ShadowMapRenderer::Begin (Initiate the ShadowPass)
	auto* shadowMapRenderer = ShadowMapRenderer::Get();
	shadowMapRenderer->Begin(m_SceneContext);

	//2. DRAW_LOOP > For every GameObject (m_pChildren), call GameObject::RootShadowMapDraw
	for(const auto* gameObj : m_pChildren)
		gameObj->RootShadowMapDraw(m_SceneContext);

	//3. END > ShadowMapRenderer::End (Terminate the ShadowPass)
	shadowMapRenderer->End(m_SceneContext);

#pragma endregion

#pragma region USER PASS
	//USER_PASS
	//+++++++++
	//User-Scene Draw
	Draw();

	//Object-Scene Draw
	for (const auto pChild : m_pChildren)
	{
		pChild->RootDraw(m_SceneContext);
	}

	//SpriteRenderer Draw
	SpriteRenderer::Get()->Draw(m_SceneContext);

	//TextRenderer Draw
	TextRenderer::Get()->Draw(m_SceneContext);

	//Object-Scene Post-Draw
	PostDraw();
	for (const auto pChild : m_pChildren)
	{
		pChild->RootPostDraw(m_SceneContext);
	}

	//Draw PhysX
	m_pPhysxProxy->Draw(m_SceneContext);

	//Draw Debug Stuff
	DebugRenderer::Draw(m_SceneContext);
#pragma endregion

#pragma region POST-PROCESSING PASS
	//POST-PROCESSING_PASS
	//++++++++++++++++++++

	//TODO_W10(L"Add Post-Processing PASS logic")

	//No need to swap RenderTargets if there aren't any PP Effects...
	if (m_PostProcessingMaterials.size() > 0)
	{
		//1. [PREV_RT & INIT_RT] Retrieve the current RenderTarget (OverlordGame::GetRenderTarget, every scene has access to the OverlordGame > m_pGame)
		auto* pPrevRT = m_pGame->GetRenderTarget();
		auto* pInitRT = pPrevRT;

		//2. Iterate the vector of PostProcessingMaterials (m_PostProcessingMaterials)
		for (auto* pMat : m_PostProcessingMaterials)
		{
			//For Each Material
			//	- If the material is disabled, skip
			if (pMat->IsEnabled() == false)
				continue;

			//	- Call the Draw function, the Source RenderTarget is our PREV_RT
			pMat->Draw(m_SceneContext, pPrevRT);

			//	- After drawing the effect, we want to swap PREV_RT with output from material we just used to draw with
			pPrevRT = pMat->GetOutput();
		}

		//3. All Materials are applied after each other, time to draw the final result to the screen
		//		- If PREV_RT is still equal to INIT_RT, do nothing (means no PP effect was applied, nothing has changed)
		if (pPrevRT == pInitRT)
			return;

		//		- Else, reset the RenderTarget of the game to default (OverlordGame::SetRenderTarget)
		m_pGame->SetRenderTarget(pInitRT);

		//		- Use SpriteRenderer::DrawImmediate to render the ShaderResourceView from PREV_RT to the screen
		SpriteRenderer::Get()->DrawImmediate(m_SceneContext.d3dContext, pPrevRT->GetColorShaderResourceView(), XMFLOAT2(0, 0));

		//Done!
	}
#pragma endregion
}

void GameScene::RootOnSceneActivated()
{
	//Start Timer
	m_SceneContext.pGameTime->Start();
	OnSceneActivated();
}

void GameScene::RootOnSceneDeactivated()
{
	//Stop Timer
	m_SceneContext.pGameTime->Stop();
	OnSceneDeactivated();
}

void GameScene::RootOnGUI()
{
	if (!m_SceneContext.settings.showInfoOverlay)
		return;

#pragma region Main Overlay Begin
#pragma region Window Config
	static int corner = 0;
	//ImGuiIO& io = ImGui::GetIO();
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;// | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings;
	if (corner != -1)
	{
		constexpr float PAD = 10.0f;
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		const ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
		const ImVec2 work_size = viewport->WorkSize;
		ImVec2 window_pos, window_pos_pivot;
		window_pos.x = (corner & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
		window_pos.y = (corner & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD);
		window_pos_pivot.x = (corner & 1) ? 1.0f : 0.0f;
		window_pos_pivot.y = (corner & 2) ? 1.0f : 0.0f;
		ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
		window_flags |= ImGuiWindowFlags_NoMove;
	}
	ImGui::SetNextWindowBgAlpha(0.15f); // Transparent background
#pragma endregion
	if (ImGui::Begin(StringUtil::utf8_encode(m_SceneName).c_str(), nullptr, window_flags))
	{
		ImGui::Dummy(ImVec2{ 150.f,0.f });
		ImGui::PushFont(ImguiFonts::pFont_DIN_Black_18);
		if (ImGui::CollapsingHeader(StringUtil::utf8_encode(m_SceneName).c_str()))
		{
			/*ImGui::Text("%s", utf8_encode(m_SceneName).c_str());*/

			ImGui::Separator();

			const PerfStats gameStats{ GameStats::GetStats() };
			ImGui::PushFont(nullptr);
			ImGui::Text("FPS %.1f (%.1f ms)", gameStats.averageFps_interim, gameStats.averageMs_interim);
			ImGui::Dummy(ImVec2{ 0,10.f });
			ImGui::PopFont();
#pragma endregion
#pragma region Scene Settings
			ImGui::PushFont(ImguiFonts::pFont_DIN_Black_16);
			if (ImGui::CollapsingHeader("Scene Settings", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::PushFont(nullptr);
				ImGui::ColorEdit3("Clear Color", reinterpret_cast<float*>(&m_SceneContext.settings.clearColor), ImGuiColorEditFlags_NoInputs);
				ImGui::Checkbox("V-Sync", &m_SceneContext.settings.vSyncEnabled);
				ImGui::Dummy(ImVec2{ 0,10.f });

				if (!DebugRenderer::IsEnabled())
				{
					ImGui::Text("*Debug Renderer [F2]");
				}
				else
				{
					ImGui::Checkbox("Draw Grid", &m_SceneContext.settings.drawGrid);
					ImGui::Checkbox("Draw PhysX Debug", &m_SceneContext.settings.drawPhysXDebug);
					ImGui::Checkbox("Draw User Debug", &m_SceneContext.settings.drawUserDebug);
				}

				ImGui::Dummy(ImVec2{ 0,10.f });
				ImGui::PopFont(); //Default
			}
			ImGui::PopFont(); //DIN_Black_16

#pragma endregion
#pragma region User Parameters
			if (m_SceneContext.settings.enableOnGUI)
			{
				ImGui::PushFont(ImguiFonts::pFont_DIN_Black_16);
				if (ImGui::CollapsingHeader("User Parameters", ImGuiTreeNodeFlags_DefaultOpen))
				{
					//User-defined GUI
					ImGui::PushFont(nullptr);
					OnGUI();
					ImGui::PopFont(); //Default
				}
				ImGui::PopFont(); //DIN_Black_16
			}
#pragma endregion	
#pragma region Main Overlay End
		}
		else
		{
			const PerfStats gameStats{ GameStats::GetStats() };
			ImGui::PushFont(ImguiFonts::pFont_DIN_Regular_10);
			ImGui::Text("FPS %.1f (%.1f ms)", gameStats.averageFps_interim, gameStats.averageMs_interim);
			ImGui::PopFont(); //DIN_Regular_10
		}
		ImGui::PopFont(); //DIN_Black_18

		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::MenuItem("Custom", nullptr, corner == -1)) corner = -1;
			if (ImGui::MenuItem("Top-left", nullptr, corner == 0)) corner = 0;
			if (ImGui::MenuItem("Top-right", nullptr, corner == 1)) corner = 1;
			if (ImGui::MenuItem("Bottom-left", nullptr, corner == 2)) corner = 2;
			if (ImGui::MenuItem("Bottom-right", nullptr, corner == 3)) corner = 3;

			ImGui::EndPopup();
		}
	}
	ImGui::End();
#pragma endregion
}

void GameScene::RootWindowStateChanged(int state, bool active) const
{
	//TIMER
	if (state == 0)
	{
		if (active)m_SceneContext.pGameTime->Start();
		else m_SceneContext.pGameTime->Stop();
	}
}

void GameScene::AddPostProcessingEffect(PostProcessingMaterial* pMaterial)
{
	m_PostProcessingMaterials.push_back(pMaterial);
}

void GameScene::AddPostProcessingEffect(UINT materialId)
{
	AddPostProcessingEffect(MaterialManager::Get()->GetMaterial<PostProcessingMaterial>(materialId));
}

void GameScene::RemovePostProcessingEffect(UINT materialId)
{
	RemovePostProcessingEffect(MaterialManager::Get()->GetMaterial<PostProcessingMaterial>(materialId));
}

void GameScene::RemovePostProcessingEffect(PostProcessingMaterial* pMaterial)
{
	if (std::ranges::find(m_PostProcessingMaterials, pMaterial) != m_PostProcessingMaterials.end())
		m_PostProcessingMaterials.erase(std::ranges::remove(m_PostProcessingMaterials, pMaterial).begin());
}
 
void GameScene::SetActiveCamera(CameraComponent* pCameraComponent)
{
	//Prevent recursion!
	if (pCameraComponent == m_pActiveCamera)
		return;

	//Disable current active camera (if set)
	const auto currActiveCamera = m_pActiveCamera;
	m_pActiveCamera = nullptr;
	if (currActiveCamera)
		currActiveCamera->SetActive(false);

	//Set new camera
	m_pActiveCamera = (pCameraComponent) ? pCameraComponent : m_pDefaultCamera;
	m_pActiveCamera->SetActive(true);
	m_SceneContext.pCamera = m_pActiveCamera; //Change SceneContext
}