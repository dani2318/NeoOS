#include "fat.h"
#include "mbr.h"
#include <stddef.h>
#include <stdint.h>

#define SECTOR_SIZE 512
#define MAX_PATH_SIZE 256
#define MAX_FILE_HANDLES 10
#define ROOT_DIRECTORY_HANDLE -1

#define FAT_CACHE_SIZE 5        // In sectors

typedef struct{
    // extended boot record
    uint8_t  DriveNumber;
    uint8_t  _Reserved;
    uint8_t  Signature;
    uint32_t VolumeId;          // serial number, value doesn't matter
    uint8_t  VolumeLabel[11];    // 11 bytes, padded with spaces
    uint8_t  SystemId[8];
} __attribute__((packed)) FATExtendedBootRecord;

typedef struct{
    uint32_t SectorsPerFat;
    uint16_t Flags;
    uint16_t VersionNumber;
    uint32_t RootdirCluster;
    uint16_t FSInfoSector;
    uint16_t BackupBootSector;
    uint8_t  _Reserved[12];
    // extended boot record
    FATExtendedBootRecord EBR;
} __attribute__((packed)) FAT32ExtendedBootRecord;


struct FAT_BootSector
{
    uint8_t  BootJumpInstruction[3];
    uint8_t  OemIdentifier[8];
    uint16_t BytesPerSector;
    uint8_t  SectorsPerCluster;
    uint16_t ReservedSectors;
    uint8_t  FatCount;
    uint16_t DirEntryCount;
    uint16_t TotalSectors;
    uint8_t  MediaDescriptorType;
    uint16_t SectorsPerFat;
    uint16_t SectorsPerTrack;
    uint16_t Heads;
    uint32_t HiddenSectors;
    uint32_t LargeSectorCount;

    union{
        FATExtendedBootRecord   EBR1216;
        FAT32ExtendedBootRecord EBR32;
    };


} __attribute__((packed));


typedef struct FAT_BootSector FAT_BootSector;


struct FAT_FileData{
    uint8_t  Buffer[SECTOR_SIZE];
    FAT_File Public;
    bool     Opened;
    uint32_t FirstCluster;
    uint32_t CurrentCluster;
    uint32_t CurrentSectorInCluster;
};

typedef struct FAT_FileData FAT_FileData;

typedef struct {
    uint8_t Order;
    int16_t Chars[13];
} FATLFNBlock;

struct FAT_Data
{
    union{
        FAT_BootSector BootSector;
        uint8_t        BootSectorBytes[SECTOR_SIZE];
    } BS;

    FAT_FileData RootDirectory;
    FAT_FileData OpenedFiles[MAX_FILE_HANDLES];

    uint8_t  FatCache[FAT_CACHE_SIZE * SECTOR_SIZE];
    uint32_t FatCachePosition;

};

typedef struct FAT_Data FAT_Data;

static FAT_Data* g_Data;
static uint32_t  g_DataSectionLBA;
static uint8_t   g_FatType;
static uint32_t  g_TotalSectors;
static uint32_t  g_SectorsPerFat;

uint32_t FAT_ClusterToLba(uint32_t cluster)
{
    return g_DataSectionLBA + (cluster - 2) * g_Data->BS.BootSector.SectorsPerCluster;
}


int FAT_CompareLFNBlocks(const void* blockA, const void* blockB)
{
    FATLFNBlock* a = (FATLFNBlock*)blockA;
    FATLFNBlock* b = (FATLFNBlock*)blockB;
    return ((int)a->Order) - ((int)b->Order);
}

bool FAT_ReadBootSector(Partition* disk)
{
    return Partition_ReadSectors(disk, 0, 1, g_Data->BS.BootSectorBytes);
}

bool FAT_ReadFat(Partition* disk, size_t lbaIndex)
{
    return Partition_ReadSectors(
        disk,
        g_Data->BS.BootSector.ReservedSectors + lbaIndex,
        FAT_CACHE_SIZE,
        g_Data->FatCache
    );
}

void FAT_Detect(Partition* disk) {
    uint32_t dataCluster = (g_TotalSectors - g_DataSectionLBA) / g_Data->BS.BootSector.SectorsPerCluster;
    printf("datacluster=%x\r\n",dataCluster);
    if (dataCluster < 0xFF5) {
        g_FatType = FAT12;
    } else if (g_Data->BS.BootSector.SectorsPerFat != 0) {
        g_FatType = FAT16;
    } else{
        g_FatType = FAT32;
    }
}

