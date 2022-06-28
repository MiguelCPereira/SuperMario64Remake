#include "stdafx.h"
#include "GoombaCharacter.h"

#include "MarioCharacter.h"
#include "SquishedGoomba.h"
#include "Materials/Shadow/DiffuseMaterial_Shadow.h"
#include "Scenes/Mario64/MainScene.h"

GoombaCharacter::GoombaCharacter(const GoombaDesc& characterDesc, MarioCharacter* pMario, SpawnInfo* pSpawnInfo) :
	m_GoombaDesc{ characterDesc },
	m_WanderAcceleration(characterDesc.maxWanderSpeed / characterDesc.wanderAccelerationTime),
	m_ChaseAcceleration(characterDesc.maxChaseSpeed / characterDesc.chaseAccelerationTime),
	m_FallAcceleration(characterDesc.maxFallSpeed / characterDesc.fallAccelerationTime),
	m_TargetPosition{},
	m_pMario{ pMario },
	m_pSpawnInfo{pSpawnInfo}
{
}


void GoombaCharacter::GetSquished()
{
	m_pChaseChannel->setPaused(true);
	SoundManager::Get()->GetSystem()->playSound(m_pJumpedOnSound, nullptr, false, &m_pSFXChannel);

	// Spawn squished goomba
 	auto spawnPos = GetTransform()->GetWorldPosition();
	spawnPos.y -= m_GoombaDesc.controller.height + 0.8f;
	GetScene()->AddChild(new SquishedGoomba(spawnPos));

	m_pSpawnInfo->alive = false;
	if (m_pControllerComponent != nullptr)
	{
		m_pControllerComponent->SetCollisionIgnoreGroup(CollisionGroup::Group1);
		m_pControllerComponent->Translate(XMFLOAT3(m_pControllerComponent->GetPosition().x - 999.f, -999.f, m_pControllerComponent->GetPosition().z - 999.f));
		GetScene()->GetPhysxProxy()->GetPhysxScene()->removeActor(*static_cast<PxActor*>(m_pControllerComponent->GetPxController()->getActor()));
		RemoveComponent(m_pControllerComponent, true);
		m_pControllerComponent = nullptr;
	}
	GetParent()->RemoveChild(this, true);
}

void GoombaCharacter::Initialize(const SceneContext&)
{
	//Controller
	m_pControllerComponent = AddComponent(new ControllerComponent(m_GoombaDesc.controller));


	//Character Mesh
	const auto pCharacterMaterial = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow>();
	pCharacterMaterial->SetDiffuseTexture(L"Textures/Goomba_Diffuse.png");
	m_pModelParentGO = AddChild(new GameObject);
	m_pModelGO = m_pModelParentGO->AddChild(new GameObject);
	const auto model = m_pModelGO->AddComponent(new ModelComponent(L"Meshes/Goomba/Goomba.ovm"));
	model->SetMaterial(pCharacterMaterial);
	m_pModelParentGO->GetTransform()->Scale(m_OriginalSize, m_OriginalSize, m_OriginalSize);
	m_pModelParentGO->GetTransform()->Translate(0, -m_GoombaDesc.controller.height / 2.f - 1.f, 0);


	//Sound
	auto* pSoundManagerSystem = SoundManager::Get()->GetSystem();
	pSoundManagerSystem->createStream("Resources/Sounds/goomba-run.mp3", FMOD_LOOP_NORMAL, nullptr, &m_pChaseSound);
	pSoundManagerSystem->createStream("Resources/Sounds/goomba-flattened.wav", FMOD_DEFAULT, nullptr, &m_pJumpedOnSound);
	pSoundManagerSystem->createStream("Resources/Sounds/goomba-alert.mp3", FMOD_DEFAULT, nullptr, &m_pAlertSound);
	pSoundManagerSystem->createStream("Resources/Sounds/goomba-punched.wav", FMOD_DEFAULT, nullptr, &m_pPunchedSound);
}

