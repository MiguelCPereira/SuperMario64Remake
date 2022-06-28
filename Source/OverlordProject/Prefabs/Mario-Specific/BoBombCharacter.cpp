#include "stdafx.h"
#include "BoBombCharacter.h"

#include "ExplosionAnimation.h"
#include "MarioCharacter.h"
#include "Materials/Shadow/DiffuseMaterial_Shadow.h"
#include "Scenes/Mario64/MainScene.h"

BoBombCharacter::BoBombCharacter(const BoBombDesc& characterDesc, MarioCharacter* pMario, SpawnInfo* pSpawnInfo) :
	m_BoBombDesc{ characterDesc },
	m_WanderAcceleration(characterDesc.maxWanderSpeed / characterDesc.wanderAccelerationTime),
	m_ChaseAcceleration(characterDesc.maxChaseSpeed / characterDesc.chaseAccelerationTime),
	m_FallAcceleration(characterDesc.maxFallSpeed / characterDesc.fallAccelerationTime),
	m_TargetPosition{},
	m_pMario{ pMario },
	m_pSpawnInfo(pSpawnInfo)
{
}

void BoBombCharacter::BlowUp()
{
	const XMVECTOR marioDistanceVec = XMVector3Length(XMLoadFloat3(&m_pMario->GetTransform()->GetWorldPosition()) - XMLoadFloat3(&GetTransform()->GetWorldPosition()));
	float marioDistance = 0.0f;
	XMStoreFloat(&marioDistance, marioDistanceVec);

	m_pMario->m_GrabbingBobOmb = false;

	// Damage Mario
	if (m_pMario->GetState() != MidDamaged && m_pMario->GetState() != Dead)
	{
		if (marioDistance < m_BlowUpRadius)
			m_pMario->GetDamaged(2, GetTransform()->GetWorldPosition());
	}

	//Play the SFX
	SoundManager::Get()->GetSystem()->playSound(m_pExplodeSound, nullptr, false, &m_pExplodeChannel);
	m_pChaseChannel->setPaused(true);
	m_pFumeChannel->setPaused(true);

	// Spawn explosion
	GetScene()->AddChild(new ExplosionAnimation(GetTransform()->GetWorldPosition()));

	// Delete itself;
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


void BoBombCharacter::Initialize(const SceneContext&)
{
	//Controller
	m_pControllerComponent = AddComponent(new ControllerComponent(m_BoBombDesc.controller));

	//Character Mesh
	const auto pCharacterMaterial = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow>();
	pCharacterMaterial->SetDiffuseTexture(L"Textures/BobOmb_Diffuse.png");
	m_pModelParentGO = AddChild(new GameObject);
	m_pModelGO = m_pModelParentGO->AddChild(new GameObject);
	const auto pModel = m_pModelGO->AddComponent(new ModelComponent(L"Meshes/BobOmb/BobOmb.ovm"));
	pModel->SetMaterial(pCharacterMaterial);
	m_pModelParentGO->GetTransform()->Scale(0.06f, 0.06f, 0.06f);
	m_pModelParentGO->GetTransform()->Translate(0, -m_BoBombDesc.controller.height - 0.5f, 0);

	//Sound
	auto* pSoundManagerSystem = SoundManager::Get()->GetSystem();
	pSoundManagerSystem->createStream("Resources/Sounds/bobomb-run.WAV", FMOD_LOOP_NORMAL, nullptr, &m_pChaseSound);
	pSoundManagerSystem->createStream("Resources/Sounds/bobomb-fuse-loop.WAV", FMOD_LOOP_NORMAL, nullptr, &m_pFumeSound);
	pSoundManagerSystem->createStream("Resources/Sounds/bobomb-explosion.WAV", FMOD_DEFAULT, nullptr, &m_pExplodeSound);

	// Particle Settings
	m_pSmokeParticleSettings.velocity = { 0.f,6.f,0.f };
	m_pSmokeParticleSettings.minSize = 1.f;
	m_pSmokeParticleSettings.maxSize = 2.f;
	m_pSmokeParticleSettings.minEnergy = .3f;
	m_pSmokeParticleSettings.maxEnergy = .8f;
	m_pSmokeParticleSettings.minScale = 0.2f;
	m_pSmokeParticleSettings.maxScale = 0.5f;
	m_pSmokeParticleSettings.minEmitterRadius = .2f;
	m_pSmokeParticleSettings.maxEmitterRadius = .3f;
	m_pSmokeParticleSettings.color = { 1.f,1.f,1.f, .6f };

}

void BoBombCharacter::Update(const SceneContext& sceneContext)
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

		switch (m_State)
		{
		case Wandering:
			// If picked up
			if (marioDistanceLength <= m_PickedUpDistance && m_pMario->GetState() == Punching)
			{
				// Play the SFX
				SoundManager::Get()->GetSystem()->playSound(m_pFumeSound, nullptr, false, &m_pFumeChannel);

				// Ignore collisions with Mario
				m_pControllerComponent->SetCollisionIgnoreGroup(CollisionGroup::Group1);

				// Inform Mario (for animation purposes)
				m_pMario->m_GrabbingBobOmb = true;

				m_StoppedPickUpPunch = false;
				m_pChaseChannel->setPaused(true);
				m_State = PickedUp;
				break;
			}

			//If Mario's close by (and not to far above/bellow), switch to chasing
			if (marioDistanceLength <= m_LitFuseDistance &&
				marioDistance.y < m_LitFuseMaxHeightDifference && marioDistance.y > -m_LitFuseMaxHeightDifference)
			{
				// Play the SFX
				// Only initializing the sounds here makes it so that each bobomb will play them at a unique offset
				SoundManager::Get()->GetSystem()->playSound(m_pChaseSound, nullptr, false, &m_pChaseChannel);
				SoundManager::Get()->GetSystem()->playSound(m_pFumeSound, nullptr, false, &m_pFumeChannel);
				m_State = Chasing;
			}

			// Calculate the Movement Speed
			m_MoveSpeed += m_WanderAcceleration * elapsedTime;
			if (m_MoveSpeed > m_BoBombDesc.maxWanderSpeed)
				m_MoveSpeed = m_BoBombDesc.maxWanderSpeed;

			// Calculate a random point semi-forward and set it as the target
			// Once in a while, that is
			m_WanderChangeCounter += sceneContext.pGameTime->GetElapsed();
			if (m_WanderChangeCounter > m_WanderChangeDirInterval)
			{
				m_WanderChangeCounter = 0.0f;

				// The random point is calculated by envisioning a circle in front of the bobomb (at m_WanderOffset distance)
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

			// Apply the movement
			ApplyMovement(elapsedTime);

			//Rotate model to match direction
			RotateMesh();

			break;

		case Chasing:
			if (UpdateLitFuseCounter(elapsedTime))
			{
				BlowUp();
				return;
			}

			// If picked up
			if (marioDistanceLength <= m_PickedUpDistance && m_pMario->GetState() == Punching)
			{
				// Make the controller component ignore collisions with Mario
				m_pControllerComponent->SetCollisionIgnoreGroup(CollisionGroup::Group1);

				// Inform Mario (for animation purposes)
				m_pMario->m_GrabbingBobOmb = true;

				m_StoppedPickUpPunch = false;
				m_pChaseChannel->setPaused(true);
				m_State = PickedUp;
				break;
			}

			// Calculate the Movement Speed
			m_MoveSpeed += m_ChaseAcceleration * elapsedTime;
			if (m_MoveSpeed > m_BoBombDesc.maxChaseSpeed)
				m_MoveSpeed = m_BoBombDesc.maxChaseSpeed;

			// Make mario's position the target
			m_TargetPosition = m_pMario->GetTransform()->GetWorldPosition();

			// Create Smoke Particle Emitter, if needed
			if (m_pSmokeEmitter == nullptr)
			{
				const auto pObject = AddChild(new GameObject);
				m_pSmokeEmitter = pObject->AddComponent(new ParticleEmitterComponent(L"Textures/BobOmbSmoke.png", m_pSmokeParticleSettings, 200));
			}

			// Update Emitter Position
			m_pSmokeEmitter->GetTransform()->Translate(GetTransform()->GetWorldPosition().x, GetTransform()->GetWorldPosition().y + 1.8f,
				GetTransform()->GetWorldPosition().z);

			// Apply the movement
			ApplyMovement(elapsedTime);

			//Rotate model to match direction
			RotateMesh();

			break;

		case PickedUp:
			if (UpdateLitFuseCounter(elapsedTime))
			{
				BlowUp();
				return;
			}

			// If Mario's punching, and it isn't the same punch he used to pick up, throw
			if (m_pMario->GetState() != Punching)
			{
				m_StoppedPickUpPunch = true;
			}
			else if (m_StoppedPickUpPunch)
			{
				// Inform Mario (for animation purposes)
				m_pMario->m_GrabbingBobOmb = false;

				m_StoppedPickUpPunch = false;
				auto throwOrigin = m_pMario->GetTransform()->GetWorldPosition();
				throwOrigin.y -= 5.f;
				const auto throwVector = XMVector3Normalize(XMLoadFloat3(&GetTransform()->GetWorldPosition()) - XMLoadFloat3(&throwOrigin)) * m_BoBombDesc.throwSpeed;
				XMStoreFloat3(&m_TotalVelocity, throwVector);
				m_TotalVelocity.y += 8.f;
				m_ThrownForceXPositive = m_TotalVelocity.x > 0.0f;
				m_ThrownForceZPositive = m_TotalVelocity.z > 0.0f;
				m_State = Thrown;
				break;
			}

			StickToMario();

			// Create Smoke Particle Emitter, if needed (in case the bobomb didn't chase Mario before)
			if (m_pSmokeEmitter == nullptr)
			{
				const auto pObject = AddChild(new GameObject);
				m_pSmokeEmitter = pObject->AddComponent(new ParticleEmitterComponent(L"Textures/BobOmbSmoke.png", m_pSmokeParticleSettings, 200));
			}

			// Update Emitter Position
			m_pSmokeEmitter->GetTransform()->Translate(GetTransform()->GetWorldPosition().x, GetTransform()->GetWorldPosition().y + 1.8f,
				GetTransform()->GetWorldPosition().z);

			break;

		case Thrown:
			if (UpdateLitFuseCounter(elapsedTime))
			{
				BlowUp();
				return;
			}

			//	Apply gravity
			m_TotalVelocity.y -= m_FallAcceleration * elapsedTime;
			if (m_TotalVelocity.y < -m_BoBombDesc.maxFallSpeed)
				m_TotalVelocity.y = -m_BoBombDesc.maxFallSpeed;

			// Decrease the velocity with time
			DecreaseThrownVelocity(elapsedTime);

			// Calculate the displacement for the current frame and move the ControllerComponent
			auto displacement = m_TotalVelocity;
			displacement.x *= elapsedTime;
			displacement.y *= elapsedTime;
			displacement.z *= elapsedTime;
			m_pControllerComponent->Move(displacement);

			// Update Emitter Position
			m_pSmokeEmitter->GetTransform()->Translate(GetTransform()->GetWorldPosition().x, GetTransform()->GetWorldPosition().y + 1.8f,
				GetTransform()->GetWorldPosition().z);

			break;

		default:
			break;
		}
	}
}

