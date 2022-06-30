#include "stdafx.h"
#include "MainScene.h"

#include "Prefabs/Mario-Specific/GoombaCharacter.h"
#include "Prefabs/Mario-Specific/MarioCharacter.h"
#include "Materials/PureDiffuseMaterial.h"
#include "Materials/Shadow/DiffuseMaterial_Shadow.h"
#include "Prefabs/Mario-Specific/BoBombCharacter.h"
#include "Prefabs/Mario-Specific/CannonballCharacter.h"
#include "Prefabs/Mario-Specific/ChainchompCharacter.h"
#include "Prefabs/Mario-Specific/Star.h"
#include "Prefabs/Mario-Specific/Tube.h"

MainScene::~MainScene()
{
	for (auto& goombaPair : m_pGoombas)
	{
		delete goombaPair.second;
		goombaPair.first = nullptr;
		goombaPair.second = nullptr;
	}

	for (auto& bobOmbPair : m_pBobOmbs)
	{
		delete bobOmbPair.second;
		bobOmbPair.first = nullptr;
		bobOmbPair.second = nullptr;
	}

	for (auto& cannonball : m_pCannonballs)
	{
		delete cannonball.second;
		cannonball.first = nullptr;
		cannonball.second = nullptr;
	}
}

void MainScene::Initialize()
{
	m_SceneContext.settings.showInfoOverlay = false;
	m_SceneContext.settings.drawPhysXDebug = false;
	m_SceneContext.settings.drawGrid = false;
	m_SceneContext.settings.enableOnGUI = false;

	const auto pDefaultMaterial = PxGetPhysics().createMaterial(0.5f, 0.5f, 0.5f);


	// Mario
	MarioDesc marioDesc{ pDefaultMaterial };
	marioDesc.actionId_MoveForward = CharacterMoveForward;
	marioDesc.actionId_MoveBackward = CharacterMoveBackward;
	marioDesc.actionId_MoveLeft = CharacterMoveLeft;
	marioDesc.actionId_MoveRight = CharacterMoveRight;
	marioDesc.actionId_Jump = CharacterJump;
	marioDesc.actionId_Crouch = CharacterCrouch;
	marioDesc.actionId_Punch = CharacterPunch;
	marioDesc.actionId_TurnCameraLeft = CharacterTurnCameraLeft;
	marioDesc.actionId_TurnCameraRight = CharacterTurnCameraRight;

	m_pCharacter = AddChild(new MarioCharacter(marioDesc));
	m_pCharacter->GetTransform()->Translate(m_MarioSpawnPosition);
	m_pCharacter->m_TotalYaw = 225.f;

	m_SceneContext.pLights->SetDirectionalLight(XMFLOAT3{ 0, 80, 0 }, XMFLOAT3{ -0.15f, -.85f, 0.f });



	//Input
	auto inputAction = InputAction(CharacterMoveLeft, InputState::down, 'A', -1, XINPUT_GAMEPAD_DPAD_LEFT);
	m_SceneContext.pInput->AddInputAction(inputAction);

	inputAction = InputAction(CharacterMoveRight, InputState::down, 'D', -1, XINPUT_GAMEPAD_DPAD_RIGHT);
	m_SceneContext.pInput->AddInputAction(inputAction);

	inputAction = InputAction(CharacterMoveForward, InputState::down, 'W', -1, XINPUT_GAMEPAD_DPAD_UP);
	m_SceneContext.pInput->AddInputAction(inputAction);

	inputAction = InputAction(CharacterMoveBackward, InputState::down, 'S', -1, XINPUT_GAMEPAD_DPAD_DOWN);
	m_SceneContext.pInput->AddInputAction(inputAction);

	inputAction = InputAction(CharacterJump, InputState::pressed, VK_SPACE, -1, XINPUT_GAMEPAD_A);
	m_SceneContext.pInput->AddInputAction(inputAction);

	inputAction = InputAction(CharacterCrouch, InputState::down, VK_SHIFT, -1, XINPUT_GAMEPAD_X);
	m_SceneContext.pInput->AddInputAction(inputAction);

	inputAction = InputAction(CharacterPunch, InputState::pressed, -1, VK_LBUTTON, XINPUT_GAMEPAD_B);
	m_SceneContext.pInput->AddInputAction(inputAction);

	inputAction = InputAction(CharacterTurnCameraLeft, InputState::down, -1, -1, XINPUT_GAMEPAD_LEFT_SHOULDER);
	m_SceneContext.pInput->AddInputAction(inputAction);

	inputAction = InputAction(CharacterTurnCameraRight, InputState::down, -1, -1, XINPUT_GAMEPAD_RIGHT_SHOULDER);
	m_SceneContext.pInput->AddInputAction(inputAction);



	// Pause Input
	inputAction = InputAction(Pause1, InputState::pressed, 'P', -1, XINPUT_GAMEPAD_START);
	m_SceneContext.pInput->AddInputAction(inputAction);

	inputAction = InputAction(Pause2, InputState::pressed, VK_ESCAPE, -1, XINPUT_GAMEPAD_BACK);
	m_SceneContext.pInput->AddInputAction(inputAction);

	inputAction = InputAction(PauseSelectUp, InputState::pressed, 'W', -1, XINPUT_GAMEPAD_DPAD_UP);
	m_SceneContext.pInput->AddInputAction(inputAction);

	inputAction = InputAction(PauseSelectDown, InputState::pressed, 'S', -1, XINPUT_GAMEPAD_DPAD_DOWN);
	m_SceneContext.pInput->AddInputAction(inputAction);



	// Pause Background
	auto pBackgroundGO = AddChild(new GameObject());
	m_pPauseBackground = pBackgroundGO->AddComponent(new SpriteComponent(L"Textures/PauseScreen/Background.png", { 0.5f,0.5f }, { 1.f,1.f,1.f,1.f }));
	m_pPauseBackground->SetColor({ 1.f,1.f,1.f,0.f });
	pBackgroundGO->GetTransform()->Translate(m_SceneContext.windowWidth / 2.f, m_SceneContext.windowHeight / 2.f, .94f);
	pBackgroundGO->GetTransform()->Scale(0.68f, 0.68f, 0.68f);

	// Pause Title
	auto pTitleGO = AddChild(new GameObject());
	m_pPauseTitle = pTitleGO->AddComponent(new SpriteComponent(L"Textures/PauseScreen/Title.png", { 0.5f,0.5f }, { 1.f,1.f,1.f,1.f }));
	m_pPauseTitle->SetColor({ 1.f,1.f,1.f,0.f });
	pTitleGO->GetTransform()->Translate(m_SceneContext.windowWidth / 2.f, m_SceneContext.windowHeight * 0.25f, .95f);
	pTitleGO->GetTransform()->Scale(0.68f, 0.68f, 0.68f);

	// Pause Buttons
	auto pResumeGO = AddChild(new GameObject());
	m_pPauseResumeButton = pResumeGO->AddComponent(new SpriteComponent(L"Textures/PauseScreen/ResumeButton.png", { 0.5f,0.5f }, { 1.f,1.f,1.f,1.f }));
	m_pPauseResumeButton->SetColor({1.f,1.f,1.f,0.f});
	pResumeGO->GetTransform()->Translate(m_SceneContext.windowWidth / 2.f, m_SceneContext.windowHeight * 0.55f, .95f);
	pResumeGO->GetTransform()->Scale(0.68f, 0.68f, 0.68f);

	auto pBackGO = AddChild(new GameObject());
	m_pPauseBackButton = pBackGO->AddComponent(new SpriteComponent(L"Textures/PauseScreen/BackButton.png", { 0.5f,0.5f }, { 1.f,1.f,1.f,1.f }));
	m_pPauseBackButton->SetColor({ 1.f,1.f,1.f,0.f });
	pBackGO->GetTransform()->Translate(m_SceneContext.windowWidth / 2.f, m_SceneContext.windowHeight * 0.65f, .95f);
	pBackGO->GetTransform()->Scale(0.68f, 0.68f, 0.68f);

	auto pQuitGO = AddChild(new GameObject());
	m_pPauseQuitButton = pQuitGO->AddComponent(new SpriteComponent(L"Textures/PauseScreen/QuitButton.png", { 0.5f,0.5f }, { 1.f,1.f,1.f,1.f }));
	m_pPauseQuitButton->SetColor({ 1.f,1.f,1.f,0.f });
	pQuitGO->GetTransform()->Translate(m_SceneContext.windowWidth / 2.f, m_SceneContext.windowHeight * 0.75f, .95f);
	pQuitGO->GetTransform()->Scale(0.68f, 0.68f, 0.68f);



	// Moving Platform - static for now
	const auto pPlatform = AddChild(new GameObject());
	const auto pPlatformMesh = pPlatform->AddComponent(new ModelComponent(L"Meshes/Tilting Platform.ovm"));
	auto* pPlatformMaterial = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow>();
	pPlatformMaterial->SetDiffuseTexture(L"Textures/Bob-omb Battlefield/Level - wood.png");
	pPlatformMesh->SetMaterial(pPlatformMaterial);
	const auto pPlatformActor = pPlatform->AddComponent(new RigidBodyComponent(true));
	const auto pPxTriangleMesh = ContentManager::Load<PxTriangleMesh>(L"Meshes/Tilting Platform.ovpt");
	pPlatformActor->AddCollider(PxTriangleMeshGeometry(pPxTriangleMesh, PxMeshScale({ 500.f,500.f,500.f })), *pDefaultMaterial);
	pPlatformMesh->GetTransform()->Scale(500.f, 500.f, 500.f);
	pPlatformActor->SetCollisionGroup(CollisionGroup::Group9);
	pPlatform->GetTransform()->Translate(47.3f, 11.2f, 26.3f);
	pPlatform->GetTransform()->Rotate(90, 45, 0);



	InitializeLevel(pDefaultMaterial);
	SpawnGoombas(pDefaultMaterial);
	SpawnBobOmbs(pDefaultMaterial);
	SpawnCannonballs(pDefaultMaterial);


	// Spawn chainchomp
	const auto chainchompSpawnPos = XMFLOAT3(-30.2f, 67.f, -58.32f);
	ChainchompDesc chainchompDesc{ pDefaultMaterial };
	m_pChainchomp = AddChild(new ChainchompCharacter(chainchompDesc, m_pCharacter, chainchompSpawnPos));


	// Spawn final star
	const auto starSpawnPos = XMFLOAT3(-30.2f, 67.f, -58.32f);
	m_pStar = AddChild(new Star(starSpawnPos, m_pCharacter));


	// Spawn Initial Tube
	m_pTube = AddChild(new Tube(m_TubeSpawnPos, m_TubeFinalPos,m_TubeTimeToHide));



	// SFX
	auto* pSoundManagerSystem = SoundManager::Get()->GetSystem();
	pSoundManagerSystem->createStream("Resources/Sounds/Theme Song Start.wav", FMOD_DEFAULT, nullptr, &m_pTrackStartSound);
	m_pTrackStartSound->setMode(FMOD_LOOP_OFF);
	pSoundManagerSystem->playSound(m_pTrackStartSound, nullptr, true, &m_pTrackStartChannel);

	pSoundManagerSystem->createStream("Resources/Sounds/Theme Song Loop.wav", FMOD_DEFAULT, nullptr, &m_pTrackLoopSound);
	m_pTrackLoopSound->setMode(FMOD_LOOP_NORMAL);
	pSoundManagerSystem->playSound(m_pTrackLoopSound, nullptr, true, &m_pTrackLoopChannel);

	pSoundManagerSystem->createStream("Resources/Sounds/menu-change.WAV", FMOD_DEFAULT, nullptr, &m_pPauseChangeSound);
	pSoundManagerSystem->createStream("Resources/Sounds/menu-confirm.WAV", FMOD_DEFAULT, nullptr, &m_pPauseConfirmSound);
}

