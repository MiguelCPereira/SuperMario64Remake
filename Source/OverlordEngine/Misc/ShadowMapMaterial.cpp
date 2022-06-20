#include "stdafx.h"
#include "ShadowMapMaterial.h"

ShadowMapMaterial::ShadowMapMaterial():
	Material(L"Effects/Shadow/ShadowMapGenerator.fx")
{
}

void ShadowMapMaterial::InitializeEffectVariables()
{
}
