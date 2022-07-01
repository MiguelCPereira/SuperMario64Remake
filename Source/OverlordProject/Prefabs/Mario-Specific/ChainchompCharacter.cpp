#include "stdafx.h"
#include "ChainchompCharacter.h"
#include "MarioCharacter.h"
#include "Materials/Shadow/DiffuseMaterial_Shadow.h"
#include "Prefabs/SpherePrefab.h"
#include "Scenes/Mario64/MainScene.h"


ChainchompCharacter::ChainchompCharacter(const ChainchompDesc& characterDesc, MarioCharacter* pMario, const XMFLOAT3& spawnPos) :
	m_ChainchompDesc{ characterDesc },
	m_LungeAcceleration(characterDesc.maxLungeSpeed / characterDesc.lungeAccelerationTime),
	m_RecoilAcceleration(characterDesc.maxRecoilSpeed / characterDesc.recoilAccelerationTime),
	m_FallAcceleration(characterDesc.maxFallSpeed / characterDesc.fallAccelerationTime),
	m_TargetDirection{},
	m_TargetDistance{},
	m_pMario{ pMario },
	m_SpawnPosition{ spawnPos }
{
}

void ChainchompCharacter::Initialize(const SceneContext&)
{
	//Controller
	m_pControllerComponent = AddComponent(new ControllerComponent(m_ChainchompDesc.controller));

	//Character Mesh
	const auto pCharacterMaterial = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow>();
	pCharacterMaterial->SetDiffuseTexture(L"Textures/Chainchomp_Diffuse.png");
	auto pModelGO = AddChild(new GameObject);
	const auto pModel = pModelGO->AddComponent(new ModelComponent(L"Meshes/Chainchomp/Chainchomp.ovm"));
	pModel->SetMaterial(pCharacterMaterial);
	pModelGO->GetTransform()->Scale(0.14f * 1.5f, 0.14f * 1.5f, 0.14f * 1.5f);
	pModelGO->GetTransform()->Rotate(90, -45, 0);

	//Sound
	auto* pSoundManagerSystem = SoundManager::Get()->GetSystem();
	pSoundManagerSystem->createStream("Resources/Sounds/chainchomp-chain.wav", FMOD_DEFAULT, nullptr, &m_pChainSound);
	pSoundManagerSystem->createStream("Resources/Sounds/chainchomp-lunge.wav", FMOD_DEFAULT, nullptr, &m_pLungingSound);
}

