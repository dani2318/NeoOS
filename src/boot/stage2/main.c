#include <stdint.h>
#include "stdio.h"
#include "disk.h"
#include "fat.h"
#include "mbr.h"
#include "elf.h"


uint8_t* KernelLoadBuffer = (uint8_t*)MEMORY_LOAD_KERNEL;
uint8_t* Kernel = (uint8_t*)MEMORY_KERNEL_ADDR;


typedef void (*KernelStart)();

void __attribute__((cdecl)) start(uint16_t bootDrive,void* partition){
    clrscr();

    printf("Loaded stage2 !!!\r\n");
    DISK disk;
    if(!DISK_Initialize(&disk, bootDrive)){
        printf("[BOOT] Disk init error!\r\n");
        goto end;
    }

    printf("[BOOT] Main partiton addr. : 0x%x\n\r", partition);
    Partition part;
    MBR_DetectPartition(&part, &disk, partition);

    if(!FAT_Initialize(&part)){
        printf("[BOOT] FAT init error!\r\n");
        goto end;
    }

    //Load kernel
    KernelStart kernelEntry;
    if (!ELF_Read(&part, "/boot/kernel.elf", (void**)&kernelEntry))
    {
        printf("ELF read failed, booting halted!");
        goto end;
    }

    // execute kernel
    kernelEntry();

    end:
        for(;;);

}
