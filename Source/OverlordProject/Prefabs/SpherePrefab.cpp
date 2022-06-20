#include "stdafx.h"
#include "SpherePrefab.h"

SpherePrefab::SpherePrefab(float radius, int steps, XMFLOAT4 color) :
	m_Radius(radius),
	m_Steps(steps),
	m_Color(color)
{
}

void SpherePrefab::Initialize(const SceneContext& )
{
	const auto vertCount = m_Steps * (m_Steps - 1) + 2;
	const auto pMesh = new MeshIndexedDrawComponent(vertCount, (m_Steps - 2) * m_Steps * 6 + 6 * m_Steps);

	//Vertices
	const float deltaTheta = XM_PI / m_Steps;
	const float deltaPhi = XM_2PI / m_Steps;
	float theta = 0;
	float phi = 0;

	//TOP
	pMesh->AddVertex(VertexPosNormCol(XMFLOAT3(0, m_Radius, 0), XMFLOAT3(0, 1, 0), m_Color));

	//SPHERE
	for (auto i = 0; i < m_Steps - 1; ++i)
	{
		theta += deltaTheta;
		for (auto j = 0; j < m_Steps; ++j)
		{
			phi += deltaPhi;
			XMFLOAT3 pos;
			pos.x = m_Radius * sin(theta) * cos(phi);
			pos.z = m_Radius * sin(theta) * sin(phi);
			pos.y = m_Radius * cos(theta);

			const XMVECTOR vPos = XMLoadFloat3(&pos);
			XMFLOAT3 normal;
			XMStoreFloat3(&normal, XMVector3Normalize(vPos));

			pMesh->AddVertex(VertexPosNormCol(pos, normal, m_Color));
		}
	}

	//BOTTOM
	pMesh->AddVertex(VertexPosNormCol(XMFLOAT3(0, -m_Radius, 0), XMFLOAT3(0, -1, 0), m_Color));

	//Indices
	//TOP
	for (auto i = 1; i < m_Steps + 1; ++i)
	{
		pMesh->AddIndex(i);

		auto v1 = i + 1;
		if (i % m_Steps == 0)
			v1 -= m_Steps;

		pMesh->AddIndex(v1);
		pMesh->AddIndex(0);
	}

	//MIDDLE
	for (auto i = 1; i < vertCount - 1 - m_Steps; ++i)
	{
		const auto v0 = i;
		auto v1 = i + 1;

		if (i % m_Steps == 0)
			v1 -= m_Steps;

		const auto v2 = v1 + m_Steps;
		const auto v3 = v0 + m_Steps;

		pMesh->AddIndex(v0);
		pMesh->AddIndex(v1);
		pMesh->AddIndex(v2);
		pMesh->AddIndex(v2);
		pMesh->AddIndex(v3);
		pMesh->AddIndex(v0);
	}

	//BOTTOM
	for (auto i = vertCount - m_Steps - 1; i < vertCount - 1; ++i)
	{
		pMesh->AddIndex(i);

		auto v1 = i + 1;
		if (i % m_Steps == 0)
			v1 -= m_Steps;

		pMesh->AddIndex(v1);
		pMesh->AddIndex(vertCount - 1);
	}


	AddComponent(pMesh);
}
