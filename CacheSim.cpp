#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>

using namespace std;

#define		DBG				1
#define		DRAM_SIZE		(64*1024*1024)
#define		CACHE_SIZE		(32*1024)


enum cacheResType { MISS = 0, HIT = 1 };

unsigned int m_w = 0xABABAB55;    /* must not be zero, nor 0x464fffff */
unsigned int m_z = 0x05080902;    /* must not be zero, nor 0x9068ffff */

unsigned int rand_()
{
	m_z = 36969 * (m_z & 65535) + (m_z >> 16);
	m_w = 18000 * (m_w & 65535) + (m_w >> 16);
	return (m_z << 16) + m_w;  /* 32-bit result */
}

unsigned int memGen1()
{
	static unsigned int addr = 0;
	return (addr++) % (DRAM_SIZE);
}

unsigned int memGen2()
{
	return rand_() % (DRAM_SIZE);
}

unsigned int memGen3()
{
	static unsigned int addr = 0;
	return (addr++) % (1024 * 8);
}

unsigned int memGen4()
{
	static unsigned int addr = 0;
	return (addr++) % (1024 * 64);
}

// Direct Mapped Cache Simulator
cacheResType cacheSimDM(unsigned int addr)
{

	return MISS;
}

struct FullyAssociativeLine
{
	unsigned int tag;
	bool valid = false;
};

FullyAssociativeLine* cacheBlocks;
int blockNumber;
int shiftAmount;

// Fully Associative Cache Simulator
cacheResType cacheSimFA(unsigned int addr)
{
	int tag = addr >> shiftAmount;

	for (int i = 0; i < blockNumber; ++i)
	{
		if (cacheBlocks[i].tag == tag && cacheBlocks[i].valid)
			return HIT;
	}

	int blockReplaced = rand() % blockNumber;

	cacheBlocks[blockReplaced].tag = tag;
	cacheBlocks[blockReplaced].valid = true;

	return MISS;
}

void runCacheBlockSize(int blockSize)
{
	cout << "Cache line size: " << blockSize << endl;

	blockNumber = CACHE_SIZE / blockSize;
	shiftAmount = log2(blockSize);

	cacheBlocks = new FullyAssociativeLine[blockNumber]{};

	cacheResType r;

	unsigned int addr;
	cout << "Fully Associative Cache Simulator\n";
	int hits1 = 0;
	int hits2 = 0;
	int hits3 = 0;
	int hits4 = 0;
	for (int inst = 0; inst < 1000000; inst++)
	{
		addr = memGen1();
		r = cacheSimFA(addr);
		if (r == HIT)
			hits1++;

		addr = memGen2();
		r = cacheSimFA(addr);
		if (r == HIT)
			hits2++;

		addr = memGen3();
		r = cacheSimFA(addr);
		if (r == HIT)
			hits3++;

		addr = memGen4();
		r = cacheSimFA(addr);
		if (r == HIT)
			hits4++;
	}

	cout << "memGen1 Hits: " << hits1 << endl;
	cout << "memGen2 Hits: " << hits2 << endl;
	cout << "memGen3 Hits: " << hits3 << endl;
	cout << "memGen4 Hits: " << hits4 << endl << endl;

	delete[] cacheBlocks;
}

int main()
{
	srand(time(NULL));

	for (int i = 8; i <= 128; i *= 2)
	{
		runCacheBlockSize(i);
	}
}