#include "stdafx.h"
#include "CannonballCharacter.h"

#include "MarioCharacter.h"
#include "Materials/Shadow/DiffuseMaterial_Shadow.h"
#include "Prefabs/SpherePrefab.h"
#include "Scenes/Mario64/MainScene.h"

CannonballCharacter::CannonballCharacter(const CannonballDesc& characterDesc, MarioCharacter* pMario, CannonballSpawnInfo* pSpawnInfo)
	: m_CannonballDesc { characterDesc }
	, m_MoveAcceleration(characterDesc.maxMoveSpeed / characterDesc.moveAccelerationTime)
	, m_FallAcceleration(characterDesc.maxFallSpeed / characterDesc.fallAccelerationTime)
	, m_pMario{ pMario }
	, m_pSpawnInfo{ pSpawnInfo }
{
}

void CannonballCharacter::Initialize(const SceneContext&)
{
	m_TargetPositions = m_pSpawnInfo->targetPositions;

	//Controller
	m_pControllerComponent = AddComponent(new ControllerComponent(m_CannonballDesc.controller));

	//Character Mesh
	//const auto pCharacterMaterial = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow>();
	auto pBall = AddChild(new SpherePrefab(0.5f, 10, XMFLOAT4{ Colors::Black }));
	pBall->GetTransform()->Scale(6.3984f, 6.3984f, 6.3984f);

	//Sound
	SoundManager::Get()->GetSystem()->createStream("Resources/Sounds/cannonball-moving.wav", FMOD_LOOP_NORMAL, nullptr, &m_pMovingSound);
}

void CannonballCharacter::Update(const SceneContext& sceneContext)
{
	if (m_Paused == false)
	{
		const auto elapsedTime = sceneContext.pGameTime->GetElapsed();

		const XMVECTOR marioDistanceVec = XMVector3Length(XMLoadFloat3(&m_pMario->GetTransform()->GetWorldPosition()) - XMLoadFloat3(&GetTransform()->GetWorldPosition()));
		float marioDistance = 0.0f;
		XMStoreFloat(&marioDistance, marioDistanceVec);

		Update3DSound();

		// The sound must fade out after it is updated (the Update3DSounds)
		if (m_SoundFadingOut)
			UpdateSoundFadeOut(elapsedTime);

		// Make sure
		if (CheckIfGrounded() == false)
		{
			m_TotalVelocity.y -= m_FallAcceleration * elapsedTime;
			if (m_TotalVelocity.y < -m_CannonballDesc.maxFallSpeed)
				m_TotalVelocity.y = -m_CannonballDesc.maxFallSpeed;
		}

		// Move to the next target
		if (m_TargetPositions.size() > 1)
		{
			// Calculate the Movement Speed
			m_MoveSpeed += m_MoveAcceleration * elapsedTime;
			if (m_MoveSpeed > m_CannonballDesc.maxMoveSpeed)
				m_MoveSpeed = m_CannonballDesc.maxMoveSpeed;

			// Check if close enough to change target
			auto current2DPos = XMFLOAT2(GetTransform()->GetWorldPosition().x, GetTransform()->GetWorldPosition().z);
			auto target2DPos = XMFLOAT2(m_TargetPositions[m_CurrentTargetPos].x, m_TargetPositions[m_CurrentTargetPos].z);
			XMVECTOR targetDistanceVec = XMVector2Length(XMLoadFloat2(&target2DPos) - XMLoadFloat2(&current2DPos));
			float targetDistance = 0.0f;
			XMStoreFloat(&targetDistance, targetDistanceVec);
			if (targetDistance <= m_SeekErrorMargin)
			{
				m_MoveSpeed = 0.0f;
				m_TotalVelocity = XMFLOAT3(0, 0, 0);

				if (m_InvertedDir)
				{
					m_CurrentTargetPos--;
					if (m_CurrentTargetPos < 0)
					{
						m_InvertedDir = false;
						m_CurrentTargetPos = 1;
					}
				}
				else
				{
					m_CurrentTargetPos++;
					if (m_CurrentTargetPos > int(m_TargetPositions.size()) - 1)
					{
						if (m_pSpawnInfo->resetAfterAllTargets == true)
						{
							ResetPath();
							return;
						}

						m_InvertedDir = true;
						m_CurrentTargetPos = int(m_TargetPositions.size()) - 2;
					}
				}
			}

			// Seek the next target
			SeekNextTarget(elapsedTime);
		}

		// Check if Damage
		if (m_pMario->GetState() != MidDamaged && m_pMario->GetState() != Dead)
		{
			if (marioDistance < m_DamageDistance)
				m_pMario->GetDamaged(3, GetTransform()->GetWorldPosition());
		}
	}
}

