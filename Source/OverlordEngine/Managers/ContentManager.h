#pragma once
#include "Content/ContentLoader.h"

class ContentManager
{
public:
	ContentManager(const ContentManager& other) = delete;
	ContentManager(ContentManager&& other) noexcept = delete;
	ContentManager& operator=(const ContentManager& other) = delete;
	ContentManager& operator=(ContentManager&& other) noexcept = delete;

	static void Initialize(const GameContext& gameContext);
	static void AddLoader(BaseLoader* loader);
	static fs::path GetFullAssetPath(const std::wstring& subPath);

	template<class T> 
	static T* Load(const std::wstring& assetFile, void* pUserData = nullptr, const std::source_location& location = std::source_location::current())
	{
		const type_info& ti = typeid(T);
		for(BaseLoader* loader:m_Loaders)
		{
			const type_info& loadertype = loader->GetType();
			if(loadertype == ti)
			{
				const auto fullPath = GetFullAssetPath(assetFile);
				ASSERT_IF(!fs::exists(fullPath), LogString(L"File not found!\n\nAsset: {}\n\nFull Path: {}", location), assetFile, fullPath.wstring())

				return (static_cast<ContentLoader<T>*>(loader))->GetContent({ fullPath, assetFile, pUserData });
			}
		}

		return nullptr;
	}

	static void Release();

private:
	ContentManager() = default;
	~ContentManager() = default;

	static std::vector<BaseLoader*> m_Loaders;
	static GameContext m_GameContext;
	static bool m_IsInitialized;
};

