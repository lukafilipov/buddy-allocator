#include "BuddyAllocator.h"

using namespace std;

BuddyAllocator::BuddyAllocator(const size_t totalSize, const size_t minimumSize):m_totalSize(totalSize),m_freeBitmap(1 << log2(totalSize)),m_minimumSize(minimumSize), m_logMinimumSize(log2(minimumSize))
{
    if(!isLog(totalSize) || !isLog(minimumSize)) 
        throw("Arguments must be a power of two");
    if(totalSize <= minimumSize)
        throw("MinimumSize must be smaller than TotalSize");
    if(minimumSize < 16)
        throw("MinimumSize must be at least 16");
}

void BuddyAllocator::Init()
{
    m_memory = malloc(m_totalSize);
    if (m_memory == nullptr) 
        throw("Requested memory size is too big");    
    (char*)m_memory[0] = 0x80 | log2(m_totalSize); 
    m_freeChunks[m_logTotalSize].emplace_back(0);
}

void* BuddyAllocator::Allocate(size_t size)
{
    if(size == 0) return nullptr;
    unsigned logToAllocate = firstBiggerLog(size);
    unsigned index = firstFreeChunk(logToAllocate);
    if(index == sizeof(unsigned) * 8) return nullptr
    size_t address = fragmentAndAllocate(index, logToAllocate);
    return (void*) (m_memory + address);
}

void Free(void* ptr)
{
    size_t address = (size_t)(ptr - m_memory);
    while((size_t buddyAddress = findBuddy(address) != address)
        merge(address, buddyAddress);
    deallocate(address);
}

void deallocate(size_t address, unsigned index)
{
    (char*)m_memory[address] = 0x80 | (index);
    m_freeChunks[index].push_front(address);
    m_freeBitmap |= 1 << (index);
}

void merge(size_t &address, size_t buddyAddress)
{
    unsigned sizeLog = (char*)m_memory[buddyAddress];
    for(auto i:m_freeChunks[sizeLog])
        if(i == buddyAddress)
        {
            m_freeChunks[sizeLog].erase(i)
            if(m_freeChunks[sizeLog].empty())
                m_freeBitmap &= ~(1 << sizeLog);
            break();
        }
    address = address < buddyAddress ? address : buddyAddress;
    deallocate(address, sizeLog + 1);
}

size_t BuddyAllocator::findBuddy(size_t address)
{
    unsigned sizeLog = (char*)m_memory[address];
    if(sizeLog == log2(m_totalSize))
        return address;
    unsigned buddyAddress = address ^ (1 << (sizeLog + 1));
    if((char*)m_memory[buddyAddress] & 0x80)
        return buddyAddress;
    else   
        return address;
}

size_t BuddyAllocator::fragmentAndAllocate(unsigned index, unsigned logToAllocate)
{
    while(index != logToAllocate)
    {
        size_t address = extractChunk(index)
        m_freeChunks[index - 1].emplace_front(address);
        m_freeChunks[index - 1].emplace_front(address | (1 << (m_logMinimumSize + index - 1));
        
    }
    size_t ret = extractChunk(index);
    (char*)m_memory[ret] = index + m_logMinimumSize;
    return ret;
}

size_t BuddyAllocator::extractChunk(unsigned index)
{
    size_t address = m_freeChunks[index].back();
    m_freeChunks[index].pop_back();
    if(m_freeChunks[index].size == 0) m_freeBitmap & ~(1 << index)
    return address;
}

~BuddyAllocator::BuddyAllocator()
{
    free(m_memory);
    m_memory = nullptr;
}

