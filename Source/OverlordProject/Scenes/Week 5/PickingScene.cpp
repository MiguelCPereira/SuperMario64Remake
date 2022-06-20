#include "stdafx.h"
#include "PickingScene.h"

#include "Prefabs/SpherePrefab.h"

void PickingScene::Initialize()
{
	m_SceneContext.settings.drawGrid = false;

	const auto pMaterial = PxGetPhysics().createMaterial(.5f, .5f, .5f);

	constexpr int amount{ 5 };
	for (auto col{ -amount }; col <= amount; ++col)
	{
		for (auto row{ -amount }; row <= amount; ++row)
		{
			const auto pSphere = AddChild(new SpherePrefab(4.f));
			const auto pRigidBody = pSphere->AddComponent(new RigidBodyComponent(true));
			pRigidBody->AddCollider(PxSphereGeometry{ 4.f }, *pMaterial);

			pSphere->GetTransform()->Translate(20.f * col, 0.f, 20.f * row);
		}
	}
}

void PickingScene::Update()
{
	if (InputManager::IsMouseButton(InputState::pressed, VK_RBUTTON))
	{
		if (const auto pPickedObject = m_SceneContext.pCamera->Pick())
		{
			RemoveChild(pPickedObject, true);
		}
	}
}
