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
    unsigned char logToAllocate = firstBiggerLog(size);
    unsigned char freeChunk = firstFreeChunk(logToAllocate);
    size_t address = fragmentAndAllocate(freeChunk, logToAllocate);
    
}

void BuddyAllocator::Free(void *ptr)
<<<<<<< HEAD
{}
=======
{
    size_t address = (size_t)((unsigned char *)ptr - (unsigned char *)m_memory);
    size_t buddyAddress;
    while ((buddyAddress = findBuddy(address)) != address)
    {
        merge(address, buddyAddress);
    }
    deallocate(address, (((unsigned char *)m_memory)[address] & ~0x80) - intLog2(m_minimumSize));
}

void BuddyAllocator::Reset()
{
    m_freeBitmap = 1 << (m_possibleSizes - 1);
    ((unsigned char *)m_memory)[0] = 0x80 | intLog2(m_totalSize);
    for (auto &i : m_freeChunks)
        i.clear();
}

void BuddyAllocator::deallocate(size_t address, unsigned logSize)
{
    ((unsigned char *)m_memory)[address] = 0x80 | logSize + intLog2(m_minimumSize);
    m_freeChunks[logSize].emplace(address);
    m_freeBitmap |= 1 << (logSize);
}

void BuddyAllocator::merge(size_t &address, size_t buddyAddress)
{
    unsigned sizeLog = (((unsigned char *)m_memory)[buddyAddress] & ~0x80) - intLog2(m_minimumSize);
    auto buddy = m_freeChunks[sizeLog].find(buddyAddress);
    if (buddy != m_freeChunks[sizeLog].end())
    {
        m_freeChunks[sizeLog].erase(buddy);
        if (m_freeChunks[sizeLog].empty())
            m_freeBitmap &= ~(1 << sizeLog);
    }
    address = address < buddyAddress ? address : buddyAddress;
    ((unsigned char *)m_memory)[address] = 0x80 + sizeLog + 1 + intLog2(m_minimumSize);
}

size_t BuddyAllocator::findBuddy(size_t address)
{
    unsigned sizeLog = ((unsigned char *)m_memory)[address] & 0x7F;
    if (sizeLog == intLog2(m_totalSize))
        return address;
    unsigned buddyAddress = address ^ (1 << sizeLog);
    if (((unsigned char *)m_memory)[buddyAddress] == ((unsigned char *)m_memory)[address])
        return buddyAddress;
    else
        return address;
}
>>>>>>> df5122b41fcd6b0759e50bf16ceb4c43450a65df

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
