#include "stdafx.h"
#include "FixedCamera.h"

void FixedCamera::Initialize(const SceneContext& )
{
	AddComponent(new CameraComponent());
}

void FixedCamera::SetRotation(float pitch, float yaw)
{
	m_Pitch = pitch;
	m_Yaw = yaw;
	m_RotationChanged = true;
}

void FixedCamera::Update(const SceneContext&)
{
	if (m_RotationChanged)
	{
		GetTransform()->Rotate(m_Pitch, m_Yaw, 0);
		m_RotationChanged = false;
	}
}


