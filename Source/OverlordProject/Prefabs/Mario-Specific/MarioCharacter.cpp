#include "stdafx.h"
#include "MarioCharacter.h"

#include "GoombaCharacter.h"
#include "Materials/Post/PostBlur.h"
#include "Materials/Post/PostColor.h"
#include "Materials/Post/PostTransition.h"
#include "Materials/Shadow/DiffuseMaterial_Shadow_Skinned.h"

MarioCharacter::MarioCharacter(const MarioDesc& characterDesc) :
	m_CharacterDesc{ characterDesc },
	m_MoveAcceleration(characterDesc.maxMoveSpeed / characterDesc.moveAccelerationTime),
	m_FallAcceleration(characterDesc.maxFallSpeed / characterDesc.fallAccelerationTime),
	m_CrouchAcceleration(characterDesc.maxCrouchSpeed / characterDesc.crouchAccelerationTime),
	m_GroundPoundAcceleration(characterDesc.maxGroundPoundSpeed / characterDesc.groundPoundAccelerationTime)
{}



void MarioCharacter::Initialize(const SceneContext& sceneContext)
{
	//Controller
	m_pControllerComponent = AddComponent(new ControllerComponent(m_CharacterDesc.controller));

	//Character Mesh
	const auto pCharacterMaterial = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow_Skinned>();
	pCharacterMaterial->SetDiffuseTexture(L"Textures/Mario_Diffuse.png");
	m_pModelGO = AddChild(new GameObject);
	const auto pModel = m_pModelGO->AddComponent(new ModelComponent(L"Meshes/Mario/Mario.ovm"));
	pModel->SetMaterial(pCharacterMaterial);
	m_pModelGO->GetTransform()->Scale(5.f, 5.f, 5.f);
	m_pModelGO->GetTransform()->Translate(0, -m_CharacterDesc.controller.height / 2.f - 0.6f, 0);
	
	m_pAnimator = pModel->GetAnimator();
	pModel->GetTransform()->Rotate(0.f, 180.f, 0.f);


	//Camera
	const auto pCamera = AddChild(new FixedCamera());
	m_pCameraComponent = pCamera->GetComponent<CameraComponent>();
	m_pCameraComponent->SetActive(true);

	auto cameraOffsetDir = XMFLOAT3(m_pModelGO->GetTransform()->GetForward().x, 0.f, m_pModelGO->GetTransform()->GetForward().z);
	XMFLOAT3 normalizedCameraOffsetDir{};
	XMStoreFloat3(&normalizedCameraOffsetDir, XMVector3Normalize(XMLoadFloat3(&cameraOffsetDir)) *
		(1.f - m_CameraVerticalInclination));
	normalizedCameraOffsetDir.y += m_CameraVerticalInclination;
	auto cameraOffsetVec = XMLoadFloat3(&normalizedCameraOffsetDir) * m_CameraOffsetDist;
	XMFLOAT3 cameraFinalOffset{};
	XMStoreFloat3(&cameraFinalOffset, cameraOffsetVec);

	m_CameraOffsetLocalPos = cameraFinalOffset;
	m_pCameraComponent->GetTransform()->Translate(cameraFinalOffset);

	m_TotalPitch += 13;

	// Lives HUD
	auto* pLivesGO = AddChild(new GameObject());
	m_LivesHUD = pLivesGO->AddComponent(new SpriteComponent(L"Textures/HUD/HUD8.png", { 0.5f,0.5f }, { 1.f,1.f,1.f,0.f }));
	pLivesGO->GetTransform()->Translate(sceneContext.windowWidth / 2.f, sceneContext.windowHeight * 0.2f, .8f);


	// Sound Effects
	auto* pSoundManagerSystem = SoundManager::Get()->GetSystem();
	FMOD::Sound* pWalkSound{};
	pSoundManagerSystem->createStream("Resources/Sounds/step-grass-improved.WAV", FMOD_DEFAULT, nullptr, &pWalkSound);
	pWalkSound->setMode(FMOD_LOOP_NORMAL);
	pSoundManagerSystem->playSound(pWalkSound, nullptr, true, &m_pWalkChannel);
	FMOD::Sound* pCrawlSound{};
	pSoundManagerSystem->createStream("Resources/Sounds/crawl-improved.WAV", FMOD_DEFAULT, nullptr, &pCrawlSound);
	pCrawlSound->setMode(FMOD_LOOP_NORMAL);
	pSoundManagerSystem->playSound(pCrawlSound, nullptr, true, &m_pCrawlChannel);
	pSoundManagerSystem->createStream("Resources/Sounds/damaged2.WAV", FMOD_DEFAULT, nullptr, &m_pDamagedSound);
	pSoundManagerSystem->createStream("Resources/Sounds/jump1a.WAV", FMOD_DEFAULT, nullptr, &m_pJump1Sound);
	pSoundManagerSystem->createStream("Resources/Sounds/jump2b.WAV", FMOD_DEFAULT, nullptr, &m_pJump2Sound);
	pSoundManagerSystem->createStream("Resources/Sounds/jump3b.WAV", FMOD_DEFAULT, nullptr, &m_pJump3Sound);
	pSoundManagerSystem->createStream("Resources/Sounds/jump3a.WAV", FMOD_DEFAULT, nullptr, &m_pBackFlipSound);
	pSoundManagerSystem->createStream("Resources/Sounds/punch.WAV", FMOD_DEFAULT, nullptr, &m_pPunchSound);
	pSoundManagerSystem->createStream("Resources/Sounds/ground-slam.WAV", FMOD_DEFAULT, nullptr, &m_pGroundSlamSound);
	pSoundManagerSystem->createStream("Resources/Sounds/camera-spin.WAV", FMOD_DEFAULT, nullptr, &m_pCameraSpinSound);
	pSoundManagerSystem->createStream("Resources/Sounds/tube-warp.WAV", FMOD_DEFAULT, nullptr, &m_pSpawnSound);


	// Post Process
	m_pPostBlur = MaterialManager::Get()->CreateMaterial<PostBlur>();
	GetScene()->AddPostProcessingEffect(m_pPostBlur);
	m_pPostBlur->SetIntensity(0.0f);
	m_pPostBlur->SetIsEnabled(false);

	m_pPostRed = MaterialManager::Get()->CreateMaterial<PostColor>();
	m_pPostRed->SetColor(XMFLOAT3(1.f, 0.f, 0.f));
	m_pPostRed->SetIntensity(0.0f);
	GetScene()->AddPostProcessingEffect(m_pPostRed);
	m_pPostRed->SetIsEnabled(false);

	m_pPostTransition = MaterialManager::Get()->CreateMaterial<PostTransition>();
	m_pPostTransition->SetBgColor(XMFLOAT3(0.f, 0.f, 0.f));
	m_pPostTransition->SetCircleCenterPos(XMFLOAT2{ 0.5f, 0.5f });
	m_pPostTransition->SetCircleRadius(1.f);
	GetScene()->AddPostProcessingEffect(m_pPostTransition);
	m_pPostTransition->SetIsEnabled(false);


	// Walking Particle
	ParticleEmitterSettings walkSettings{};
	walkSettings.velocity = { 0.f,3.f,0.f };
	walkSettings.minSize = 0.3f;
	walkSettings.maxSize = 0.4f;
	walkSettings.minEnergy = .3f;
	walkSettings.maxEnergy = .4f;
	walkSettings.minScale = 0.5f;
	walkSettings.maxScale = 1.0f;
	walkSettings.minEmitterRadius = .2f;
	walkSettings.maxEmitterRadius = .2f;
	walkSettings.color = { 1.f,1.f,1.f, 0.5f };
	const auto pObject = AddChild(new GameObject);
	m_pWalkDustEmitter = pObject->AddComponent(new ParticleEmitterComponent(L"Textures/WalkDust.png", walkSettings, 2, false));


	ParticleEmitterSettings slamStarSettings{};
	slamStarSettings.velocity = { 3.f,2.f,3.f };
	slamStarSettings.minSize = 0.3f;
	slamStarSettings.maxSize = 0.3f;
	slamStarSettings.minEnergy = m_GroundSlamParticleLifeTime;
	slamStarSettings.maxEnergy = m_GroundSlamParticleLifeTime;
	slamStarSettings.minScale = 0.9f;
	slamStarSettings.maxScale = 1.0f;
	slamStarSettings.minEmitterRadius = .2f;
	slamStarSettings.maxEmitterRadius = .2f;
	slamStarSettings.color = { 1.f,1.f,1.f, 2.f };
	const auto pSlamObject = AddChild(new GameObject);
	m_pSlamStarEmitter = pSlamObject->AddComponent(new ParticleEmitterComponent(L"Textures/Star.png",
		slamStarSettings, 8, false, true, m_GroundSlamParticleSpawnRadius));

	slamStarSettings.minSize = 0.3f;
	slamStarSettings.maxSize = 0.4f;
	slamStarSettings.velocity = { 1.f,1.f,1.f };
	slamStarSettings.color = { 1.f,1.f,1.f, 0.6f };
	m_pSlamDustEmitter = pSlamObject->AddComponent(new ParticleEmitterComponent(L"Textures/WalkDust.png",
		slamStarSettings, 8, false, true, m_GroundSlamParticleSpawnRadius / 2.f));


	// Initial Spawn Jump
	m_TotalVelocity.y = m_CharacterDesc.ThirdJumpSpeed;
	m_pAnimator->SetAnimation(L"FrontFlip");
	m_pAnimator->SetAnimationSpeed(1.2f);
	m_pAnimator->Play(true);
}