void BoBombCharacter::ApplyMovement(float elapsedTime)
{
	// Set the Y component of m_TotalVelocity (aka, apply gravity) 
	if (CheckIfGrounded() == false)
	{
		m_TotalVelocity.y -= m_FallAcceleration * elapsedTime;
		if (m_TotalVelocity.y < -m_BoBombDesc.maxFallSpeed)
			m_TotalVelocity.y = -m_BoBombDesc.maxFallSpeed;
	}

	// Set the X & Z components of m_TotalVelocity (aka, the target seek)
	XMStoreFloat3(&m_TargetDirection, XMVector3Normalize(XMLoadFloat3(&m_TargetPosition) - XMLoadFloat3(&GetTransform()->GetWorldPosition())));
	XMFLOAT3 horizontalVelocity = m_TargetDirection;
	horizontalVelocity.x *= m_MoveSpeed;
	horizontalVelocity.z *= m_MoveSpeed;
	m_TotalVelocity.x = horizontalVelocity.x;
	m_TotalVelocity.z = horizontalVelocity.z;

	// Calculate the displacement for the current frame and move the ControllerComponent
	auto displacement = m_TotalVelocity;
	displacement.x *= elapsedTime;
	displacement.y *= elapsedTime;
	displacement.z *= elapsedTime;
	m_pControllerComponent->Move(displacement);
}

