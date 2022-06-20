#pragma once
#include <foundation/PxErrorCallback.h>

class PhysxErrorCallback final : public PxErrorCallback
{
public:
	void reportError(PxErrorCode::Enum code, const char* message, const char* file, int line) override;
};