void MainScene::InitializeLevel(const PxMaterial* pDefaultMaterial)
{
	auto* pMatManager = MaterialManager::Get();

	std::vector<std::wstring> piecesNames;
	piecesNames.push_back(L"Cobblestone");
	piecesNames.push_back(L"Dirt Edges");
	piecesNames.push_back(L"Dirt");
	piecesNames.push_back(L"Flower Sides");
	piecesNames.push_back(L"Flower Tops");
	piecesNames.push_back(L"Gate Rock");
	piecesNames.push_back(L"Gate");
	piecesNames.push_back(L"Grass");
	piecesNames.push_back(L"Green Rocks");
	piecesNames.push_back(L"Marble");
	piecesNames.push_back(L"Platform Details");
	piecesNames.push_back(L"Saturated Dirt");
	piecesNames.push_back(L"Starting Rock");
	piecesNames.push_back(L"Wood");

	// Initialize each piece individually
	for(int i = 0; i < piecesNames.size(); i++)
	{
		const auto pPieceObject = AddChild(new GameObject());
		const auto pPieceMesh = pPieceObject->AddComponent(new ModelComponent(L"Meshes/Bob-omb Battlefield/Split/Level - " + piecesNames[i] + L".ovm"));
		auto* pPieceMaterial = pMatManager->CreateMaterial<DiffuseMaterial_Shadow>();
		pPieceMaterial->SetDiffuseTexture(L"Textures/Bob-omb Battlefield/Level - " + piecesNames[i] + L".png");
		pPieceMesh->SetMaterial(pPieceMaterial);

		const auto pPieceActor = pPieceObject->AddComponent(new RigidBodyComponent(true));
		const auto pPxTriangleMesh = ContentManager::Load<PxTriangleMesh>(L"Meshes/Bob-omb Battlefield/Split/Level - " + piecesNames[i] + L".ovpt");
		pPieceActor->AddCollider(PxTriangleMeshGeometry(pPxTriangleMesh, PxMeshScale({ 15.f,15.f,15.f })), *pDefaultMaterial);
		pPieceMesh->GetTransform()->Scale(15.f, 15.f, 15.f);
		pPieceActor->SetCollisionGroup(CollisionGroup::Group9);
	}

	// And add the skysphere
	const auto pSkySphereObject = AddChild(new GameObject());
	const auto  pSkySphereMesh = pSkySphereObject->AddComponent(new ModelComponent(L"Meshes/Bob-omb Battlefield/SkySphere.ovm"));
	auto* pSkySphereMaterial = pMatManager->CreateMaterial<PureDiffuseMaterial>();
	pSkySphereMaterial->SetDiffuseTexture(L"Textures/Bob-omb Battlefield/SkySphere.png");
	pSkySphereMesh->SetMaterial(pSkySphereMaterial);
	pSkySphereMesh->GetTransform()->Scale(3800.f, 3800.f, 3800.f);
	pSkySphereMesh->GetTransform()->Rotate(90, 0, 0);
}