void MarioCharacter::Update(const SceneContext& sceneContext)
{
	if (m_pCameraComponent->IsActive() && m_State != Paused)
	{
		const auto elapsedTime = sceneContext.pGameTime->GetElapsed();

		// If spawning, don't allow any input and just update the jump position
		if(m_State == Spawning)
		{
			if (m_SpawnSoundPlayed == false)
			{
				SoundManager::Get()->GetSystem()->playSound(m_pSpawnSound, nullptr, false, &m_pSFXChannel);
				m_SpawnSoundPlayed = true;
			}

			if (CheckIfGrounded())
			{
				m_pAnimator->SetAnimationSpeed(1.f);
				m_pAnimator->SetAnimation(L"Idle");
				m_pAnimator->Play();
				m_State = Idle;
			}

			HandleCamera(sceneContext);
			ApplyGravity(sceneContext, false);
			ApplyMovement(elapsedTime);
			return;
		}

		// Update particle emitters
		auto currentPartPos = GetTransform()->GetWorldPosition();
		currentPartPos.y -= m_ParticleOffset;
		m_pWalkDustEmitter->GetTransform()->Translate(currentPartPos);
		m_pSlamStarEmitter->GetTransform()->Translate(currentPartPos);
		m_pSlamDustEmitter->GetTransform()->Translate(currentPartPos);
		if(m_JustGroundSlammed)
		{
			m_GroundSlamParticleCounter += elapsedTime;
			// Since the particles only spawns once, we can disable the spawning once half their life-time has passed
			if(m_GroundSlamParticleCounter >= m_GroundSlamParticleLifeTime / 2.f)
			{
				m_pSlamStarEmitter->ToggleSpawning(false);
				m_pSlamStarEmitter->ToggleSpawning(false);
				m_pSlamDustEmitter->ToggleSpawning(false);
				m_GroundSlamParticleCounter = 0.0f;
				m_JustGroundSlammed = false;
			}
		}

		if (m_State != Dead)
			HandleCamera(sceneContext);
		else
			AvoidCameraClipping();

		UpdateUI(elapsedTime);

		switch (m_State)
		{
		case Idle:
			if (CheckIfGrounded() == false)
			{
				// Do a small counter before transitioning, to make sure mario's actually falling
				// and not just walking through an irregular surface (avoiding animation jitters)
				// (for the idle, the switch time is doubled, since a jitter while crouched is much
				// more noticeable, since Mario stands up
				ApplyGravity(sceneContext, false);
				m_AirTimeCounter += elapsedTime;
				if (m_AirTimeCounter >= m_MaxAirTimeBeforeStateSwitch * 2.f )
				{
					m_AirTimeCounter = 0.0f;
					m_State = MidAir;
					break;
				}
			}
			else m_AirTimeCounter = 0.0f;

			if (sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_Crouch))
			{
				m_pAnimator->SetAnimation(L"CrouchIdle");
				m_pAnimator->Play();
				m_State = CrouchIdle;
				break;
			}
			else if (sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_Jump))
			{
				Jump(false);
				break;
			}
			else if (sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_Punch))
			{
				SoundManager::Get()->GetSystem()->playSound(m_pPunchSound, nullptr, false, &m_pSFXChannel);
				m_pAnimator->SetAnimation(L"Punch");
				m_pAnimator->SetAnimationSpeed(1.5f);
				m_pAnimator->Play();
				m_MoveSpeed = 0.0f;
				m_State = Punching;
				break;
			}
			else if (CheckIfAnyMovementInput(sceneContext))
			{
				m_pWalkChannel->setPaused(false);

				if (m_GrabbingBobOmb)
					m_pAnimator->SetAnimation(L"GrabRun");
				else
					m_pAnimator->SetAnimation(L"Run");

				m_pAnimator->Play();
				m_pWalkDustEmitter->ToggleSpawning(true);
				m_State = Running;
				break;
			}

			UpdateJumpTimer(elapsedTime);
			DecreaseSpeed(elapsedTime);

			break;

		case CrouchIdle:
			if (CheckIfGrounded() == false)
			{
				// Do a small counter before transitioning, to make sure mario's actually falling
				// and not just walking through an irregular surface (avoiding animation jitters)
				ApplyGravity(sceneContext, false);
				m_AirTimeCounter += elapsedTime;
				if (m_AirTimeCounter >= m_MaxAirTimeBeforeStateSwitch)
				{
					m_AirTimeCounter = 0.0f;
					m_State = MidAir;
					break;
				}
			}
			else m_AirTimeCounter = 0.0f;

			if (sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_Crouch) == false)
			{
				if (m_GrabbingBobOmb)
					m_pAnimator->SetAnimation(L"GrabIdle");
				else
					m_pAnimator->SetAnimation(L"Idle");
				m_pAnimator->Play();
				m_State = Idle;
				break;
			}
			else if (sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_Jump))
			{
				Jump(true);
				break;
			}
			else if (sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_Punch))
			{
				SoundManager::Get()->GetSystem()->playSound(m_pPunchSound, nullptr, false, &m_pSFXChannel);
				m_pAnimator->SetAnimation(L"Punch");
				m_pAnimator->SetAnimationSpeed(1.5f);
				m_pAnimator->Play();
				m_MoveSpeed = 0.0f;
				m_State = Punching;
				break;
			}
			else if (CheckIfAnyMovementInput(sceneContext))
			{
				m_pCrawlChannel->setPaused(false);
				m_pAnimator->SetAnimation(L"CrouchWalk");
				m_State = CrouchMoving;
				break;
			}

			UpdateJumpTimer(elapsedTime);
			DecreaseSpeed(elapsedTime);

			break;

		case Running:
			if (CheckIfGrounded() == false)
			{
				// Do a small counter before transitioning, to make sure mario's actually falling
				// and not just walking through an irregular surface (avoiding animation jitters)
				ApplyGravity(sceneContext, false);
				m_AirTimeCounter += elapsedTime;
				if(m_AirTimeCounter >= m_MaxAirTimeBeforeStateSwitch)
				{
					m_pWalkChannel->setPaused(true);
					m_pWalkDustEmitter->ToggleSpawning(false);
					m_AirTimeCounter = 0.0f;
					m_State = MidAir;
					break;
				}
			}
			else m_AirTimeCounter = 0.0f;

			if (sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_Crouch))
			{
				m_pWalkChannel->setPaused(true);
				m_pWalkDustEmitter->ToggleSpawning(false);
				m_pAnimator->SetAnimation(L"CrouchIdle");
				m_pAnimator->Play();
				m_State = CrouchIdle;
				break;
			}
			else if (sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_Jump))
			{
				m_pWalkChannel->setPaused(true);
				m_pWalkDustEmitter->ToggleSpawning(false);
				Jump(false);
				break;
			}
			else if (sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_Punch))
			{
				m_pWalkChannel->setPaused(true);
				m_pWalkDustEmitter->ToggleSpawning(false);
				SoundManager::Get()->GetSystem()->playSound(m_pPunchSound, nullptr, false, &m_pSFXChannel);
				m_pAnimator->SetAnimation(L"Punch");
				m_pAnimator->SetAnimationSpeed(1.5f);
				m_pAnimator->Play();
				m_MoveSpeed = 0.0f;
				m_State = Punching;
				break;
			}
			else if (!CheckIfAnyMovementInput(sceneContext))
			{
				m_pWalkChannel->setPaused(true);
				m_pWalkDustEmitter->ToggleSpawning(false);
				if (m_GrabbingBobOmb)
					m_pAnimator->SetAnimation(L"GrabIdle");
				else
					m_pAnimator->SetAnimation(L"Idle");
				m_pAnimator->Play();
				m_State = Idle;
				break;
			}

			UpdateJumpTimer(elapsedTime);
			MoveSideways(sceneContext, false);

			break;

		case CrouchMoving:
			if (CheckIfGrounded() == false)
			{
				// Do a small counter before transitioning, to make sure mario's actually falling
				// and not just walking through an irregular surface (avoiding animation jitters)
				ApplyGravity(sceneContext, false);
				m_AirTimeCounter += elapsedTime;
				if (m_AirTimeCounter >= m_MaxAirTimeBeforeStateSwitch)
				{
					m_pCrawlChannel->setPaused(true);
					m_AirTimeCounter = 0.0f;
					m_State = MidAir;
					break;
				}
			}
			else m_AirTimeCounter = 0.0f;

			if (sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_Crouch) == false)
			{
				m_pCrawlChannel->setPaused(true);
				if (m_GrabbingBobOmb)
					m_pAnimator->SetAnimation(L"GrabIdle");
				else
					m_pAnimator->SetAnimation(L"Idle");
				m_pAnimator->Play();
				m_State = Idle;
				break;
			}
			else if (sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_Jump))
			{
				m_pCrawlChannel->setPaused(true);
				Jump(true);
				break;
			}
			else if (sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_Punch))
			{
				m_pCrawlChannel->setPaused(true);
				SoundManager::Get()->GetSystem()->playSound(m_pPunchSound, nullptr, false, &m_pSFXChannel);
				m_pAnimator->SetAnimation(L"Punch");
				m_pAnimator->SetAnimationSpeed(1.5f);
				m_pAnimator->Play();
				m_MoveSpeed = 0.0f;
				m_State = Punching;
				break;
			}
			else if (!CheckIfAnyMovementInput(sceneContext))
			{
				m_pCrawlChannel->setPaused(true);
				m_pAnimator->SetAnimation(L"CrouchIdle");
				m_pAnimator->Play();
				m_State = CrouchIdle;
				break;
			}

			UpdateJumpTimer(elapsedTime);
			MoveSideways(sceneContext, true);

			break;

		case MidAir:
			if (CheckIfGrounded())
			{
				if (m_GrabbingBobOmb)
					m_pAnimator->SetAnimation(L"GrabIdle");
				else
					m_pAnimator->SetAnimation(L"Idle");
				m_pAnimator->Play();
				m_State = Idle;
				break;
			}
			else if (sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_Crouch) && (m_JumpCounter == 1 || m_JumpCounter == 2))
			{
				m_JumpCounter = 0;
				m_pAnimator->SetAnimation(L"GroundSlam");
				m_pAnimator->Play(true);
				m_State = GroundSlamming;
				break;
			}

			if(m_JumpCounter == 4)
				MoveSidewaysBackflip(sceneContext);
			else
				MoveSideways(sceneContext, false);
			ApplyGravity(sceneContext, false);
			break;

		case GroundSlamming:
			if (CheckIfGrounded())
			{
				m_pSlamStarEmitter->ToggleSpawning(true);
				m_pSlamStarEmitter->ToggleSpawning(true);
				m_pSlamDustEmitter->ToggleSpawning(true);
				m_JustGroundSlammed = true;
				SoundManager::Get()->GetSystem()->playSound(m_pGroundSlamSound, nullptr, false, &m_pSFXChannel);
				m_State = Idle;
				break;
			}

			MoveSideways(sceneContext, false);
			ApplyGravity(sceneContext, true);
			break;

		case Punching:
			UpdateJumpTimer(elapsedTime);
			m_PunchTimer += elapsedTime;
			if (m_PunchTimer >= m_PunchMaxTime)
			{
				m_PunchTimer = 0.0f;
				if (m_GrabbingBobOmb)
					m_pAnimator->SetAnimation(L"GrabIdle");
				else
					m_pAnimator->SetAnimation(L"Idle");
				m_pAnimator->Play();
				m_State = Idle;
				break;
			}
			break;

		case MidDamaged:
			if ((m_DmgBackToIdleOnLand && CheckIfGrounded()) || m_DamagedForTooLong)
			{
				if (m_GrabbingBobOmb)
					m_pAnimator->SetAnimation(L"GrabIdle");
				else
					m_pAnimator->SetAnimation(L"Idle");
				m_pAnimator->Play();
				m_State = Idle;
				break;
			}

			ApplyGravity(sceneContext, false);
			UpdateJumpTimer(elapsedTime);
			DecreaseLungedVelocity(elapsedTime);

			break;

		case Dead:
			if (CheckIfGrounded() == false)
				ApplyGravity(sceneContext, false);

			DecreaseLungedVelocity(elapsedTime);

			m_TransitionCounter += sceneContext.pGameTime->GetElapsed();
			if (m_TransitionCounter < m_TransitionMaxTime)
			{
				m_pPostTransition->SetCircleRadius(m_TransitionMaxRadius * (1.f - m_TransitionCounter / m_TransitionMaxTime));
			}
			else
			{
				m_TransitionCounter = 0.0f;
				SceneManager::Get()->SetActiveGameScene(L"DeathScreen");
			}

		default:
			break;
		}

		ApplyMovement(elapsedTime);
		UpdateRecovery(elapsedTime);
	}
}