void GoombaCharacter::Update(const SceneContext& sceneContext)
{
	if (m_State != Paused)
	{
		const auto elapsedTime = sceneContext.pGameTime->GetElapsed();

		Update3DSounds();

		// The sound must fade out after it is updated (the Update3DSounds)
		if (m_SoundFadingOut)
			UpdateSoundFadeOut(elapsedTime);

		const XMVECTOR marioDistanceVec = XMLoadFloat3(&m_pMario->GetTransform()->GetWorldPosition()) - XMLoadFloat3(&GetTransform()->GetWorldPosition());
		XMFLOAT3 marioDistance{};
		XMStoreFloat3(&marioDistance, marioDistanceVec);
		float marioDistanceLength = 0.0f;
		XMStoreFloat(&marioDistanceLength, XMVector3Length(marioDistanceVec));

		float wanderAngle;
		float randomPerc;
		bool readyToDestroyOnLand = false;
		bool damagedForTooLong = false;

		switch (m_State)
		{
		case Wandering:
			//If Mario's close by (and not to far above/bellow), switch to chasing
			if (marioDistanceLength <= m_ChaseDistance &&
				marioDistance.y < m_ChaseMaxHeightDifference && marioDistance.y > -m_ChaseMaxHeightDifference)
			{
				SoundManager::Get()->GetSystem()->playSound(m_pAlertSound, nullptr, false, &m_pSFXChannel);
				m_TargetPosition = m_pMario->GetTransform()->GetWorldPosition();
				m_MoveSpeed = 0.0f;
				m_TotalVelocity.y = m_GoombaDesc.jumpSpeed; // Jump
				m_State = AlertJump;
			}

			// Calculate the Movement Speed
			m_MoveSpeed += m_WanderAcceleration * elapsedTime;
			if (m_MoveSpeed > m_GoombaDesc.maxWanderSpeed)
				m_MoveSpeed = m_GoombaDesc.maxWanderSpeed;

			// Calculate a random point semi-forward and set it as the target
			// Once in a while, that is
			m_WanderChangeCounter += sceneContext.pGameTime->GetElapsed();
			if (m_WanderChangeCounter > m_WanderChangeDirInterval)
			{
				m_WanderChangeCounter = 0.0f;

				// The random point is calculated by envisioning a circle in front of the goomba (at m_WanderOffset distance)
				// and getting picking a position in its circumference (within the angle limit of m_WanderAngleChange)
				XMVECTOR wanderTarget = XMLoadFloat3(&GetTransform()->GetForward()); // Get the forward vector
				wanderTarget *= m_WanderOffset; // Multiply forward vector by the offset to get the vector between the agent and the center of the wander circle
				wanderTarget += XMLoadFloat3(&GetTransform()->GetWorldPosition()); // Add the agent position to the vector to get the position of the center of the circle
				randomPerc = static_cast <float> (rand()) / static_cast <float> (RAND_MAX); // Get a float between 0 and 1
				wanderAngle = m_WanderAngleChange / 2.f + randomPerc * m_WanderAngleChange; // Define wanderAngle with the percentage and m_AngleChange
				const XMFLOAT2 displacement2DDir = XMFLOAT2(cos(wanderAngle), sin(wanderAngle));
				XMFLOAT2 displacement2D;
				XMStoreFloat2(&displacement2D, XMLoadFloat2(&displacement2DDir) * m_WanderRadius); // Calculate the vector between the circle center and the final target (with the calculated angle)
				const XMFLOAT3 displacement = XMFLOAT3(displacement2D.x, displacement2D.y, 0.f);
				XMStoreFloat3(&m_TargetPosition, wanderTarget + XMLoadFloat3(&displacement)); // Add the circle center position to the displacement vector to get the final target position
			}
			break;

		case AlertJump:
			m_JumpCounter += elapsedTime;
			if (m_JumpCounter > m_TimeToLeaveFloor && CheckIfGrounded())
			{
				// Only initializing the sound here makes it so that each goomba will play the sound at a unique offset
				SoundManager::Get()->GetSystem()->playSound(m_pChaseSound, nullptr, false, &m_pChaseChannel);
				m_JumpCounter = 0.0f;
				m_State = Chasing;
			}
			break;

		case Chasing:
			//If Mario ran away, switch to wandering
			if (marioDistanceLength > m_ChaseDistance)
			{
				m_pChaseChannel->setPaused(true);
				m_WanderChangeCounter = m_WanderChangeDirInterval;
				m_State = Wandering;
			}

			// Calculate the Movement Speed
			m_MoveSpeed += m_ChaseAcceleration * elapsedTime;
			if (m_MoveSpeed > m_GoombaDesc.maxChaseSpeed)
				m_MoveSpeed = m_GoombaDesc.maxChaseSpeed;

			// Make mario's position the target
			m_TargetPosition = m_pMario->GetTransform()->GetWorldPosition();
			break;

		case Punched:
			// Shrink model after punched
			m_pModelParentGO->GetTransform()->Scale(m_OriginalSize * (1.f - m_DamagedCounter / m_MaxDamageTime),
				m_OriginalSize * (1.f - m_DamagedCounter / m_MaxDamageTime), m_OriginalSize * (1.f - m_DamagedCounter / m_MaxDamageTime));

			// Check if the the max time period to be destroyed as passed
			m_DamagedCounter += elapsedTime;
			if (m_DamagedCounter >= m_MinDamageTime)
			{
				readyToDestroyOnLand = true;
				if (m_DamagedCounter >= m_MaxDamageTime)
					damagedForTooLong = true;
			}

			// Destroy goomba either when it lands back on the ground, or after a max time period passes
			// (The minimum time is just so that the goomba has the time to leave the ground)
			if ((readyToDestroyOnLand && CheckIfGrounded()) || damagedForTooLong)
			{
				m_DamagedCounter = 0.0f;
				m_pSpawnInfo->alive = false;
				if (m_pControllerComponent != nullptr)
				{
					m_pControllerComponent->SetCollisionIgnoreGroup(CollisionGroup::Group1);
					m_pControllerComponent->Translate(XMFLOAT3(m_pControllerComponent->GetPosition().x - 999.f, -999.f, m_pControllerComponent->GetPosition().z - 999.f));
					GetScene()->GetPhysxProxy()->GetPhysxScene()->removeActor(*static_cast<PxActor*>(m_pControllerComponent->GetPxController()->getActor()));
					RemoveComponent(m_pControllerComponent, true);
					m_pControllerComponent = nullptr;
				}
				GetParent()->RemoveChild(this, true);
				return;
			}
			break;

		default:
			break;
		}

		// Check if Stomped On, Punched or Doing Damage
		if (m_pMario->GetState() != MidDamaged && m_pMario->GetState() != Dead)
		{
			// If Mario is close enough to suffer/do any effects (punch dist > damage dist)
			if (marioDistanceLength < m_PunchDistance)
			{
				// If Mario is punching and facing the right direction, throw the goomba
				if (m_pMario->GetState() == Punching)
				{
					m_pChaseChannel->setPaused(true);
					SoundManager::Get()->GetSystem()->playSound(m_pPunchedSound, nullptr, false, &m_pSFXChannel);
					auto dmgOrigin = m_pMario->GetTransform()->GetWorldPosition();
					dmgOrigin.y -= 5.f;
					const auto throwVector = XMVector3Normalize(XMLoadFloat3(&GetTransform()->GetWorldPosition()) - XMLoadFloat3(&dmgOrigin)) * m_GoombaDesc.damagedThrowSpeed;
					XMStoreFloat3(&m_TotalVelocity, throwVector);
					m_TotalVelocity.y += 8.f;

					m_pControllerComponent->SetCollisionIgnoreGroup(CollisionGroup::Group1);

					m_State = Punched;
				}

				// If Mario is close enough to take either get damaged or stomp
				if (marioDistanceLength < m_DamageDistance)
				{
					// If Mario is jumping and above the goomba's head, stomp
					if ((m_pMario->GetState() == MidAir &&
						m_pMario->GetTransform()->GetWorldPosition().y > GetTransform()->GetWorldPosition().y + m_GoombaDesc.controller.height)
						|| m_pMario->GetState() == GroundSlamming)
					{
						m_pMario->GetLunged(XMFLOAT3(0, 15, 0));
						GetSquished();
						return;
					}

					// If none of the previous conditions are truth, just damage Mario
					m_pMario->GetDamaged(1, GetTransform()->GetWorldPosition());
				}
			}
		}


		// Apply the movement
		ApplyMovement(elapsedTime);


		//Rotate model to match direction
		RotateMesh();
	}
}

