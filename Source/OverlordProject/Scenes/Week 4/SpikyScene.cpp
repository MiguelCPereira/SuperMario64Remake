#include "stdafx.h"
#include "SpikyScene.h"
#include "Materials/SpikyMaterial.h"

void SpikyScene::Initialize()
{
	auto& sceneSettings = GetSceneSettings();
	auto* matManager = MaterialManager::Get();

	// Sphere
	m_pSphere = AddChild(new GameObject());
	m_pSphere->GetTransform()->Scale(15, 15, 15);
	m_pSphereMaterial = matManager->CreateMaterial<SpikyMaterial>();
	m_pSphereMaterial->SetDiffuseColor(XMFLOAT4{ 1,0,0,1 });
	auto* pSphereComponent = m_pSphere->AddComponent(new ModelComponent(L"Meshes/OctaSphere.ovm"));
	pSphereComponent->SetMaterial(m_pSphereMaterial);

	sceneSettings.drawGrid = false;
	sceneSettings.enableOnGUI = true;
}

void SpikyScene::OnGUI()
{
	m_pSphereMaterial->DrawImGui();
}

void SpikyScene::Update()
{
	const auto frameRotation = m_RotationPerSec * GetSceneContext().pGameTime->GetElapsed();
	m_CurrentRotY += frameRotation;
	if (m_CurrentRotY >= 360.f)
		m_CurrentRotY -= 360.f;

	m_pSphere->GetTransform()->Rotate(0, m_CurrentRotY, 0);
}
