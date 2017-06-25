#ifndef BLOCK_H
#define BLOCK_H

#include <stdint.h>
#include "Config.h"

typedef struct Block block_t;
struct Block
{
	uint8_t content[BLOCK_SIZE];

	uint64_t index;
};


typedef struct Disk disk_t;
struct Disk
{
	block_t blocks[DISK_SIZE / BLOCK_SIZE];
};

block_t* get_block(disk_t* _disk, uint32_t _index);

void init_block(disk_t* _disk);

#endif