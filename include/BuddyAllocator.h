#ifndef BUDDYALLOCATOR_H
#define BUDDYALLOCATOR_H

#include "Allocator.h"
#include <cmath>
#include <string>
class BuddyAllocator : public Allocator 
{
protected:
    void *m_data = nullptr;
	std::size_t m_minimumSize;
	unsigned char m_levels;
	
	std::size_t *m_freeLists;
	unsigned char *m_index;
	unsigned char *m_blockLevels;
	
	std::size_t m_sizeFreeLists;
	std::size_t m_sizeIndex;
	std::size_t m_sizeBlockLevels;
	std::size_t m_sizeMetadata;

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
	void error(std::string error)
	{
		std::cout << error << std::endl;
		exit(1);
	}
	void initializeSizes();
	void initializePointers();
};

#endif /* BUDDYALLOCATOR_H */
