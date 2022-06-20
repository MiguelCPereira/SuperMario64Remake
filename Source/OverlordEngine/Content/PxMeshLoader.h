#pragma once

//CONVEX MESH
//***********
class PxConvexMeshLoader : public ContentLoader<PxConvexMesh>
{
public:
	PxConvexMeshLoader() = default;
	~PxConvexMeshLoader() override = default;

	PxConvexMeshLoader(const PxConvexMeshLoader& other) = delete;
	PxConvexMeshLoader(PxConvexMeshLoader&& other) noexcept = delete;
	PxConvexMeshLoader& operator=(const PxConvexMeshLoader& other) = delete;
	PxConvexMeshLoader& operator=(PxConvexMeshLoader&& other) noexcept = delete;

protected:
	PxConvexMesh* LoadContent(const ContentLoadInfo& loadInfo) override;
	void Destroy(PxConvexMesh* /*pObjToDestroy*/) override {};
};

//TRIANGLE MESH
//*************
class PxTriangleMeshLoader : public ContentLoader<PxTriangleMesh>
{
public:
	PxTriangleMeshLoader() = default;
	~PxTriangleMeshLoader() override = default;

	PxTriangleMeshLoader(const PxTriangleMeshLoader& other) = delete;
	PxTriangleMeshLoader(PxTriangleMeshLoader&& other) noexcept = delete;
	PxTriangleMeshLoader& operator=(const PxTriangleMeshLoader& other) = delete;
	PxTriangleMeshLoader& operator=(PxTriangleMeshLoader&& other) noexcept = delete;
protected:
	PxTriangleMesh* LoadContent(const ContentLoadInfo& loadInfo) override;
	void Destroy(PxTriangleMesh* /*pObjToDestroy*/) override {};
};

