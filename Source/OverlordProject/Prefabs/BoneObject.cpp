#include "stdafx.h"
#include "BoneObject.h"

BoneObject::BoneObject(BaseMaterial* pMaterial, float length)
	: m_Length(length)
	, m_pBaseMaterial(pMaterial)
{
}

void BoneObject::AddBone(BoneObject* pBone)
{
	pBone->GetTransform()->Translate(m_Length, 0, 0);
	this->AddChild(pBone);
	pBone->CalculateBindPose();
}

void BoneObject::CalculateBindPose()
{
	const auto worldMatrix = XMLoadFloat4x4(&GetTransform()->GetWorld());
	const auto invWorldMatrix = XMMatrixInverse(nullptr, worldMatrix);
	XMStoreFloat4x4(&m_BindPose, invWorldMatrix);
}


void BoneObject::Initialize(const SceneContext&)
{
	auto* pEmpty = new GameObject();
	this->AddChild(pEmpty);
	auto* pModelComp = new ModelComponent(L"Meshes/Bone.ovm");
	pEmpty->AddComponent(pModelComp);
	pModelComp->SetMaterial(m_pBaseMaterial);
	pEmpty->GetTransform()->Rotate(0, -90, 0);
	pEmpty->GetTransform()->Scale(m_Length);
}


