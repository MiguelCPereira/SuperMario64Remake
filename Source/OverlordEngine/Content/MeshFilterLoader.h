#pragma once
class MeshFilterLoader : public ContentLoader<MeshFilter>
{
public:
	MeshFilterLoader() = default;
	~MeshFilterLoader() override = default;
	MeshFilterLoader(const MeshFilterLoader& other) = delete;
	MeshFilterLoader(MeshFilterLoader&& other) noexcept = delete;
	MeshFilterLoader& operator=(const MeshFilterLoader& other) = delete;
	MeshFilterLoader& operator=(MeshFilterLoader&& other) noexcept = delete;

protected:
	MeshFilter* LoadContent(const ContentLoadInfo& loadInfo) override;
	void Destroy(MeshFilter* objToDestroy) override;

private:
	static MeshFilter* ParseOVM11(BinaryReader* pReader);
	MeshFilter* ParseOVM20(BinaryReader* pReader);
};