void ChainchompCharacter::Update(const SceneContext& sceneContext)
{
	if (m_State != Paused)
	{
		const auto elapsedTime = sceneContext.pGameTime->GetElapsed();

		const XMVECTOR marioDistanceVec = XMLoadFloat3(&m_pMario->GetTransform()->GetWorldPosition()) - XMLoadFloat3(&GetTransform()->GetWorldPosition());
		XMFLOAT3 marioDistance{};
		XMStoreFloat3(&marioDistance, marioDistanceVec);
		float marioDistanceLength = 0.0f;
		XMStoreFloat(&marioDistanceLength, XMVector3Length(marioDistanceVec));

		Update3DSound(marioDistanceLength);

		// The sound must fade out after it is updated (the Update3DSounds)
		if (m_SoundFadingOut)
			UpdateSoundFadeOut(elapsedTime);



		float toCenterLength{};
		XMFLOAT2 toCenterDirection{};
		switch (m_State)
		{
		case Idle:
			// Start rotation (lining up an attack) as soon as Mario gets close enough
			if (marioDistanceLength <= m_ActivationDistance)
			{
				// Do the first warning hop
				//// Set the TotalVelocity's Y
				m_TotalVelocity.y = m_ChainchompDesc.hopSpeed;
				//// Play the SFX
				m_pChainChannel->setPosition(0, FMOD_TIMEUNIT_MS);
				SoundManager::Get()->GetSystem()->playSound(m_pChainSound, nullptr, false, &m_pChainChannel);
				m_PauseBetweenHopsCounter = 0.f;

				m_State = Rotating;
				break;
			}

			if (CheckIfGrounded() == false)
				ApplyGravity(elapsedTime);
			else
				m_TotalVelocity.y = 0;

			break;


		case Rotating:
			// If Mario gets out of range, go back to idle
			if (marioDistanceLength > m_ActivationDistance)
			{
				m_State = Idle;
				break;
			}

			// If facing Mario (or max time runs out), start lunging
			m_RotationTimeCounter += elapsedTime;
			if (m_RotationTimeCounter >= m_MaxRotationTime)
			{
				m_pLungingChannel->setPosition(0, FMOD_TIMEUNIT_MS);
				SoundManager::Get()->GetSystem()->playSound(m_pLungingSound, nullptr, false, &m_pLungingChannel);
				m_RotationTimeCounter = 0.f;
				auto targetPosition = m_pMario->GetTransform()->GetWorldPosition();

				// Make sure the Chainchomp's target is adjusted so it doesn't clip through the ground
				// (since Mario's model is smaller than Chainchomp, it's center, and therefore world position,
				// will be lower than chainchomp even while both are idle in the same surface)
				if (targetPosition.y < GetTransform()->GetWorldPosition().y)
					targetPosition.y = GetTransform()->GetWorldPosition().y;

				auto targetDirectionVec = XMLoadFloat3(&targetPosition) - XMLoadFloat3(&GetTransform()->GetWorldPosition());
				XMStoreFloat(&m_TargetDistance, XMVector3Length(targetDirectionVec));
				XMStoreFloat3(&m_TargetDirection, XMVector3Normalize(targetDirectionVec));
				m_State = Lunging;
				break;
			}

			if (CheckIfGrounded() == false)
				ApplyGravity(elapsedTime);
			else
				m_TotalVelocity.y = 0;

			// Hop in place center
			m_PauseBetweenHopsCounter += elapsedTime;
			if (m_PauseBetweenHopsCounter >= m_PauseBetweenHopsRot)
			{
				// Set the TotalVelocity's Y
				m_TotalVelocity.y = m_ChainchompDesc.hopSpeed;

				// Play the SFX
				m_pChainChannel->setPosition(0, FMOD_TIMEUNIT_MS);
				SoundManager::Get()->GetSystem()->playSound(m_pChainSound, nullptr, false, &m_pChainChannel);
				m_PauseBetweenHopsCounter = 0.f;
			}

			// Rotate as to face Mario
			RotateToFaceTarget(m_pMario->GetTransform()->GetPosition());

			break;


		case Lunging:
			m_MoveSpeed += m_LungeAcceleration * elapsedTime;
			if (m_MoveSpeed > m_ChainchompDesc.maxLungeSpeed)
				m_MoveSpeed = m_ChainchompDesc.maxLungeSpeed;

			// Update distance lunged
			m_DistanceLunged += m_MoveSpeed * elapsedTime;

			// Start recoiling once the target is reached
			if (m_DistanceLunged >= m_TargetDistance - m_LungeTargetErrorMargin)
			{
				m_TotalVelocity = { 0,0,0 };
				m_DistanceLunged = 0.f;
				m_MoveSpeed = 0.f;
				m_State = Recoiling;
				break;
			}

			// Calculate the velocity towards the lunge target
			const auto velocityVec = XMLoadFloat3(&m_TargetDirection) * m_MoveSpeed;
			XMStoreFloat3(&m_TotalVelocity, velocityVec);

			// Rotate as to face Mario
			RotateToFaceTarget(m_pMario->GetTransform()->GetPosition());

			break;


		case Recoiling:
			// If close enough to the center, change behaviour
			const XMFLOAT2 toCenterHorizDist{ m_SpawnPosition.x - GetTransform()->GetWorldPosition().x,
				m_SpawnPosition.z - GetTransform()->GetWorldPosition().z };
			XMStoreFloat(&toCenterLength, XMVector2Length(XMLoadFloat2(&toCenterHorizDist)));
			XMStoreFloat2(&toCenterDirection, XMVector2Normalize(XMLoadFloat2(&toCenterHorizDist)));
			if (toCenterLength < m_RecoilMinCenterOffset)
			{
				m_TotalVelocity.x = 0.0f;
				m_TotalVelocity.z = 0.0f;
				// Change to Idle if Mario is out of range
				if (marioDistanceLength > m_ActivationDistance)
				{
					m_State = Idle;
					break;
				}

				// Or change to rotating if Mario's still in range
				m_State = Rotating;
				break;
			}

			if (CheckIfGrounded() == false)
				ApplyGravity(elapsedTime);
			else
				m_TotalVelocity.y = 0;

			// Hop back to center
			//// If enough time has passed, hop
			m_PauseBetweenHopsCounter += elapsedTime;
			if (m_PauseBetweenHopsCounter >= m_PauseBetweenHopsRecoil)
			{
				// Set the Y of TotalVelocity and the MoveSpeed (making the speed in all directions spike each jump start)
				m_TotalVelocity.y = m_ChainchompDesc.hopSpeed * 2.f;
				m_MoveSpeed = m_ChainchompDesc.hopSpeed;

				// Play the SFX
				m_pChainChannel->setPosition(0, FMOD_TIMEUNIT_MS);
				SoundManager::Get()->GetSystem()->playSound(m_pChainSound, nullptr, false, &m_pChainChannel);
				m_PauseBetweenHopsCounter = 0.f;
			}

			//// Calculate the velocity towards the center and use it to set X and Z of TotalVelocity
			const auto velocityToCenterVec = XMLoadFloat2(&toCenterDirection) * m_MoveSpeed;
			XMFLOAT2 velocityToCenter{};
			XMStoreFloat2(&velocityToCenter, velocityToCenterVec);
			m_TotalVelocity.x = velocityToCenter.x;
			m_TotalVelocity.z = velocityToCenter.y;

			// Reduce the speed progressively (as to slow down after every jump)
			m_MoveSpeed -= m_ChainchompDesc.hopSpeed * 1.5f * elapsedTime;
			if (m_MoveSpeed < 0.0f)
				m_MoveSpeed = 0.0f;

			// Rotate as to face center
			RotateToFaceTarget(m_SpawnPosition);

			break;


		default:
			break;
		}

		// Check if Damage
		if (m_pMario->GetState() != MidDamaged && m_pMario->GetState() != Dead)
		{
			if (marioDistanceLength < m_DamageDistance)
				m_pMario->GetDamaged(3, GetTransform()->GetWorldPosition());
		}

		ApplyMovement(elapsedTime);
	}
}


