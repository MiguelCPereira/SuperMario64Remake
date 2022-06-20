#include "stdafx.h"
#include "DeathScreen.h"

void DeathScreen::Initialize()
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
	auto pBackgroundGO = AddChild(new GameObject());
	pBackgroundGO->AddComponent(new SpriteComponent(L"Textures/DeathScreen/Background.png", { 0.5f,0.5f }, { 1.f,1.f,1.f,1.f }));
	pBackgroundGO->GetTransform()->Translate(m_SceneContext.windowWidth / 2.f, m_SceneContext.windowHeight / 2.f, .8f);
	pBackgroundGO->GetTransform()->Scale(0.68f, 0.68f, 0.68f);


	// Title
	m_pGameOverTitleGO = AddChild(new GameObject());
	m_pGameOverTitle = m_pGameOverTitleGO->AddComponent(new SpriteComponent(L"Textures/DeathScreen/Title.png", { 0.5f,0.5f }, { 1.f,1.f,1.f,1.f }));
	m_pGameOverTitle->SetColor({ 1.f,  1.f, 1.f, 0.0f });
	m_pGameOverTitleGO->GetTransform()->Translate(m_SceneContext.windowWidth / 2.f, m_SceneContext.windowHeight * 0.25f, .9f);
	m_pGameOverTitleGO->GetTransform()->Scale(0.68f, 0.68f, 0.68f);


	// Restart Text
	auto pRestartGO = AddChild(new GameObject());
	m_pRestartText = pRestartGO->AddComponent(new SpriteComponent(L"Textures/DeathScreen/Restart.png", { 0.5f,0.5f }, { 1.f,1.f,1.f,1.f }));
	m_pRestartText->SetColor({ 1.f,  1.f, 1.f, 0.0f });
	pRestartGO->GetTransform()->Translate(m_SceneContext.windowWidth / 2.f, m_SceneContext.windowHeight * 0.55f, .9f);
	pRestartGO->GetTransform()->Scale(0.68f, 0.68f, 0.68f);


	// Buttons
	auto pYesGO = AddChild(new GameObject());
	m_pYesButton = pYesGO->AddComponent(new SpriteComponent(L"Textures/DeathScreen/YesButton.png", { 0.5f,0.5f }, { 1.f,1.f,1.f,1.f }));
	m_pYesButton->SetColor({ 1.f,  1.f, 1.f, 0.0f });
	pYesGO->GetTransform()->Translate(m_SceneContext.windowWidth * 0.36f, m_SceneContext.windowHeight * 0.75f, .9f);
	pYesGO->GetTransform()->Scale(0.68f, 0.68f, 0.68f);

	auto pNoGO = AddChild(new GameObject());
	m_pNoButton = pNoGO->AddComponent(new SpriteComponent(L"Textures/DeathScreen/NoButton.png", { 0.5f,0.5f }, { 1.f,1.f,1.f,1.f }));
	m_pNoButton->SetColor({ m_UnselectedColor.x,  m_UnselectedColor.y, m_UnselectedColor.z, 0.0f});
	pNoGO->GetTransform()->Translate(m_SceneContext.windowWidth * 0.64f, m_SceneContext.windowHeight * 0.75f, .9f);
	pNoGO->GetTransform()->Scale(0.68f, 0.68f, 0.68f);


	// SFX
	auto* pSoundManagerSystem = SoundManager::Get()->GetSystem();
	pSoundManagerSystem->createStream("Resources/Sounds/menu-change.WAV", FMOD_DEFAULT, nullptr, &m_pChangeSound);
	pSoundManagerSystem->createStream("Resources/Sounds/menu-confirm.WAV", FMOD_DEFAULT, nullptr, &m_pConfirmSound);
	pSoundManagerSystem->createStream("Resources/Sounds/menu-start-level.WAV", FMOD_DEFAULT, nullptr, &m_pStartLevelSound);
}

