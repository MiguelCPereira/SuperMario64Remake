#pragma once
#include "PhysX/PhysxProxy.h"
#include "Scenegraph/GameScene.h"

namespace GameSceneExt
{
	inline void CreatePhysXGroundPlane(const GameScene& scene, PxMaterial* pMaterial = nullptr)
	{
		const auto pActor = PxGetPhysics().createRigidStatic(PxTransform{ PxQuat{PxPiDivTwo, PxVec3{0.f,0.f,1.f}} });
		if (!pMaterial) pMaterial = PxGetPhysics().createMaterial(.5f, .5f, .5f);
		PxRigidActorExt::createExclusiveShape(*pActor, PxPlaneGeometry{}, *pMaterial);

		scene.GetPhysxProxy()->AddActor(*pActor);
	}
}

namespace StringUtil
{
	// Convert a wide Unicode string to an UTF8 string
	inline std::string utf8_encode(const std::wstring& wstr)
	{
		if (wstr.empty()) return {};

		const int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], int(wstr.size()), nullptr, 0, nullptr, nullptr);
		std::string strTo(size_needed, 0);
		WideCharToMultiByte(CP_UTF8, 0, &wstr[0], int(wstr.size()), &strTo[0], size_needed, nullptr, nullptr);
		return strTo;
	}

	// Convert an UTF8 string to a wide Unicode String
	inline std::wstring utf8_decode(const std::string& str)
	{
		if (str.empty()) return {};

		const int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], int(str.size()), nullptr, 0);
		std::wstring wstrTo(size_needed, 0);
		MultiByteToWideChar(CP_UTF8, 0, &str[0], int(str.size()), &wstrTo[0], size_needed);
		return wstrTo;
	}

	// Convert an UTF8 string to a wide Unicode String
	inline std::wstring utf8_decode(const char* str)
	{
		return utf8_decode(std::string{ str });
	}

	//inline std::wstring utf8_decode(LPCSTR str)
	//{
	//	return utf8_decode(std::string(str));
	//}
}

namespace ConvertUtil
{
	inline float* ToImFloatPtr(const XMFLOAT2& v)
	{
		return reinterpret_cast<float*>(const_cast<XMFLOAT2*>(&v));
	}

	inline float* ToImFloatPtr(const XMFLOAT3& v)
	{
		return reinterpret_cast<float*>(const_cast<XMFLOAT3*>(&v));
	}

	inline float* ToImFloatPtr(const XMFLOAT4& v)
	{
		return reinterpret_cast<float*>(const_cast<XMFLOAT4*>(&v));
	}
}

