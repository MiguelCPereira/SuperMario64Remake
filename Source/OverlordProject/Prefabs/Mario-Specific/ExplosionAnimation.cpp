#include "stdafx.h"
#include "ExplosionAnimation.h"

ExplosionAnimation::ExplosionAnimation(const XMFLOAT3& position)
	: m_Position(position)
{
}

void ExplosionAnimation::Initialize(const SceneContext&)
{
	ParticleEmitterSettings settings{};
	settings.velocity = { 0.f,0.f,0.f };
	settings.minSize = 5.f;
	settings.maxSize = 5.f;
	settings.minEnergy = m_LifeTime;
	settings.maxEnergy = m_LifeTime;
	settings.minScale = 1.f;
	settings.maxScale = 1.f;
	settings.minEmitterRadius = .2f;
	settings.maxEmitterRadius = .3f;
	settings.color = { 1.f,1.f,1.f, 1.f };
	GetTransform()->Translate(m_Position);
	m_pEmitter = AddComponent(new ParticleEmitterComponent(L"Textures/BobOmbExplosion/Explosion0.png", settings, 1));
}


void ExplosionAnimation::Update(const SceneContext& sceneContext)
{
	m_TimeCounter += sceneContext.pGameTime->GetElapsed();
	if(m_TimeCounter > m_LifeTime / float(m_FrameNumber))
	{
		m_SpriteCounter++;

		// Delete the object if the animation's finished
		if(m_SpriteCounter >= m_FrameNumber - 1)
		{
			SetAwaitingDeletion(true);
			//GetScene()->RemoveChild(this, true);
			return;
		}

		// Change the sprite to the next animation frame
		m_TimeCounter = 0.0f;
		const std::wstring spriteName = L"Textures/BobOmbExplosion/Explosion" + std::to_wstring(m_SpriteCounter) + L".png";
		m_pEmitter->SwitchTexture(spriteName);
	}
}



