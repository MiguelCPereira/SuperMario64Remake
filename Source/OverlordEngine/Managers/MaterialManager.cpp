#include "stdafx.h"
#include "MaterialManager.h"

MaterialManager::~MaterialManager()
{
	//Delete Model Materials
	for(auto& pMaterial:m_Materials)
	{
		SafeDelete(pMaterial);
	}

	m_Materials.clear();

	//Delete PostProcessing Materials
	for (auto& pMaterial : m_MaterialsPP)
	{
		SafeDelete(pMaterial);
	}

	m_MaterialsPP.clear();
}

BaseMaterial* MaterialManager::GetMaterial(UINT materialId) const
{
	BaseMaterial* pBase{};
	if (IsBaseMaterial(materialId))
	{
		if (materialId < m_Materials.size())
		{
			pBase = m_Materials[materialId];
			return pBase;
		}
	}

	if (pBase == nullptr)
	{
		Logger::LogWarning(L"Material with ID={} not found", materialId);
	}

	return nullptr;
}

PostProcessingMaterial* MaterialManager::GetMaterial_Post(UINT materialId) const
{
	PostProcessingMaterial* pBase{};
	if (IsPostProcessingMaterial(materialId))
	{
		materialId = FromPPID(materialId);
		if (materialId < m_MaterialsPP.size())
		{
			pBase = m_MaterialsPP[materialId];
			return pBase;
		}
	}

	if (pBase == nullptr)
	{
		Logger::LogWarning(L"Material with ID={} not found", materialId);
	}

	return nullptr;
}

void MaterialManager::RemoveMaterial(UINT materialId, bool deleteObj)
{
	//Valid Material id?
	if (!IsValid(materialId))
	{
		Logger::LogWarning(L"MaterialManager::RemoveMaterial >> Invalid Material ID");
		return;
	}

	if(IsBaseMaterial(materialId))
	{
		if (m_Materials.size() < materialId || !m_Materials[materialId])
		{
			Logger::LogWarning(L"MaterialManager::RemoveMaterial >> Invalid Material ID");
			return;
		}

		m_Materials[materialId]->ResetMaterialId();

		if (deleteObj)
		{
			delete m_Materials[materialId];
		}

		m_Materials[materialId] = nullptr;
	}
	else
	{
		materialId = FromPPID(materialId);
		if (m_MaterialsPP.size() < materialId || !m_MaterialsPP[materialId])
		{
			Logger::LogWarning(L"MaterialManager::RemoveMaterial >> Invalid Material ID");
			return;
		}

		m_MaterialsPP[materialId]->ResetMaterialId();

		if (deleteObj)
		{
			delete m_MaterialsPP[materialId];
		}

		m_MaterialsPP[materialId] = nullptr;
	}
	
}

void MaterialManager::RemoveMaterial(BaseMaterial* pMaterial, bool deleteObj)
{
	const auto materialId = pMaterial->GetMaterialId();
	if (!IsValid(materialId) || m_Materials.size() < materialId || !m_Materials[materialId])
	{
		Logger::LogWarning(L"MaterialManager::RemoveMaterial >> Invalid Material ID");
		return;
	}

	if(m_Materials[materialId] == pMaterial)
	{
		RemoveMaterial(materialId, deleteObj);
	}
}

void MaterialManager::RemoveMaterial(PostProcessingMaterial* pMaterial, bool deleteObj)
{
	const auto materialId = pMaterial->GetMaterialId();
	if (!IsValid(materialId) || m_MaterialsPP.size() < materialId || !m_MaterialsPP[materialId])
	{
		Logger::LogWarning(L"MaterialManager::RemoveMaterial >> Invalid Material ID");
		return;
	}

	if (m_MaterialsPP[materialId] == pMaterial)
	{
		RemoveMaterial(materialId, deleteObj);
	}
}
