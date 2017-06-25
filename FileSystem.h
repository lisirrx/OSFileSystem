#ifndef FILESYSTEM_H
#define FILESYSTEM_H
#include <stdio.h>
#include <stdint.h>
#include "Config.h"
#include "Block.h"

typedef struct file file_t;
struct file
{
	char name[FILE_NAME + 1]; //9 9
	char ext[FILE_EXT + 1]; //4 13
	uint32_t start; // 4 17
	uint8_t date[6]; // 6 23
	uint32_t len; // 4 27
	uint8_t is_dir; //1 28
	uint8_t place_holder[4];
};


typedef struct FAT fat;
struct FAT {
	uint32_t fat_entries[DISK_SIZE / BLOCK_SIZE];
};


typedef uint32_t first_free;

typedef struct file_dir file_dir_table;
struct file_dir
{
	char file_names[1][FILE_NAME + FILE_EXT];
	block_t* file_list[FILE_MAX];
};

file_t* create_file(disk_t* _disk, fat* _fat, file_t* _current_dir, char _name[], char _ext[]);
file_t* create_dir(disk_t* _disk, fat* _fat, file_t* current_dir, char _name[]);

uint32_t get_next_free_block(fat* _fat);

file_t* file_sys_init(disk_t* _disk, fat* _fat);

void save_disk(disk_t * _disk, fat* _fat, FILE* _file);
uint32_t get_a_int(uint8_t * _src);

void add_dir_entry(file_t* _new_file, file_t * _father_dir, disk_t* _disk, fat* _fat);
void write_file(file_t* _file, uint8_t buf[], uint32_t len, fat* _fat, disk_t* _disk);

file_t* file_sys_read(disk_t* _disk, fat* _fat, FILE* virtual_disk);
void read_file(file_t* _file, uint8_t buf[], disk_t* _disk, fat* _fat);

file_t** get_children(file_t* dir, disk_t* _disk, fat* _fat);

file_t* my_open(char path[], file_t* _root, disk_t* _disk, fat* _fat);
void get_path(file_t* dir, disk_t* _disk, fat* fat, file_t* _root);
file_t * get_file_from_binary(uint8_t data[]);
file_t* get_father(file_t* _file, disk_t* _disk, fat * _fat);
void write_binary_from_file(file_t* _file, disk_t* _disk, fat * _fat);
#endif