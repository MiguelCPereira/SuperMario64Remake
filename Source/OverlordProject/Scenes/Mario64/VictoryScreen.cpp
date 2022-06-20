#include "stdafx.h"
#include "VictoryScreen.h"

//#include "MainScene.h"

void VictoryScreen::Initialize()
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
	pBackgroundGO->AddComponent(new SpriteComponent(L"Textures/VictoryScreen/Background.png", { 0.5f,0.5f }, { 1.f,1.f,1.f,1.f }));
	pBackgroundGO->GetTransform()->Translate(m_SceneContext.windowWidth / 2.f, m_SceneContext.windowHeight /2.f, .8f);
	pBackgroundGO->GetTransform()->Scale(0.68f, 0.68f, 0.68f);

	// Mario
	auto pMarioGO = AddChild(new GameObject());
	pMarioGO->AddComponent(new SpriteComponent(L"Textures/VictoryScreen/Mario.png", { 0.5f,0.5f }, { 1.f,1.f,1.f,1.f }));
	pMarioGO->GetTransform()->Translate(m_SceneContext.windowWidth / 2.f, m_SceneContext.windowHeight * 0.6f, .9f);
	pMarioGO->GetTransform()->Scale(0.68f, 0.68f, 0.68f);

	// Title
	m_pVictoryTitle = AddChild(new GameObject());
	m_pVictoryTitle->AddComponent(new SpriteComponent(L"Textures/VictoryScreen/VictoryTitle.png", { 0.5f,0.5f }, { 1.f,1.f,1.f,1.f }));
	m_pVictoryTitle->GetTransform()->Translate(m_SceneContext.windowWidth / 2.f, m_SceneContext.windowHeight * 0.19f, .9f);
	m_pVictoryTitle->GetTransform()->Scale(0.68f, 0.68f, 0.68f);


	// Buttons
	auto pRetryGO = AddChild(new GameObject());
	m_pRetryButton = pRetryGO->AddComponent(new SpriteComponent(L"Textures/VictoryScreen/RetryButton.png", { 0.5f,0.5f }, { 1.f,1.f,1.f,1.f }));
	pRetryGO->GetTransform()->Translate(m_SceneContext.windowWidth * 0.19f, m_SceneContext.windowHeight * 0.75f, .9f);
	pRetryGO->GetTransform()->Scale(0.68f, 0.68f, 0.68f);

	auto pBackGO = AddChild(new GameObject());
	m_pBackButton = pBackGO->AddComponent(new SpriteComponent(L"Textures/VictoryScreen/BackButton.png", { 0.5f,0.5f }, { 1.f,1.f,1.f,1.f }));
	m_pBackButton->SetColor(m_UnselectedColor);
	pBackGO->GetTransform()->Translate(m_SceneContext.windowWidth * 0.81f, m_SceneContext.windowHeight * 0.75f, .9f);
	pBackGO->GetTransform()->Scale(0.68f, 0.68f, 0.68f);


	// Soundtrack
	auto* pSoundManagerSystem = SoundManager::Get()->GetSystem();
	pSoundManagerSystem->createStream("Resources/Sounds/Victory Screen Theme.mp3", FMOD_LOOP_NORMAL, nullptr, &m_pTrackSound);
	m_pTrackSound->setMode(FMOD_LOOP_NORMAL);
	pSoundManagerSystem->playSound(m_pTrackSound, nullptr, true, &m_pTrackChannel);


	// SFX
	pSoundManagerSystem->createStream("Resources/Sounds/menu-change.WAV", FMOD_DEFAULT, nullptr, &m_pChangeSound);
	pSoundManagerSystem->createStream("Resources/Sounds/menu-confirm.WAV", FMOD_DEFAULT, nullptr, &m_pConfirmSound);
	pSoundManagerSystem->createStream("Resources/Sounds/menu-start-level.WAV", FMOD_DEFAULT, nullptr, &m_pStartLevelSound);
}

void VictoryScreen::Update()
{
	//Soundtrack
	if (m_MusicStarted == false)
	{
		m_pTrackChannel->setPaused(false);
		m_MusicStarted = true;
	}

	// Wobble Title
	m_pVictoryTitle->GetTransform()->Translate(m_SceneContext.windowWidth / 2.f, m_SceneContext.windowHeight * 0.19f +
		m_SceneContext.windowHeight * (sinf(m_SceneContext.pGameTime->GetTotal()) * m_MaxWobbleDistScreenPerc), 0.8f);


	if (m_ChangingScreen == false)
	{
		// Handle Input
		if (m_SceneContext.pInput->IsActionTriggered(ConfirmSelection1) || m_SceneContext.pInput->IsActionTriggered(ConfirmSelection2))
		{
			if (m_RetrySelected)
				SoundManager::Get()->GetSystem()->playSound(m_pStartLevelSound, nullptr, false, &m_pSFXChannel);
			else
				SoundManager::Get()->GetSystem()->playSound(m_pConfirmSound, nullptr, false, &m_pSFXChannel);

			m_pSFXChannel->setVolume(0.4f);
			m_ChangingScreen = true;
			return;
		}

		if (m_SceneContext.pInput->IsActionTriggered(SelectLeft) && m_RetrySelected == false)
		{
			SoundManager::Get()->GetSystem()->playSound(m_pChangeSound, nullptr, false, &m_pChangeChannel);
			m_pChangeChannel->setVolume(0.4f);
			m_pRetryButton->SetColor({ 1.f, 1.f, 1.f, 1.f });
			m_pBackButton->SetColor(m_UnselectedColor);
			m_RetrySelected = true;
		}
		else if (m_SceneContext.pInput->IsActionTriggered(SelectRight) && m_RetrySelected == true)
		{
			SoundManager::Get()->GetSystem()->playSound(m_pChangeSound, nullptr, false, &m_pChangeChannel);
			m_pChangeChannel->setVolume(0.4f);
			m_pRetryButton->SetColor(m_UnselectedColor);
			m_pBackButton->SetColor({ 1.f, 1.f, 1.f, 1.f });
			m_RetrySelected = false;
		}
	}
	else
	{
		m_ChangeCounter += m_SceneContext.pGameTime->GetElapsed();
		if(m_ChangeCounter >= m_TimeForSFX)
		{
			if (m_RetrySelected)
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

void VictoryScreen::Reset()
{
	SoundManager::Get()->GetSystem()->playSound(m_pTrackSound, nullptr, true, &m_pTrackChannel);
	m_ChangeCounter = 0.0f;
	m_ChangingScreen = false;
	m_MusicStarted = false;
	m_pRetryButton->SetColor({ 1.f, 1.f, 1.f, 1.f });
	m_pBackButton->SetColor(m_UnselectedColor);
	m_RetrySelected = true;
}

