#include <stdint.h>
#include "stdio.h"
#include "disk.h"
#include "x86.h"
// #include "fat.h"

void* g_data = (void*) 0x20000;

void __attribute__((cdecl)) start(uint16_t bootDrive){
    clrscr();
    printf("Loaded stage2 !!!\r\n");
    DISK disk;
    if(!DISK_Initialize(&disk, bootDrive)){
        printf("[BOOT] Disk init error!\r\n");
        goto end;
    }

    end:
    for(;;);


    // if(!FAT_Initialize(&disk)){
    //     printf("[BOOT] FAT init error!\r\n");
    //     goto end;
    // }

    // FAT_File * fd = FAT_Open(&disk, "/");
    // FAT_DirectoryEntry entry;
    // int i = 0;
    // while(FAT_ReadEntry(&disk, fd, &entry) && i <= 3){
    //     printf("    ");
    //     for(int i = 0; i < 11; i++){
    //         putc(entry.Name[i]);
    //     }
    //     printf("\r\n");
    //     i++;
    // }
    // FAT_Close(fd);

}
