#include "stdafx.h"
#include "FreeCamera.h"

void FreeCamera::Initialize(const SceneContext& )
{
	m_pCamera = new CameraComponent();
	AddComponent(m_pCamera);
}

void FreeCamera::SetRotation(float pitch, float yaw)
{
	m_TotalPitch = pitch;
	m_TotalYaw = yaw;
}

void FreeCamera::Update(const SceneContext& sceneContext)
{
	// see https://stackoverflow.com/questions/21688529/binary-directxxmvector-does-not-define-this-operator-or-a-conversion
	if (m_pCamera->IsActive())
	{
		//HANDLE INPUT
		XMFLOAT2 move{ 0, 0 };
		move.y = InputManager::IsKeyboardKey(InputState::down, 'W') ? 1.0f : 0.0f;
		if (move.y == 0) move.y = -(InputManager::IsKeyboardKey(InputState::down, 'S') ? 1.0f : 0.0f);
		if (move.y == 0) move.y = InputManager::GetThumbstickPosition().y;

		move.x = InputManager::IsKeyboardKey(InputState::down, 'D') ? 1.0f : 0.0f;
		if (move.x == 0) move.x = -(InputManager::IsKeyboardKey(InputState::down, 'A') ? 1.0f : 0.0f);
		if (move.x == 0) move.x = InputManager::GetThumbstickPosition().x;

		float currSpeed{ m_MoveSpeed };
		if (InputManager::IsKeyboardKey(InputState::down, VK_LSHIFT))
			currSpeed *= m_SpeedMultiplier;

		XMFLOAT2 look{ 0, 0 };
		bool mouseMoved{ false };
		if (InputManager::IsMouseButton(InputState::down, VK_LBUTTON))
		{
			const auto& mouseMove = InputManager::GetMouseMovement();
			look.x = static_cast<float>(mouseMove.x);
			look.y = static_cast<float>(mouseMove.y);

			mouseMoved = mouseMove.x != 0 || mouseMove.y != 0;
		}

		if (!mouseMoved)
		{
			look = InputManager::GetThumbstickPosition(false);
		}

		//CALCULATE TRANSFORMS
		const auto forward = XMLoadFloat3(&GetTransform()->GetForward());
		const auto right = XMLoadFloat3(&GetTransform()->GetRight());
		auto currPos = XMLoadFloat3(&GetTransform()->GetPosition());
		const auto elapsedTime = sceneContext.pGameTime->GetElapsed();

		currPos += forward * move.y * currSpeed * elapsedTime;
		currPos += right * move.x * currSpeed * elapsedTime;

		m_TotalYaw += look.x * m_RotationSpeed * elapsedTime;
		m_TotalPitch += look.y * m_RotationSpeed * elapsedTime;

		GetTransform()->Rotate(m_TotalPitch, m_TotalYaw, 0);
		GetTransform()->Translate(currPos);
	}
}
