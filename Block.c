#include "Block.h"
#include <memory.h>
#include "Config.h"

void init_block(disk_t* _disk) {
	for (size_t i = 0; i < DISK_SIZE / BLOCK_SIZE; i++) {
		_disk->blocks[i].index = i;
		memset(_disk->blocks[i].content, 0, BLOCK_SIZE);
	}
}
block_t* get_block(disk_t* _disk, uint32_t _index){
	return &_disk->blocks[_index];
}


