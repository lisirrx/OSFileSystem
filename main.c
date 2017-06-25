#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <malloc.h>
#include <memory.h>
#include <zconf.h>
#include <stdlib.h>
#include "Config.h"
#include "FileSystem.h"

int main(){
    disk_t disk;
    init_block(&disk);
    fat fat;
    file_t* current;
    int flag = access("os.vdisk", 0);
    FILE* f;
    if(flag == -1) {
        f = fopen("os.vdisk","ab+");
        current = file_sys_init(&disk, &fat);
    } else{
        f = fopen("os.vdisk", "wb+");
        current = file_sys_read(&disk, &fat, f);
    }



    char dir_name[1000] = "/";
    printf("Welcome to XFS\n");
    printf("cd <dir> --open a dir\n");
    printf("ls <dir> -- list files\n");
    printf("mkdir <dir> --create a dir\n");
    printf("touch <file> --create a file\n");
    printf("rm <file>/<dir> --remove a file/dir\n");
    printf("save --save files\n");
    printf("quit --quit the system\n");
    printf("$ ");

    while(1){
        char ins[100];
        char name[100];
        scanf("%s", ins);


        if((strcmp(ins, "ls") == 0)) {
//            int cnt = current->len / ENTRY_SIZE;
//            if (cnt != 0) {
            file_t **files = get_children(current, &disk, &fat);
            int i = 0;
            while(&files[i] != NULL) {
                printf("%s\t%s\t%d-%d-%d %d:%d:%d\t%d bytes\n", files[i]->name, files[i]->ext,
                       files[i]->date[0] + 1900, files[i]->date[1] + 1, files[i]->date[2],
                       files[i]->date[3], files[i]->date[4], files[i]->date[5], files[i]->len);
                i++;
            }


        } else if(strcmp(ins, "exit") == 0) {
            save_disk(&disk, &fat, f);
            exit(0);

        } else {
            scanf("%s", name);
            if (strcmp(ins, "cd") == 0) {
                int cnt = current->len / ENTRY_SIZE;
                file_t **files = get_children(current, &disk, &fat);
                int flag = 0;
                for (int i = 0; i < cnt; ++i) {
                    if (strcmp(files[i]->name, name) == 0) {
                        flag = 1;
                        current = files[i];
                        strcat(dir_name, files[i]->name);
                    }
                }
                if (flag == 0) {
                    printf("Not exist.\n");
                }
            } else if (strcmp(ins, "mkdir") == 0) {
                create_dir(&disk, &fat, current, name);
            } else if (strcmp(ins, "touch") == 0) {
                file_t *f = malloc(sizeof(file_t));
                if (strstr(name, ".") == NULL) {
                    create_file( &disk, &fat, current, name, "");
                } else {
                    char ext[3];
                    ext[0] = name[strlen(name) - 3];
                    ext[1] = name[strlen(name) - 2];
                    ext[2] = name[strlen(name) - 1];
                    char new_name[strlen(name) - 4];
                    strncpy(new_name, name, strlen(name) - 4);
                    create_file(&disk, &fat, current, new_name, ext);
                }

            }
        }
        printf("$ ");
    }
}