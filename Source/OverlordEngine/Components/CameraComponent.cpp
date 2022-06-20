#include "stdafx.h"
#include "CameraComponent.h"

CameraComponent::CameraComponent() :
	m_FarPlane(2500.0f),
	m_NearPlane(0.1f),
	m_FOV(XM_PIDIV4),
	m_Size(25.0f),
	m_PerspectiveProjection(true)
{
	XMStoreFloat4x4(&m_Projection, XMMatrixIdentity());
	XMStoreFloat4x4(&m_View, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ViewInverse, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ViewProjection, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ViewProjectionInverse, XMMatrixIdentity());
}

void CameraComponent::Update(const SceneContext& sceneContext)
{
	// see https://stackoverflow.com/questions/21688529/binary-directxxmvector-does-not-define-this-operator-or-a-conversion
	using namespace DirectX;

	XMMATRIX projection{};

	if (m_PerspectiveProjection)
	{
		projection = XMMatrixPerspectiveFovLH(m_FOV, sceneContext.aspectRatio, m_NearPlane, m_FarPlane);
	}
	else
	{
		const float viewWidth = (m_Size > 0) ? m_Size * sceneContext.aspectRatio : sceneContext.windowWidth;
		const float viewHeight = (m_Size > 0) ? m_Size : sceneContext.windowHeight;
		projection = XMMatrixOrthographicLH(viewWidth, viewHeight, m_NearPlane, m_FarPlane);
	}

	const XMVECTOR worldPosition = XMLoadFloat3(&GetTransform()->GetWorldPosition());
	const XMVECTOR lookAt = XMLoadFloat3(&GetTransform()->GetForward());
	const XMVECTOR upVec = XMLoadFloat3(&GetTransform()->GetUp());

	const XMMATRIX view = XMMatrixLookAtLH(worldPosition, worldPosition + lookAt, upVec);
	const XMMATRIX viewInv = XMMatrixInverse(nullptr, view);
	const XMMATRIX viewProjectionInv = XMMatrixInverse(nullptr, view * projection);

	XMStoreFloat4x4(&m_Projection, projection);
	XMStoreFloat4x4(&m_View, view);
	XMStoreFloat4x4(&m_ViewInverse, viewInv);
	XMStoreFloat4x4(&m_ViewProjection, view * projection);
	XMStoreFloat4x4(&m_ViewProjectionInverse, viewProjectionInv);
}

void CameraComponent::SetActive(bool active)
{
	if (m_IsActive == active) return;

	const auto pGameObject = GetGameObject();
	ASSERT_IF(!pGameObject, L"Failed to set active camera. Parent game object is null");

	if (!pGameObject) return; //help the compiler... (C6011)
	const auto pScene = pGameObject->GetScene();
	ASSERT_IF(!pScene, L"Failed to set active camera. Parent game scene is null");

	m_IsActive = active;
	pScene->SetActiveCamera(active?this:nullptr); //Switch to default camera if active==false
}

GameObject* CameraComponent::Pick(CollisionGroup ignoreGroups) const
{
	// TODO_W5(L"Implement Picking Logic")

	auto* pScene = GetScene();

	// Convert mouse coordinates to NDC space
	const auto mouseCoordInt = pScene->GetSceneContext().pInput->GetMousePosition();
	XMFLOAT3 mouseCoord = XMFLOAT3(float(mouseCoordInt.x), float(mouseCoordInt.y), 1.f);
	const auto halfWidth = pScene->GetSceneContext().windowWidth / 2.f;
	const auto halfHeight = pScene->GetSceneContext().windowHeight / 2.f;
	mouseCoord.x = (mouseCoord.x - halfWidth) / halfWidth;
	mouseCoord.y = (halfHeight - mouseCoord.y) / halfHeight;

	// Calculate near and far points
	const auto nearPointMouseCoord = XMFLOAT3(mouseCoord.x, mouseCoord.y, 0);
	auto nearPoint = XMVector3TransformCoord(XMLoadFloat3(&nearPointMouseCoord), XMLoadFloat4x4(&GetViewProjectionInverse()));
	const auto farPointMouseCoord = XMFLOAT3(mouseCoord.x, mouseCoord.y, 1);
	auto farPoint = XMVector3TransformCoord(XMLoadFloat3(&farPointMouseCoord), XMLoadFloat4x4(&GetViewProjectionInverse()));

	// Set up the objects to be filtered during query
	PxQueryFilterData filterData{};
	filterData.data.word0 = ~UINT(ignoreGroups);

	// Calculate the ray
	const auto direction = XMVector3Normalize(farPoint - nearPoint);
	XMFLOAT3 rayStart{};
	XMFLOAT3 rayEnd{};
	XMFLOAT3 rayDirection{};
	XMStoreFloat3(&rayStart, nearPoint);
	XMStoreFloat3(&rayEnd, farPoint);
	XMStoreFloat3(&rayDirection, direction);

	// And raycast
	PxRaycastBuffer hit{};
	if (pScene->GetPhysxProxy()->Raycast(PxVec3(rayStart.x, rayStart.y, rayStart.z),
		PxVec3(rayDirection.x, rayDirection.y, rayDirection.z), PX_MAX_F32,
		hit, PxHitFlag::eDEFAULT, filterData))
	{
		//Logger::LogDebug(L"Clicked on shape!");
		const auto& goBaseComp = static_cast<BaseComponent*>(hit.getAnyHit(static_cast<PxU32>(0)).actor->userData);
		return goBaseComp->GetGameObject();
	}

	return nullptr;
}