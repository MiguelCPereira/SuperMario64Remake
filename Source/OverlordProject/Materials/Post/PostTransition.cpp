//Resharper Disable All

#include "stdafx.h"
#include "PostTransition.h"

PostTransition::PostTransition() :
	PostProcessingMaterial(L"Effects/Post/Transition.fx")
{
}

void PostTransition::SetBgColor(const XMFLOAT3& color)
{
	m_BackgroundColor = color;
	m_pBaseEffect->GetVariableByName("gBackgroundColor")->AsVector()->SetFloatVector(&m_BackgroundColor.x);
}

void PostTransition::SetCircleRadius(float radius)
{
	m_CircleRadius = radius;
	m_pBaseEffect->GetVariableByName("gCircleRadius")->AsScalar()->SetFloat(m_CircleRadius);
}

void PostTransition::SetCircleCenterPos(const XMFLOAT2& screenPos)
{
	m_CircleCenterPos = screenPos;
	m_pBaseEffect->GetVariableByName("gCircleCenterPos")->AsVector()->SetFloatVector(&m_CircleCenterPos.x);
}


void PostTransition::SetScreenSize(const XMFLOAT2& screenSize)
{
	m_pBaseEffect->GetVariableByName("gScreenWidth")->AsScalar()->SetFloat(screenSize.x);
	m_pBaseEffect->GetVariableByName("gScreenHeight")->AsScalar()->SetFloat(screenSize.y);
}

void PostTransition::Initialize(const GameContext& gameContext)
{
	SetScreenSize(XMFLOAT2(float(gameContext.windowWidth), float(gameContext.windowHeight)));
}

