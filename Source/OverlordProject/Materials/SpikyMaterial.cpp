#include "stdafx.h"
#include "SpikyMaterial.h"

SpikyMaterial::SpikyMaterial() :
	Material<SpikyMaterial>(L"Effects/SpikyShader.fx")
{
}

void SpikyMaterial::InitializeEffectVariables()
{
}

void SpikyMaterial::SetDiffuseColor(XMFLOAT4 diffuseColor) const
{
	SetVariable_Vector(L"gColorDiffuse", diffuseColor);
}
