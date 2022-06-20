#include "stdafx.h"
#include "Star.h"
#include "MarioCharacter.h"
#include "Materials/Shadow/DiffuseMaterial_Shadow.h"

Star::Star(const XMFLOAT3& position, MarioCharacter* pMario)
	: m_pMario(pMario)
	, m_OriginalPosition(position)
{
}

void Star::Initialize(const SceneContext&)
{
	const auto pStarMaterial = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow>();
	pStarMaterial->SetDiffuseTexture(L"Textures/Star_Diffuse.png");
	m_pModelGO = AddChild(new GameObject);
	const auto pModel = m_pModelGO->AddComponent(new ModelComponent(L"Meshes/Star/Star.ovm"));
	pModel->SetMaterial(pStarMaterial);
	m_pModelGO->GetTransform()->Translate(m_OriginalPosition);
	m_pModelGO->GetTransform()->Rotate(90.f, 0.f, 0.f);
	m_pModelGO->GetTransform()->Scale(m_OriginalSize, m_OriginalSize, m_OriginalSize);

	//Sounds
	SoundManager::Get()->GetSystem()->createStream("Resources/Sounds/star-idle.wav", FMOD_LOOP_NORMAL, nullptr, &m_pIdleSound);
	SoundManager::Get()->GetSystem()->createStream("Resources/Sounds/star-caught.WAV", FMOD_DEFAULT, nullptr, &m_pPickedUpSound);
}

void Star::Update(const SceneContext& sceneContext)
{
	const auto elapsedTime = sceneContext.pGameTime->GetElapsed();

	// Rotate the mesh so it spins around itself
	const auto frameRot = m_RotationSpeed * elapsedTime;
	m_TotalRotation += frameRot;

	if(m_TotalRotation > 360)
		m_TotalRotation -= 360;
	else if(m_TotalRotation < 0)
		m_TotalRotation += 360;

	m_pModelGO->GetTransform()->Rotate(90.f, m_TotalRotation, 0.f);

	if (m_Captured == false)
	{
		Update3DSounds();

		// The sound must fade out after it is updated (the Update3DSounds)
		if (m_SoundFadingOut)
			UpdateSoundFadeOut(elapsedTime);

		// Translate the mesh up and down (and scale it slightly), as to create a slight wobble effect
		m_pModelGO->GetTransform()->Translate(m_OriginalPosition.x, m_OriginalPosition.y +
			(sinf(sceneContext.pGameTime->GetTotal()) * m_WobbleHeight), m_OriginalPosition.z);
		const auto newScale = m_OriginalSize + (sinf(sceneContext.pGameTime->GetTotal()) * m_WobbleScale);
		m_pModelGO->GetTransform()->Scale(newScale, newScale, newScale);


		// If the player gets close enough, complete the level
		XMVECTOR marioDistanceVec = XMVector3Length(XMLoadFloat3(&m_pMario->GetTransform()->GetWorldPosition()) - XMLoadFloat3(&m_pModelGO->GetTransform()->GetWorldPosition()));
		float marioDistance = 0.0f;
		XMStoreFloat(&marioDistance, marioDistanceVec);

		if (marioDistance < m_TriggerDistance)
		{
			SoundManager::Get()->GetSystem()->playSound(m_pPickedUpSound, nullptr, false, &m_pPickedUpChannel);
			m_CapturedSize = newScale;
			m_CapturedPosY = m_OriginalPosition.y +	(sinf(sceneContext.pGameTime->GetTotal()) * m_WobbleHeight);
			m_Captured = true;
		}
	}
	else
	{
		m_PostCaptureCounter += elapsedTime;

		// Move up after being captured
		m_pModelGO->GetTransform()->Translate(m_OriginalPosition.x, m_CapturedPosY +
			m_CapturedHeightIncrement * (m_PostCaptureCounter / m_TimeShrinkingBeforeEnd), m_OriginalPosition.z);

		// And shrink until the mesh disappears
		const auto newScale = m_CapturedSize * (1 - m_PostCaptureCounter/m_TimeShrinkingBeforeEnd);
		if(newScale > 0.0f)
			m_pModelGO->GetTransform()->Scale(newScale, newScale, newScale);
		else
			m_pModelGO->GetTransform()->Scale(0.0f, 0.0f, 0.0f);

		const auto newVolume = m_IdleMaxVolume * (1 - m_PostCaptureCounter / m_TimeShrinkingBeforeEnd);
		if (newVolume > 0.0f)
			m_pIdleChannel->setVolume(newVolume);
		else
			m_pIdleChannel->setVolume(0.0f);

		// And then wait for half a second and close the level
		if (m_PostCaptureCounter >= m_TimeShrinkingBeforeEnd + 0.5f)
		{
			Reset();
			GetScene()->Reset();
			SceneManager::Get()->SetActiveGameScene(L"VictoryScreen");
		}
	}
}

void Star::Reset()
{
	m_pIdleChannel->setPaused(true);
	m_pPickedUpChannel->setPaused(true);
	m_SoundFadeOutCounter = 0.0f;
	m_SoundFadingOut = false;
	m_Captured = false;
	m_PostCaptureCounter = 0.0f;
	m_pModelGO->GetTransform()->Translate(m_OriginalPosition);
	m_pModelGO->GetTransform()->Rotate(90.f, 0.f, 0.f);
	m_pModelGO->GetTransform()->Scale(m_OriginalSize, m_OriginalSize, m_OriginalSize);
	m_IdlePlaying = false;
	m_pIdleChannel->setPaused(true);
}

void Star::Update3DSounds()
{
	float marioDistance;
	XMStoreFloat(&marioDistance, XMVector3Length(XMLoadFloat3(&m_pMario->GetTransform()->GetWorldPosition()) - XMLoadFloat3(&m_OriginalPosition)));

	auto intendedVolume = 1.f - marioDistance / m_SoundStartDistance;

	if (intendedVolume <= 0.0f)
		intendedVolume = 0.0f;

	m_pIdleChannel->setVolume(intendedVolume * m_IdleMaxVolume);
	m_pPickedUpChannel->setVolume(intendedVolume * m_PickedUpMaxVolume);
	m_CurrentIdleVol = intendedVolume * m_IdleMaxVolume;
	m_CurrentPickedUpVol = intendedVolume * m_PickedUpMaxVolume;

	//Start playing SFX
	if (m_IdlePlaying == false && intendedVolume > 0.0f)
	{
		SoundManager::Get()->GetSystem()->playSound(m_pIdleSound, nullptr, false, &m_pIdleChannel);
		m_IdlePlaying = true;
	}
}

void Star::ToggleSoundFadeOut(float fadeTime)
{
	m_SoundTotalFadeTime = fadeTime;
	m_SoundFadingOut = true;
}

void Star::UpdateSoundFadeOut(float elapsedTime)
{
	m_SoundFadeOutCounter += elapsedTime;
	auto currentVolPercent = 1.f - m_SoundFadeOutCounter / m_SoundTotalFadeTime;

	if (m_SoundFadeOutCounter > m_SoundTotalFadeTime)
	{
		currentVolPercent = 0.0f;
		m_SoundFadeOutCounter = 0.0f;
	}

	m_pIdleChannel->setVolume(m_CurrentIdleVol * currentVolPercent);
	m_pPickedUpChannel->setVolume(m_CurrentPickedUpVol * currentVolPercent);
}



