#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "Block.h"
#include "FileSystem.h"

file_t* create_file(disk_t* _disk, fat* _fat, file_t* _current_dir, char _name[], char _ext[]) {
	file_t* new_file = malloc(sizeof(file_t));
	memcpy(new_file->name, _name, strlen(_name) + 1);
	memcpy(new_file->ext, _ext, strlen(_ext) + 1);
	new_file->len = 0;
	if (strcmp(_ext, "dir") == 0){
		new_file->is_dir = 1;
	}
	
	time_t timep;
	struct tm *p;
	time(&timep);
	p = localtime(&timep);
	new_file->date[0] = p->tm_year;
	new_file->date[1] = p->tm_mon;
	new_file->date[2] = p->tm_mday;
	new_file->date[3] = p->tm_hour;
	new_file->date[4] = p->tm_min;
	new_file->date[5] = p->tm_sec;
	new_file->start = get_next_free_block(_fat);
	add_dir_entry(new_file, _current_dir, _disk, _fat);
	return new_file;
}

file_t* create_dir(disk_t* _disk, fat* _fat, file_t* _current_dir, char _name[]) {
	file_t* new_file =  create_file(_disk, _fat, _current_dir, _name, "dir");



	file_t* child = malloc(sizeof(file_t));
	file_t* father = malloc(sizeof(file_t));
	
	memcpy(child->date, new_file->date, 6 * sizeof(uint8_t));
	child->is_dir = new_file->is_dir;
	memcpy(child->name, ".\0", strlen(".") + 1);
	memcpy(child->ext, "dir\0", strlen("dir") + 1);
	child->len = new_file->len;
	child->start = new_file->start;

	memcpy(father->date, _current_dir->date, 6 * sizeof(uint8_t));
	father->is_dir = _current_dir->is_dir;
	memcpy(father->name, "..\0", strlen("..") + 1);
	memcpy(child->ext, "dir\0", strlen("dir") + 1);
	father->len = _current_dir->len;
	father->start = _current_dir->start;

	add_dir_entry(child, new_file, _disk, _fat);
	add_dir_entry(father, new_file, _disk, _fat);

}

void add_dir_entry(file_t* _new_file, file_t * _father_dir, disk_t* _disk, fat* _fat) {

    uint8_t buf[32];
	memset(buf, 0, 32);
	memcpy(buf, _new_file->name, 9);
	memcpy(buf + 9, _new_file->ext, 4);
	memcpy(buf + 13, &_new_file->start, 4);
	memcpy(buf + 17, _new_file->date, 6);
	memcpy(buf + 23, &_new_file->len, 4);
	memcpy(buf + 28, &_new_file->is_dir, 1);
	write_file(_father_dir, buf, 32, _fat, _disk);
}
void write_file(file_t* _file, uint8_t buf[], uint32_t len, fat* _fat, disk_t* _disk) {
	int block_cnt = _file->len / BLOCK_SIZE;
	int block_remain = _file->len % BLOCK_SIZE;
	uint32_t block_num = _file->start;
	for (size_t i = 0; i < block_cnt; i++) {
		block_num = _fat->fat_entries[block_num];
	}
    int new_len = len;
	int index = block_num;

	while (len >= BLOCK_SIZE - block_remain){
		len = len - (BLOCK_SIZE - block_remain);
		_fat->fat_entries[index] = get_next_free_block(_fat);
		memcpy(_disk->blocks[index].content + block_remain, buf + (new_len - len), BLOCK_SIZE - block_remain);
		index = _fat->fat_entries[index];
		block_remain = 0;
	}

	memcpy(_disk->blocks[index].content + block_remain, buf+(new_len - len) , len);
	_fat->fat_entries[index] = 0x0FFFFFFF;

	_file->len += len;
}

file_t** get_children(file_t* _dir, disk_t* _disk, fat* _fat) {
	int index = _dir->start;
	uint8_t buf[_dir->len];
	read_file(_dir, buf, _disk, _fat);

	uint8_t entries[_dir->len / ENTRY_SIZE][ENTRY_SIZE];
	file_t **children = malloc((_dir->len/ENTRY_SIZE + 1) * sizeof(file_t*));

    for (int i = 0; i <_dir->len/ENTRY_SIZE ; ++i) {
        children[i] = malloc(sizeof(file_t));
    }
    children[_dir->len/ENTRY_SIZE] = NULL;
	for (size_t i = 0; i < _dir->len / ENTRY_SIZE; i++) {
		memcpy(entries[i], buf + i * ENTRY_SIZE, ENTRY_SIZE);
		memcpy(children[i]->name, entries[i], 9);
		memcpy(children[i]->ext, entries[i] + 9, 4);
		children[i]->start = get_a_int(entries[i] + 13);
		children[i]->date[0] = entries[i][17];
		children[i]->date[1] = entries[i][18];
		children[i]->date[2] = entries[i][19];
		children[i]->date[3] = entries[i][20];
		children[i]->date[4] = entries[i][21];
		children[i]->date[5] = entries[i][22];
		children[i]->len = get_a_int(entries[i] + 23);
		children[i]->is_dir = entries[i][27];
	}

	return children;

}



uint32_t get_a_int(uint8_t * _src) {

    return *(uint32_t*)_src;

//    return ((uint32_t)(*_src) << 24) + ((uint32_t)(*_src + 1) << 16)
//		   +((uint32_t)(*_src + 2) << 8) + ((uint32_t)(*_src + 3));
}



