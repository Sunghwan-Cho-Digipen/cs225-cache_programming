/*
 * cachelab.c - Cache Lab helper functions
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "cachelab.h"
#include <time.h>

/* 
 * printSummary - Summarize the cache simulation statistics. Student cache simulators
 *                must call this function in order to be properly autograded. 
 */
void printSummary(int hits, int misses, int evictions)
{
    printf("hits:%d misses:%d evictions:%d\n", hits, misses, evictions);
    FILE* output_fp = fopen(".csim_results", "w");
    assert(output_fp);
    fprintf(output_fp, "%d %d %d\n", hits, misses, evictions);
    fclose(output_fp);
}


//Sunghwan Cho
//cs200
//cache assignment
//fall 2020

//#include "cachelab.h"
#include <vector>
#include <string>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <getopt.h>
#include <cmath>
#include <fstream>
#include <sstream>
#include <chrono>

struct Line;
struct Set;
struct Cache;

namespace helper
{
	using ms = std::chrono::duration<double, std::milli>;

	static constexpr long long HexDigitToDec(char hexDigit)
	{
		long long dec = 0;

		if (hexDigit >= '0' && hexDigit <= '9')
		{
			dec = hexDigit - '0';
		}
		else if (hexDigit >= 'A' && hexDigit <= 'F')
		{
			dec = (hexDigit + 10) - 'A';
		}

		return dec;
	}

	long long ConvertHexToDec(std::string hexString)
	{
		long long index = hexString.length() - 1;
		long long power16 = 1;
		long long decimal = 0;

		while (index >= 0)
		{
			long long hexdigit = HexDigitToDec(hexString[index]);

			decimal += hexdigit * power16;
			power16 *= 16;
			index--;
		}

		return decimal;
	}

} // namespace helper

///////////////////////////////////////////// Classes ///////////////////////////////////////////////////

struct Line
{
public:
	long long physicalMemoryAddress = 0x0;
	long long tag = 0;
	int blockOffset = 0;
	int indexBit = 0;
	int blockBit = 0;
	bool hasValueChanged = false;
	std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
};

struct Set
{
public:
	std::vector<Line> linesPerSet;
};

struct Cache
{
public:
	std::vector<Set> setsPerCache;
};

///////////////////////////////////////////// Classes ///////////////////////////////////////////////////

