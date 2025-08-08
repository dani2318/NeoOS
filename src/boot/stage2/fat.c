#include "fat.h"

#define SECTOR_SIZE 512
#define MAX_PATH_SIZE 256
#define MAX_FILE_HANDLES 10
#define ROOT_DIRECTORY_HANDLE -1


struct FAT_BootSector
{
    uint8_t BootJumpInstruction[3];
    uint8_t OemIdentifier[8];
    uint16_t BytesPerSector;
    uint8_t SectorsPerCluster;
    uint16_t ReservedSectors;
    uint8_t FatCount;
    uint16_t DirEntryCount;
    uint16_t TotalSectors;
    uint8_t MediaDescriptorType;
    uint16_t SectorsPerFat;
    uint16_t SectorsPerTrack;
    uint16_t Heads;
    uint32_t HiddenSectors;
    uint32_t LargeSectorCount;

    // extended boot record
    uint8_t DriveNumber;
    uint8_t _Reserved;
    uint8_t Signature;
    uint32_t VolumeId;          // serial number, value doesn't matter
    uint8_t VolumeLabel[11];    // 11 bytes, padded with spaces
    uint8_t SystemId[8];

    // ... we don't care about code ...

} __attribute__((packed));


typedef struct FAT_BootSector FAT_BootSector;


struct FAT_FileData{
    uint8_t Buffer[SECTOR_SIZE];
    FAT_File Public;
    bool Opened;
    uint32_t FirstCluster;
    uint32_t CurrentCluster;
    uint32_t CurrentSectorInCluster;
};

typedef struct FAT_FileData FAT_FileData;


struct FAT_Data
{
    union{
        FAT_BootSector BootSector;
        uint8_t BootSectorBytes[SECTOR_SIZE];
    } BS;

    FAT_FileData RootDirectory;
    FAT_FileData OpenedFiles[MAX_FILE_HANDLES];
};

typedef struct FAT_Data FAT_Data;

static FAT_Data* g_Data;
static uint8_t* g_Fat = NULL;
static uint32_t g_DataSectionLBA;

uint32_t FAT_ClusterToLba(uint32_t cluster)
{
    return g_DataSectionLBA + (cluster - 2) * g_Data->BS.BootSector.SectorsPerCluster;
}

bool FAT_ReadBootSector(Partition* disk)
{
    return Partition_ReadSectors(disk, 0, 1, g_Data->BS.BootSectorBytes);
}

bool FAT_ReadFat(Partition* disk)
{
    printf("partition offset: 0x%x; partition size: %d",disk->Offset, disk->Size);
    return Partition_ReadSectors(disk, g_Data->BS.BootSector.ReservedSectors, g_Data->BS.BootSector.SectorsPerFat, g_Fat);
}

bool FAT_Initialize(Partition* disk){

    g_Data = (FAT_Data*)MEMORY_FAT_ADDR;

    if (!FAT_ReadBootSector(disk)){
        printf("partition offset: 0x%x; partition size: %d\n\r",disk->Offset, disk->Size);
        printf("======== Drive %d info ========\r\n", disk->disk->id);
        printf("Cylinders: %d\r\n", disk->disk->cylinders);
        printf("Sectors: %d\r\n", disk->disk->sectors);
        printf("Heads: %d\r\n", disk->disk->heads);
        printf("======== Drive %d info ========\r\n",  disk->disk->id);
        printf("BS.BootSector.BytesPerSector = %d\r\n",g_Data->BS.BootSector.BytesPerSector);
        printf("g_Data->BS.BootSector.SectorsPerFat = %d\r\n",g_Data->BS.BootSector.SectorsPerFat);


        printf("[FAT] [FAT_ReadBootSector] Failed to read bootsector!\r\n");
        return false;
    }


    g_Fat = (uint8_t *) g_Data + sizeof(FAT_Data);
    printf("BS.BootSector.BytesPerSector = %d\r\n",g_Data->BS.BootSector.BytesPerSector);
    printf("g_Data->BS.BootSector.SectorsPerFat = %d\r\n",g_Data->BS.BootSector.SectorsPerFat);

    uint32_t fatSize = g_Data->BS.BootSector.BytesPerSector * g_Data->BS.BootSector.SectorsPerFat;

    if(sizeof(FAT_Data) + fatSize >= MEMORY_FAT_SIZE){
        printf("[FAT] [FAT_Initialize] Not enough memory to read FAT! Required %lu, only have %lu!\r\n",sizeof(FAT_Data) + fatSize, MEMORY_FAT_SIZE);
        return false;
    }

    if(!FAT_ReadFat(disk)){
        printf("[FAT] [FAT_ReadFat] FAT read failed!\r\n");
        return false;
    }


    uint32_t rootDirLBA = g_Data->BS.BootSector.ReservedSectors + g_Data->BS.BootSector.SectorsPerFat * g_Data->BS.BootSector.FatCount;
    uint32_t rootDirSize = sizeof(FAT_DirectoryEntry) * g_Data->BS.BootSector.DirEntryCount;

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

    uint32_t rootDirectorySectors = (rootDirSize + g_Data->BS.BootSector.BytesPerSector - 1) / g_Data->BS.BootSector.BytesPerSector;
    g_DataSectionLBA = rootDirLBA + rootDirectorySectors;

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

uint32_t FAT_NextCluster(uint32_t currentCluster){
    uint32_t fatIndex = currentCluster * 3 / 2;
    if(currentCluster % 2 == 0){
        return (*(uint16_t *)(g_Fat + fatIndex)) & 0x0FFF;
    }else {
        return (*(uint16_t *)(g_Fat + fatIndex)) >> 4;
    }
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
                    fd->CurrentCluster = FAT_NextCluster(fd->CurrentCluster);
                }

                if(fd->CurrentCluster >= 0xFF8){
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

bool FAT_FindFile(Partition* disk, FAT_File * file, const char* name, FAT_DirectoryEntry* entryOut)
{
    char fatName[12];
    FAT_DirectoryEntry entry;

    // convert from name to fat name
    memset(fatName, ' ', sizeof(fatName));
    fatName[11] = '\0';

    const char* ext = strchr(name, '.');
    if (ext == NULL)
        ext = name + 11;

    for (int i = 0; i < 8 && name[i] && name + i < ext; i++)
        fatName[i] = toUpper(name[i]);

    if (ext != name + 11)
    {
        for (int i = 0; i < 3 && ext[i + 1]; i++)
            fatName[i + 8] = toUpper(ext[i + 1]);
    }

    while (FAT_ReadEntry(disk, file, &entry))
    {
        // printf("FAT name: %s\r\n", fatName);
        // printf("Entry name: %s\r\n", entry.Name);
        if (memcmp(fatName, entry.Name, 11) == 0)
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
            name[delim - path + 1] = '\0';
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