void MarioCharacter::Jump(bool crouched)
{
	//Set m_TotalVelocity.y equal to the appropriate JumpSpeed
	m_BetweenJumpsTimer = 0.f;

	// If crouched, backflip
	if (crouched)
		m_JumpCounter = 4;

	m_State = MidAir;

	switch (m_JumpCounter)
	{
	case 0:
		m_TotalVelocity.y = m_CharacterDesc.NormalJumpSpeed;
		SoundManager::Get()->GetSystem()->playSound(m_pJump1Sound, nullptr, false, &m_pSFXChannel);
		if (m_GrabbingBobOmb)
		{
			m_pAnimator->SetAnimation(L"GrabJump");
		}
		else
		{
			m_pAnimator->SetAnimation(L"Jump1");
			m_pAnimator->SetAnimationSpeed(0.8f);
		}
		m_pAnimator->Play(true);
		m_JumpCounter++;
		break;
	case 1:
		m_TotalVelocity.y = m_CharacterDesc.SecondJumpSpeed;
		SoundManager::Get()->GetSystem()->playSound(m_pJump2Sound, nullptr, false, &m_pSFXChannel);
		if (m_GrabbingBobOmb)
		{
			m_pAnimator->SetAnimation(L"GrabJump");
		}
		else
		{
			m_pAnimator->SetAnimation(L"Jump2");
			m_pAnimator->SetAnimationSpeed(0.3f);
		}
		m_pAnimator->Play(true);
		m_JumpCounter++;
		break;
	case 2:
		m_TotalVelocity.y = m_CharacterDesc.ThirdJumpSpeed;
		SoundManager::Get()->GetSystem()->playSound(m_pJump3Sound, nullptr, false, &m_pSFXChannel);
		if (m_GrabbingBobOmb)
		{
			m_pAnimator->SetAnimation(L"GrabJump");
		}
		else
		{
			m_pAnimator->SetAnimation(L"FrontFlip");
			m_pAnimator->SetAnimationSpeed(1.2f);
		}
		m_pAnimator->Play(true);
		m_JumpCounter++;
		break;
	case 3:
		SoundManager::Get()->GetSystem()->playSound(m_pJump1Sound, nullptr, false, &m_pSFXChannel);
		m_TotalVelocity.y = m_CharacterDesc.NormalJumpSpeed;
		if (m_GrabbingBobOmb)
		{
			m_pAnimator->SetAnimation(L"GrabJump");
		}
		else
		{
			m_pAnimator->SetAnimation(L"Jump1");
			m_pAnimator->SetAnimationSpeed(0.8f);
		}
		m_pAnimator->Play(true);
		m_JumpCounter = 1;
		break;
	case 4:
		const auto newDirection = XMLoadFloat3(&m_CurrentDirection) * -1.f;
		XMStoreFloat3(&m_CurrentDirection, newDirection);
		SoundManager::Get()->GetSystem()->playSound(m_pBackFlipSound, nullptr, false, &m_pSFXChannel);
		if (m_GrabbingBobOmb)
		{
			m_pAnimator->SetAnimation(L"GrabJump");
		}
		else
		{
			m_pAnimator->SetAnimation(L"BackFlip");
			m_pAnimator->SetAnimationSpeed(1.5f);
		}
		m_pAnimator->Play(true);
		m_TotalVelocity.y = m_CharacterDesc.BackFlipSpeedUp;
		break;
	}

	// UNCOMMENT THIS CODE TO PRINT POS ON JUMP
	/*std::ostringstream xs;
	xs << this->GetTransform()->GetPosition().x;
	std::string xPos(xs.str());
	std::ostringstream ys;
	ys << this->GetTransform()->GetPosition().y;
	std::string yPos(ys.str());
	std::ostringstream zs;
	zs << this->GetTransform()->GetPosition().z;
	std::string zPos(zs.str());
	auto finalPos = xPos + " / " + yPos + " / " + zPos;
	std::cout << finalPos << '\n';*/
}