void GoombaCharacter::ApplyMovement(float elapsedTime)
{
	ApplyGravity(elapsedTime);

	if (m_State != Punched)
	{
		// Set the X & Z components of m_TotalVelocity (aka, the target seek)
		XMStoreFloat3(&m_TargetDirection, XMVector3Normalize(XMLoadFloat3(&m_TargetPosition) - XMLoadFloat3(&GetTransform()->GetWorldPosition())));
		XMFLOAT3 horizontalVelocity = m_TargetDirection;
		horizontalVelocity.x *= m_MoveSpeed;
		horizontalVelocity.z *= m_MoveSpeed;
		m_TotalVelocity.x = horizontalVelocity.x;
		m_TotalVelocity.z = horizontalVelocity.z;
	}

	// Calculate the displacement for the current frame and move the ControllerComponent
	auto displacement = m_TotalVelocity;
	displacement.x *= elapsedTime;
	displacement.y *= elapsedTime;
	displacement.z *= elapsedTime;
	m_pControllerComponent->Move(displacement);
}

void GoombaCharacter::RotateMesh()
{
	// Rotate the mesh so it faces the intended forward vector
	auto temp = XMLoadFloat3(&m_pModelParentGO->GetTransform()->GetForward());
	auto rotMatrix = XMMatrixRotationY(90.f);
	XMFLOAT3 modelDir;
	XMStoreFloat3(&modelDir, XMVector3Normalize(XMVector3Transform(temp, rotMatrix)));

	// Calculate the angle between the current forward and the target
	const auto currentDir = XMFLOAT2(modelDir.x, modelDir.z);
	const auto targetDir = XMFLOAT2(m_TargetDirection.x, m_TargetDirection.z);

	const auto dot = currentDir.x * currentDir.y + targetDir.x * targetDir.y;
	const auto det = currentDir.x * targetDir.y - targetDir.x * currentDir.y;
	auto angle = -atan2(det, dot);
	angle *= m_TurningSpeed;

	// And rotate the mesh
	m_Rotated += angle;
	if (m_Rotated > 360.0f)
		m_Rotated -= 360.0f;
	else if (m_Rotated < 0)
		m_Rotated += 360.0f;

	m_pModelParentGO->GetTransform()->Rotate(0.f, m_Rotated, 0);
	m_pModelGO->GetTransform()->Rotate(90.f, 30.f, 0);
}

