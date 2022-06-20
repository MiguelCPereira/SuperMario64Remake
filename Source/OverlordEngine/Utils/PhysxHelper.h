#pragma once

namespace PhysxHelper
{
	inline PxVec3 ToPxVec3(const XMFLOAT3& vec)
	{
		return { vec.x, vec.y, vec.z };
	}

	inline PxExtendedVec3 ToPxExtendedVec3(const XMFLOAT3& vec)
	{
		return { vec.x, vec.y, vec.z };
	}

	inline PxVec2 ToPxVec2(const XMFLOAT2& vec)
	{
		return { vec.x, vec.y };
	}

	inline PxVec4 ToPxVec4(const XMFLOAT4& vec)
	{
		return { vec.x, vec.y, vec.z, vec.w };
	}

	inline PxQuat ToPxQuat(const XMFLOAT4& vec)
	{
		return { vec.x, vec.y, vec.z, vec.w };
	}
	inline XMFLOAT3 ToXMFLOAT3(const PxVec3& vec)
	{
		return { vec.x, vec.y, vec.z };
	}

	inline XMFLOAT3 ToXMFLOAT3(const PxExtendedVec3& vec)
	{
		return { static_cast<float>(vec.x), static_cast<float>(vec.y), static_cast<float>(vec.z) };
	}

	inline XMFLOAT4 ColorToXMFLOAT4(PxU32 color)
	{
		float r = static_cast<UCHAR>(color >> 16) / 255.f;
		float g = static_cast<UCHAR>(color >> 8) / 255.f;
		float b = static_cast<UCHAR>(color) / 255.f;

		return { r,g,b,1.f };
	}
}