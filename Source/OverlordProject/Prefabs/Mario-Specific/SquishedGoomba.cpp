#include "stdafx.h"
#include "SquishedGoomba.h"

#include "Materials/Shadow/DiffuseMaterial_Shadow.h"

SquishedGoomba::SquishedGoomba(const XMFLOAT3& position)
	: m_Position(position)
{
}

void SquishedGoomba::Initialize(const SceneContext&)
{
	//Character Mesh
	const auto pCharacterMaterial = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow>();
	pCharacterMaterial->SetDiffuseTexture(L"Textures/GoombaSquished_Diffuse.png");
	m_pModelComponent = AddComponent(new ModelComponent(L"Meshes/Goomba/GoombaSquished.ovm"));
	m_pModelComponent->SetMaterial(pCharacterMaterial);
	GetTransform()->Scale(3.5f, 3.5f, 3.5f);
	GetTransform()->Translate(m_Position);
	GetTransform()->Rotate(90.f, 0.f, 0.f);
}


void SquishedGoomba::Update(const SceneContext& sceneContext)
{
	m_TimeCounter += sceneContext.pGameTime->GetElapsed();
	if (m_TimeCounter > m_LifeTime)
		SetAwaitingDeletion(true);
		//GetScene()->RemoveChild(this, true);
}