bool ChainchompCharacter::CheckIfGrounded()
{
	auto origin = PxVec3(0, 0, 0);
	origin.x = m_pControllerComponent->GetTransform()->GetPosition().x;
	origin.y = m_pControllerComponent->GetTransform()->GetPosition().y;
	origin.z = m_pControllerComponent->GetTransform()->GetPosition().z;
	const auto direction = PxVec3(0, -1, 0);
	const PxReal maxDistance = m_ChainchompDesc.controller.height;
	PxRaycastBuffer hit;
	PxQueryFilterData filterData{};
	filterData.data.word0 = UINT(CollisionGroup::Group9);

	if (GetScene()->GetPhysxProxy()->Raycast(origin, direction, maxDistance, hit, PxHitFlag::eDEFAULT, filterData))
	{
		return true;
	}

	return false;
}

void ChainchompCharacter::Update3DSound(float marioDistanceLength)
{
	auto intendedVolume = 1.f - marioDistanceLength / m_SoundStartDistance;

	if (intendedVolume <= 0.0f)
	{
		intendedVolume = 0.0f;
	}

	m_CurrentVolume = intendedVolume * m_VolumeMultiplier;

	m_pChainChannel->setVolume(m_CurrentVolume);
	m_pLungingChannel->setVolume(m_CurrentVolume);
}

void ChainchompCharacter::Reset()
{
	m_MoveSpeed = 0.0f;
	m_TotalVelocity = XMFLOAT3(0, 0, 0);
	m_pControllerComponent->Translate(m_SpawnPosition);

	m_State = Idle;
	m_StateBeforePause = Idle;

	m_RotationTimeCounter = 0.f;
	m_PauseBetweenHopsCounter = 0.f;
	m_DistanceLunged = 0.f;
	m_Rotated = 0.0f;
	GetTransform()->Rotate(0.f, 0, 0);

	m_pChainChannel->setPaused(true);
	m_pLungingChannel->setPaused(true);
	m_SoundFadeOutCounter = 0.0f;
	m_SoundFadingOut = false;
}