void MainScene::Update()
{
	//Soundtrack
	if (m_MusicStarted == false)
	{
		m_pTrackStartChannel->setPaused(false);
		m_MusicStarted = true;
	}

	if (m_LoopingTrack == false)
	{
		m_TrackCounter += m_SceneContext.pGameTime->GetElapsed();
		if (m_TrackCounter >= m_TrackStartTime)
		{
			m_pTrackStartChannel->setPaused(true);
			m_pTrackLoopChannel->setPaused(false);
			m_TrackCounter = 0.0f;
			m_LoopingTrack = true;
		}
	}

	if (m_SoundFadingOut)
		UpdateSoundFadeOut();

	HandlePauseScreenInput();
}

void MainScene::Reset()
{
	TogglePause(false);

	// Soundtrack
	SoundManager::Get()->GetSystem()->playSound(m_pTrackStartSound, nullptr, true, &m_pTrackStartChannel);
	SoundManager::Get()->GetSystem()->playSound(m_pTrackLoopSound, nullptr, true, &m_pTrackLoopChannel);
	m_pTrackStartChannel->setVolume(1.0f);
	m_pTrackLoopChannel->setVolume(1.0f);
	m_SoundFadeOutCounter = 0.0f;
	m_TrackCounter = 0.0f;
	m_SoundFadingOut = false;
	m_LoopingTrack = false;
	m_MusicStarted = false;

	m_pCharacter->GetTransform()->Translate(m_MarioSpawnPosition);
	m_pCharacter->m_TotalYaw = 225.f;
	m_pCharacter->Reset();

	const auto pDefaultMaterial = PxGetPhysics().createMaterial(0.5f, 0.5f, 0.5f);

	for(auto& goombaPair : m_pGoombas)
	{
		if(goombaPair.second->alive)
		{
			goombaPair.first->GetTransform()->Translate(goombaPair.second->spawnPosition);
			goombaPair.first->Reset();
		}
		else
		{
			GoombaDesc goombaDesc{ pDefaultMaterial };
			goombaPair.second->alive = true;
			GoombaCharacter* pGoomba = m_pGoombaSpawnerGO->AddChild(new GoombaCharacter(goombaDesc, m_pCharacter, goombaPair.second));
			pGoomba->GetTransform()->Translate(goombaPair.second->spawnPosition);
			goombaPair.first = pGoomba;
		}
	}

	for (auto& bobOmbPair : m_pBobOmbs)
	{
		if (bobOmbPair.second->alive)
		{
			bobOmbPair.first->GetTransform()->Translate(bobOmbPair.second->spawnPosition);
			bobOmbPair.first->Reset();
		}
		else
		{
			BoBombDesc bobOmbDesc{ pDefaultMaterial };
			bobOmbPair.second->alive = true;
			BoBombCharacter* pBobOmb = m_pBobOmbSpawnerGO->AddChild(new BoBombCharacter(bobOmbDesc, m_pCharacter, bobOmbPair.second));
			pBobOmb->GetTransform()->Translate(bobOmbPair.second->spawnPosition);
			bobOmbPair.first = pBobOmb;
		}
	}

	for(auto& cannonball : m_pCannonballs)
	{
		cannonball.first->Reset();
	}

	m_pChainchomp->Reset();
	m_pStar->Reset();
	m_pTube->Reset();


	m_ChangingScreen = false;
	m_ChangeCounter = 0.0f;
}

