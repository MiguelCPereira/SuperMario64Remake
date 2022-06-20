#include "stdafx.h"
#include "StartScreen.h"

#include "MainScene.h"
#include "Materials/DiffuseMaterial.h"
#include "Prefabs/CubePrefab.h"


void StartScreen::Initialize()
{
	m_SceneContext.settings.showInfoOverlay = false;
	m_SceneContext.settings.drawPhysXDebug = false;
	m_SceneContext.settings.drawGrid = false;
	m_SceneContext.settings.enableOnGUI = false;

	//Input
	auto inputAction = InputAction(SelectLeft, InputState::pressed, 'A', -1, XINPUT_GAMEPAD_DPAD_LEFT);
	m_SceneContext.pInput->AddInputAction(inputAction);

	inputAction = InputAction(SelectRight, InputState::pressed, 'D', -1, XINPUT_GAMEPAD_DPAD_RIGHT);
	m_SceneContext.pInput->AddInputAction(inputAction);

	inputAction = InputAction(SelectUp, InputState::pressed, 'W', -1, XINPUT_GAMEPAD_DPAD_UP);
	m_SceneContext.pInput->AddInputAction(inputAction);

	inputAction = InputAction(SelectDown, InputState::pressed, 'S', -1, XINPUT_GAMEPAD_DPAD_DOWN);
	m_SceneContext.pInput->AddInputAction(inputAction);

	inputAction = InputAction(ConfirmSelection1, InputState::pressed, VK_RETURN, -1, XINPUT_GAMEPAD_A);
	m_SceneContext.pInput->AddInputAction(inputAction);

	inputAction = InputAction(ConfirmSelection2, InputState::pressed, VK_SPACE, -1, XINPUT_GAMEPAD_B);
	m_SceneContext.pInput->AddInputAction(inputAction);


	// Set camera
	auto* pCamera = AddChild(new FixedCamera());
	pCamera->GetTransform()->Translate(0.f, 95.f, 0.f);
	pCamera->SetRotation(90.f, 0.f);
	SetActiveCamera(pCamera->GetComponent<CameraComponent>());

	//Background
	AddChild(new CubePrefab(160, 1, 80, XMFLOAT4{ Colors::Black }));


	auto* pMatManager = MaterialManager::Get();

	// Title Mesh
	m_pTitleGO = AddChild(new GameObject());
	auto* pTitleMaterial = pMatManager->CreateMaterial<DiffuseMaterial>();
	pTitleMaterial->SetDiffuseTexture(L"Textures/StartScreen/Title_Diffuse.png");
	auto* pTitleMesh = m_pTitleGO->AddComponent(new ModelComponent(L"Meshes/Title.ovm"));
	pTitleMesh->SetMaterial(pTitleMaterial);
	m_pTitleGO->GetTransform()->Rotate(170, 0, 0);
	m_pTitleGO->GetTransform()->Scale(m_TitleRegularScale, m_TitleRegularScale, m_TitleRegularScale);
	m_pTitleGO->GetTransform()->Translate(0.f, 10.f, 7.f);


	// Buttons Sprites
	auto pStartGO = AddChild(new GameObject());
	m_pStartButton = pStartGO->AddComponent(new SpriteComponent(L"Textures/StartScreen/StartButton.png", { 0.5f,0.5f }, { 1.f,1.f,1.f,1.f }));
	pStartGO->GetTransform()->Translate(m_SceneContext.windowWidth / 2.f, m_SceneContext.windowHeight * 0.69f, .8f);
	pStartGO->GetTransform()->Scale(0.5f, 0.5f, 0.5f);

	auto pControlsGO = AddChild(new GameObject());
	m_pControlsButton = pControlsGO->AddComponent(new SpriteComponent(L"Textures/StartScreen/ControlsButton.png", { 0.5f,0.5f }, { 1.f,1.f,1.f,1.f }));
	m_pControlsButton->SetColor(m_UnselectedColor);
	pControlsGO->GetTransform()->Translate(m_SceneContext.windowWidth * 0.43f, m_SceneContext.windowHeight * 0.83f, .8f);
	pControlsGO->GetTransform()->Scale(0.5f, 0.5f, 0.5f);

	auto pQuitGO = AddChild(new GameObject());
	m_pQuitButton = pQuitGO->AddComponent(new SpriteComponent(L"Textures/StartScreen/QuitButton.png", { 0.5f,0.5f }, { 1.f,1.f,1.f,1.f }));
	m_pQuitButton->SetColor(m_UnselectedColor);
	pQuitGO->GetTransform()->Translate(m_SceneContext.windowWidth * 0.63f, m_SceneContext.windowHeight * 0.83f, .8f);
	pQuitGO->GetTransform()->Scale(0.5f, 0.5f, 0.5f);

	// DAE Banner
	auto pBannerGO = AddChild(new GameObject());
	m_pBanner = pBannerGO->AddComponent(new SpriteComponent(L"Textures/StartScreen/Banner.png", { 0.5f,1.0f }, { 1.f,1.f,1.f,1.f }));
	pBannerGO->GetTransform()->Translate(m_SceneContext.windowWidth / 2.f, m_SceneContext.windowHeight, .8f);
	pBannerGO->GetTransform()->Scale(0.99f, 0.99f, 0.99f);

	// Controls Sprite
	auto pControlsScreenGO = AddChild(new GameObject());
	m_pControlsScreen = pControlsScreenGO->AddComponent(new SpriteComponent(L"Textures/StartScreen/Controls.png", { 0.5f,0.5f }, { 1.f,1.f,1.f,0.f }));
	pControlsScreenGO->GetTransform()->Translate(m_SceneContext.windowWidth / 2.f, m_SceneContext.windowHeight / 2.f, .9f);
	pControlsScreenGO->GetTransform()->Scale(0.68f, 0.68f, 0.68f);


	// Soundtrack
	auto* pSoundManagerSystem = SoundManager::Get()->GetSystem();
	pSoundManagerSystem->createStream("Resources/Sounds/Start Screen Theme.mp3", FMOD_LOOP_NORMAL, nullptr, &m_pTrackSound);
	pSoundManagerSystem->playSound(m_pTrackSound, nullptr, true, &m_pTrackChannel);
	pSoundManagerSystem->createStream("Resources/Sounds/Start Screen SFX.wav", FMOD_DEFAULT, nullptr, &m_pReadySound);

	// SFX
	pSoundManagerSystem->createStream("Resources/Sounds/menu-change.WAV", FMOD_DEFAULT, nullptr, &m_pChangeSound);
	pSoundManagerSystem->createStream("Resources/Sounds/menu-confirm.WAV", FMOD_DEFAULT, nullptr, &m_pConfirmSound);
	pSoundManagerSystem->createStream("Resources/Sounds/menu-start-level.WAV", FMOD_DEFAULT, nullptr, &m_pStartLevelSound);

}