void MarioCharacter::UpdateJumpTimer(float elapsedTime)
{
	if (m_JumpCounter > 0)
	{
		m_BetweenJumpsTimer += elapsedTime;
		if (m_BetweenJumpsTimer > m_BetweenJumpsMaxTime)
			m_JumpCounter = 0;
	}
}


void MarioCharacter::HandleCamera(const SceneContext& sceneContext)
{
	float elapsedTime = sceneContext.pGameTime->GetElapsed();
	float finalPitch = m_TotalPitch;

	// Gamepad
	bool gamepadMoved = false;
	if (sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_TurnCameraLeft))
	{
		if (m_CameraSoundPlaying == false)
		{
			m_CameraSoundPlaying = true;
			SoundManager::Get()->GetSystem()->playSound(m_pCameraSpinSound, nullptr, false, &m_pCameraSpinChannel);
		}
		m_TotalYaw -= m_CharacterDesc.gamepadRotationSpeed * elapsedTime;
		gamepadMoved = true;
	}
	if (sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_TurnCameraRight))
	{
		if (m_CameraSoundPlaying == false)
		{
			m_CameraSoundPlaying = true;
			SoundManager::Get()->GetSystem()->playSound(m_pCameraSpinSound, nullptr, false, &m_pCameraSpinChannel);
		}
		m_TotalYaw += m_CharacterDesc.gamepadRotationSpeed * elapsedTime;
		gamepadMoved = true;
	}

	// Mouse (gamepad has priority)
	if (gamepadMoved == false)
	{
		m_pCameraSpinChannel->setPaused(true);
		m_CameraSoundPlaying = false;
		XMFLOAT2 look{ 0.f, 0.f };
		const auto& mouseMove = InputManager::GetMouseMovement();
		look.x = static_cast<float>(mouseMove.x);
		look.y = static_cast<float>(mouseMove.y);
		m_TotalYaw += look.x * m_CharacterDesc.mouseRotationSpeed * elapsedTime;
		finalPitch += look.y * m_CharacterDesc.mouseRotationSpeed * elapsedTime;
	}

	// Lock pitch within limits
	if (finalPitch < m_PitchMax)
	{
		if (finalPitch > m_PitchMin)
		{
			m_TotalPitch = finalPitch;
		}
		else m_TotalPitch = m_PitchMin;
	}
	else m_TotalPitch = m_PitchMax;

	// Rotate the camera and Mario's model based on the TotalPitch (X) and the TotalYaw (Y)
	GetTransform()->Rotate(m_TotalPitch, m_TotalYaw, 0);

	// And compensate the model's X rotation so it doesn't recline backwards/forwards
	m_pModelGO->GetTransform()->Rotate(m_TotalPitch, 180, 0);

	// But make sure the camera doesn't clip through meshes
	AvoidCameraClipping();
}