void ChainchompCharacter::TogglePause(bool paused)
{
	if (paused)
	{
		if (m_State != Paused)
		{
			m_StateBeforePause = m_State;
			m_State = Paused;

			m_pChainChannel->getPaused(&m_ChainPlayingBeforePause);
			m_pChainChannel->setPaused(true);
			m_pLungingChannel->getPaused(&m_LungingPlayingBeforePause);
			m_pLungingChannel->setPaused(true);
		}
	}
	else
	{
		m_State = m_StateBeforePause;
		m_pChainChannel->setPaused(m_ChainPlayingBeforePause);
		m_pLungingChannel->setPaused(m_LungingPlayingBeforePause);
	}
}

void ChainchompCharacter::ToggleSoundFadeOut(float fadeTime)
{
	m_SoundTotalFadeTime = fadeTime;
	m_SoundFadingOut = true;
}

void ChainchompCharacter::UpdateSoundFadeOut(float elapsedTime)
{
	m_SoundFadeOutCounter += elapsedTime;
	auto currentVolPercent = 1.f - m_SoundFadeOutCounter / m_SoundTotalFadeTime;

	if (m_SoundFadeOutCounter > m_SoundTotalFadeTime)
	{
		currentVolPercent = 0.0f;
		m_SoundFadeOutCounter = 0.0f;
	}

	m_pChainChannel->setVolume(m_CurrentVolume * currentVolPercent);
	m_pLungingChannel->setVolume(m_CurrentVolume * currentVolPercent);
}

void ChainchompCharacter::ApplyGravity(float elapsedTime)
{
	m_TotalVelocity.y -= m_FallAcceleration * elapsedTime;
	if (m_TotalVelocity.y < -m_ChainchompDesc.maxFallSpeed)
		m_TotalVelocity.y = -m_ChainchompDesc.maxFallSpeed;
}

void ChainchompCharacter::ApplyMovement(float elapsedTime)
{
	// Set the displacement required to move the Character Controller
	auto displacement = m_TotalVelocity;
	displacement.x *= elapsedTime;
	displacement.y *= elapsedTime;
	displacement.z *= elapsedTime;
	m_pControllerComponent->Move(displacement);
}

void ChainchompCharacter::RotateToFaceTarget(const XMFLOAT3& targetPos)
{
	// Get the current forward vector
	const auto modelDirVec = XMVector3Normalize(XMVector3Transform(
		XMLoadFloat3(&GetTransform()->GetForward()), XMMatrixRotationY(45.f)));
	XMFLOAT3 modelDir;
	XMStoreFloat3(&modelDir, modelDirVec);

	// Calculate the angle between the current forward and the target
	const auto currentDir = XMFLOAT2(modelDir.x, modelDir.z);
	const XMFLOAT2 targetPos2D{ targetPos.x, targetPos.z };
	const XMFLOAT2 currentPos2D{ GetTransform()->GetWorldPosition().x, GetTransform()->GetWorldPosition().z };
	const XMVECTOR targetDirVec = XMVector2Normalize(XMLoadFloat2(&targetPos2D) - XMLoadFloat2(&currentPos2D));
	XMFLOAT2 targetDir{};
	XMStoreFloat2(&targetDir, targetDirVec);

	const auto dot = currentDir.x * targetDir.x + currentDir.x * targetDir.y;
	const auto det = currentDir.x * targetDir.y - currentDir.y * targetDir.x;
	auto angle = -atan2(det, dot);
	angle *= 0.5f;

	// And rotate the mesh
	m_Rotated += angle;
	if (m_Rotated > 360.0f)
		m_Rotated -= 360.0f;
	else if (m_Rotated < 0)
		m_Rotated += 360.0f;

	GetTransform()->Rotate(0.f, m_Rotated, 0);
}

