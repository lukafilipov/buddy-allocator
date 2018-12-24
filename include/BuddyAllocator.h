#ifndef BUDDYALLOCATOR_H
#define BUDDYALLOCATOR_H

#include "Allocator.h"
#include <cmath>
#include <vector>

class BuddyAllocator : public Allocator 
{
protected:
	std::size_t m_minimumSize;
	std::size_t m_logMinimumSize;
    void *m_memory = nullptr;
    std::vector<std::list<std::size_t>> m_freeChunks;
    std::size_t m_freeBitmap = 0;

public:
	BuddyAllocator(const std::size_t totalSize);

	virtual ~BuddyAllocator();

	virtual void* Allocate(const std::size_t size) override;
	
	virtual void Free(void* ptr) override;

	virtual void Init() override;

	virtual void Reset();
private:
	BuddyAllocator(BuddyAllocator &buddyAllocator);

    unsigned log2(double n)
	{
		return (reinterpret_cast<std::size_t&>(x) >> 52) - 1023;
	}
	bool isLog(std::size_t n)
	{
		return !(n & (n - 1));
	}
	unsigned firstBiggerLog(std::size_t size)
	{
		if(size <= minimumSize) return 0;
		return isLog(size) ? log2(size) - m_logMinimumSize
		 : log2(size) + 1 - m_logMinimumSize;
	}
	unsigned firstFreeChunk(unsigned size);
	{
		return __builtin_ctz(m_freeBitmap >> size);
	}
	std::size_t fragmentAndAllocate(unsigned index, unsigned logToAllocate);
	std::size_t extractChunk(unsigned index);
	std::size_t findAndRemoveBuddy(std::size_t address);
	void deallocate(std::size_t address);
	void merge(std::size_t address, std::size_t buddyAddress);


};

#endif /* BUDDYALLOCATOR_H */