void MarioCharacter::AvoidCameraClipping()
{
	// Calculate the raycast origin - the model's center
	XMFLOAT3 originXMF{ m_pModelGO->GetTransform()->GetWorldPosition().x,
		m_pModelGO->GetTransform()->GetWorldPosition().y + m_CharacterDesc.controller.height / 2.f + 0.6f,
		m_pModelGO->GetTransform()->GetWorldPosition().z };
	PxVec3 origin{ originXMF.x, originXMF.y, originXMF.z };

	// Get the distance between the new camera pos and the model
	const auto distanceVec = XMLoadFloat3(&m_pCameraComponent->GetTransform()->GetWorldPosition()) - XMLoadFloat3(&originXMF);
	PxReal maxDistance{};
	XMStoreFloat(&maxDistance, XMVector3Length(distanceVec));

	// Get the normalized direction between the positions
	auto directionVec = XMVector3Normalize(distanceVec);
	XMFLOAT3 directionXMF{};
	XMStoreFloat3(&directionXMF, directionVec);
	const auto direction = PxVec3(directionXMF.x, directionXMF.y, directionXMF.z);

	// Set up all the raycasting requirements
	PxRaycastBuffer hit;
	PxQueryFilterData filterData{};
	filterData.data.word0 = UINT(CollisionGroup::Group9);

	// If the line of sight between the camera and the model is covered
	if (GetScene()->GetPhysxProxy()->Raycast(origin, direction, maxDistance, hit, PxHitFlag::eDEFAULT, filterData))
	{
		// Shorten the camera's distance until adequate
		XMFLOAT3 collisionPos{ hit.getAnyHit(static_cast<PxU32>(0)).position.x,
			hit.getAnyHit(static_cast<PxU32>(0)).position.y, hit.getAnyHit(static_cast<PxU32>(0)).position.z };
		const auto modelToCollVec = XMLoadFloat3(&collisionPos) - XMLoadFloat3(&originXMF);
		XMStoreFloat(&m_CameraCurrentOffsetDist, XMVector3Length(modelToCollVec));
		m_CameraCurrentOffsetDist *= 0.9f;

		UpdateCameraWCurrentOffset();
	}
	// If it's not covered and the current offset is still shortened
	else if (m_CameraCurrentOffsetDist < m_CameraOffsetDist)
	{
		// Check if there's still a collision between the full offset's position and the model
		if (GetScene()->GetPhysxProxy()->Raycast(origin, direction, m_CameraOffsetDist, hit, PxHitFlag::eDEFAULT, filterData))
		{
			// If there is, increase the camera's distance only up until the collision point
			XMFLOAT3 collisionPos{ hit.getAnyHit(static_cast<PxU32>(0)).position.x,
				hit.getAnyHit(static_cast<PxU32>(0)).position.y, hit.getAnyHit(static_cast<PxU32>(0)).position.z };
			const auto modelToCollVec = XMLoadFloat3(&collisionPos) - XMLoadFloat3(&originXMF);
			XMStoreFloat(&m_CameraCurrentOffsetDist, XMVector3Length(modelToCollVec));
			m_CameraCurrentOffsetDist *= 0.95f;
			UpdateCameraWCurrentOffset();
		}
		else // And if there's no collision at the full offset
		{
			// Return the camera to its regular offset
			m_CameraCurrentOffsetDist = m_CameraOffsetDist;
			UpdateCameraWCurrentOffset();
		}
	}
}


void MarioCharacter::UpdateCameraWCurrentOffset() const
{
	auto cameraFinalOffsetVec = XMLoadFloat3(&m_CameraOffsetLocalPos)
		* m_CameraCurrentOffsetDist / m_CameraOffsetDist;
	XMFLOAT3 cameraFinalOffset{};
	XMStoreFloat3(&cameraFinalOffset, cameraFinalOffsetVec);
	m_pCameraComponent->GetTransform()->Translate(cameraFinalOffset);
}

void MarioCharacter::DecreaseSpeed(float elapsedTime)
{
	//Decrease the current MoveSpeed with the current Acceleration (m_MoveSpeed)
	m_MoveSpeed -= m_MoveAcceleration * elapsedTime;
	//Make sure the current MoveSpeed doesn't get smaller than zero
	if (m_MoveSpeed < 0.0f)
		m_MoveSpeed = 0.0f;
}

