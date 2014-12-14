#include <iostream>
#include <iomanip>
#include <cmath>
#include <chrono>
#include <random>

using namespace std;

#define		DBG				1
#define		DRAM_SIZE		(64*1024*1024)
#define		CACHE_SIZE		(32*1024)

//Better RNG
std::mt19937 generator(unsigned int(std::chrono::system_clock::now().time_since_epoch().count()));

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

unsigned int (*genFunctions[4])() = {
	memGen1,
	memGen2,
	memGen3,
	memGen4
};

struct CacheLine
{
	unsigned int tag;
	bool valid;

	//No need to include data, we are just simulating hit rates

	CacheLine() : valid(false) {}
};

CacheLine* dmBlocks;
CacheLine* faBlocks;

int blockNumber;
int shiftAmount;
int shiftAmountIndex;

// Direct Mapped Cache Simulator
cacheResType cacheSimDM(unsigned int addr)
{
	int blockSize = CACHE_SIZE / blockNumber;
	int index = (addr / blockSize) % blockNumber;
	int tag = (addr >> shiftAmount) >> shiftAmountIndex;

	if (dmBlocks[index].valid && dmBlocks[index].tag == tag)
		return HIT;
	
	dmBlocks[index].tag = tag;
	dmBlocks[index].valid = true;
	
	return MISS;
}

// Fully Associative Cache Simulator
cacheResType cacheSimFA(unsigned int addr)
{
	int tag = addr >> shiftAmount;

	for (int i = 0; i < blockNumber; ++i)
	{
		if (faBlocks[i].tag == tag && faBlocks[i].valid)
			return HIT;
	}

	int blockReplaced = generator() % blockNumber;

	faBlocks[blockReplaced].tag = tag;
	faBlocks[blockReplaced].valid = true;

	return MISS;
}

void runCacheBlockSize(int blockSize)
{
	cout << "Cache line size: " << blockSize << endl;

	blockNumber = CACHE_SIZE / blockSize;
	shiftAmount = int(log(blockSize) / log(2));
	shiftAmountIndex = int(log(blockNumber) / log(2));

	faBlocks = new CacheLine[blockNumber]{};
	dmBlocks = new CacheLine[blockNumber]{};

	cacheResType rDM;
	cacheResType rFA;

	unsigned int addr;
	int hitsDM[4]{};
	int hitsFA[4]{};
	for (int inst = 0; inst < 1000000; inst++)
	{
		for (int i = 0; i < 4; ++i)
		{
			addr = genFunctions[i]();
			rDM = cacheSimDM(addr);
			rFA = cacheSimFA(addr);

			if (rFA == HIT)
				hitsFA[i]++;

			if (rDM == HIT)
				hitsDM[i]++;
		}
	}

	cout << "Fully Associative Cache:\n";
	cout << "memGen1 Hits: " << hitsFA[0] << endl;
	cout << "memGen2 Hits: " << hitsFA[1] << endl;
	cout << "memGen3 Hits: " << hitsFA[2] << endl;
	cout << "memGen4 Hits: " << hitsFA[3] << endl << endl;

	cout << "Direct Mapped Cache:\n";
	cout << "memGen1 Hits: " << hitsDM[0] << endl;
	cout << "memGen2 Hits: " << hitsDM[1] << endl;
	cout << "memGen3 Hits: " << hitsDM[2] << endl;
	cout << "memGen4 Hits: " << hitsDM[3] << endl << endl;

	delete[] faBlocks;
	delete[] dmBlocks;
}


int main()
{
	for (int i = 8; i <= 128; i *= 2)
	{
		runCacheBlockSize(i);
	}
}