uint32_t FAT_NextCluster(Partition* disk, uint32_t currentCluster){

    // Determine the offset of the entry to read.
    uint32_t fatIndex;
    if (g_FatType == FAT12) {
        fatIndex = currentCluster * 3 / 2;
    } else if(g_FatType == FAT16) {
        fatIndex = currentCluster * 2;
    } else /* if (g_FatType == 32)*/ {
        fatIndex = currentCluster * 4;
    }

    // Check if cache has correct number

    uint32_t fatIndexSector = fatIndex / SECTOR_SIZE;

    if( fatIndexSector < g_Data->FatCachePosition
        || fatIndexSector >= g_Data->FatCachePosition + FAT_CACHE_SIZE
    ){
        FAT_ReadFat(disk,fatIndexSector);
        g_Data->FatCachePosition = fatIndexSector;
    }

    fatIndex -= (g_Data->FatCachePosition * SECTOR_SIZE);

    uint32_t nextCluster;
    if (g_FatType == FAT12) {
        if(currentCluster % 2 == 0){
            nextCluster = (*(uint16_t *)(g_Data->FatCache + fatIndex)) & 0x0FFF;
        }else {
            nextCluster = (*(uint16_t *)(g_Data->FatCache + fatIndex)) >> 4;
        }

        if (nextCluster >= 0xFF8) {
            nextCluster |= 0xFFFFF000;
        }

    } else if(g_FatType == FAT16) {
        nextCluster = *(uint16_t *)(g_Data->FatCache + fatIndex);

        if (nextCluster >= 0xFFF8) {
            nextCluster |= 0xFFFF0000;
        }

    } else /*if (g_FatType == 32)*/ {
        nextCluster = *(uint32_t *)(g_Data->FatCache + fatIndex);
    }

    return nextCluster;
}

bool FAT_Initialize(Partition* disk) {

    g_Data = (FAT_Data*)MEMORY_FAT_ADDR;


    if (!FAT_ReadBootSector(disk)){

        printf("[FAT] [FAT_ReadBootSector] Failed to read bootsector!\r\n");
        return false;
    }
    
    g_Data->FatCachePosition = 0xFFFFFFFF;

    g_TotalSectors = g_Data->BS.BootSector.TotalSectors;

    // If 'TotalSectors' is 0 then we are using FAT32 and should use 'g_Data->BS.BootSector.LargeSectorCount'
    if(g_TotalSectors == 0){
        g_TotalSectors = g_Data->BS.BootSector.LargeSectorCount;
    }

    
    bool isFAT32 = false;
    g_SectorsPerFat = g_Data->BS.BootSector.SectorsPerFat;
    // If 'SectorsPerFat' is 0 then we are using FAT32 and should use 'g_Data->BS.BootSector.EBR32.SectorsPerFat'
    if(g_SectorsPerFat == 0){
        isFAT32 = true;
        g_SectorsPerFat = g_Data->BS.BootSector.EBR32.SectorsPerFat;
    }
    
    // Define the root directory LBA and Size.
    uint32_t rootDirLBA;
    uint32_t rootDirSize;
    if (isFAT32) {
        g_DataSectionLBA = g_Data->BS.BootSector.ReservedSectors + g_SectorsPerFat * g_Data->BS.BootSector.FatCount;
        rootDirLBA = FAT_ClusterToLba( g_Data->BS.BootSector.EBR32.RootdirCluster);
        rootDirSize = 0;
    } else {
        rootDirLBA = g_Data->BS.BootSector.ReservedSectors + g_SectorsPerFat * g_Data->BS.BootSector.FatCount;
        rootDirSize = sizeof(FAT_DirectoryEntry) * g_Data->BS.BootSector.DirEntryCount;
        uint32_t rootDirectorySectors = (rootDirSize + g_Data->BS.BootSector.BytesPerSector - 1) / g_Data->BS.BootSector.BytesPerSector;
        g_DataSectionLBA = rootDirLBA + rootDirectorySectors;
    }

    // Initialize Root Directory
    g_Data->RootDirectory.Public.Handle = ROOT_DIRECTORY_HANDLE;
    g_Data->RootDirectory.Public.IsDirectory = true;
    g_Data->RootDirectory.Public.Position = 0;
    g_Data->RootDirectory.Public.Size =  sizeof(FAT_DirectoryEntry) * g_Data->BS.BootSector.DirEntryCount;
    g_Data->RootDirectory.Opened = true;
    g_Data->RootDirectory.FirstCluster = rootDirLBA;
    g_Data->RootDirectory.CurrentCluster = rootDirLBA;
    g_Data->RootDirectory.CurrentSectorInCluster = 0;


    if (!Partition_ReadSectors(disk, rootDirLBA, 1, g_Data->RootDirectory.Buffer)){
        printf("[FAT] [FAT_Initialize] Read root directory failed!\r\n");
        return false;
    }

    FAT_Detect(disk);

    for(int i = 0; i < MAX_FILE_HANDLES; i++)
        g_Data->OpenedFiles[i].Opened = false;
    
    return true;
}