void DeathScreen::Update()
{
	// Change Title Size
	auto newScale = 1.f + sinf(m_SceneContext.pGameTime->GetTotal()) * m_MaxSizeIncrease;
	m_pGameOverTitleGO->GetTransform()->Scale(newScale, newScale, newScale);

	if (m_Active)
	{
		if (m_ChangingScreen == false)
		{
			// Handle Input
			if (m_SceneContext.pInput->IsActionTriggered(ConfirmSelection1) || m_SceneContext.pInput->IsActionTriggered(ConfirmSelection2))
			{
				if (m_YesSelected)
					SoundManager::Get()->GetSystem()->playSound(m_pStartLevelSound, nullptr, false, &m_pSFXChannel);
				else
					SoundManager::Get()->GetSystem()->playSound(m_pConfirmSound, nullptr, false, &m_pSFXChannel);

				m_ChangingScreen = true;
				return;
			}

			if (m_SceneContext.pInput->IsActionTriggered(SelectLeft) && m_YesSelected == false)
			{
				SoundManager::Get()->GetSystem()->playSound(m_pChangeSound, nullptr, false, &m_pChangeChannel);
				m_pChangeChannel->setVolume(0.4f);
				m_pYesButton->SetColor({ 1.f, 1.f, 1.f, 1.f });
				m_pNoButton->SetColor(m_UnselectedColor);
				m_YesSelected = true;
			}
			else if (m_SceneContext.pInput->IsActionTriggered(SelectRight) && m_YesSelected == true)
			{
				SoundManager::Get()->GetSystem()->playSound(m_pChangeSound, nullptr, false, &m_pChangeChannel);
				m_pChangeChannel->setVolume(0.4f);
				m_pYesButton->SetColor(m_UnselectedColor);
				m_pNoButton->SetColor({ 1.f, 1.f, 1.f, 1.f });
				m_YesSelected = false;
			}
		}
		else
		{
			m_ChangeCounter += m_SceneContext.pGameTime->GetElapsed();
			if (m_ChangeCounter >= m_TimeForSFX)
			{
				if (m_YesSelected)
				{
					Reset();
					SceneManager::Get()->SetActiveGameScene(L"MainScene", true);

					return;
				}

				Reset();
				SceneManager::Get()->SetActiveGameScene(L"StartScreen");
			}
		}
	}
	else
	{
		m_ActivationCounter += m_SceneContext.pGameTime->GetElapsed();
		if (m_ActivationCounter >= m_TimeToActivate)
		{
			m_ActivationCounter = 0.0f;
			m_pGameOverTitle->SetColor({ 1.f, 1.f, 1.f, 1.f });
			m_pRestartText->SetColor({ 1.f, 1.f, 1.f, 1.f });
			m_pYesButton->SetColor({ 1.f, 1.f, 1.f, 1.f });
			m_pNoButton->SetColor(m_UnselectedColor);
			m_Active = true;
		}
		else
		{
			auto currWValue = m_ActivationCounter / m_TimeToActivate;
			m_pGameOverTitle->SetColor({ 1.f, 1.f, 1.f, currWValue });
			m_pRestartText->SetColor({ 1.f, 1.f, 1.f, currWValue });
			m_pYesButton->SetColor({ 1.f, 1.f, 1.f, currWValue });
			m_pNoButton->SetColor({ m_UnselectedColor.x, m_UnselectedColor.y, m_UnselectedColor.z, currWValue });
		}
	}
}

void DeathScreen::Reset()
{
	m_ActivationCounter = 0.0f;
	m_Active = false;
	m_ChangeCounter = 0.0f;
	m_ChangingScreen = false;
	m_pGameOverTitle->SetColor({ 1.f, 1.f, 1.f, 0.f });
	m_pRestartText->SetColor({ 1.f, 1.f, 1.f, 0.f });
	m_pYesButton->SetColor({ 1.f, 1.f, 1.f, 0.f });
	m_pNoButton->SetColor({ m_UnselectedColor.x, m_UnselectedColor.y, m_UnselectedColor.z, 0.f });
	m_YesSelected = true;
}

