#include "stdafx.h"
#include "PxMeshLoader.h"

//CONVEX MESH
//***********
PxConvexMesh* PxConvexMeshLoader::LoadContent(const ContentLoadInfo& loadInfo)
{
	//std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	//std::string buffer = converter.to_bytes(assetPath.c_str());
	////std::string buffer = std::string(assetFile.begin(), assetFile.end());

	const auto utf8_assetPath = StringUtil::utf8_encode(loadInfo.assetFullPath);
	auto inputStream  = PxDefaultFileInputData(utf8_assetPath.c_str());
	return PxGetPhysics().createConvexMesh(inputStream);
}

//TRIANGLE MESH
//*************
PxTriangleMesh* PxTriangleMeshLoader::LoadContent(const ContentLoadInfo& loadInfo)
{
	//std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	//std::string buffer = converter.to_bytes(assetPath.c_str());
	////std::string buffer =std::string(assetFile.begin(), assetFile.end());
	const auto utf8_assetPath = StringUtil::utf8_encode(loadInfo.assetFullPath);

	auto inputStream = PxDefaultFileInputData(utf8_assetPath.c_str());
	return PxGetPhysics().createTriangleMesh(inputStream);
}