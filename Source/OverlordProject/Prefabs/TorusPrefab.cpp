#include "stdafx.h"
#include "TorusPrefab.h"

TorusPrefab::TorusPrefab(float majorRadius, unsigned int majorRadiusSteps, float minorRadius, unsigned int minorRadiusSteps,
                         XMFLOAT4 color):
	m_MajorRadius(majorRadius),
	m_MinorRadius(minorRadius),
	m_MajorRadiusSteps(majorRadiusSteps),
	m_MinorRadiusSteps(minorRadiusSteps),
	m_Color(color)
{
}

void TorusPrefab::Initialize(const SceneContext&)
{
	const auto vertCount = m_MajorRadiusSteps * m_MinorRadiusSteps;
	const auto pMesh = new MeshIndexedDrawComponent(vertCount, vertCount * 6);

	//Vertices
	const float majorInterval = XM_2PI / m_MajorRadiusSteps;
	const float minorInterval = XM_2PI / m_MinorRadiusSteps;
	for (unsigned int majorStep = 0; majorStep < m_MajorRadiusSteps; ++majorStep)
	{
		const float torusRadius = majorStep * majorInterval;

		for (unsigned int minorStep = 0; minorStep < m_MinorRadiusSteps; ++minorStep)
		{
			const float circleRadius = minorStep * minorInterval;
			XMFLOAT3 pos{};
			pos.x = (m_MajorRadius + (m_MinorRadius * cos(circleRadius))) * cos(torusRadius);
			pos.y = (m_MajorRadius + (m_MinorRadius * cos(circleRadius))) * sin(torusRadius);
			pos.z = m_MinorRadius * sin(circleRadius);

			//Normal
			XMFLOAT3 middle{};
			middle.x = m_MajorRadius * cos(torusRadius);
			middle.y = m_MajorRadius * sin(torusRadius);
			middle.z = 0;

			const XMVECTOR vecPos = XMLoadFloat3(&pos);
			const XMVECTOR vecMiddle = XMLoadFloat3(&middle);
			const XMVECTOR vecNormal = XMVector3Normalize(XMVectorSubtract(vecPos, vecMiddle));
			XMFLOAT3 normal{};
			XMStoreFloat3(&normal, vecNormal);

			pMesh->AddVertex(VertexPosNormCol(pos, normal, m_Color));
		}
	}

	//Indices
	for (unsigned int i = 0; i < vertCount; ++i)
	{
		const auto v0 = i;
		const auto v1 = (v0 + m_MinorRadiusSteps) % vertCount;
		auto v2 = v1 + 1;
		auto v3 = i + 1;

		if ((i + 1) % m_MinorRadiusSteps == 0)
		{
			v2 -= m_MinorRadiusSteps;
			v3 -= m_MinorRadiusSteps;
		}

		pMesh->AddIndex(v0);
		pMesh->AddIndex(v1);
		pMesh->AddIndex(v2);
		pMesh->AddIndex(v2);
		pMesh->AddIndex(v3);
		pMesh->AddIndex(v0);
	}

	AddComponent(pMesh);
}
