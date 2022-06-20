#pragma once
struct ContentLoadInfo
{
	fs::path assetFullPath{};
	std::wstring assetSubPath{};
	void* pUserData{};
};

class BaseLoader
{
public:
	BaseLoader(const BaseLoader& other) = delete;
	BaseLoader(BaseLoader&& other) noexcept = delete;
	BaseLoader& operator=(const BaseLoader& other) = delete;
	BaseLoader& operator=(BaseLoader&& other) noexcept = delete;
	BaseLoader() = default;
	virtual ~BaseLoader() = default;

	virtual const type_info& GetType() const = 0;
	virtual void Unload() = 0;
	void Initialize(const GameContext& gameContext) { m_GameContext = gameContext; }

protected:
	GameContext m_GameContext{};
};

template <class T>
class ContentLoader : public BaseLoader
{
public:
	ContentLoader(const ContentLoader& other) = delete;
	ContentLoader(ContentLoader&& other) noexcept = delete;
	ContentLoader& operator=(const ContentLoader& other) = delete;
	ContentLoader& operator=(ContentLoader&& other) noexcept = delete;
	ContentLoader();
	~ContentLoader() override = default;

	const type_info& GetType() const override { return typeid(T); }
	T* GetContent(const ContentLoadInfo& loadInfo);
	void Unload() override;

protected:
	virtual T* LoadContent(const ContentLoadInfo& loadInfo) = 0;
	virtual void Destroy(T* objToDestroy) = 0;

private:
	static std::unordered_map<size_t, T*> m_ContentReferences;
	static int m_LoaderReferences;
};

template <class T>
ContentLoader<T>::ContentLoader()
{
	++m_LoaderReferences;
}

template <class T>
T* ContentLoader<T>::GetContent(const ContentLoadInfo& loadInfo)
{
	size_t pathHash = fs::hash_value(loadInfo.assetSubPath);
	auto it = m_ContentReferences.find(pathHash);
	if(it != m_ContentReferences.end())
	{
		return (* it).second;
	}

	T* content = LoadContent(loadInfo);
	if (content != nullptr)m_ContentReferences.insert(std::pair<size_t, T*>(pathHash, content));

	return content;
}

#pragma warning(push)
#pragma warning(disable:4505)
template <class T>
void ContentLoader<T>::Unload()
{
	--m_LoaderReferences;

	if (m_LoaderReferences <= 0)
	{
		for (std::pair<size_t, T*> kvp : m_ContentReferences)
		{
			Destroy(kvp.second);
		}

		m_ContentReferences.clear();
	}
}
#pragma warning(pop)

template <class T>
std::unordered_map<size_t, T*> ContentLoader<T>::m_ContentReferences = {};

template <class T>
int ContentLoader<T>::m_LoaderReferences = {};
