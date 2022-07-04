#include "stdafx.h"
#include "Tube.h"

#include "Materials/Shadow/DiffuseMaterial_Shadow.h"

Tube::Tube(const XMFLOAT3& initialPosition, const XMFLOAT3& finalPosition, float moveTime)
	: m_pModelComponent()
	, m_OriginalPosition(initialPosition)
	, m_FinalPosition(finalPosition)
	, m_TimeToHide(moveTime)
{
}

void Tube::Initialize(const SceneContext&)
{
	m_pTubeMaterial = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow>();
	m_pTubeMaterial->SetDiffuseTexture(L"Textures/Tube_Diffuse.png");
	m_pModelComponent = AddComponent(new ModelComponent(L"Meshes/Tube.ovm"));
	m_pModelComponent->SetMaterial(m_pTubeMaterial);
	GetTransform()->Translate(m_OriginalPosition);
	GetTransform()->Scale(12.f, 12.f, 12.f);
	GetTransform()->Rotate(90.f, 0.f, 0.f);

	Reset();
}

void Tube::Update(const SceneContext& sceneContext)
{
	if (m_ComponentDeleted == false)
	{
		m_Counter += sceneContext.pGameTime->GetElapsed();

		if (m_Counter < m_TimeToHide)
		{
			auto newPositionVec = XMLoadFloat3(&m_OriginalPosition) +
				(XMLoadFloat3(&m_FinalPosition) - XMLoadFloat3(&m_OriginalPosition)) * (m_Counter / m_TimeToHide);
			XMFLOAT3 newPosition;
			XMStoreFloat3(&newPosition, newPositionVec);
			GetTransform()->Translate(newPosition);
		}
		else
		{
			RemoveComponent(m_pModelComponent, true);
			m_ComponentDeleted = true;
		}
	}
}


void Tube::Reset()
{
	m_Counter = 0.0f;

	if (m_ComponentDeleted)
	{
		m_pModelComponent = AddComponent(new ModelComponent(L"Meshes/Tube.ovm"));
		m_pModelComponent->SetMaterial(m_pTubeMaterial);
	}
	m_ComponentDeleted = false;

	GetTransform()->Translate(m_OriginalPosition);
}



