#include "stdafx.h"
#include "PureDiffuseMaterial.h"


PureDiffuseMaterial::PureDiffuseMaterial() :
	Material<PureDiffuseMaterial>(L"Effects/PureTex3D.fx")
{
}

void PureDiffuseMaterial::InitializeEffectVariables()
{
}

void PureDiffuseMaterial::SetDiffuseTexture(const std::wstring& assetFile)
{
	m_pDiffuseTexture = ContentManager::Load<TextureData>(assetFile);
	SetVariable_Texture(L"gDiffuseMap", m_pDiffuseTexture);
}
