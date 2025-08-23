#include "FATFileSystem.hpp"
#include <Debug.hpp>

constexpr const char* module_name = "FAT";

constexpr int ROOT_DIRECTORY_HANDLE = -1;

FATFileSystem::FATFileSystem()
    : device(), 
      Data(new FATData()),
      DataSectionLba(),
      FatType(),
      TotalSectors(),
      SectorsPerFat() { 


}

void FATFileSystem::Detect() {
    uint32_t dataCluster = (TotalSectors - DataSectionLba) / Data->BS.BootSector.SectorsPerCluster;
    if (dataCluster < 0xFF5) {
        FatType = FAT12;
    } else if (Data->BS.BootSector.SectorsPerFat != 0) {
        FatType = FAT16;
    } else{
        FatType = FAT32;
    }
}

uint32_t FATFileSystem::ClusterToLba(uint32_t cluster)
{
    return DataSectionLba + (cluster - 2) * Data->BS.BootSector.SectorsPerCluster;
}

bool FATFileSystem::ReadSectorFromCluster(uint32_t cluster, uint32_t sectorOffset, uint8_t* buffer){
    return ReadSector(this->ClusterToLba(cluster) + sectorOffset, buffer);
}


bool FATFileSystem::ReadBootSector(){
    return ReadSector(0, Data->BS.BootSectorBytes);
}

bool FATFileSystem::ReadSector(uint32_t lba, uint8_t* buffer, size_t count){
    this->device->Seek(SeekPos::Set, lba*SectorSize);
    return this->device->Read(buffer, count * SectorSize) == count *SectorSize;
}

bool FATFileSystem::Initialize(BlockDevice* device) {
    this->device = device;

    if(!ReadBootSector()){
        Debug::Error(module_name,"Failed to read bootsector!");
        return false;
    }

    Data->FatCachePosition = 0xFFFFFFFF;

    TotalSectors = Data->BS.BootSector.TotalSectors;

    // If 'TotalSectors' is 0 then we are using FAT32 and should use 'g_Data->BS.BootSector.LargeSectorCount'
    if(TotalSectors == 0){
        TotalSectors = Data->BS.BootSector.LargeSectorCount;
    }

    bool isFAT32 = false;
    SectorsPerFat = Data->BS.BootSector.SectorsPerFat;
    // If 'SectorsPerFat' is 0 then we are using FAT32 and should use 'g_Data->BS.BootSector.EBR32.SectorsPerFat'
    if(SectorsPerFat == 0){
        isFAT32 = true;
        SectorsPerFat = Data->BS.BootSector.EBR32.SectorsPerFat;
    }
    
    // Define the root directory LBA and Size.

    if (isFAT32) {
        DataSectionLba = Data->BS.BootSector.ReservedSectors + SectorsPerFat * Data->BS.BootSector.FatCount;

        // Initialize Root Directory
        FileEntry rootEntry;
        FATDirectoryEntry* rootDirectoryEntry = reinterpret_cast<FATDirectoryEntry*> (rootEntry.FSData);
        rootDirectoryEntry->FirstClusterLow = Data->BS.BootSector.EBR32.RootdirCluster & 0xFFFF;
        rootDirectoryEntry->FirstClusterHigh = Data->BS.BootSector.EBR32.RootdirCluster >> 16;
        rootDirectoryEntry->Size = 0xFFFFFFFF;
        uint32_t size = rootDirectoryEntry->Size;
        uint32_t firstCluster = rootDirectoryEntry->FirstClusterLow + ((uint32_t)rootDirectoryEntry->FirstClusterHigh << 16);
        if(!Data->RootDirectory.Open(this, Data->BS.BootSector.EBR32.RootdirCluster, 0xFFFFFFFF, true))
            return false;
    } else {
        uint32_t rootDirLBA = Data->BS.BootSector.ReservedSectors + SectorsPerFat * Data->BS.BootSector.FatCount;
        uint32_t rootDirSize = sizeof(FATDirectoryEntry) * Data->BS.BootSector.DirEntryCount;
        uint32_t rootDirectorySectors = (rootDirSize + Data->BS.BootSector.BytesPerSector - 1) / Data->BS.BootSector.BytesPerSector;
        DataSectionLba = rootDirLBA + rootDirectorySectors;
        if(!Data->RootDirectory.OpenFat1216RootDirectory(this, rootDirLBA, rootDirSize))
            return false;
    }

    Detect();


    for(int i = 0; i < MaxFileHandles; i++)
        Data->OpenedFiles[i] = FATFile();
    
    Data->LFNCount = 0;

    return true;
}

File* FATFileSystem::Open(FileEntry* file, FileOpenMode mode){
    int handle = -1;

    for (int i = 0; i < MaxFileHandles && handle < 0; i++){
        if(!Data->OpenedFiles[i].isOpened()){
            handle = i;
        }
    }
    if(handle < 0){
        Debug::Error(module_name, "Run out of HANDLES!");
        return nullptr;
    }
    const FATDirectoryEntry* directoryEntry = reinterpret_cast<const FATDirectoryEntry*>(file);
    uint32_t size = directoryEntry->Size;
    uint32_t FirstCluster = directoryEntry->FirstClusterLow + ((uint32_t)directoryEntry->FirstClusterHigh << 16);
    Data->OpenedFiles[handle].Open(this, FirstCluster, size, directoryEntry->Attributes & FAT_ATTRIBUTE_DIRECTORY);

    return &Data->OpenedFiles[handle];
}


File* FATFileSystem::RootDirectory(){
    return &Data->RootDirectory;
}


uint32_t FATFileSystem::GetNextCluster(uint32_t currentCluster){

    // Determine the offset of the entry to read.
    uint32_t fatIndex;
    if (FatType == FAT12) {
        fatIndex = currentCluster * 3 / 2;
    } else if(FatType == FAT16) {
        fatIndex = currentCluster * 2;
    } else /* if (g_FatType == 32)*/ {
        fatIndex = currentCluster * 4;
    }

    // Check if cache has correct number

    uint32_t fatIndexSector = fatIndex / SectorSize;

    if( fatIndexSector < Data->FatCachePosition
        || fatIndexSector >= Data->FatCachePosition + SectorSize
    ){
        ReadFat(fatIndexSector);
        Data->FatCachePosition = fatIndexSector;
    }

    fatIndex -= (Data->FatCachePosition * SectorSize);

    uint32_t nextCluster;
    if (FatType == FAT12) {
        if(currentCluster % 2 == 0){
            nextCluster = (*(uint16_t *)(Data->FatCache + fatIndex)) & 0x0FFF;
        }else {
            nextCluster = (*(uint16_t *)(Data->FatCache + fatIndex)) >> 4;
        }

        if (nextCluster >= 0xFF8) {
            nextCluster |= 0xFFFFF000;
        }

    } else if(FatType == FAT16) {
        nextCluster = *(uint16_t *)(Data->FatCache + fatIndex);

        if (nextCluster >= 0xFFF8) {
            nextCluster |= 0xFFFF0000;
        }

    } else /*if (g_FatType == 32)*/ {
        nextCluster = *(uint32_t *)(Data->FatCache + fatIndex);
    }

    return nextCluster;
}


bool FATFileSystem::ReadFat(uint32_t lbaOffset)
{
    return ReadSector(
        Data->BS.BootSector.ReservedSectors + lbaOffset,
        Data->FatCache,
        FatCacheSize
    );
}