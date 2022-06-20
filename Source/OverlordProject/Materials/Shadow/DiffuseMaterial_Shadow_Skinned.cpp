#include "stdafx.h"
#include "DiffuseMaterial_Shadow_Skinned.h"

DiffuseMaterial_Shadow_Skinned::DiffuseMaterial_Shadow_Skinned():
	Material(L"Effects/Shadow/PosNormTex3D_Shadow_Skinned.fx")
{}

void DiffuseMaterial_Shadow_Skinned::SetDiffuseTexture(const std::wstring& assetFile)
{
	m_pDiffuseTexture = ContentManager::Load<TextureData>(assetFile);
	SetVariable_Texture(L"gDiffuseMap", m_pDiffuseTexture);
}

void DiffuseMaterial_Shadow_Skinned::InitializeEffectVariables()
{
}

void DiffuseMaterial_Shadow_Skinned::OnUpdateModelVariables(const SceneContext& sceneContext, const ModelComponent* pModel) const
{
	// TODO_W8 - Update The Shader Variables

	// Retrieve The Animator from the ModelComponent
	auto* animator = pModel->GetAnimator();

	// Make sure the animator is not NULL (ASSERT_NULL_)
	if (animator == nullptr)
	{
		Logger::LogError(L"DiffuseMaterial_Shadow_Skinned.cpp::OnUpdateModelVariables > the given ModelComponent doesn't have an animator");
		return;
	}

	// 1. Update the LightWVP > Used to Transform a vertex into Light clipping space
	// LightWVP = model_world * light_viewprojection
	// (light_viewprojection [LightVP] can be acquired from the ShadowMapRenderer)
	const auto* pShadowMapRenderer = ShadowMapRenderer::Get();
	XMMATRIX world = XMLoadFloat4x4(&pModel->GetTransform()->GetWorld());
	XMMATRIX lightVP = XMLoadFloat4x4(&pShadowMapRenderer->GetLightVP());
	const auto lightWVP = XMMatrixMultiply(world, lightVP);
	XMFLOAT4X4 lightWVPFloat;
	XMStoreFloat4x4(&lightWVPFloat, lightWVP);
	SetVariable_Matrix(L"gWorldViewProj_Light", lightWVPFloat);
	
	// 2. Update the ShadowMap texture
	SetVariable_Texture(L"gShadowMap", pShadowMapRenderer->GetShadowMap());

	// 3. Update the Light Direction (retrieve the direction from the LightManager > sceneContext)
	SetVariable_Vector(L"gLightDirection", sceneContext.pLights->GetDirectionalLight().direction);

	// 4. Update Bones
	const auto& boneTransf = animator->GetBoneTransforms();
	SetVariable_MatrixArray(L"gBones", &boneTransf[0]._11, static_cast<UINT>(boneTransf.size()));
}
