#include "stdafx.h"
#include "PongScene.h"
#include "Prefabs/SpherePrefab.h"
#include "Prefabs/CubePrefab.h"

void PongScene::Initialize()
{
	const auto& sceneContext = GetSceneContext();
	const auto* pDefaultMaterial = PxGetPhysics().createMaterial(0.f, 0.f, 1.f);

	// Set camera
	auto* pCamera = AddChild(new FixedCamera());
	pCamera->GetTransform()->Translate(0.f, 25.f, 0.f);
	pCamera->SetRotation(90.f, 0.f);
	SetActiveCamera(pCamera->GetComponent<CameraComponent>());



	//Background
	AddChild(new CubePrefab(50, 1, 50, XMFLOAT4{ Colors::Black }));



	// Ball
	m_pBall = AddChild(new SpherePrefab(0.5f, 10, XMFLOAT4{ Colors::Red }));
	m_pBall->GetTransform()->Translate(0.f, 2.f, 0.f);
	const auto* pBallMaterial = PxGetPhysics().createMaterial(0.f, 0.f, 1.f);
	auto* pSphereActor = m_pBall->AddComponent(new RigidBodyComponent());
	pSphereActor->SetDensity(0.01f);
	pSphereActor->AddCollider(PxSphereGeometry(0.5f), *pBallMaterial);
	pSphereActor->SetConstraint(RigidBodyConstraint::TransY, false);



	// Players

	//// Left Player
	m_pLeftPlayer = AddChild(new CubePrefab(m_PlayerHeightWidth, m_PlayerHeightWidth, m_PlayerLength, XMFLOAT4{ Colors::Beige }));
	m_pLeftPlayer->GetTransform()->Translate(-15.f, 2.f, 0.f);
	auto* pLeftPlayerActor = m_pLeftPlayer->AddComponent(new RigidBodyComponent());
	pLeftPlayerActor->AddCollider(PxBoxGeometry(m_PlayerHeightWidth / 2.f, m_PlayerHeightWidth / 2.f,
		m_PlayerLength / 2.f), *pDefaultMaterial);
	//pLeftPlayerActor->SetConstraint(RigidBodyConstraint::TransY, false);
	//pLeftPlayerActor->SetConstraint(RigidBodyConstraint::TransX, false);
	//pLeftPlayerActor->SetConstraint(RigidBodyConstraint::AllRot, false);
	pLeftPlayerActor->SetKinematic(true);

	//// Right Player
	m_pRightPlayer = AddChild(new CubePrefab(m_PlayerHeightWidth, m_PlayerHeightWidth, m_PlayerLength, XMFLOAT4{ Colors::Beige }));
	m_pRightPlayer->GetTransform()->Translate(15.f, 2.f, 0.f);
	auto* pRightPlayerActor = m_pRightPlayer->AddComponent(new RigidBodyComponent());
	pRightPlayerActor->AddCollider(PxBoxGeometry(m_PlayerHeightWidth / 2.f, m_PlayerHeightWidth / 2.f,
		m_PlayerLength / 2.f), *pDefaultMaterial);
	pRightPlayerActor->SetKinematic(true);



	// Map Limits

	//// Top Limit
	auto* pTopLimitActor = PxGetPhysics().createRigidStatic(PxTransform{ PxVec3{0.f, 0.f, 11.5f} });
	PxRigidActorExt::createExclusiveShape(*pTopLimitActor, PxBoxGeometry{ 20.f, 2.f, 2.f }, *pDefaultMaterial);
	GetPhysxProxy()->AddActor(*pTopLimitActor);

	//// Bottom Limit
	auto* pBottomLimitActor = PxGetPhysics().createRigidStatic(PxTransform{ PxVec3{0.f, 0.f, -11.5f} });
	PxRigidActorExt::createExclusiveShape(*pBottomLimitActor, PxBoxGeometry{ 20.f, 2.f, 2.f }, *pDefaultMaterial);
	GetPhysxProxy()->AddActor(*pBottomLimitActor);



	// Map Triggers

	//// Left Trigger
	auto* pLeftTrigger = AddChild(new GameObject());
	pLeftTrigger->GetTransform()->Translate(20.f, 2.f, 0.f);
	auto* pLeftTriggerActor = pLeftTrigger->AddComponent(new RigidBodyComponent());
	pLeftTriggerActor->AddCollider(PxBoxGeometry(2.f, 2.f, 20.f), *pDefaultMaterial, true);
	pLeftTriggerActor->SetKinematic(true);
	//const UINT colliderId = pRightTriggerActor->AddCollider(PxBoxGeometry(2.f, 2.f, 20.f), *pNoFrictionMaterial, true);
	//pRightTriggerActor->GetCollider(colliderId);
	pLeftTrigger->SetOnTriggerCallBack([=](GameObject*, GameObject* pOtherObject, PxTriggerAction)
		{
			std::cout << "P2 scored!";
			if (pOtherObject == m_pBall)
				ResetGame();
		});

	//// Right Trigger
	auto* pRightTrigger = AddChild(new GameObject());
	pRightTrigger->GetTransform()->Translate(-20.f, 2.f, 0.f);
	auto* pRightTriggerActor = pRightTrigger->AddComponent(new RigidBodyComponent());
	pRightTriggerActor->AddCollider(PxBoxGeometry(2.f, 2.f, 20.f), *pDefaultMaterial, true);
	pRightTriggerActor->SetKinematic(true);
	pRightTrigger->SetOnTriggerCallBack([=](GameObject*, GameObject* pOtherObject, PxTriggerAction)
		{
			std::cout << "P1 scored!";
			if (pOtherObject == m_pBall)
				ResetGame();
		});



	// Set input actions
	const InputAction actionP1Up{ int(InputIds::P1MoveUp), InputState::down, 'U', -1 };
	const InputAction actionP1Down{ int(InputIds::P1MoveDown), InputState::down, 'J', -1 };
	const InputAction actionP2Up{ int(InputIds::P2MoveUp), InputState::down, VK_UP, -1 };
	const InputAction actionP2Down{ int(InputIds::P2MoveDown), InputState::down, VK_DOWN, -1 };
	const InputAction actionThrowBall{ int(InputIds::ThrowBall), InputState::pressed, VK_SPACE, -1 };
	sceneContext.pInput->AddInputAction(actionP1Up);
	sceneContext.pInput->AddInputAction(actionP1Down);
	sceneContext.pInput->AddInputAction(actionP2Up);
	sceneContext.pInput->AddInputAction(actionP2Down);
	sceneContext.pInput->AddInputAction(actionThrowBall);
}

