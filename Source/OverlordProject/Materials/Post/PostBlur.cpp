//Resharper Disable All

	#include "stdafx.h"
	#include "PostBlur.h"

	PostBlur::PostBlur():
		PostProcessingMaterial(L"Effects/Post/Blur.fx")
	{
	}

	void PostBlur::SetIntensity(float intensity)
	{
		m_Intensity = intensity;
		m_pBaseEffect->GetVariableByName("gIntensity")->AsScalar()->SetFloat(m_Intensity);
	}


