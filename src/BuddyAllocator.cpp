#include "BuddyAllocator.h"
#include "stdlib.h"
#include <exception>
#include <iostream>
#include <cstring>

using namespace std;

BuddyAllocator::BuddyAllocator(const size_t totalSize, const size_t minimumSize) : 
Allocator(totalSize), m_minimumSize(minimumSize), 
m_levels(intLog2(totalSize) - intLog2(minimumSize) + 1)
{
    if (!isLog(totalSize) || !isLog(minimumSize)) 
        error("Arguments must be a power of two");
    if (totalSize <= minimumSize)
        error("MinimumSize must be smaller than TotalSize");
    if (minimumSize < 16)
        error("MinimumSize must be at least 16");
    initializeSizes();
}

BuddyAllocator::~BuddyAllocator()
{
    free(m_data);
    m_data = nullptr;
}

void BuddyAllocator::Init()
{
    if (m_data == nullptr)
        m_data = malloc(m_totalSize + m_sizeMetadata);
    if (m_data == nullptr)
        error("Requested size is too big");
    Reset();
}

void BuddyAllocator::Reset()
{
    initializePointers();
    m_freeLists[0] = (size_t)m_data;
}

void *BuddyAllocator::Allocate(const size_t size, const std::size_t alignment)
{
    if(size == 0)
        return nullptr;
    unsigned char levelToAllocate = firstBiggerLog(size);
    unsigned char freeLevel = firstFreeLevel(levelToAllocate);
    if(freeLevel == 0xFF)
        return nullptr;
    size_t address = fragmentAndAllocate(freeLevel, levelToAllocate);
    
}

void BuddyAllocator::Free(void *ptr)
{}

void BuddyAllocator::initializeSizes()
{
    m_sizeFreeLists = m_levels * sizeof(size_t);
    m_sizeIndex = (((1 << m_levels) - 1) / 8 + 1) * sizeof(unsigned char);
    m_sizeBlockLevels = ((1 << (m_levels - 1)) + 1) * sizeof(unsigned char);
    m_sizeMetadata = m_sizeIndex + m_sizeFreeLists + m_sizeBlockLevels;
}

void BuddyAllocator::initializePointers()
{
    m_freeLists = (size_t*)m_data;
    m_index = (unsigned char*)m_data + m_sizeFreeLists;
    m_blockLevels = (unsigned char*)m_data + m_sizeFreeLists + m_sizeIndex;
    memset(m_data, 0, m_sizeMetadata);
    m_data = (unsigned char*)m_data + m_sizeMetadata;
}

unsigned char BuddyAllocator::firstFreeLevel(unsigned char levelToAllocate)
{
    for(unsigned char i = levelToAllocate; i >= 0; i--)
    {
        if(m_freeLists[i] != 0)
            return m_freeLists[i];
    }
    return 0xFF;
}

void *BuddyAllocator::fragmentAndAllocate(unsigned char freeLevel, unsigned char levelToAllocate)
{
    while (freeLevel != levelToAllocate)
    {
        size_t address = getBlock(freeLevel--);
        putBlock(address, freeLevel);
        putBlock(address | (1 << (intLog2(m_minimumSize) + freeLevel)), freeLevel);
    }
    size_t ret = getBlock(freeLevel);
    return (void*)ret;
}

size_t BuddyAllocator::getBlock(unsigned char freeLevel)
{

}

void BuddyAllocator::putBlock(size_t address, unsigned char freeLevel)
{

}