FAT_File* FAT_OpenEntry(Partition* disk, FAT_DirectoryEntry* entry){
    int handle = -1;

    for (int i = 0; i < MAX_FILE_HANDLES && handle < 0; i++){
        if(!g_Data->OpenedFiles[i].Opened){
            handle = i;
        }
    }
    if(handle < 0){
        printf("[FAT] [FAT_OpenEntry] Run out of HANDLES!\r\n");
        return false;
    }

    FAT_FileData * fd = &g_Data->OpenedFiles[handle];
    fd->Public.Handle = handle;
    fd->Public.IsDirectory = (entry->Attributes & FAT_ATTRIBUTE_DIRECTORY) != 0;
    fd->Public.Position = 0;
    fd->Public.Size = entry->Size;
    fd->FirstCluster = entry->FirstClusterLow + ((uint32_t)entry->FirstClusterHigh << 16);
    fd->CurrentCluster = fd->FirstCluster;
    fd->CurrentSectorInCluster = 0;

    if(!Partition_ReadSectors(disk, FAT_ClusterToLba(fd->CurrentCluster), 1, fd->Buffer)){
        printf("[FAT] [FAT_OpenEntry] Read error!\r\n");
        return false;
    }

    fd->Opened = true;
    return &fd->Public;
}



uint32_t FAT_Read(Partition* disk, FAT_File * file, uint32_t byteCount, void* dataOut){
    FAT_FileData * fd = (file->Handle == ROOT_DIRECTORY_HANDLE)
                                ? &g_Data->RootDirectory
                                : &g_Data->OpenedFiles[file->Handle];

    uint8_t* u8dataOut = (uint8_t*)dataOut;

    // don't read past the end of the file
    if (!fd->Public.IsDirectory || (fd->Public.IsDirectory && fd->Public.Size != 0))
        byteCount = min(byteCount, fd->Public.Size - fd->Public.Position);

    while(byteCount > 0){
        uint32_t leftInBuffer = SECTOR_SIZE - (fd->Public.Position % SECTOR_SIZE);
        uint32_t take = min(byteCount,leftInBuffer);

        memcpy(u8dataOut, fd->Buffer + fd->Public.Position % SECTOR_SIZE, take);
        u8dataOut += take;
        fd->Public.Position += take;
        byteCount -= take;
        if(leftInBuffer == take){
            if(fd->Public.Handle == ROOT_DIRECTORY_HANDLE){
                ++fd->CurrentCluster;
                if(!Partition_ReadSectors(disk, fd->CurrentCluster, 1, fd->Buffer)){
                    printf("[FAT] [FAT_Read] Read error!\r\n");
                    break;
                }
            } else {
                if(++fd->CurrentSectorInCluster >= g_Data->BS.BootSector.SectorsPerCluster){
                    fd->CurrentSectorInCluster = 0;
                    fd->CurrentCluster = FAT_NextCluster(disk, fd->CurrentCluster);
                }

                if(fd->CurrentCluster >= 0xFFFFFFF8){
                    fd->Public.Size = fd->Public.Position;
                    break;
                }

                if(!Partition_ReadSectors(disk, FAT_ClusterToLba(fd->CurrentCluster) + fd->CurrentSectorInCluster, 1, fd->Buffer)){
                    printf("[FAT] [FAT_Read] Read error!\r\n");
                    break;
                }
            }
        }
    }

    return u8dataOut - (uint8_t*) dataOut;
}

bool FAT_ReadEntry(Partition* disk, FAT_File * file, FAT_DirectoryEntry* dirEntry){
    return FAT_Read(disk,file,sizeof(FAT_DirectoryEntry),dirEntry) == sizeof(FAT_DirectoryEntry);
}