bool GoombaCharacter::CheckIfGrounded() const
{
	auto origin = PxVec3(0, 0, 0);
	origin.x = m_pControllerComponent->GetTransform()->GetPosition().x; // - m_CharacterDesc.controller.height;
	origin.y = m_pControllerComponent->GetTransform()->GetPosition().y; // - m_CharacterDesc.controller.height;
	origin.z = m_pControllerComponent->GetTransform()->GetPosition().z; // - m_CharacterDesc.controller.height;
	const auto direction = PxVec3(0, -1, 0);
	const PxReal maxDistance = 2.0f;//m_CharacterDesc.controller.height;
	PxRaycastBuffer hit;
	PxQueryFilterData filterData{};
	filterData.data.word0 = UINT(CollisionGroup::Group9);

	return GetScene()->GetPhysxProxy()->Raycast(origin, direction, maxDistance, hit, PxHitFlag::eDEFAULT, filterData);
}

void GoombaCharacter::Reset()
{
	if(m_pControllerComponent == nullptr)
		m_pControllerComponent = AddComponent(new ControllerComponent(m_GoombaDesc.controller));

	m_pControllerComponent->SetCollisionIgnoreGroup(CollisionGroup::None);
	m_DamagedCounter = 0.0f;
	m_pChaseChannel->setPaused(true);
	m_pSFXChannel->setPaused(true);
	m_SoundFadeOutCounter = 0.0f;
	m_SoundFadingOut = false;
	m_MoveSpeed = 0.0f;
	m_TotalVelocity = XMFLOAT3(0, 0, 0);
	m_State = Wandering;
	m_TargetPosition = XMFLOAT3(0, 0, 0);
	m_TargetDirection = XMFLOAT3(1, 0, 0);
	m_Rotated = 0.0f;

	m_WanderChangeCounter = 0.0f;
	m_JumpCounter = 0.0f;
}