void MainScene::SpawnGoombas(PxMaterial* pDefaultMaterial)
{
	std::vector<XMFLOAT3> spawnPositions;
	spawnPositions.push_back(XMFLOAT3(65.f, 5.f, 103.f));
	spawnPositions.push_back(XMFLOAT3(19.15f, 10.f, 95.8f));
	spawnPositions.push_back(XMFLOAT3(-49.3f, 10.f, 94.8f));
	spawnPositions.push_back(XMFLOAT3(-53.7f, 20.f, 40.45f));
	spawnPositions.push_back(XMFLOAT3(-42.5f, 15.f, 104.23f));
	spawnPositions.push_back(XMFLOAT3(41.5f, 2.f, 24.84f));

	for (const auto& pos : spawnPositions)
	{
		auto* pGoombaInfo = new SpawnInfo{};
		pGoombaInfo->spawnPosition = pos;
		GoombaDesc goombaDesc{ pDefaultMaterial };
		m_pGoombaSpawnerGO = AddChild(new GameObject());
		GoombaCharacter* pGoomba = m_pGoombaSpawnerGO->AddChild(new GoombaCharacter(goombaDesc, m_pCharacter, pGoombaInfo));
		pGoomba->GetTransform()->Translate(pGoombaInfo->spawnPosition);
		m_pGoombas.push_back(std::make_pair(pGoomba, pGoombaInfo));
	}
}

