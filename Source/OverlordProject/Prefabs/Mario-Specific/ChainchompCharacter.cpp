#include "stdafx.h"
#include "ChainchompCharacter.h"
#include "MarioCharacter.h"
#include "Materials/Shadow/DiffuseMaterial_Shadow.h"
#include "Prefabs/SpherePrefab.h"
#include "Scenes/Mario64/MainScene.h"


ChainchompCharacter::ChainchompCharacter(const ChainchompDesc& characterDesc, MarioCharacter* pMario, ChainchompSpawnInfo* pSpawnInfo) :
	m_ChainchompDesc{ characterDesc },
	m_MoveAcceleration(characterDesc.maxMoveSpeed / characterDesc.moveAccelerationTime),
	m_RotationAcceleration(characterDesc.maxRotationSpeed / characterDesc.rotationAccelerationTime),
	m_FallAcceleration(characterDesc.maxFallSpeed / characterDesc.fallAccelerationTime),
	m_TargetPosition{},
	m_pMario{ pMario },
	m_pSpawnInfo{ pSpawnInfo }
{
}

void ChainchompCharacter::Initialize(const SceneContext&)
{
	//Controller
	m_pControllerComponent = AddComponent(new ControllerComponent(m_ChainchompDesc.controller));

	//Character Mesh
	auto pBall = AddChild(new SpherePrefab(0.5f, 10, XMFLOAT4{ Colors::Black }));
	pBall->GetTransform()->Scale(6.3984f, 6.3984f, 6.3984f);

	/*const auto pCharacterMaterial = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow>();
	pCharacterMaterial->SetDiffuseTexture(L"Textures/BobOmb_Diffuse.png");
	m_pModelParentGO = AddChild(new GameObject);
	m_pModelGO = m_pModelParentGO->AddChild(new GameObject);
	const auto pModel = m_pModelGO->AddComponent(new ModelComponent(L"Meshes/BobOmb/BobOmb.ovm"));
	pModel->SetMaterial(pCharacterMaterial);
	m_pModelParentGO->GetTransform()->Scale(0.06f, 0.06f, 0.06f);
	m_pModelParentGO->GetTransform()->Translate(0, -m_BoBombDesc.controller.height - 0.5f, 0);*/

	//Sound
	auto* pSoundManagerSystem = SoundManager::Get()->GetSystem();
	pSoundManagerSystem->createStream("Resources/Sounds/chainchomp-chain-loop.WAV", FMOD_LOOP_NORMAL, nullptr, &m_pChainSound);
	pSoundManagerSystem->createStream("Resources/Sounds/chainchomp-bite.WAV", FMOD_DEFAULT, nullptr, &m_pBittingSound);
	pSoundManagerSystem->createStream("Resources/Sounds/chainchomp-lunge.WAV", FMOD_DEFAULT, nullptr, &m_pLungingSound);
}

void ChainchompCharacter::Update(const SceneContext& sceneContext)
{
	if (m_State != Paused)
	{
		const auto elapsedTime = sceneContext.pGameTime->GetElapsed();

		Update3DSound();

		// The sound must fade out after it is updated (the Update3DSounds)
		if (m_SoundFadingOut)
			UpdateSoundFadeOut(elapsedTime);

		const XMVECTOR marioDistanceVec = XMLoadFloat3(&m_pMario->GetTransform()->GetWorldPosition()) - XMLoadFloat3(&GetTransform()->GetWorldPosition());
		XMFLOAT3 marioDistance{};
		XMStoreFloat3(&marioDistance, marioDistanceVec);
		float marioDistanceLength = 0.0f;
		XMStoreFloat(&marioDistanceLength, XMVector3Length(marioDistanceVec));

		switch (m_State)
		{
		case Idle:
			break;
		case Rotating:
			break;
		case Lunging:
			break;
		case Recoiling:
			break;
		default:
			break;
		}
	}
}


bool ChainchompCharacter::CheckIfGrounded()
{
	auto origin = PxVec3(0, 0, 0);
	origin.x = m_pControllerComponent->GetTransform()->GetPosition().x;
	origin.y = m_pControllerComponent->GetTransform()->GetPosition().y;
	origin.z = m_pControllerComponent->GetTransform()->GetPosition().z;
	const auto direction = PxVec3(0, -1, 0);
	const PxReal maxDistance = 4.f;
	PxRaycastBuffer hit;
	PxQueryFilterData filterData{};
	filterData.data.word0 = UINT(CollisionGroup::Group9);

	if (GetScene()->GetPhysxProxy()->Raycast(origin, direction, maxDistance, hit, PxHitFlag::eDEFAULT, filterData))
	{
		m_TotalVelocity.y = 0;
		return true;
	}

	return false;
}

void ChainchompCharacter::Update3DSound()
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
	if (m_SFXPlaying == false && intendedVolume > 0.0f)
	{
		SoundManager::Get()->GetSystem()->playSound(m_pChainSound, nullptr, false, &m_pSFXChannel);
		m_SFXPlaying = true;
	}
}

void ChainchompCharacter::Reset()
{
	m_MoveSpeed = 0.0f;
	m_TotalVelocity = XMFLOAT3(0, 0, 0);
	m_pControllerComponent->Translate(m_pSpawnInfo->spawnPosition);
	m_CurrentTargetPos = 0;

	m_pSFXChannel->setVolume(0.0f);
	m_SoundFadeOutCounter = 0.0f;
	m_SoundFadingOut = false;
}

void ChainchompCharacter::TogglePause(bool paused)
{
	m_Paused = paused;

	if (paused)
		m_pSFXChannel->setPaused(true);
	else
		m_pSFXChannel->setPaused(false);
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

	m_pSFXChannel->setVolume(m_CurrentSFXVol * currentVolPercent);
}
