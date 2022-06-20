#include "stdafx.h"
#include "SoftwareSkinningScene_1.h"

#include "Materials/ColorMaterial.h"
#include "Prefabs/BoneObject.h"

void SoftwareSkinningScene_1::Initialize()
{
	auto* pBoneColor = MaterialManager::Get()->CreateMaterial<ColorMaterial>();
	pBoneColor->SetColor(XMFLOAT4{ Colors::Red });

	auto* pRoot = AddChild(new GameObject());
	m_pBone0 = new BoneObject(pBoneColor, 15.f);
	pRoot->AddChild(m_pBone0);

	m_pBone1 = new BoneObject(pBoneColor, 15.f);
	m_pBone0->AddBone(m_pBone1);

	GetSceneSettings().enableOnGUI = true;
}

void SoftwareSkinningScene_1::Update()
{
	if (m_AutoRotate)
	{
		// Calculate the bone rotation
		m_BoneAutoRotation += m_BoneRotationPerSec * GetSceneContext().pGameTime->GetElapsed() * m_RotationSign;

		// Adjust the rotation and flip RotationSign in case the MaxRotation has been reached
		if (m_RotationSign > 0)
		{
			if (m_BoneAutoRotation >= m_BoneMaxRotation)
			{
				m_BoneAutoRotation = m_BoneMaxRotation;
				m_RotationSign = -1;
			}
		}
		else
		{
			if (m_BoneAutoRotation <= -m_BoneMaxRotation)
			{
				m_BoneAutoRotation = -m_BoneMaxRotation;
				m_RotationSign = 1;
			}
		}

		// Rotate the bones
		m_Bone1Rotation = XMFLOAT3(0, 0, m_BoneAutoRotation);
		m_Bone2Rotation = XMFLOAT3(0, 0, -m_BoneAutoRotation * 2.f);
		m_pBone0->GetTransform()->Rotate(m_Bone1Rotation);
		m_pBone1->GetTransform()->Rotate(m_Bone2Rotation);
	}

}

void SoftwareSkinningScene_1::OnGUI()
{
	ImGui::SliderFloat3("Bone 0 - ROT", &m_Bone1Rotation.x, -359, 359);
	ImGui::SliderFloat3("Bone 1 - ROT", &m_Bone2Rotation.x, -359, 359);
	ImGui::Checkbox("Automatic Rotation", &m_AutoRotate);

	m_pBone0->GetTransform()->Rotate(m_Bone1Rotation);
	m_pBone1->GetTransform()->Rotate(m_Bone2Rotation);
}