void BoBombCharacter::RotateMesh()
{
	// Rotate the mesh so it faces the intended forward vector
	auto temp = XMLoadFloat3(&m_pModelParentGO->GetTransform()->GetForward());
	auto rotMatrix = XMMatrixRotationY(90.f);
	XMFLOAT3 modelDir;
	XMStoreFloat3(&modelDir, XMVector3Transform(temp, rotMatrix));

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
	m_pModelGO->GetTransform()->Rotate(0.f, 30.f, 0);
}

void BoBombCharacter::StickToMario()
{
	m_MoveSpeed = 0.0f;
	m_TotalVelocity = XMFLOAT3(0, 0, 0);
	m_TargetPosition = XMFLOAT3(0, 0, 0);
	m_TargetDirection = XMFLOAT3(0, 0, 0);

	const auto finalPosVec = XMLoadFloat3(&m_pMario->GetTransform()->GetWorldPosition()) +
		XMVector3Normalize(XMLoadFloat3(&m_pMario->GetTransform()->GetForward())) * m_PickedUpMarioOffset;
	XMFLOAT3 finalPos;
	XMStoreFloat3(&finalPos, finalPosVec);
	m_pControllerComponent->Translate(finalPos);
}

void BoBombCharacter::DecreaseThrownVelocity(float elapsedTime)
{
	if (m_ThrownForceXPositive)
	{
		m_TotalVelocity.x -= m_ChaseAcceleration * elapsedTime;
		if (m_TotalVelocity.x < 0.0f)
			m_TotalVelocity.x = 0.0f;
	}
	else
	{
		m_TotalVelocity.x += m_ChaseAcceleration * elapsedTime;
		if (m_TotalVelocity.x > 0.0f)
			m_TotalVelocity.x = 0.0f;
	}

	if (m_ThrownForceZPositive)
	{
		m_TotalVelocity.z -= m_ChaseAcceleration * elapsedTime;
		if (m_TotalVelocity.z < 0.0f)
			m_TotalVelocity.z = 0.0f;
	}
	else
	{
		m_TotalVelocity.z += m_ChaseAcceleration * elapsedTime;
		if (m_TotalVelocity.z > 0.0f)
			m_TotalVelocity.z = 0.0f;
	}
}