void MainScene::SpawnBobOmbs(PxMaterial* pDefaultMaterial)
{
	std::vector<XMFLOAT3> spawnPositions;
	spawnPositions.push_back(XMFLOAT3(90.3f, 15.f, 39.5f));
	spawnPositions.push_back(XMFLOAT3(86.9f, 18.f, -4.1f));
	spawnPositions.push_back(XMFLOAT3(113.5f, 18.f, -26.f));
	spawnPositions.push_back(XMFLOAT3(-98.89f, 35.f, -98.32f));
	spawnPositions.push_back(XMFLOAT3(50.22f, 1.9f, -44.781f));

	for (const auto& pos : spawnPositions)
	{
		auto* pBobOmbInfo = new SpawnInfo{};
		pBobOmbInfo->spawnPosition = pos;
		BoBombDesc bobOmbDesc{ pDefaultMaterial };
		m_pBobOmbSpawnerGO = AddChild(new GameObject());
		BoBombCharacter* pBobOmb = m_pBobOmbSpawnerGO->AddChild(new BoBombCharacter(bobOmbDesc, m_pCharacter, pBobOmbInfo));
		pBobOmb->GetTransform()->Translate(pBobOmbInfo->spawnPosition);
		m_pBobOmbs.push_back(std::make_pair(pBobOmb, pBobOmbInfo));
	}
}

