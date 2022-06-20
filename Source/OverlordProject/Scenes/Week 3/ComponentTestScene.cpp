#include "stdafx.h"
#include "ComponentTestScene.h"
#include "Prefabs/SpherePrefab.h"


void ComponentTestScene::Initialize()
{
	auto* pDefaultMaterial = PxGetPhysics().createMaterial(.5f, .5f, 1.f);

	GameSceneExt::CreatePhysXGroundPlane(*this, pDefaultMaterial);

	//Sphere 1 (in group 1)
	auto* pSphereObject = AddChild(new SpherePrefab(1.f, 10, XMFLOAT4{Colors::Red}));
	auto* pSphereActor = pSphereObject->AddComponent(new RigidBodyComponent());
	pSphereActor->AddCollider(PxSphereGeometry(1.f), *pDefaultMaterial);
	pSphereObject->GetTransform()->Translate(0.f, 50.f, 0.f); // is the same as pSphereObject->GetComponent<TransformComponent>()
	pSphereActor->SetCollisionGroup(CollisionGroup::Group1);

	// To access an actor's collider
	//const UINT colliderId = pSphereActor->AddCollider(PxSphereGeometry(1.f), *pDefaultMaterial);
	//pSphereActor->GetCollider(colliderId);

	// To create trigger
	//pSphereObject->SetOnTriggerCallBack([=](GameObject* pTrigger, GameObject* pOther, PxTriggerAction action)
	//	{
	//
	//	});

	//Sphere 2 (in group 0)
	pSphereObject = AddChild(new SpherePrefab(1.f, 10, XMFLOAT4{ Colors::Green }));
	pSphereActor = pSphereObject->AddComponent(new RigidBodyComponent());
	pSphereActor->AddCollider(PxSphereGeometry(1.f), *pDefaultMaterial);
	pSphereObject->GetTransform()->Translate(0.f, 40.f, 0.f);
	pSphereActor->SetCollisionIgnoreGroups(CollisionGroup::Group1); // could do more groups, by adding  " | CollisionGroup::Group2"

	//Sphere 3 (in group 0)
	pSphereObject = AddChild(new SpherePrefab(1.f, 10, XMFLOAT4{ Colors::Blue }));
	pSphereActor = pSphereObject->AddComponent(new RigidBodyComponent());
	pSphereActor->AddCollider(PxSphereGeometry(1.f), *pDefaultMaterial);
	pSphereObject->GetTransform()->Translate(0.f, 30.f, 0.f);
}