void StartScreen::Update()
{
	if (m_Active)
	{
		if (m_ChangingScreen == false)
		{
			if (m_SceneContext.pInput->IsActionTriggered(ConfirmSelection1) || m_SceneContext.pInput->IsActionTriggered(ConfirmSelection2))
			{
				if(m_ControlsScreenOpen)
				{
					m_pTrackChannel->setVolume(1.f);
					m_pControlsScreen->SetColor({ 1.f,1.f,1.f,0.f });
					m_ControlsScreenOpen = false;
				}
				else
				{
					switch (m_SelectedButton)
					{
					case Start:
						SoundManager::Get()->GetSystem()->playSound(m_pStartLevelSound, nullptr, false, &m_pSFXChannel);
						m_ChangingScreen = true;
						return;
						break;
					case Controls:
						SoundManager::Get()->GetSystem()->playSound(m_pConfirmSound, nullptr, false, &m_pSFXChannel);
						m_ChangingScreen = true;
						break;
					case Quit:
						SoundManager::Get()->GetSystem()->playSound(m_pConfirmSound, nullptr, false, &m_pSFXChannel);
						m_ChangingScreen = true;
						break;
					default:
						break;
					}
				}
			}
			else if(m_ControlsScreenOpen == false)
			{
				if (m_SceneContext.pInput->IsActionTriggered(SelectUp) && m_SelectedButton != Start)
				{
					SoundManager::Get()->GetSystem()->playSound(m_pChangeSound, nullptr, false, &m_pChangeChannel);
					m_pChangeChannel->setVolume(0.4f);
					m_pStartButton->SetColor({ 1.f, 1.f, 1.f, 1.f });
					m_pControlsButton->SetColor(m_UnselectedColor);
					m_pQuitButton->SetColor(m_UnselectedColor);
					m_SelectedButton = Start;
				}
				else if (m_SceneContext.pInput->IsActionTriggered(SelectLeft) && m_SelectedButton != Controls)
				{
					SoundManager::Get()->GetSystem()->playSound(m_pChangeSound, nullptr, false, &m_pChangeChannel);
					m_pChangeChannel->setVolume(0.4f);
					m_pStartButton->SetColor(m_UnselectedColor);
					m_pControlsButton->SetColor({ 1.f, 1.f, 1.f, 1.f });
					m_pQuitButton->SetColor(m_UnselectedColor);
					m_SelectedButton = Controls;
				}
				else if (m_SceneContext.pInput->IsActionTriggered(SelectRight) && m_SelectedButton != Quit)
				{
					SoundManager::Get()->GetSystem()->playSound(m_pChangeSound, nullptr, false, &m_pChangeChannel);
					m_pChangeChannel->setVolume(0.4f);
					m_pStartButton->SetColor(m_UnselectedColor);
					m_pControlsButton->SetColor(m_UnselectedColor);
					m_pQuitButton->SetColor({ 1.f, 1.f, 1.f, 1.f });
					m_SelectedButton = Quit;
				}
				else if (m_SceneContext.pInput->IsActionTriggered(SelectDown))
				{
					if (m_SelectedButton == Start)
					{
						SoundManager::Get()->GetSystem()->playSound(m_pChangeSound, nullptr, false, &m_pChangeChannel);
						m_pChangeChannel->setVolume(0.4f);
						m_pStartButton->SetColor(m_UnselectedColor);
						m_pControlsButton->SetColor({ 1.f,1.f,1.f,1.f });
						m_SelectedButton = Controls;
					}
				}
			}
		}
		else
		{
			m_ChangeCounter += m_SceneContext.pGameTime->GetElapsed();
			if (m_ChangeCounter >= m_TimeForSFX)
			{
				switch (m_SelectedButton)
				{
				case Start:
					Reset();
					SceneManager::Get()->SetActiveGameScene(L"MainScene", true);
					return;
					break;
				case Controls:
					m_pTrackChannel->setVolume(0.4f);
					m_pControlsScreen->SetColor({ 1.f,1.f,1.f,1.f });
					m_ChangeCounter = 0.0f;
					m_ChangingScreen = false;
					m_ControlsScreenOpen = true;	
					break;
				case Quit:
					m_ChangingScreen = false;
					PostQuitMessage(0);
					return;
				default:
					break;
				}
			}
		}
	}
	else
	{
		if (m_SoundPlayed == false)
		{
			m_SoundPlayed = true;
			SoundManager::Get()->GetSystem()->playSound(m_pReadySound, nullptr, false, nullptr);
		}
		m_ActivationCounter += m_SceneContext.pGameTime->GetElapsed();
		if(m_ActivationCounter >= m_TimeToActivate)
		{
			m_ActivationCounter = 0;
			m_pTitleGO->GetTransform()->Scale(m_TitleRegularScale, m_TitleRegularScale, m_TitleRegularScale);
			m_pTrackChannel->setPaused(false);
			m_Active = true;
		}
		else
		{
			auto currScale = m_TitleRegularScale * (m_ActivationCounter / m_TimeToActivate);
			m_pTitleGO->GetTransform()->Scale(currScale, currScale, currScale);
		}
	}
}

void StartScreen::Reset()
{
	SoundManager::Get()->GetSystem()->playSound(m_pTrackSound, nullptr, true, &m_pTrackChannel);
	m_ActivationCounter = 0;
	m_Active = false;
	m_SoundPlayed = false;
	m_ChangeCounter = 0.0f;
	m_ChangingScreen = false;
	m_ControlsScreenOpen = false;
	m_pTrackChannel->setVolume(1.f);
	m_SelectedButton = Start;
	m_pStartButton->SetColor({ 1.f, 1.f, 1.f, 1.f });
	m_pControlsButton->SetColor(m_UnselectedColor);
	m_pQuitButton->SetColor(m_UnselectedColor);

}