bool BoBombCharacter::UpdateLitFuseCounter(float elapsedTime)
{
	m_LitFuseCounter += elapsedTime;
	return m_LitFuseCounter > m_LitFuseTime;
}


bool BoBombCharacter::CheckIfGrounded() const
{
	auto origin = PxVec3(0, 0, 0);
	origin.x = m_pControllerComponent->GetTransform()->GetPosition().x; // - m_CharacterDesc.controller.height;
	origin.y = m_pControllerComponent->GetTransform()->GetPosition().y; // - m_CharacterDesc.controller.height;
	origin.z = m_pControllerComponent->GetTransform()->GetPosition().z; // - m_CharacterDesc.controller.height;
	const auto direction = PxVec3(0, -1, 0);
	const PxReal maxDistance = 0.12f;//m_CharacterDesc.controller.height;
	PxRaycastBuffer hit;
	PxQueryFilterData filterData{};
	filterData.data.word0 = UINT(CollisionGroup::Group9);

	return GetScene()->GetPhysxProxy()->Raycast(origin, direction, maxDistance, hit, PxHitFlag::eDEFAULT, filterData);
}


void BoBombCharacter::Reset()
{
	m_pChaseChannel->setPaused(true);
	m_pFumeChannel->setPaused(true);
	m_pExplodeChannel->setPaused(true);
	m_SoundFadeOutCounter = 0.0f;
	m_SoundFadingOut = false;

	m_MoveSpeed = 0.0f;
	m_TotalVelocity = XMFLOAT3(0, 0, 0);
	m_State = Wandering;

	m_TargetPosition = XMFLOAT3(0, 0, 0);
	m_TargetDirection = XMFLOAT3(1, 0, 0);
	m_Rotated = 0.0f;

	if(m_pMario != nullptr)
		m_pMario->m_GrabbingBobOmb = false;

	if(m_pControllerComponent == nullptr)
		m_pControllerComponent = AddComponent(new ControllerComponent(m_BoBombDesc.controller));

	m_pControllerComponent->SetCollisionIgnoreGroup(CollisionGroup::None);

	m_WanderChangeCounter = 0.0f;
	m_LitFuseCounter = 0.0f;
	m_StoppedPickUpPunch = false;
}

