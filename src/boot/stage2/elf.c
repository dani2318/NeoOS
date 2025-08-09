#include "elf.h"
#include "fat.h"
#include "memdefs.h"
#include "minmax.h"
#include "memory.h"
#include <stdio.h>

bool ELF_Read(Partition* partition, const char* path,void** entryPoint){


    uint8_t* headerBuffer = (uint8_t*)MEMORY_ELF_ADDR;
    uint8_t* loadBuffer = (uint8_t*)MEMORY_LOAD_KERNEL;
    uint32_t filePos = 0;
    uint32_t read;
    FAT_File* fd = FAT_Open(partition, path);
    if ((read = FAT_Read(partition, fd, sizeof(ELFHeader), headerBuffer)) != sizeof(ELFHeader))
    {
        printf("ELF Load error!\n");
        return false;
    }
    filePos += read;
    bool ok = true;

    ELFHeader* header  = (ELFHeader*) headerBuffer;
    ok &= (memcmp(header->Magic,ELF_MAGIC, 4) != 0);
    ok &= header->Bitness == ELF_BITNESS_32BIT;
    ok &= header->Endianness == ELF_ENDIANNESS_LITTLE;
    ok &= header->ELFHeaderVersion == 1;
    ok &= header->ElfVersion == 1;
    ok &= header->Type == ELF_TYPE_EXECUTABLE;
    ok &= header->InstructionSet == ELF_INSTRUCTIONSET_X86;

    *entryPoint = (void*) header->ProgramEntryPosition;

    uint32_t programHeaderOffset = header->ProgramHeaderTablePosition;
    uint32_t programHeaderSize = header->ProgramHeaderTableEntrySize * header->ProgramHeaderTableEntryCount;
    uint32_t programHeaderTableEntrySize = header->ProgramHeaderTableEntrySize;
    uint32_t programHeaderTableEntryCount = header->ProgramHeaderTableEntryCount;
    filePos += FAT_Read(partition, fd, programHeaderOffset - filePos, headerBuffer);

    if((read = FAT_Read(partition, fd, programHeaderSize, headerBuffer)) != programHeaderSize){
        printf("[ELF][ELFOpen] Load error!1\r\n");
        return false;
    }

    filePos += read;
    FAT_Close(fd);

    for(uint32_t i = 0; i < programHeaderTableEntryCount; i++){
        ELFProgramHeader* progHeader = (ELFProgramHeader*)(headerBuffer + i * programHeaderTableEntrySize);
        if(progHeader->Type == ELF_PROGRAM_TYPE_LOAD){
            uint8_t* virtAddr = (uint8_t*)progHeader->VirtualAddress;
            //TODO: validate that the program do not overwrite the stage2.
            memset(virtAddr, 0, progHeader->MemorySize);
            //TODO: Proper seeking here.
            fd = FAT_Open(partition, path);

            while(progHeader->Offset > 0){
                uint32_t shouldRead = min(progHeader->Offset, MEMORY_LOAD_SIZE);
                read = FAT_Read(partition, fd, shouldRead, loadBuffer);
                if(read != shouldRead){
                    printf("[ELF][ELFOpen] Load error!2\r\n");
                    return false;
                }
                progHeader->Offset -= read;
                printf("[ELF][ELFOpen] Offset: %d\r\n",progHeader->Offset);
                printf("[ELF][ELFOpen] Read: %d\r\n",read);
                printf("[ELF][ELFOpen] ShouldRead: %d\r\n",shouldRead);
            }

            while(progHeader->FileSize > 0){
                uint32_t shouldRead = min(progHeader->FileSize, MEMORY_LOAD_SIZE);
                read = FAT_Read(partition, fd, shouldRead, loadBuffer);
                if(read != shouldRead){
                    printf("[ELF][ELFOpen] Load error!3\r\n");
                    return false;
                }

                printf("[ELF][ELFOpen] File size: %d\r\n",progHeader->FileSize);
                printf("[ELF][ELFOpen] Read: %d\r\n",read);
                printf("[ELF][ELFOpen] ShouldRead: %d\r\n",shouldRead);

                progHeader->FileSize -= read;
                memcpy(virtAddr, loadBuffer, read);
                virtAddr += read;
                printf("[ELF][ELFOpen] Virtaddr: 0x%x\r\n",virtAddr);
            }
            FAT_Close(fd);
        }
    }


    return true;
}