void MainScene::SpawnCannonballs(PxMaterial* pDefaultMaterial)
{
	std::vector<CannonballSpawnInfo*> cannonballs;

	// The first looping pair
	auto* pCannonBallInfo = new CannonballSpawnInfo{};
	pCannonBallInfo->targetPositions.push_back(XMFLOAT3(35.4104f, 18.f, -51.6746f));
	pCannonBallInfo->targetPositions.push_back(XMFLOAT3(24.031f, 18.f, -55.4012f));
	pCannonBallInfo->resetAfterAllTargets = false;
	cannonballs.push_back(pCannonBallInfo);

	pCannonBallInfo = new CannonballSpawnInfo{};
	pCannonBallInfo->targetPositions.push_back(XMFLOAT3(17.5684f, 18.f, -44.4077f));
	pCannonBallInfo->targetPositions.push_back(XMFLOAT3(28.5352f, 18.f, -39.2283f));
	pCannonBallInfo->resetAfterAllTargets = false;
	cannonballs.push_back(pCannonBallInfo);


	// The descending pair
	pCannonBallInfo = new CannonballSpawnInfo{};
	pCannonBallInfo->targetPositions.push_back(XMFLOAT3(19.2877f, 55.f, -70.1916f));
	pCannonBallInfo->targetPositions.push_back(XMFLOAT3(-33.7604f, 42.f, -100.914f));
	pCannonBallInfo->targetPositions.push_back(XMFLOAT3(-75.5462f, 36.f, -82.6151f));
	pCannonBallInfo->targetPositions.push_back(XMFLOAT3(-73.424f, 34.f, -34.5874f));
	pCannonBallInfo->targetPositions.push_back(XMFLOAT3(-35.839f, 30.f, -12.1882f));
	pCannonBallInfo->resetAfterAllTargets = true;
	cannonballs.push_back(pCannonBallInfo);

	pCannonBallInfo = new CannonballSpawnInfo{};
	std::vector<XMFLOAT3> targetPos04;
	pCannonBallInfo->targetPositions.push_back(XMFLOAT3(-3.66327f, 72.f, -76.4811f));
	pCannonBallInfo->targetPositions.push_back(XMFLOAT3(-34.3582f, 60.f, -93.2189f));
	pCannonBallInfo->targetPositions.push_back(XMFLOAT3(-63.7088f, 53.f, -75.5881f));
	pCannonBallInfo->targetPositions.push_back(XMFLOAT3(-63.1872f, 51.f, -40.6746f));
	pCannonBallInfo->targetPositions.push_back(XMFLOAT3(-36.1968f, 49.f, -23.512f));
	pCannonBallInfo->resetAfterAllTargets = true;
	cannonballs.push_back(pCannonBallInfo);

	for (CannonballSpawnInfo* pInfo : cannonballs)
	{
		CannonballDesc cannonballDesc{ pDefaultMaterial };
		m_pCannonballsSpawnerGO = AddChild(new GameObject());
		CannonballCharacter* pCannonball = m_pCannonballsSpawnerGO->AddChild(new CannonballCharacter(cannonballDesc, m_pCharacter, pInfo));
		pCannonball->GetTransform()->Translate(pInfo->targetPositions[0]);
		m_pCannonballs.push_back(std::make_pair(pCannonball, pInfo));
	}
}

