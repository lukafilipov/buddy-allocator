#ifndef BUDDYALLOCATOR_H
#define BUDDYALLOCATOR_H

#include "Allocator.h"
#include <cmath>
#include <vector>
#include <unordered_set>
class BuddyAllocator : public Allocator 
{
protected:
	std::size_t m_minimumSize;
    void *m_memory = nullptr;
	unsigned m_possibleSizes;
    std::vector<std::unordered_set<std::size_t>> m_freeChunks;
    std::size_t m_freeBitmap = 0;

public:
	BuddyAllocator(const std::size_t totalSize, const std::size_t minimumSize = 16);

	virtual ~BuddyAllocator();

	virtual void* Allocate(const std::size_t size, const std::size_t alignment = 0) override;
	
	virtual void Free(void* ptr) override;

	virtual void Init() override;

	virtual void Reset();
private:
	BuddyAllocator(BuddyAllocator &buddyAllocator);

    unsigned intLog2(double n)
	{
		return (reinterpret_cast<std::size_t&>(n) >> 52) - 1023;
	}
	bool isLog(std::size_t n)
	{
		return !(n & (n - 1));
	}
	unsigned firstBiggerLog(std::size_t size)
	{
		if(size <= m_minimumSize) return 0;
		return isLog(size) ? intLog2(size) - intLog2(m_minimumSize)
		 : intLog2(size) + 1 - intLog2(m_minimumSize);
	}
	unsigned firstFreeChunk(unsigned size)
	{
		return __builtin_ctz(m_freeBitmap >> size) + size;
	}
	std::size_t fragmentAndAllocate(unsigned index, unsigned logToAllocate);
	std::size_t extractChunk(unsigned index);
	std::size_t findAndRemoveBuddy(std::size_t address);
	std::size_t findBuddy(std::size_t address);
	void deallocate(std::size_t address, unsigned index);
	void merge(std::size_t &address, std::size_t buddyAddress);
	void putChunk(std::size_t address, unsigned index);


};

#endif /* BUDDYALLOCATOR_H */
