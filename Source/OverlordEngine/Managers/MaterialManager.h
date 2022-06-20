#pragma once
class BaseMaterial;
class PostProcessingMaterial;

class MaterialManager final : public Singleton<MaterialManager>
{
public:
	MaterialManager(const MaterialManager& other) = delete;
	MaterialManager(MaterialManager&& other) noexcept = delete;
	MaterialManager& operator=(const MaterialManager& other) = delete;
	MaterialManager& operator=(MaterialManager&& other) noexcept = delete;

	template<typename T>
	std::enable_if<std::is_base_of_v<BaseMaterial, T>, T>::type*
	CreateMaterial();

	template<typename T>
	std::enable_if<std::is_base_of_v<PostProcessingMaterial, T>, T>::type*
	CreateMaterial();

	template<typename T>
	std::enable_if<std::is_base_of_v<BaseMaterial, T>, T>::type*
		GetMaterial(UINT materialId) const;

	template<typename T>
	std::enable_if<std::is_base_of_v<PostProcessingMaterial, T>, T>::type*
	GetMaterial(UINT materialId) const;

	BaseMaterial* GetMaterial(UINT materialId) const;

	void RemoveMaterial(UINT materialId, bool deleteObj = false);
	void RemoveMaterial(BaseMaterial* pMaterial, bool deleteObj = false);
	void RemoveMaterial(PostProcessingMaterial* pMaterial, bool deleteObj = false);

protected:
	void Initialize() override {};

private:
	friend Singleton<MaterialManager>;
	MaterialManager() = default;
	~MaterialManager();

	static constexpr UINT MATERIAL_PP_ID_OFFSET{ UINT_MAX / 2 };
	static bool IsValid(UINT id) { return id != UINT_MAX; }
	static bool IsBaseMaterial(UINT id) { return id < MATERIAL_PP_ID_OFFSET; }
	static bool IsPostProcessingMaterial(UINT id) { return id != UINT_MAX && id >= MATERIAL_PP_ID_OFFSET; }

	static UINT FromPPID(UINT id) { return id - MATERIAL_PP_ID_OFFSET; }
	static UINT ToPPID(UINT id) { return id + MATERIAL_PP_ID_OFFSET; }
	PostProcessingMaterial* GetMaterial_Post(UINT materialId) const;

	std::vector<BaseMaterial*> m_Materials{};
	std::vector<PostProcessingMaterial*> m_MaterialsPP{};
};

template <typename T>
std::enable_if<std::is_base_of_v<BaseMaterial, T>, T>::type*
MaterialManager::CreateMaterial()
{
	auto pMaterial = new T();

	UINT newMaterialId{ UINT_MAX};
	for (size_t i{ 0 }; i < m_Materials.size(); ++i)
	{
		if (m_Materials[i] == nullptr)
		{
			newMaterialId = UINT(i);
			break;
		}
	}

	if (newMaterialId == UINT_MAX)
	{
		newMaterialId = UINT(m_Materials.size());
		m_Materials.push_back(pMaterial);
	}
	else m_Materials[newMaterialId] = pMaterial;

	pMaterial->SetMaterialName(StringUtil::utf8_decode(typeid(T).name()));
	pMaterial->Initialize(m_GameContext.d3dContext, newMaterialId);

	return pMaterial;
}

template <typename T>
std::enable_if<std::is_base_of_v<PostProcessingMaterial, T>, T>::type*
MaterialManager::CreateMaterial()
{
	auto pMaterial = new T();

	UINT newMaterialId{ UINT_MAX };
	for (size_t i{ 0 }; i < m_MaterialsPP.size(); ++i)
	{
		if (m_MaterialsPP[i] == nullptr)
		{
			newMaterialId = UINT(i);
			break;
		}
	}

	if (newMaterialId == UINT_MAX)
	{
		newMaterialId = UINT(m_MaterialsPP.size());
		m_MaterialsPP.push_back(pMaterial);
	}
	else m_MaterialsPP[newMaterialId] = pMaterial;

	pMaterial->InitializeBase(m_GameContext, ToPPID(newMaterialId)); //Todo: Fix Virtual Overload Initialize

	return pMaterial;
}

template<typename T>
std::enable_if<std::is_base_of_v<BaseMaterial, T>, T>::type*
MaterialManager::GetMaterial(UINT materialId) const
{
	if(auto pBase = GetMaterial(materialId))
	{
		if(T * pDerived = dynamic_cast<T*>(pBase))
		{
			return pDerived;
		}
		
		HANDLE_ERROR(L"Failed to cast Material (BaseMaterial) with ID={} to \'{}\'", materialId, StringUtil::utf8_decode(typeid(T).name()));
	}

	return nullptr;
}

template<typename T>
std::enable_if<std::is_base_of_v<PostProcessingMaterial, T>, T>::type*
MaterialManager::GetMaterial(UINT materialId) const
{
	if (auto pBase = GetMaterial_Post(materialId))
	{
		if (T* pDerived = dynamic_cast<T*>(pBase))
		{
			return pDerived;
		}

		HANDLE_ERROR(L"Failed to cast Material (PostProcessingMaterial) with ID={} to \'{}\'", materialId, StringUtil::utf8_decode(typeid(T).name()));
	}

	return nullptr;
}

