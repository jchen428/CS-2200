/* CS 2200 - Project 4 - Spring 2016
 * Name - Jesse Chen
 * GTID - 903006652
 */

#include "cachesim.h"

typedef struct cacheBlock {
	int accesses;
	char dirty;
	char valid;
	uint64_t tag;
} cacheBlock;

// Global variables
cacheBlock **myCache;
int offsetBits;
int indexBits;
int tagBits;
int cacheSize;
int blockSize;
int associativity;	// Number of blocks per line
int numLines;		// Number of lines in cache
int numBlocks;		// Total number of data blocks in cache

/**
 * Function to extract the tag from a memory address.
 *
 * @param address The address that is being accessed
 */
uint64_t getTag(uint64_t address) {
	uint64_t tag = (address & (((1 << tagBits) - 1) << (offsetBits + indexBits))) >> (offsetBits + indexBits);
	//printf("tag = %" PRIx64 "\n", tag);
	return tag;
}

/**
 * Function to extract the index from a memory address.
 *
 * @param address The address that is being accessed
 */
uint64_t getIndex(uint64_t address) {
	uint64_t index = (address & (((1 << indexBits) - 1) << offsetBits)) >> offsetBits;
	//printf("index = %" PRIx64 "\n", index);
	return index;
}

/**
 * Function to extract the offset from a memory address.
 *
 * @param address The address that is being accessed
 */
uint64_t getOffset(uint64_t address) {
	uint64_t offset = address & ((1 << offsetBits) - 1);
	//printf("offset = %" PRIx64 "\n", offset);
	return offset;
}

/**
 * Gets a block from within a cache line by either finding an empty 
 * block or evicting one via LRU.
 *
 * @param line The cache line to search
 * @param stats The struct that you are supposed to store the stats in
 */
cacheBlock* getBlock(cacheBlock *line, struct cache_stats_t *stats) {
	cacheBlock *block = NULL;

	// Look for an invalid block
	for (int i = 0; i < associativity; i++) {
		if (!line[i].valid) {
			block = &line[i];
			break;
		}
	}

	// If no invalid blocks, find LFU block to evict
	if (!block) {
		block = &line[0];
		for (int i = 0; i < associativity; i++) {
			if (line[i].accesses < block->accesses)
				block = &line[i];
		}
		// If dirty, write back
		if (block->dirty) {
			stats->write_backs++;
		}
	}

	return block;
}

/**
 * Sub-routine for initializing your cache with the parameters.
 * You may initialize any global variables here.
 *
 * @param C The total size of your cache is 2^C bytes
 * @param S The set associativity is 2^S
 * @param B The size of your block is 2^B bytes
 */
void cache_init(uint64_t C, uint64_t S, uint64_t B) {
	offsetBits = B;											// 5 bits
	indexBits = C - B - S;									// 7 bits
	tagBits = 64 - offsetBits - indexBits;					// 52 bits

	cacheSize = 1 << C;										// 2^15 = 32 KB
	blockSize = 1 << B;										// 2^5 = 32 B
	associativity = 1 << S;									// 2^3 = 8 blocks per line
	numLines = cacheSize / (blockSize * associativity);		// 2^7 = 128 lines
	numBlocks = cacheSize / blockSize;						// 2^10 = 1024 blocks

	// malloc space in memory for cache
	myCache = malloc(sizeof(cacheBlock *) * numLines);
	for (int i = 0; i < numLines; i++) {
		myCache[i] = malloc(sizeof(cacheBlock) * associativity);
	}

	/*printf("total bits per block = %d\n", 2 + tagBits);
	printf("numBlocks = %d\n", numBlocks);
	printf("total cache size = %d\n\n", (2 + tagBits) * numBlocks);*/

	/*printf("cacheSize = %d\n", cacheSize);
	printf("blockSize = %d\n", blockSize);
	printf("associativity = %d\n", associativity);
	printf("numLines = %d\n", numLines);
	printf("numBlocks = %d\n", numBlocks);

	printf("address = %" PRIx64 "\n", 0x64AE61B681E186A1);
	printf("offsetBits = %d\n", offsetBits);
	printf("indexBits = %d\n", indexBits);
	printf("tagBits = %d\n\n", tagBits);

	getTag(0x64AE61B681E186A1);
	getIndex(0x64AE61B681E186A1);
	getOffset(0x64AE61B681E186A1);*/
}

/**
 * Subroutine that simulates one cache event at a time.
 *
 * @param rw The type of access, READ or WRITE
 * @param address The address that is being accessed
 * @param stats The struct that you are supposed to store the stats in
 */
void cache_access (char rw, uint64_t address, struct cache_stats_t *stats) {
	// Split address components
	uint64_t tag = getTag(address);
	uint64_t index = getIndex(address);
	//uint64_t offset = getOffset(address);
	stats->accesses++;

	// Index into cache line and look for matching block
	cacheBlock *line = myCache[index];
	cacheBlock *block = NULL;
	for (int i = 0; i < associativity; i++) {
		if (tag == line[i].tag && line[i].valid) {
			line[i].accesses++;
			block = &line[i];
			break;
		}
	}

	if (rw == READ) {
		stats->reads++;

		// Read miss
		if (!block) {
			stats->read_misses++;
			block = getBlock(line, stats);

			block->accesses = 1;
			block->dirty = 0;
			block->valid = 1;
			block->tag = tag;
		}
	} else if (rw == WRITE) {
		stats->writes++;

		// Write miss
		if (!block) {
			stats->write_misses++;
			block = getBlock(line, stats);

			block->accesses = 1;
			block->valid = 1;
			block->tag = tag;
		}
		block->dirty = 1;		// Block always has to be set to dirty after write
	}
}

/**
 * Subroutine for cleaning up memory operations and doing any calculations
 * Make sure to free malloced memory here.
 *
 * @param stats The struct that you are supposed to store the stats in
 */
void cache_cleanup (struct cache_stats_t *stats) {
	// Free memory from cache
	for (int i = 0; i < numLines; i++) {
		free(myCache[i]);
	}
	free(myCache);

	// Finalize stats
	stats->misses = stats->read_misses + stats->write_misses;
	stats->miss_rate = (double)stats->misses / stats->accesses;
	stats->avg_access_time = stats->access_time + stats->miss_rate * stats->miss_penalty;
}