void BoBombCharacter::Update3DSounds()
{
	float marioDistance;
	XMStoreFloat(&marioDistance, XMVector3Length(XMLoadFloat3(&m_pMario->GetTransform()->GetWorldPosition()) - XMLoadFloat3(&GetTransform()->GetWorldPosition())));

	auto intendedVolume = 1.f - marioDistance / m_SoundStartDistance;

	if (intendedVolume <= 0.0f)
		intendedVolume = 0.0f;
	
	m_pChaseChannel->setVolume(intendedVolume);
	m_pFumeChannel->setVolume(intendedVolume);
	m_pExplodeChannel->setVolume(intendedVolume);
	m_CurrentChaseVol = intendedVolume;
	m_CurrentFumeVol = intendedVolume;
	m_CurrentExplodeVol = intendedVolume;
}

void BoBombCharacter::TogglePause(bool paused)
{
	if (paused)
	{
		if (m_State != Paused)
		{
			m_StateBeforePause = m_State;
			m_State = Paused;

			m_pChaseChannel->getPaused(&m_ChasePlayingBeforePause);
			m_pChaseChannel->setPaused(true);
			m_pFumeChannel->getPaused(&m_FumePlayingBeforePause);
			m_pFumeChannel->setPaused(true);
			m_pExplodeChannel->getPaused(&m_ExplodePlayingBeforePause);
			m_pExplodeChannel->setPaused(true);

			if(m_pSmokeEmitter != nullptr)
				m_pSmokeEmitter->TogglePause(true);
		}
	}
	else
	{
		m_State = m_StateBeforePause;
		m_pChaseChannel->setPaused(m_ChasePlayingBeforePause);
		m_pFumeChannel->setPaused(m_FumePlayingBeforePause);
		m_pExplodeChannel->setPaused(m_ExplodePlayingBeforePause);

		if(m_pSmokeEmitter != nullptr)
			m_pSmokeEmitter->TogglePause(false);
	}
}

void BoBombCharacter::ToggleSoundFadeOut(float fadeTime)
{
	m_SoundTotalFadeTime = fadeTime;
	m_SoundFadingOut = true;
}

void BoBombCharacter::UpdateSoundFadeOut(float elapsedTime)
{
	m_SoundFadeOutCounter += elapsedTime;
	auto currentVolPercent = 1.f - m_SoundFadeOutCounter / m_SoundTotalFadeTime;

	if (m_SoundFadeOutCounter > m_SoundTotalFadeTime)
	{
		currentVolPercent = 0.0f;
		m_SoundFadeOutCounter = 0.0f;
	}

	m_pChaseChannel->setVolume(m_CurrentChaseVol * currentVolPercent);
	m_pFumeChannel->setVolume(m_CurrentFumeVol * currentVolPercent);
	m_pExplodeChannel->setVolume(m_CurrentExplodeVol * currentVolPercent);
}