void CannonballCharacter::SeekNextTarget(float elapsedTime)
{
	// Set the X & Z components of m_TotalVelocity (aka, the target seek)
	XMFLOAT3 targetDirection{};
	XMStoreFloat3(&targetDirection, XMVector3Normalize(XMLoadFloat3(&m_TargetPositions[m_CurrentTargetPos]) - XMLoadFloat3(&GetTransform()->GetWorldPosition())));
	XMFLOAT3 horizontalVelocity = targetDirection;
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

bool CannonballCharacter::CheckIfGrounded()
{
	auto origin = PxVec3(0, 0, 0);
	origin.x = m_pControllerComponent->GetTransform()->GetPosition().x; // - m_CharacterDesc.controller.height;
	origin.y = m_pControllerComponent->GetTransform()->GetPosition().y; // - m_CharacterDesc.controller.height;
	origin.z = m_pControllerComponent->GetTransform()->GetPosition().z; // - m_CharacterDesc.controller.height;
	const auto direction = PxVec3(0, -1, 0);
	const PxReal maxDistance = 4.f;//m_CharacterDesc.controller.height;
	PxRaycastBuffer hit;
	PxQueryFilterData filterData{};
	filterData.data.word0 = UINT(CollisionGroup::Group9);

	if(GetScene()->GetPhysxProxy()->Raycast(origin, direction, maxDistance, hit, PxHitFlag::eDEFAULT, filterData))
	{
		m_TotalVelocity.y = 0;
		return true;
	}

	return false;
}

void CannonballCharacter::ResetPath()
{
	m_MoveSpeed = 0.0f;
	m_TotalVelocity = XMFLOAT3(0, 0, 0);
	m_pControllerComponent->Translate(m_pSpawnInfo->targetPositions[0]);
	m_CurrentTargetPos = 0;
	m_InvertedDir = false;
}

void CannonballCharacter::Update3DSound()
{
	float marioDistance;
	XMStoreFloat(&marioDistance, XMVector3Length(XMLoadFloat3(&m_pMario->GetTransform()->GetWorldPosition()) - XMLoadFloat3(&GetTransform()->GetWorldPosition())));

	auto intendedVolume = 1.f - marioDistance / m_SoundStartDistance;

	if (intendedVolume <= 0.0f)
	{
		intendedVolume = 0.0f;
	}

	m_pSFXChannel->setVolume(intendedVolume * m_VolumeMultiplier);
	m_CurrentSFXVol = intendedVolume * m_VolumeMultiplier;

	//Start playing SFX
	//// Only initializing the sound here makes it so that each cannonball will play the sound at a unique offset
	if (m_SFXPlaying == false && intendedVolume > 0.0f)
	{
		SoundManager::Get()->GetSystem()->playSound(m_pMovingSound, nullptr, false, &m_pSFXChannel);
		m_SFXPlaying = true;
	}
}

void CannonballCharacter::Reset()
{
	ResetPath();
	m_pSFXChannel->setVolume(0.0f);
	m_SoundFadeOutCounter = 0.0f;
	m_SoundFadingOut = false;
}

void CannonballCharacter::TogglePause(bool paused)
{
	m_Paused = paused;

	if (paused)
		m_pSFXChannel->setPaused(true);
	else
		m_pSFXChannel->setPaused(false);
}

void CannonballCharacter::ToggleSoundFadeOut(float fadeTime)
{
	m_SoundTotalFadeTime = fadeTime;
	m_SoundFadingOut = true;
}

void CannonballCharacter::UpdateSoundFadeOut(float elapsedTime)
{
	m_SoundFadeOutCounter += elapsedTime;
	auto currentVolPercent = 1.f - m_SoundFadeOutCounter / m_SoundTotalFadeTime;

	if (m_SoundFadeOutCounter > m_SoundTotalFadeTime)
	{
		currentVolPercent = 0.0f;
		m_SoundFadeOutCounter = 0.0f;
	}

	m_pSFXChannel->setVolume(m_CurrentSFXVol * currentVolPercent);
}