void MainScene::TogglePause(bool paused)
{
	m_Paused = paused;
	m_PauseSelectedButton = Resume;

	if(paused)
	{
		// Update Music
		m_pTrackLoopChannel->setVolume(0.3f);
		if (m_LoopingTrack == false)
			m_pTrackStartChannel->setVolume(0.3f);

		// Update Visuals
		m_pPauseBackground->SetColor({ 1.f,1.f,1.f,1.f });
		m_pPauseTitle->SetColor({ 1.f,1.f,1.f,1.f });
		m_pPauseResumeButton->SetColor({ 1.f,1.f,1.f,1.f });
		m_pPauseBackButton->SetColor(m_UnselectedColor);
		m_pPauseQuitButton->SetColor(m_UnselectedColor);
	}
	else
	{
		// Update Music
		m_pTrackLoopChannel->setVolume(1.f);
		if (m_LoopingTrack == false)
			m_pTrackStartChannel->setVolume(1.f);

		// Update Visuals
		m_pPauseBackground->SetColor({ 1.f,1.f,1.f,0.f });
		m_pPauseTitle->SetColor({ 1.f,1.f,1.f,0.f });
		m_pPauseResumeButton->SetColor({ 1.f,1.f,1.f,0.f });
		m_pPauseBackButton->SetColor({ 1.f,1.f,1.f,0.f });
		m_pPauseQuitButton->SetColor({ 1.f,1.f,1.f,0.f });
	}

	// Freeze/Unfreeze the game
	m_pCharacter->TogglePause(paused);

	for (auto& goombaPair : m_pGoombas)
	{
		if (goombaPair.second->alive)
			goombaPair.first->TogglePause(paused);
	}

	for (auto& bobOmbPair : m_pBobOmbs)
	{
		if (bobOmbPair.second->alive)
			bobOmbPair.first->TogglePause(paused);
	}

	for (auto& cannonball : m_pCannonballs)
	{
		cannonball.first->TogglePause(paused);
	}

	m_pStar->TogglePause(paused);
	m_pChainchomp->TogglePause(paused);
}

void MainScene::ToggleSoundFadeOut(float fadeTime)
{
	m_SoundTotalFadeTime = fadeTime;
	m_SoundFadingOut = true;

	// Tell Spawned Objects To Start Fading Out Sounds

	for (auto& goombaPair : m_pGoombas)
	{
		if (goombaPair.second->alive)
			goombaPair.first->ToggleSoundFadeOut(fadeTime);
	}

	for (auto& bobOmbPair : m_pBobOmbs)
	{
		if (bobOmbPair.second->alive)
			bobOmbPair.first->ToggleSoundFadeOut(fadeTime);
	}

	for (auto& cannonball : m_pCannonballs)
	{
		cannonball.first->ToggleSoundFadeOut(fadeTime);
	}

	m_pChainchomp->ToggleSoundFadeOut(fadeTime);
	m_pStar->ToggleSoundFadeOut(fadeTime);
}

void MainScene::UpdateSoundFadeOut()
{
	// Fade Out Level Track

	m_SoundFadeOutCounter += m_SceneContext.pGameTime->GetElapsed();
	auto currentVolPercent = 1.f - m_SoundFadeOutCounter / m_SoundTotalFadeTime;

	if(m_SoundFadeOutCounter > m_SoundTotalFadeTime)
	{
		currentVolPercent = 0.0f;
		m_SoundFadeOutCounter = 0.0f;
		m_SoundFadingOut = false;
	}

	if (m_MusicStarted)
	{
		if (m_LoopingTrack)
		{
			m_pTrackLoopChannel->setVolume(currentVolPercent);
		}
		else
		{
			m_pTrackStartChannel->setVolume(currentVolPercent);
		}
	}
}


