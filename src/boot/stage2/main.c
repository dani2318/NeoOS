#include <stdint.h>
#include "stdio.h"
#include "disk.h"
#include "fat.h"

uint8_t* KernelLoadBuffer = (uint8_t*)MEMORY_LOAD_KERNEL;
uint8_t* Kernel = (uint8_t*)MEMORY_KERNEL_ADDR;

typedef void (*KernelStart)();

void __attribute__((cdecl)) start(uint16_t bootDrive){
    clrscr();
    printf("Loaded stage2 !!!\r\n");
    DISK disk;
    if(!DISK_Initialize(&disk, bootDrive)){
        printf("[BOOT] Disk init error!\r\n");
        goto end;
    }

    
    if(!FAT_Initialize(&disk)){
        printf("[BOOT] FAT init error!\r\n");
        goto end;
    }

    //Load kernel
    FAT_File * fd = FAT_Open(&disk, "/kernel.bin");
    uint32_t read;
    uint8_t* KernelBuffer = Kernel;
    while((read = FAT_Read(&disk, fd, MEMORY_LOAD_SIZE, KernelLoadBuffer))){
        memcpy(KernelBuffer, KernelLoadBuffer, read);
        KernelBuffer += read;
    }
    FAT_Close(fd);

    //Kernel start
    KernelStart kernelstart = (KernelStart)Kernel;
    kernelstart();

    end:
        for(;;);

}