void MarioCharacter::DecreaseLungedVelocity(float elapsedTime)
{
	if (m_LungeForceXPositive)
	{
		m_TotalVelocity.x -= m_MoveAcceleration * elapsedTime;
		if (m_TotalVelocity.x < 0.0f)
			m_TotalVelocity.x = 0.0f;
	}
	else
	{
		m_TotalVelocity.x += m_MoveAcceleration * elapsedTime;
		if (m_TotalVelocity.x > 0.0f)
			m_TotalVelocity.x = 0.0f;
	}

	if (m_LungeForceZPositive)
	{
		m_TotalVelocity.z -= m_MoveAcceleration * elapsedTime;
		if (m_TotalVelocity.z < 0.0f)
			m_TotalVelocity.z = 0.0f;
	}
	else
	{
		m_TotalVelocity.z += m_MoveAcceleration * elapsedTime;
		if (m_TotalVelocity.z > 0.0f)
			m_TotalVelocity.z = 0.0f;
	}
}

void MarioCharacter::MoveSideways(const SceneContext& sceneContext, bool crouched)
{
	// Gather Input
	XMFLOAT2 move{};

	if (sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_MoveForward))
		move.y = 1;
	else if (sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_MoveBackward))
		move.y = -1;

	if (sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_MoveRight))
		move.x = 1;
	else if (sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_MoveLeft))
		move.x = -1;


	// Gather Transform Info
	const auto transform = GetTransform();
	XMVECTOR forward = XMLoadFloat3(&transform->GetForward());
	XMVECTOR right = XMLoadFloat3(&transform->GetRight());
	const auto elapsedTime = sceneContext.pGameTime->GetElapsed();


	// Horizontal Velocity (Forward/Backward/Right/Left)
	if (move.x != 0.f || move.y != 0.f) //If the character is moving (= input is pressed)
	{
		//Calculate & Store the current direction (m_CurrentDirection) >> based on the forward/right vectors and the pressed input
		XMStoreFloat3(&m_CurrentDirection, XMVector3Normalize(forward * move.y + right * move.x));

		if (crouched)
		{
			// Switch the animation to the adequate direction
			if (m_JumpCounter == 0)
			{
				if (move.y != 0.0f)
				{
					if (m_CurrentDirection.y < 0.0f)
					{
						if (m_pAnimator->GetClipName() != L"CrouchWalk")
						{
							m_pAnimator->SetAnimation(L"CrouchWalk");
							m_pAnimator->Play();
						}
					}
					else
					{
						if (m_pAnimator->GetClipName() != L"CrouchWalkBack")
						{
							m_pAnimator->SetAnimation(L"CrouchWalkBack");
							m_pAnimator->Play();
						}
					}
				}
				else
				{
					if (move.x > 0.0f)
					{
						if (m_pAnimator->GetClipName() != L"CrouchWalkRight")
						{
							m_pAnimator->SetAnimation(L"CrouchWalkRight");
							m_pAnimator->Play();
						}
					}
					else
					{
						if (m_pAnimator->GetClipName() != L"CrouchWalkLeft")
						{
							m_pAnimator->SetAnimation(L"CrouchWalkLeft");
							m_pAnimator->Play();
						}
					}
				}
			}

			// Apply the speed
			m_MoveSpeed += m_CrouchAcceleration * elapsedTime;
			if (m_MoveSpeed > m_CharacterDesc.maxCrouchSpeed)
				m_MoveSpeed = m_CharacterDesc.maxCrouchSpeed;
		}
		else
		{
			// Switch the animation to the adequate direction
			if (m_JumpCounter == 0 && m_State != GroundSlamming)
			{
				if (move.y != 0.0f)
				{
					if(move.y > 0.f)
					{
						if (m_pAnimator->GetClipName() != L"Run" && m_pAnimator->GetClipName() != L"GrabRun")
						{
							if (m_GrabbingBobOmb)
								m_pAnimator->SetAnimation(L"GrabRun");
							else
								m_pAnimator->SetAnimation(L"Run");
							m_pAnimator->Play();
						}
					}
					else
					{
						if (m_pAnimator->GetClipName() != L"RunBack" && m_pAnimator->GetClipName() != L"GrabRunBack")
						{
							if (m_GrabbingBobOmb)
								m_pAnimator->SetAnimation(L"GrabRunBack");
							else
								m_pAnimator->SetAnimation(L"RunBack");
							m_pAnimator->Play();
						}
					}
				}
				else
				{
					if (move.x > 0.0f)
					{
						if (m_pAnimator->GetClipName() != L"RunRight" && m_pAnimator->GetClipName() != L"GrabRunRight")
						{
							if (m_GrabbingBobOmb)
								m_pAnimator->SetAnimation(L"GrabRunRight");
							else
								m_pAnimator->SetAnimation(L"RunRight");
							m_pAnimator->Play();
						}
					}
					else
					{
						if (m_pAnimator->GetClipName() != L"RunLeft" && m_pAnimator->GetClipName() != L"GrabRunLeft")
						{
							if (m_GrabbingBobOmb)
								m_pAnimator->SetAnimation(L"GrabRunLeft");
							else
								m_pAnimator->SetAnimation(L"RunLeft");
							m_pAnimator->Play();
						}
					}
				}
			}

			// Apply the speed
			m_MoveSpeed += m_MoveAcceleration * elapsedTime;
			if (m_MoveSpeed > m_CharacterDesc.maxMoveSpeed)
				m_MoveSpeed = m_CharacterDesc.maxMoveSpeed;
		}
	}
}

void MarioCharacter::MoveSidewaysBackflip(const SceneContext& sceneContext)
{
	// Gather Input
	float move{};

	if (sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_MoveRight))
		move = 0.5f;
	else if (sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_MoveLeft))
		move = -0.5f;


	// Gather Transform Info
	const auto transform = GetTransform();
	XMVECTOR forward = XMLoadFloat3(&transform->GetForward());
	XMVECTOR right = XMLoadFloat3(&transform->GetRight());
	const auto elapsedTime = sceneContext.pGameTime->GetElapsed();


	//Calculate & Store the current direction (m_CurrentDirection) >> based on the forward/right vectors and the pressed input
	XMStoreFloat3(&m_CurrentDirection, XMVector3Normalize(forward * -1.f + right * move));

	m_MoveSpeed += m_MoveAcceleration * elapsedTime;
	if (m_MoveSpeed > m_CharacterDesc.BackFlipSpeedBack)
		m_MoveSpeed = m_CharacterDesc.BackFlipSpeedBack;
}

