#include "stdafx.h"
#include "ModelTestScene.h"

#include "Materials/ColorMaterial.h"
#include "Materials/DiffuseMaterial.h"

void ModelTestScene::Initialize()
{
	//const auto& sceneContext = GetSceneContext();
	const auto* pDefaultMaterial = PxGetPhysics().createMaterial(0.5f, 0.5f, 0.5f);
	auto* matManager = MaterialManager::Get();

	// Ground plane
	GameSceneExt::CreatePhysXGroundPlane(*this);

	// Chair
	m_pChair = AddChild(new GameObject());
	auto* pChairMaterial = matManager->CreateMaterial<DiffuseMaterial>();
	pChairMaterial->SetDiffuseTexture(L"Textures/Chair_Dark.dds");
	auto* pChairComponent = m_pChair->AddComponent(new ModelComponent(L"Meshes/Chair.ovm"));
	pChairComponent->SetMaterial(pChairMaterial);


	auto* pChairActor = m_pChair->AddComponent(new RigidBodyComponent());
	PxConvexMesh* pChairConvexMesh = ContentManager::Load<PxConvexMesh>(L"Meshes/Chair.ovpc");
	pChairActor->AddCollider(PxConvexMeshGeometry(pChairConvexMesh), *pDefaultMaterial);
	pChairActor->SetDensity(50.f);

	m_pChair->GetTransform()->Translate(0.f, 15.f, 0.f);
	m_pChair->GetTransform()->Rotate(XMFLOAT3(15, 0, 15));
}

void ModelTestScene::Update()
{
	
}