int main(int argc, char* argv[])
try 
{
	Cache cache;
	bool isVerboseMode = false;
	
	int setIndexBitsNum = 0;
	int linesPerSetNum = 0;
	int blockOffsetBitsNum = 0;

	int hitNumsCount = 0;
	int missNumsCount = 0;
	int evictionNumsCount = 0;

	long long address = 0;

	int SET_INDEX_BIT_MASK = 0;

	int index;

	std::string traceFileLocation = " ";

	int opt;
	while((opt = getopt(argc, argv, "hvs:E:b:t")) != -1)
	{
		switch (opt)
		{
		case 'h':
			{
				std::cout << "Usage: ./csim [-hv] -s <num> -E <num> -b <num> -t <file>" << std::endl;
				std::cout << "Options:" << std::endl;
				std::cout << "\t-h         Print this help message." << std::endl;
				std::cout << "\t-v         Optional verbose flag." << std::endl;
				std::cout << "\t-s <num>   Number of set index bits." << std::endl;
				std::cout << "\t-E <num>   Number of lines per set." << std::endl;
				std::cout << "\t-b <num>   Number of block offset bits." << std::endl;
				std::cout << "\t-t <file>  Trace file." << std::endl;

				std::cout << "Examples:" << std::endl;
				std::cout << "\tlinux>  ./csim -s 4 -E 1 -b 4 -t traces/yi.trace" << std::endl;
				std::cout << "\tlinux>  ./csim -v -s 8 -E 2 -b 4 -t traces/yi.trace" << std::endl;
			}
			break;

		case 'v':
			{
				isVerboseMode = true;
			}
		 	break;

		case 's':
			{
				setIndexBitsNum = std::stoi(optarg);
				int numOfSets = 1 << setIndexBitsNum;
				cache.setsPerCache.resize(numOfSets);
				SET_INDEX_BIT_MASK = numOfSets - 1;
			}
			break;

		case 'E':
			{
				linesPerSetNum = std::stoi(optarg);
				for(auto& set: cache.setsPerCache)
				{
					set.linesPerSet.resize(linesPerSetNum);
				}
			}
			break;

		case 'b':
			{
				blockOffsetBitsNum = std::stoi(optarg);
				SET_INDEX_BIT_MASK <<= blockOffsetBitsNum;
			}
			break;

		case 't':
			{
				traceFileLocation = optarg;
			}
			break;

		case '?':
			break;
		default:
			break;
		}
	}

	std::ifstream inFile(traceFileLocation, std::ios::in);

	if(!inFile)
	{
		throw std::runtime_error("Unable to open Trace File Location: " + traceFileLocation + ". Please recheck the trace file location.");
	}

	inFile.seekg(0, std::ios::beg);
	std::string line;

	while(std::getline(inFile, line))
	{
		std::istringstream traceFileInfo{ line };

		std::string functionName;
		std::string physicalAddress;
		traceFileInfo >> functionName >> physicalAddress;

		if(functionName != "I" || functionName != "L" || functionName != "S" || functionName != "M")
		{
			throw std::runtime_error("Bad Function Name" + functionName + ". Please recheck the trace file's function name.");
		}

		if(isVerboseMode == true)
		{
			if(functionName == "I")
			{
				continue;
			}
			std::cout << line;
		}

		if(functionName == "I")
		{
			continue;
		} // "I"
		else if(functionName == "L")
		{
			address = helper::ConvertHexToDec(physicalAddress);
			index = (address & SET_INDEX_BIT_MASK) >> blockOffsetBitsNum;
			int tag = address >> setIndexBitsNum >> blockOffsetBitsNum;

			bool isHit = false;

			auto lineIter = cache.setsPerCache[index].linesPerSet.begin();

			for(; lineIter != cache.setsPerCache[index].linesPerSet.end(); ++lineIter)
			{
				if(lineIter->tag == tag)
				{
					lineIter->start = std::chrono::system_clock::now();
					lineIter->hasValueChanged = false;
					isHit = true;
					break;
				}				
			}

			if(isHit == true)
			{
				if (isVerboseMode == true)
				{
					std::cout << " hit";
				}
				++hitNumsCount;
			}
			else
			{
				if (isVerboseMode == true)
				{
					std::cout << " miss";
				}
				++missNumsCount;

				if(linesPerSetNum == 1)
				{
					lineIter->tag = tag;
				}
				else
				{
					std::chrono::system_clock::time_point now = std::chrono::system_clock::now(); 
					auto oldFashion = cache.setsPerCache[index].linesPerSet.begin();
					double oldestTime = 0.0;
					double time = 0.0;
					for (lineIter = cache.setsPerCache[index].linesPerSet.begin(); lineIter != cache.setsPerCache[index].linesPerSet.end(); ++lineIter)
					{
						oldestTime = time;
						time = std::chrono::duration_cast<helper::ms>(now - lineIter->start).count();
						if(oldestTime != 0.0 && time != 0.0)
						{
							if(oldestTime < time)
							{
								oldFashion = lineIter;
							}
						}
					}
					if(oldFashion->hasValueChanged == true)
					{
						if(isVerboseMode == true)
						{
							std::cout << " eviction";
						}
						++evictionNumsCount;
					}
					oldFashion->hasValueChanged = true;
					oldFashion->tag = tag;
				}
			}
		} //"L"
		else if(functionName == "S")
		{
			address = helper::ConvertHexToDec(physicalAddress);
			index = (address & SET_INDEX_BIT_MASK) >> blockOffsetBitsNum;
			int tag = address >> setIndexBitsNum >> blockOffsetBitsNum;

			bool isHit = false;

			auto lineIter = cache.setsPerCache[index].linesPerSet.begin();

			for(; lineIter != cache.setsPerCache[index].linesPerSet.end(); ++lineIter)
			{
				if(lineIter->tag == tag)
				{
					lineIter->start = std::chrono::system_clock::now();
					lineIter->hasValueChanged = false;
					isHit = true;
					break;
				}				
			}

			if(isHit == true)
			{
				if (isVerboseMode == true)
				{
					std::cout << " hit";
				}
				++hitNumsCount;
			}
			else
			{
				if (isVerboseMode == true)
				{
					std::cout << " miss";
				}
				++missNumsCount;

				if(linesPerSetNum == 1)
				{
					lineIter->tag = tag;
				}
				else
				{
					std::chrono::system_clock::time_point now = std::chrono::system_clock::now(); 
					auto oldFashion = cache.setsPerCache[index].linesPerSet.begin();
					double oldestTime = 0.0;
					double time = 0.0;
					for (lineIter = cache.setsPerCache[index].linesPerSet.begin(); lineIter != cache.setsPerCache[index].linesPerSet.end(); ++lineIter)
					{
						oldestTime = time;
						time = std::chrono::duration_cast<helper::ms>(now - lineIter->start).count();
						if(oldestTime != 0.0 && time != 0.0)
						{
							if(oldestTime < time)
							{
								oldFashion = lineIter;
							}
						}
					}
					if(oldFashion->hasValueChanged == true)
					{
						if(isVerboseMode == true)
						{
							std::cout << " eviction";
						}
						++evictionNumsCount;
					}
					oldFashion->hasValueChanged = true;
					oldFashion->tag = tag;
				}
			}
		} // S
		else
		{
			address = helper::ConvertHexToDec(physicalAddress);
			index = (address & SET_INDEX_BIT_MASK) >> blockOffsetBitsNum;
			int tag = address >> setIndexBitsNum >> blockOffsetBitsNum;

			bool isHit = false;
			for (int i = 0; i < 2; ++i)
			{
				auto lineIter = cache.setsPerCache[index].linesPerSet.begin();

				for (; lineIter != cache.setsPerCache[index].linesPerSet.end(); ++lineIter)
				{
					if (lineIter->tag == tag)
					{
						lineIter->start = std::chrono::system_clock::now();
						lineIter->hasValueChanged = false;
						isHit = true;
						break;
					}
				}

				if (isHit == true)
				{
					if (isVerboseMode == true)
					{
						std::cout << " hit";
					}
					++hitNumsCount;
				}
				else
				{
					if (isVerboseMode == true)
					{
						std::cout << " miss";
					}
					++missNumsCount;

					if (linesPerSetNum == 1)
					{
						lineIter->tag = tag;
					}
					else
					{
						std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
						auto oldFashion = cache.setsPerCache[index].linesPerSet.begin();
						double oldestTime = 0.0;
						double time = 0.0;
						for (lineIter = cache.setsPerCache[index].linesPerSet.begin(); lineIter != cache.setsPerCache[index].linesPerSet.end(); ++lineIter)
						{
							oldestTime = time;
							time = std::chrono::duration_cast<helper::ms>(now - lineIter->start).count();
							if (oldestTime != 0.0 && time != 0.0)
							{
								if (oldestTime < time)
								{
									oldFashion = lineIter;
								}
							}
						}
						if (oldFashion->hasValueChanged == true)
						{
							if (isVerboseMode == true)
							{
								std::cout << " eviction";
							}
							++evictionNumsCount;
						}
						oldFashion->hasValueChanged = true;
						oldFashion->tag = tag;
					}
				}
			}
		} // "M"
	}

    printSummary(hitNumsCount, missNumsCount, evictionNumsCount);
    return 0;
}
catch(std::exception& e)
{
	std::cerr << e.what() << std::endl;
	return -1;
}