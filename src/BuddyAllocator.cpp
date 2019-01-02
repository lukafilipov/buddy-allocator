#include "BuddyAllocator.h"
#include "stdlib.h"
#include <exception>
#include <iostream>
#include <string>

using namespace std;

string get_bits(unsigned int x)
{
    string ret;
    for (unsigned int mask = 0x80000000; mask; mask >>= 1)
    {
        ret += (x & mask) ? "1" : "0";
    }
    return ret;
}

BuddyAllocator::BuddyAllocator(const size_t totalSize, const size_t minimumSize) : Allocator(totalSize),
                                                                                   m_minimumSize(minimumSize), m_possibleSizes(intLog2(totalSize) - intLog2(minimumSize) + 1),
                                                                                   m_freeBitmap(1 << (m_possibleSizes - 1)), m_freeChunks(m_possibleSizes)
{
    try
    {
        if (!isLog(totalSize) || !isLog(minimumSize))
            throw("Arguments must be a power of two");
        if (totalSize <= minimumSize)
            throw("MinimumSize must be smaller than TotalSize");
        if (minimumSize < 16)
            throw("MinimumSize must be at least 16");
    }
    catch (const char *exc)
    {
        cout << exc << endl;
        exit(1);
    }
}

BuddyAllocator::~BuddyAllocator()
{
    free(m_memory);
    m_memory = nullptr;
}

void BuddyAllocator::Init()
{

    if (m_memory != nullptr)
        Reset();
    else
        m_memory = malloc(m_totalSize);
    if (m_memory == nullptr)
    {
        cout << "Requested memory size is too big" << endl;
        exit(1);
    }
    ((unsigned char *)m_memory)[0] = 0x80 | intLog2(m_totalSize);
    m_freeChunks[m_possibleSizes - 1].emplace(0);
}

void *BuddyAllocator::Allocate(const size_t size, const std::size_t alignment)
{
    if (size == 0)
        return nullptr;
    unsigned logToAllocate = firstBiggerLog(size);
    if (m_freeBitmap >> logToAllocate == 0)
        return nullptr;
    unsigned index = firstFreeChunk(logToAllocate);
    
    size_t address = fragmentAndAllocate(index, logToAllocate);
    return (void *)((unsigned char *)m_memory + address);
}

void BuddyAllocator::Free(void *ptr)
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

size_t BuddyAllocator::fragmentAndAllocate(unsigned index, unsigned logToAllocate)
{
    while (index != logToAllocate)
    {
        size_t address = extractChunk(index--);
        putChunk(address, index);
        putChunk(address | (1 << (intLog2(m_minimumSize) + index)), index);
    }
    size_t ret = extractChunk(index);
    ((unsigned char *)m_memory)[ret] = index + intLog2(m_minimumSize);
    return ret;
}

size_t BuddyAllocator::extractChunk(unsigned index)
{
    auto address = m_freeChunks[index].begin();
    m_freeChunks[index].erase(address);
    if (m_freeChunks[index].size() == 0)
        m_freeBitmap &= ~(1 << index);
    return *address;
}

void BuddyAllocator::putChunk(size_t address, unsigned index)
{
    m_freeChunks[index].emplace(address);
    m_freeBitmap |= (1 << (index));
    ((unsigned char *)m_memory)[address] = 0x80 | index + intLog2(m_minimumSize);
}