#pragma once
#include <foundation/PxAllocatorCallback.h>

class PhysxAllocator final : public PxAllocatorCallback
{
public:
	void* allocate(size_t size, const char*, const char*, int) override
	{
		return _aligned_malloc(size, 16);
	}

	void deallocate(void* ptr) override
	{
		_aligned_free(ptr);
	}
};