void MainScene::HandlePauseScreenInput()
{
	// If a pause button hasn't been pressed yet, check for PauseScreen specific inputs
	if (m_ChangingScreen == false)
	{
		// Check for the pause toggle input
		if (m_SceneContext.pInput->IsActionTriggered(Pause1) || m_SceneContext.pInput->IsActionTriggered(Pause2))
		{
			if (m_Paused)
				TogglePause(false);
			else
				TogglePause(true);

			return;
		}

		// If the pause screen is open, execute the selection inputs
		if (m_Paused)
		{
			if (m_SceneContext.pInput->IsActionTriggered(CharacterJump)) // CharacterJump serves as a confirm input
			{
				SoundManager::Get()->GetSystem()->playSound(m_pPauseConfirmSound, nullptr, false, nullptr);
				m_ChangingScreen = true;
				return;
			}
			else if (m_SceneContext.pInput->IsActionTriggered(PauseSelectUp))
			{
				switch(m_PauseSelectedButton)
				{
				case Back:
					SoundManager::Get()->GetSystem()->playSound(m_pPauseChangeSound, nullptr, false, &m_pPauseChangeChannel);
					m_pPauseChangeChannel->setVolume(0.4f);
					m_PauseSelectedButton = Resume;
					m_pPauseResumeButton->SetColor({ 1.f,1.f,1.f,1.f });
					m_pPauseBackButton->SetColor(m_UnselectedColor);
					break;
				case Quit:
					SoundManager::Get()->GetSystem()->playSound(m_pPauseChangeSound, nullptr, false, &m_pPauseChangeChannel);
					m_pPauseChangeChannel->setVolume(0.4f);
					m_PauseSelectedButton = Back;
					m_pPauseBackButton->SetColor({ 1.f,1.f,1.f,1.f });
					m_pPauseQuitButton->SetColor(m_UnselectedColor);
					break;
				default:
					break;
				}
			}
			else if (m_SceneContext.pInput->IsActionTriggered(PauseSelectDown))
			{
				switch (m_PauseSelectedButton)
				{
				case Resume:
					SoundManager::Get()->GetSystem()->playSound(m_pPauseChangeSound, nullptr, false, &m_pPauseChangeChannel);
					m_pPauseChangeChannel->setVolume(0.4f);
					m_PauseSelectedButton = Back;
					m_pPauseResumeButton->SetColor(m_UnselectedColor);
					m_pPauseBackButton->SetColor({ 1.f,1.f,1.f,1.f });
					break;
				case Back:
					SoundManager::Get()->GetSystem()->playSound(m_pPauseChangeSound, nullptr, false, &m_pPauseChangeChannel);
					m_pPauseChangeChannel->setVolume(0.4f);
					m_PauseSelectedButton = Quit;
					m_pPauseBackButton->SetColor(m_UnselectedColor);
					m_pPauseQuitButton->SetColor({ 1.f,1.f,1.f,1.f });
					break;
				default:
					break;
				}
			}
		}
	}
	else
	{
		m_ChangeCounter += m_SceneContext.pGameTime->GetElapsed();
		if (m_ChangeCounter >= m_TimeForSFX)
		{
			switch (m_PauseSelectedButton)
			{
			case Resume:
				m_ChangeCounter = 0.0f;
				m_ChangingScreen = false;
				TogglePause(false);
				break;
			case Back:
				m_ChangeCounter = 0.0f;
				m_ChangingScreen = false;
				m_pTrackStartChannel->setPaused(true);
				m_pTrackLoopChannel->setPaused(true);
				SceneManager::Get()->SetActiveGameScene(L"StartScreen");
				return;
				break;
			case Quit:
				m_ChangeCounter = 0.0f;
				m_ChangingScreen = false;
				PostQuitMessage(0);
				return;
				break;
			default:
				break;
			}
		}
	}
}




