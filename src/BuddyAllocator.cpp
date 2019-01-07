#include "BuddyAllocator.h"
#include "stdlib.h"
#include <exception>
#include <iostream>
#include <cstring>

using namespace std;

BuddyAllocator::BuddyAllocator(const size_t totalSize, const size_t minimumSize) : Allocator(totalSize), m_minimumSize(minimumSize),
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
}

void *BuddyAllocator::Allocate(const size_t size, const std::size_t alignment)
{
    if (size == 0)
        return nullptr;
    byte levelToAllocate = firstBiggerLog(size);
    byte level = firstFreeLevel(levelToAllocate);
    if (level == 0xFF)
        return nullptr;
    void *address = fragmentAndAllocate(level, levelToAllocate);
}

void BuddyAllocator::Free(void *ptr)
{
    size_t address = (size_t)((byte *)ptr - (byte *)m_data);
    size_t buddyAddress;
    while ((buddyAddress = findFreeBuddy(address)) != address)
        merge(address, buddyAddress);
    putBlock(address, m_blockLevels[address / m_minimumSize]);
}

void BuddyAllocator::initializeSizes()
{
    m_sizeFreeLists = m_levels * sizeof(size_t);
    m_sizeIndex = (((1 << m_levels) - 1) / 8 + 1) * sizeof(byte);
    m_sizeBlockLevels = ((1 << (m_levels - 1)) + 1) * sizeof(byte);
    m_sizeMetadata = m_sizeIndex + m_sizeFreeLists + m_sizeBlockLevels;
    m_sizeMetadata += sizeof(size_t) - m_sizeMetadata % sizeof(size_t);
}

void BuddyAllocator::initializePointers()
{
    m_freeLists = (size_t *)m_data;
    for(int i = 1; i < m_levels; i ++)
        m_freeLists[i] = SIZE_T_MAX;
    m_blockIndex = (byte *)m_data + m_sizeFreeLists;
    m_blockLevels = (byte *)m_data + m_sizeFreeLists + m_sizeIndex;
    memset(m_data, 0, m_sizeMetadata);
    m_data = (byte *)m_data + m_sizeMetadata;
}

byte BuddyAllocator::firstFreeLevel(byte levelToAllocate)
{
    for (byte i = levelToAllocate; i >= 0; i--)
    {
        if (m_freeLists[i] != SIZE_T_MAX)
            return i;
    }
    return 0xFF;
}

void *BuddyAllocator::fragmentAndAllocate(byte level, byte levelToAllocate)
{
    while (level != levelToAllocate)
    {
        size_t address = getBlock(level--);
        putBlock(address, level);
        putBlock(address | (1 << (intLog2(m_minimumSize) + level)), level);
    }
    return (void *)getBlock(level);
}

size_t BuddyAllocator::getBlock(byte level)
{
    size_t address = m_freeLists[level];
    //Update head of the list to point to the next field of the current head
    size_t *block = (size_t *)((byte *)m_data + address);
    m_freeLists[level] = block[1];

    //Mark block as allocated
    size_t index = (1 << level) - 1 + address / level;
    m_blockIndex[index / 8] |= 1 << (7 - index % 8);
}

void BuddyAllocator::putBlock(size_t address, byte level)
{
    putBlockInFreeList(address, level);

    //Mark block as deallocated
    size_t index = (1 << level) - 1 + address / level;
    m_blockIndex[index / 8] &= ~(1 << (7 - index % 8));

    //Set size in blockLevels array
    m_blockLevels[address / m_minimumSize] = level;
}

void BuddyAllocator::putBlockInFreeList(size_t address, byte level)
{
    size_t headAddress = m_freeLists[level];
    size_t *block = (size_t *)((byte *)m_data + address);
    if (headAddress == SIZE_T_MAX)
    {
        block[0] = SIZE_T_MAX;
        block[1] = SIZE_T_MAX;
    }
    else
    {
        size_t *head = (size_t *)((byte *)m_data + headAddress);
        head[0] = address;
        block[1] = headAddress;
        block[0] = SIZE_T_MAX;
    }
    m_freeLists[level] = address;
}

size_t BuddyAllocator::findFreeBuddy(size_t address)
{
    byte blockLevel = m_blockLevels[address / m_minimumSize];
    if (blockLevel == 0)
        return address;
    size_t index = (1 << blockLevel) - 1 + address / blockLevel;
    bool isBuddyFree = (m_blockIndex[index / 8] & (1 << (7 - index % 8)) == 0);
    if (isBuddyFree)
        return address ^ (1 << (m_levels - blockLevel - 1 + intLog2(m_minimumSize)));
    else
        return address;
}

void BuddyAllocator::merge(size_t &address, size_t buddyAddress)
{
    byte blockLevel = m_blockLevels[buddyAddress / m_minimumSize];
    eraseBlock(buddyAddress, blockLevel);
    address = address < buddyAddress ? address : buddyAddress;
    m_blockLevels[address / m_minimumSize]++;
}

void BuddyAllocator::eraseBlock(size_t address, byte level)
{
    size_t *block = (size_t *)((byte *)m_data + address);
    if (m_freeLists[level] == address)
        m_freeLists[level] = block[1];
    else if (block[1] == SIZE_T_MAX)
    {
        size_t *prevBlock = (size_t *)((byte *)m_data + block[0]);
        prevBlock[1] = SIZE_T_MAX;
    }
    else
    {
        size_t *prevBlock = (size_t *)((byte *)m_data + block[0]);
        size_t *nextBlock = (size_t *)((byte *)m_data + block[1]);
        prevBlock[1] = block[1];
        nextBlock[0] = block[0];
    }
}