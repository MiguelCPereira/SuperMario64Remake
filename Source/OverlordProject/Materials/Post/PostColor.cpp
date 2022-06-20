//Resharper Disable All

#include "stdafx.h"
#include "PostColor.h"

PostColor::PostColor() :
	PostProcessingMaterial(L"Effects/Post/Color.fx")
{
}

void PostColor::SetColor(const XMFLOAT3& color)
{
	m_Color = color;
	m_pBaseEffect->GetVariableByName("gColor")->AsVector()->SetFloatVector(&m_Color.x);
}

void PostColor::SetIntensity(float intensity)
{
	m_Intensity = intensity;
	m_pBaseEffect->GetVariableByName("gIntensity")->AsScalar()->SetFloat(m_Intensity);
}