uint32_t get_next_free_block(fat* _fat) {
	for (uint32_t i = 2; i < DISK_SIZE / BLOCK_SIZE; i++)	{
		if (_fat->fat_entries[i] == 0x00000000) {
			return i;
		}
	}
}


file_t*  file_sys_init(disk_t* _disk, fat* _fat) {

	block_t* block_0 = &(_disk->blocks[0]);
	int first = get_a_int(block_0->content);
	memset(block_0->content + 4, DISK_SIZE, sizeof(DISK_SIZE));
	memset(block_0->content + 4 + sizeof(DISK_SIZE), BLOCK_SIZE, sizeof(BLOCK_SIZE));
	block_t * block_1 = &(_disk->blocks[1]);
	memcpy(_fat->fat_entries, block_1->content, BLOCK_SIZE);
	
	//Create root
	block_t* block_2 = &(_disk->blocks[2]);
	if (*(block_0->content + 4 + sizeof(DISK_SIZE) + sizeof(BLOCK_SIZE)) != 1) {
		memset(block_0->content + 4 + sizeof(DISK_SIZE) + sizeof(BLOCK_SIZE), 1, sizeof(uint32_t));
		_fat->fat_entries[2] = 0x0FFFFFFF;
	}

	file_t * root = malloc(sizeof(file_t));

	memcpy(root->name, "/\0", sizeof("/\0"));
	memcpy(root->ext, "dir\0", sizeof("dir\0"));
	root->start = 2;

	time_t timep;
	struct tm *p;
	time(&timep);
	p = localtime(&timep);

	root->date[0] = p->tm_year;
	root->date[1] = p->tm_mon;
	root->date[2] = p->tm_mday;
	root->date[3] = p->tm_hour;
	root->date[4] = p->tm_min;
	root->date[5] = p->tm_sec;

	root->len = 0;
	root->is_dir = 1;


    root->len += 32;

    file_t* child = malloc(sizeof(file_t));

    memcpy(child->date, root->date, 6 * sizeof(uint8_t));
    child->is_dir = root->is_dir;
    memcpy(child->name, ".\0", strlen(".") + 1);
    memcpy(child->ext, "dir\0", strlen("dir") + 1);
    child->len = root->len;
    child->start = root->start;



    add_dir_entry(child, root, _disk, _fat);


    return root;

	
}


file_t* file_sys_read(disk_t* _disk, fat* _fat, FILE* virtual_disk) {
	for (size_t i = 0; i < DISK_SIZE / BLOCK_SIZE; i++) {
		fread(_disk->blocks[i].content, BLOCK_SIZE, 1, virtual_disk);
	}
    return file_sys_init(_disk, _fat);

}





void read_file(file_t* _file, uint8_t buf[] ,disk_t* _disk, fat* _fat) {
	int index = _file->start;
	int cnt = 0;
	while (_fat->fat_entries[index] != 0x0FFFFFFF) {
		memcpy(buf + cnt * BLOCK_SIZE, _disk->blocks[index].content, BLOCK_SIZE);
		index = _fat->fat_entries[index];
		cnt++;
	}

	memcpy(buf + cnt * BLOCK_SIZE, _disk->blocks[index].content, _file->len % BLOCK_SIZE);

}


void save_disk(disk_t * _disk, fat* _fat, FILE* _file) {

    memcpy(_disk->blocks[1].content, _fat->fat_entries, BLOCK_SIZE);

	for (size_t i = 0; i < DISK_SIZE / BLOCK_SIZE; i++)	{
		fwrite(_disk->blocks[i].content, sizeof(uint8_t), BLOCK_SIZE, _file);
	}
}

file_t* my_lsopen(char path[], file_t* _root, disk_t* _disk, fat* _fat) {

    int cnt = 0;
    if(path[strlen(path) - 1] != '/'){
        cnt ++;
    }
    for (int i = 0; i < strlen(path); ++i) {
        if(path[i] == '/'){
            cnt ++;
        }
    }
    char paths[cnt][9];
    memcpy(paths[0], "/\0", strlen("/\0"));
    int m = 0;
    char *p = NULL;
    p = strtok(path, "/");
    memcpy(paths[1], p, strlen(p));
    int itor = 2;
    while((p = strtok(NULL, "/"))) {
        memcpy(paths[itor], p, strlen(p));
        itor++;
    }
    file_t** children;
    for (int j = 0; j < cnt; ++j) {
        children =  get_children(_root, _disk, _fat);
        for (int i = 0; i < _root->len / ENTRY_SIZE; ++i) {
            if(strcmp(children[i]->name, paths[j]) == 0){
                _root = children[i];
            }
        }
    }

    p = NULL;

    return _root;
}


file_t * get_file_from_binary(uint8_t data[]){
    file_t* f = malloc(sizeof(file_t));
    memcpy(f->name, data, 9);
    memcpy(f->ext, data+ 9, 4);
    f->start = get_a_int(data + 13);
    f->date[0] = data[17];
    f->date[1] = data[18];
    f->date[2] = data[19];
    f->date[3] = data[20];
    f->date[4] = data[21];
    f->date[5] = data[22];
    f->len = get_a_int(data + 23);
    f->is_dir = data[28];

}


file_t* get_father(file_t* _file, disk_t* _disk, fat * _fat){
    uint8_t father_data[ENTRY_SIZE];
    memcpy(father_data, _disk->blocks[_file->start].content + ENTRY_SIZE, ENTRY_SIZE);
    file_t* father = get_file_from_binary(father_data);
    return father;

}