void MarioCharacter::ApplyGravity(const SceneContext& sceneContext, bool groundSlamming)
{
	const auto elapsedTime = sceneContext.pGameTime->GetElapsed();

	if (groundSlamming) // If the player is groundslamming
	{
		//m_TotalVelocity.y = -m_CharacterDesc.maxGroundPoundSpeed;
		m_TotalVelocity.y -= m_GroundPoundAcceleration * elapsedTime;
		if (m_TotalVelocity.y < -m_CharacterDesc.maxGroundPoundSpeed)
			m_TotalVelocity.y = -m_CharacterDesc.maxGroundPoundSpeed;
	}
	else // If it's a normal fall
	{
		m_TotalVelocity.y -= m_FallAcceleration * elapsedTime;
		if (m_TotalVelocity.y < -m_CharacterDesc.maxFallSpeed)
			m_TotalVelocity.y = -m_CharacterDesc.maxFallSpeed;
	}
}

void MarioCharacter::ApplyMovement(float elapsedTime)
{
	if (m_State != MidDamaged && m_State != Dead)
	{
		//Now we can calculate the Horizontal Velocity which should be stored in m_TotalVelocity.xz
		auto horizontalVelocity = m_CurrentDirection;
		horizontalVelocity.x *= m_MoveSpeed;
		horizontalVelocity.z *= m_MoveSpeed;

		//Set the x/z component of m_TotalVelocity (horizontal_velocity x/z)
		m_TotalVelocity.x = horizontalVelocity.x;
		m_TotalVelocity.z = horizontalVelocity.z;
	}

	//The displacement required to move the Character Controller (ControllerComponent::Move) can be calculated using our TotalVelocity (m/s)
	//Calculate the displacement (m) for the current frame and move the ControllerComponent
	auto displacement = m_TotalVelocity;
	displacement.x *= elapsedTime;
	displacement.y *= elapsedTime;
	displacement.z *= elapsedTime;
	m_pControllerComponent->Move(displacement);
}

bool MarioCharacter::CheckIfGrounded()
{
	auto origin = PxVec3(0, 0, 0);
	origin.x = m_pControllerComponent->GetTransform()->GetPosition().x; // - m_CharacterDesc.controller.height;
	origin.y = m_pControllerComponent->GetTransform()->GetPosition().y; // - m_CharacterDesc.controller.height;
	origin.z = m_pControllerComponent->GetTransform()->GetPosition().z; // - m_CharacterDesc.controller.height;
	const auto direction = PxVec3(0, -1, 0);
	const PxReal maxDistance = 1.8f;//m_CharacterDesc.controller.height;
	PxRaycastBuffer hit;
	PxQueryFilterData filterData{};
	filterData.data.word0 = UINT(CollisionGroup::Group9);


	if(GetScene()->GetPhysxProxy()->Raycast(origin, direction, maxDistance, hit, PxHitFlag::eDEFAULT, filterData))
	{
		m_TotalVelocity.y = 0;

		if(m_JumpCounter == 4)
			m_JumpCounter = 0;

		return true;
	}

	return false;
}

bool MarioCharacter::CheckIfAnyMovementInput(const SceneContext& sceneContext) const
{
	return sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_MoveForward) ||
		sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_MoveBackward) ||
		sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_MoveRight) ||
		sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_MoveLeft);
}

void MarioCharacter::UpdateRecovery(float elapsedTime)
{
	if(m_Recovering)
	{
		m_RecoveryCounter += elapsedTime;
		if (m_RecoveryCounter >= m_MinFreezeDamageTime)
		{
			m_DmgBackToIdleOnLand = true;

			if (m_RecoveryCounter >= m_MaxFreezeDamageTime)
			{
				m_DamagedForTooLong = true;

				if (m_RecoveryCounter >= m_RecoveryTime)
				{
					m_RecoveryCounter = 0;
					m_Recovering = false;
				}
			}
		}
	}
}

void MarioCharacter::GetDamaged(int dmgAmount, XMFLOAT3 dmgOrigin)
{
	if (m_Recovering == false && m_State != Dead)
	{
		m_pPostBlur->SetIsEnabled(true);
		m_pPostRed->SetIsEnabled(true);
		SoundManager::Get()->GetSystem()->playSound(m_pDamagedSound, nullptr, false, &m_pSFXChannel);
		m_pAnimator->SetAnimation(L"Hit");
		m_pAnimator->Play(true);
		m_pWalkDustEmitter->ToggleSpawning(false);

		m_CurrentHealth -= dmgAmount;

		if(m_CurrentHealth < 0)
			m_LivesHUD->SetTexture(L"Textures/HUD/HUD0.png");
		else
			m_LivesHUD->SetTexture(L"Textures/HUD/HUD" + std::to_wstring(m_CurrentHealth) + L".png");

		m_LivesHUD->SetColor({ 1.f,1.f,1.f,0.f });
		if(m_ShowLivesHUD)
		{
			m_HUDCounter = m_HUDFadeInTime;
		}
		else
		{
			m_HUDCounter = 0.0f;
			m_ShowLivesHUD = true;
		}
		m_Recovering = true;
		m_DmgBackToIdleOnLand = false;
		m_DamagedForTooLong = false;
		const auto throwVector = XMVector3Normalize(XMLoadFloat3(&GetTransform()->GetWorldPosition()) - XMLoadFloat3(&dmgOrigin)) * m_CharacterDesc.DamagedThrowSpeed;
		XMStoreFloat3(&m_TotalVelocity, throwVector);
		m_TotalVelocity.y += 16.f;
		m_LungeForceXPositive = m_TotalVelocity.x > 0.f;
		m_LungeForceZPositive = m_TotalVelocity.z > 0.f;
		m_State = MidDamaged;

		if (m_CurrentHealth <= 0)
		{
			m_pPostTransition->SetIsEnabled(true);
			m_pPostTransition->SetCircleRadius(1.0f);
			m_pPostTransition->SetBgColor({0.f,0.f,0.f});
			m_pPostTransition->SetCircleCenterPos({ 0.5f,0.5f });
			GetScene()->ToggleSoundFadeOut(m_TransitionMaxTime);
			m_pWalkChannel->setPaused(true);
			m_pCrawlChannel->setPaused(true);
			m_State = Dead;
		}
	}
}

void MarioCharacter::GetLunged(XMFLOAT3 lungeVector)
{
	if(m_State == GroundSlamming)
	{
		m_pSlamStarEmitter->ToggleSpawning(true);
		m_pSlamDustEmitter->ToggleSpawning(true);
		m_JustGroundSlammed = true;
		SoundManager::Get()->GetSystem()->playSound(m_pGroundSlamSound, nullptr, false, &m_pSFXChannel);
	}

	m_State = MidAir;
	XMStoreFloat3(&m_TotalVelocity, XMLoadFloat3(&lungeVector));
}

