#include "stdafx.h"
#include "UberMaterial.h"

UberMaterial::UberMaterial() :
	Material<UberMaterial>(L"Effects/UberShader.fx")
{
}

void UberMaterial::InitializeEffectVariables()
{
}

void UberMaterial::SetDiffuseTexture(const std::wstring& assetFile, XMFLOAT4 diffuseColor)
{
	m_pDiffuseTexture = ContentManager::Load<TextureData>(assetFile);
	SetVariable_Texture(L"gTextureDiffuse", m_pDiffuseTexture);
	SetVariable_Scalar(L"gUseTextureDiffuse", true);
	SetVariable_Vector(L"gColorDiffuse", diffuseColor);
}

void UberMaterial::SetNormalTexture(const std::wstring& assetFile)
{
	m_pNormalTexture = ContentManager::Load<TextureData>(assetFile);
	SetVariable_Texture(L"gTextureNormal", m_pNormalTexture);
	SetVariable_Scalar(L"gUseTextureNormal", true);
	SetVariable_Scalar(L"gFlipGreenChannel", false);
}

void UberMaterial::SetCubemapTexture(const std::wstring& assetFile, float reflectionStrength, float refractionStrength, float fresnelHardness)
{
	m_pCubemapTexture = ContentManager::Load<TextureData>(assetFile);
	SetVariable_Texture(L"gCubeEnvironment", m_pCubemapTexture);
	SetVariable_Scalar(L"gUseEnvironmentMapping", true);
	SetVariable_Scalar(L"gUseFresnelFalloff", true);
	SetVariable_Scalar(L"gFresnelHardness", fresnelHardness);
	SetVariable_Scalar(L"gReflectionStrength", reflectionStrength);
	SetVariable_Scalar(L"gRefractionStrength", refractionStrength);
}

void UberMaterial::SetSpecularTexture(const std::wstring& assetFile, bool usePhong, bool useBlinn)
{
	m_pSpecularTexture = ContentManager::Load<TextureData>(assetFile);
	SetVariable_Texture(L"gTextureSpecularIntensity", m_pSpecularTexture);
	SetVariable_Scalar(L"gUseTextureSpecularIntensity", true);
	SetVariable_Scalar(L"gUseSpecularPhong", usePhong);
	SetVariable_Scalar(L"gUseSpecularBlinn", useBlinn);
}

void UberMaterial::SetOpacityTexture(const std::wstring& assetFile)
{
	m_pOpacityTexture = ContentManager::Load<TextureData>(assetFile);
	SetVariable_Texture(L"gTextureOpacity", m_pOpacityTexture);
	SetVariable_Scalar(L"gTextureOpacityIntensity", true);
}



