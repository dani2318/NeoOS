#include <stdint.h>
// #include "stdio.h"
// #include "disk.h"
// #include "fat.h"


void __attribute__((cdecl)) start(uint16_t bootDrive){

    // DISK disk;
    // if(!DISK_Initialize(&disk, bootDrive)){
    //     printf("[BOOT] Disk init error!\r\n");
    //     goto end;
    // }

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

    // end:
    //     for(;;);
}