void MarioCharacter::UpdateUI(float elapsedTime)
{
	if(m_ShowLivesHUD)
	{
		m_HUDCounter += elapsedTime;
		if (m_HUDCounter < m_HUDFadeInTime) // HUD and Effects Fading In
		{
			auto fadeInValue = m_HUDCounter / m_HUDFadeInTime;
			m_LivesHUD->SetColor({ 1.f,1.f,1.f,fadeInValue });
			m_pPostBlur->SetIntensity(m_MaxBlurIntensity * fadeInValue);
			m_pPostRed->SetIntensity(m_MaxRedIntensity * fadeInValue);
		}
		else // HUD and Effects Done Fading In
		{
			// EFFECTS
			if (m_HUDCounter > m_HurtPostFullIntensityTime + m_HUDFadeInTime) // Effects Fading Out
			{
				if(m_HUDCounter > m_HurtPostFullIntensityTime + m_HUDFadeInTime + m_HurtPostFadeOutTime) // Effects Done Fading Out
				{
					m_pPostBlur->SetIsEnabled(false);
					m_pPostRed->SetIsEnabled(false);
				}
				else // Effects Still Fading Out
				{
					auto fadeOutValue = 1.f - (m_HUDCounter - m_HurtPostFullIntensityTime - m_HUDFadeInTime) / m_HurtPostFadeOutTime;
					m_pPostBlur->SetIntensity(m_MaxBlurIntensity * fadeOutValue);
					m_pPostRed->SetIntensity(m_MaxRedIntensity * fadeOutValue);
				}
			}
			else  // Effects Full Intensity
			{
				m_pPostBlur->SetIntensity(m_MaxBlurIntensity);
				m_pPostRed->SetIntensity(m_MaxRedIntensity);
			}

			// HUD
			if (m_HUDCounter < m_HUDShowTime + m_HUDFadeInTime) // HUD Full Intensity
			{
				m_LivesHUD->SetColor({ 1.f,1.f,1.f,1.f });
			}
			else
			{
				if (m_HUDCounter >= m_HUDShowTime + m_HUDFadeInTime + m_HUDFadeOutTime) // HUD Done Fading Out
				{
					m_LivesHUD->SetColor({ 1.f,1.f,1.f,0.f });
					m_ShowLivesHUD = false;
					m_HUDCounter = 0.0f;
				}
				else // HUD Fading Out
				{
					auto fadeOutValue = 1.f - (m_HUDCounter - m_HUDShowTime - m_HUDFadeInTime) / m_HUDFadeOutTime;
					m_LivesHUD->SetColor({ 1.f,1.f,1.f,fadeOutValue });
				}
			}
		}
	}
}


void MarioCharacter::Reset()
{
	m_pPostBlur->SetIntensity(0.0f);
	m_pPostBlur->SetIsEnabled(false);
	m_pPostRed->SetIntensity(0.0f);
	m_pPostRed->SetIsEnabled(false);
	m_TransitionCounter = 0.0f;
	m_pPostTransition->SetIsEnabled(false);
	m_pPostTransition->SetCircleRadius(1.f);
	m_pWalkDustEmitter->ToggleSpawning(false);
	m_pSlamStarEmitter->ToggleSpawning(false);
	m_pSlamDustEmitter->ToggleSpawning(false);
	m_GroundSlamParticleCounter = 0.0f;
	m_JustGroundSlammed = false;

	m_pSFXChannel->setPaused(true);
	m_pCameraSpinChannel->setPaused(true);
	m_pWalkChannel->setPaused(true);
	m_pCrawlChannel->setPaused(true);
	m_MoveSpeed = 0.0f;
	m_CurrentDirection = XMFLOAT3(0, 0, 0);
	m_JumpCounter = 0;
	m_BetweenJumpsTimer = 0.0f;
	m_CurrentHealth = 8;
	m_Recovering = false;
	m_DmgBackToIdleOnLand = false;
	m_DamagedForTooLong = false;
	m_RecoveryCounter = 0.0f;
	m_LivesHUD->SetColor({1.f,1.f,1.f,0.f});
	m_ShowLivesHUD = false;
	m_HUDCounter = 0.0f;

	// Initial Spawn Jump
	m_TotalVelocity = XMFLOAT3(0, m_CharacterDesc.ThirdJumpSpeed, 0);
	m_pAnimator->SetAnimation(L"FrontFlip");
	m_pAnimator->SetAnimationSpeed(1.2f);
	m_pAnimator->Play(true);
	m_State = Spawning;
	m_SpawnSoundPlayed = false;
}

void MarioCharacter::TogglePause(bool paused)
{
	if (paused)
	{
		if (m_State != Paused)
		{
			m_StateBeforePause = m_State;
			m_State = Paused;
			m_AnimatorSpeedBeforePause = m_pAnimator->GetAnimationSpeed();
			m_pAnimator->SetAnimationSpeed(0.0f);

			m_pWalkChannel->getPaused(&m_WalkPlayingBeforePause);
			m_pCrawlChannel->getPaused(&m_CrawlPlayingBeforePause);
			m_pCameraSpinChannel->getPaused(&m_CameraSpinPlayingBeforePause);
			m_pSFXChannel->getPaused(&m_SFXPlayingBeforePause);
			m_pWalkChannel->setPaused(true);
			m_pCrawlChannel->setPaused(true);
			m_pCameraSpinChannel->setPaused(true);
			m_pSFXChannel->setPaused(true);

			m_pPostBlur->SetIsEnabled(false);
			m_pPostRed->SetIsEnabled(false);
			m_pWalkDustEmitter->TogglePause(true);
			m_pSlamStarEmitter->TogglePause(true);
			m_pSlamDustEmitter->TogglePause(true);
		}
	}
	else
	{
		m_State = m_StateBeforePause;
		m_pAnimator->SetAnimationSpeed(m_AnimatorSpeedBeforePause);

		m_pWalkChannel->setPaused(m_WalkPlayingBeforePause);
		m_pCrawlChannel->setPaused(m_CrawlPlayingBeforePause);
		m_pCameraSpinChannel->setPaused(m_CameraSpinPlayingBeforePause);
		m_pSFXChannel->setPaused(m_SFXPlayingBeforePause);

		m_pPostBlur->SetIsEnabled(true);
		m_pPostRed->SetIsEnabled(true);
		m_pWalkDustEmitter->TogglePause(false);
		m_pSlamStarEmitter->TogglePause(false);
		m_pSlamDustEmitter->TogglePause(false);
	}
}




