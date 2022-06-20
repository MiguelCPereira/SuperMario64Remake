#include "stdafx.h"
#include "LightManager.h"

LightManager::LightManager()
{
	m_DirectionalLight.direction = { -0.577f, -0.577f, 0.577f , 1.0f };
	m_DirectionalLight.color = { 0.9f, 0.9f, 0.8f, 1.0f };
}

LightManager::~LightManager()
{
	m_Lights.clear();
}

void LightManager::SetDirectionalLight(const XMFLOAT3& position, const XMFLOAT3& direction)
{
	m_DirectionalLight.direction = { direction.x, direction.y, direction.z , 1.0f };
	m_DirectionalLight.position = { position.x, position.y, position.z , 1.0f };
}