void PongScene::Update()
{
	// Move the players
	//constexpr auto playerMoveForce{ 20.f };

	if (GetSceneContext().pInput->IsActionTriggered(int(InputIds::P1MoveUp)))
	{
		//m_pLeftPlayer->GetComponent<RigidBodyComponent>()->AddForce(XMFLOAT3{ playerMoveForce, 0.f, 0.f }, PxForceMode::eFORCE);
		const auto leftPlayerPos = m_pLeftPlayer->GetTransform()->GetPosition();
		const auto frameTransZ = m_PlayerMoveSpeed * GetSceneContext().pGameTime->GetElapsed();
		const auto newPlayerPosZ = leftPlayerPos.z + frameTransZ;
		if(newPlayerPosZ <= m_PlayerMaxPosY - m_PlayerLength / 2.f)
			m_pLeftPlayer->GetTransform()->Translate(leftPlayerPos.x, leftPlayerPos.y, newPlayerPosZ);
		else
			m_pLeftPlayer->GetTransform()->Translate(leftPlayerPos.x, leftPlayerPos.y, m_PlayerMaxPosY - m_PlayerLength / 2.f);
	}

	if (GetSceneContext().pInput->IsActionTriggered(int(InputIds::P1MoveDown)))
	{
		const auto leftPlayerPos = m_pLeftPlayer->GetTransform()->GetPosition();
		const auto frameTransZ = m_PlayerMoveSpeed * GetSceneContext().pGameTime->GetElapsed();
		const auto newPlayerPosZ = leftPlayerPos.z - frameTransZ;
		if (newPlayerPosZ >= m_PlayerMinPosY + m_PlayerLength / 2.f)
			m_pLeftPlayer->GetTransform()->Translate(leftPlayerPos.x, leftPlayerPos.y, newPlayerPosZ);
		else
			m_pLeftPlayer->GetTransform()->Translate(leftPlayerPos.x, leftPlayerPos.y, m_PlayerMinPosY + m_PlayerLength / 2.f);
	}

	if (GetSceneContext().pInput->IsActionTriggered(int(InputIds::P2MoveUp)))
	{
		const auto leftPlayerPos = m_pRightPlayer->GetTransform()->GetPosition();
		const auto frameTransZ = m_PlayerMoveSpeed * GetSceneContext().pGameTime->GetElapsed();
		const auto newPlayerPosZ = leftPlayerPos.z + frameTransZ;
		if (newPlayerPosZ <= m_PlayerMaxPosY - m_PlayerLength / 2.f)
			m_pRightPlayer->GetTransform()->Translate(leftPlayerPos.x, leftPlayerPos.y, newPlayerPosZ);
		else
			m_pRightPlayer->GetTransform()->Translate(leftPlayerPos.x, leftPlayerPos.y, m_PlayerMaxPosY - m_PlayerLength / 2.f);
	}

	if (GetSceneContext().pInput->IsActionTriggered(int(InputIds::P2MoveDown)))
	{
		const auto leftPlayerPos = m_pRightPlayer->GetTransform()->GetPosition();
		const auto frameTransZ = m_PlayerMoveSpeed * GetSceneContext().pGameTime->GetElapsed();
		const auto newPlayerPosZ = leftPlayerPos.z - frameTransZ;
		if (newPlayerPosZ >= m_PlayerMinPosY + m_PlayerLength / 2.f)
			m_pRightPlayer->GetTransform()->Translate(leftPlayerPos.x, leftPlayerPos.y, newPlayerPosZ);
		else
			m_pRightPlayer->GetTransform()->Translate(leftPlayerPos.x, leftPlayerPos.y, m_PlayerMinPosY + m_PlayerLength / 2.f);
	}

	// Throw the ball, if the game hasn't started yet
	if (m_RoundStarted == false && GetSceneContext().pInput->IsActionTriggered(int(InputIds::ThrowBall)))
	{
		m_RoundStarted = true;
		m_pBall->GetComponent<RigidBodyComponent>()->AddForce(XMFLOAT3{ m_BallThrowImpulse, 0.f, m_BallThrowImpulse }, PxForceMode::eIMPULSE);
	}
}

void PongScene::ResetGame()
{
	m_pLeftPlayer->GetTransform()->Translate(-15.f, 2.f, 0.f);
	m_pRightPlayer->GetTransform()->Translate(15.f, 2.f, 0.f);
	m_pBall->GetTransform()->Translate(0.f, 2.f, 0.f);
	m_RoundStarted = false;
}