void GoombaCharacter::Update3DSounds()
{
	float marioDistance;
	XMStoreFloat(&marioDistance, XMVector3Length(XMLoadFloat3(&m_pMario->GetTransform()->GetWorldPosition()) - XMLoadFloat3(&GetTransform()->GetWorldPosition())));

	auto intendedVolume = 1.f - marioDistance / m_SoundStartDistance;

	if (intendedVolume <= 0.0f)
		intendedVolume = 0.0f;

	m_pChaseChannel->setVolume(intendedVolume);
	m_pSFXChannel->setVolume(intendedVolume);
	m_CurrentChaseVol = intendedVolume;
	m_CurrentSFXVol = intendedVolume;
}

void GoombaCharacter::ApplyGravity(float elapsedTime)
{
	// Set the Y component of m_TotalVelocity (aka, apply gravity) 
	if (CheckIfGrounded() == false)
	{
		m_TotalVelocity.y -= m_FallAcceleration * elapsedTime;
		if (m_TotalVelocity.y < -m_GoombaDesc.maxFallSpeed)
			m_TotalVelocity.y = -m_GoombaDesc.maxFallSpeed;
	}
}

void GoombaCharacter::TogglePause(bool paused)
{
	if (paused)
	{
		if (m_State != Paused)
		{
			m_StateBeforePause = m_State;
			m_State = Paused;

			m_pChaseChannel->getPaused(&m_ChasePlayingBeforePause);
			m_pChaseChannel->setPaused(true);
			m_pSFXChannel->getPaused(&m_SFXPlayingBeforePause);
			m_pSFXChannel->setPaused(true);
		}
	}
	else
	{
		m_State = m_StateBeforePause;
		m_pChaseChannel->setPaused(m_ChasePlayingBeforePause);
		m_pSFXChannel->setPaused(m_SFXPlayingBeforePause);
	}
}

void GoombaCharacter::ToggleSoundFadeOut(float fadeTime)
{
	m_SoundTotalFadeTime = fadeTime;
	m_SoundFadingOut = true;
}

void GoombaCharacter::UpdateSoundFadeOut(float elapsedTime)
{
	m_SoundFadeOutCounter += elapsedTime;
	auto currentVolPercent = 1.f - m_SoundFadeOutCounter / m_SoundTotalFadeTime;

	if (m_SoundFadeOutCounter > m_SoundTotalFadeTime)
	{
		currentVolPercent = 0.0f;
		m_SoundFadeOutCounter = 0.0f;
	}

	m_pChaseChannel->setVolume(m_CurrentChaseVol * currentVolPercent);
	m_pSFXChannel->setVolume(m_CurrentSFXVol * currentVolPercent);
}