void FAT_Close(FAT_File * file){
    if (file->Handle == ROOT_DIRECTORY_HANDLE){
        file->Position = 0;
        g_Data->RootDirectory.CurrentCluster = g_Data->RootDirectory.FirstCluster;
    }else{
        g_Data->OpenedFiles[file->Handle].Opened = false;
    }
}

void FAT_GetShortName(const char* fileName, char* shortName, int length){

    // convert from name to fat name
    memset(shortName, ' ', length);
    shortName[11] = '\0';

    const char* ext = strchr(fileName, '.');
    if (ext == NULL)
        ext = fileName + 11;

    for (int i = 0; i < 8 && fileName[i] && fileName + i < ext; i++)
        shortName[i] = toUpper(fileName[i]);

    if (ext != fileName + 11)
    {
        for (int i = 0; i < 3 && ext[i + 1]; i++)
            shortName[i + 8] = toUpper(ext[i + 1]);
    }

}

bool FAT_FindFile(Partition* disk, FAT_File * file, const char* name, FAT_DirectoryEntry* entryOut)
{
    char shortName[12];
    //char longName[256];
    FAT_DirectoryEntry entry;

    FAT_GetShortName(name, shortName, 12);

    while (FAT_ReadEntry(disk, file, &entry))
    {

        /*if (entry.Attributes == FAT_ATTRIBUTE_LFN) {
            FAT_LongFileEntry* lfn = (FAT_LongFileEntry*)&entry;

            int idx = g_Data->LFNCount++;
            g_Data->LFNBlocks[idx].Order = lfn->Order & (FAT_LFN_LAST - 1);
            memcpy(g_Data->LFNBlocks[idx].Chars, lfn->Chars1, sizeof(lfn->Chars1));
            memcpy(g_Data->LFNBlocks[idx].Chars + 5, lfn->Chars2, sizeof(lfn->Chars2));
            memcpy(g_Data->LFNBlocks[idx].Chars + 11, lfn->Chars1, sizeof(lfn->Chars3));

            // is this the last LFN block
            if ((lfn->Order & FAT_LFN_LAST) != 0) {
                qsort(g_Data->LFNBlocks, g_Data->LFNCount, sizeof(FAT_LFNBlock), FAT_CompareLFNBlocks);
                char* namePos = longName;
                for (int i = 0; i < g_Data->LFNCount; i++)
                {
                    int16_t* chars = g_Data->LFNBlocks[i].Chars;
                    int16_t* charsLimit = chars + 13;

                    while (chars < charsLimit && *chars != 0)
                    {
                        int codepoint;
                        chars = utf16_to_codepoint(chars, &codepoint);
                        namePos = codepoint_to_utf8(codepoint, namePos);
                    }
                }
                *namePos = 0;
                printf("LFN: %s\n", longName);
            }
        }*/
        
        //Search for the file:
        if (memcmp(shortName, entry.Name, 11) == 0)
        {
            *entryOut = entry;
            return true;
        }
    }

    return false;
}


FAT_File * FAT_Open(Partition* disk, const char* path){
    char name[MAX_PATH_SIZE];

    // ignore leading slash
    if (path[0] == '/')
        path++;

    FAT_File* current = &g_Data->RootDirectory.Public;

    while (*path) {
        // extract next file name from path
        bool isLast = false;
        const char* delim = strchr(path, '/');
        if (delim != NULL)
        {
            memcpy(name, path, delim - path);
            name[delim - path] = '\0';
            path = delim + 1;
        }
        else
        {
            unsigned len = strlen(path);
            memcpy(name, path, len);
            name[len + 1] = '\0';
            path += len;
            isLast = true;
        }

        printf("Searching for: %s\r\n", name);

        // find directory entry in current directory
        FAT_DirectoryEntry entry;
        if (FAT_FindFile(disk, current, name, &entry))
        {
            FAT_Close(current);

            // check if directory
            if (!isLast && entry.Attributes & FAT_ATTRIBUTE_DIRECTORY == 0)
            {
                printf("FAT: %s not a directory\r\n", name);
                return NULL;
            }

            // open new directory entry
            current = FAT_OpenEntry(disk, &entry);
        }
        else
        {
            FAT_Close(current);

            printf("FAT: %s not found\r\n", name);
            return NULL;
        }
    }

    return current;
}
