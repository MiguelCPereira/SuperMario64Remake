#include "stdafx.h"
#include "UberMaterialScene.h"
#include "Materials/UberMaterial.h"

void UberMaterialScene::Initialize()
{
	auto& sceneSettings = GetSceneSettings();
	auto* matManager = MaterialManager::Get();

	// Sphere
	m_pSphere = AddChild(new GameObject());
	m_pSphere->GetTransform()->Scale(25, 25, 25);
	m_pSphereMaterial = matManager->CreateMaterial<UberMaterial>();
	m_pSphereMaterial->SetDiffuseTexture(L"Textures/Skulls_Textures/Skulls_Diffusemap.tga", XMFLOAT4{ 1,0,0,1 });
	m_pSphereMaterial->SetNormalTexture(L"Textures/Skulls_Textures/Skulls_Normalmap.tga");
	m_pSphereMaterial->SetSpecularTexture(L"Textures/Skulls_Textures/Skulls_HeightMap.tga");
	m_pSphereMaterial->SetCubemapTexture(L"Textures/Sunol_Cubemap.dds", 0.8f, 0.5f);
	auto* pSphereComponent = m_pSphere->AddComponent(new ModelComponent(L"Meshes/Sphere.ovm"));
	pSphereComponent->SetMaterial(m_pSphereMaterial);

	sceneSettings.drawGrid = false;
	sceneSettings.enableOnGUI = true;
}

void UberMaterialScene::OnGUI()
{
	m_pSphereMaterial->DrawImGui();
}

void UberMaterialScene::Update()
{
	const auto frameRotation = m_RotationPerSec * GetSceneContext().pGameTime->GetElapsed();
	m_CurrentRotY += frameRotation;
	if (m_CurrentRotY >= 360.f)
		m_CurrentRotY -= 360.f;

	m_pSphere->GetTransform()->Rotate(0, m_CurrentRotY, 0);
}


