#include "FATFileSystem.hpp"
#include <Debug.hpp>

constexpr const char* module_name = "FAT";

constexpr int SectorSize      =  512;
constexpr int MaxFileHandles  =  10;
constexpr int FatCacheSize    =  5;    
constexpr int ROOT_DIRECTORY_HANDLE = -1;

FATFileSystem::FATFileSystem(void* fsMemory)
    : device(), 
      Data(reinterpret_cast<FATData*>(fsMemory)),
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


bool FATFileSystem::ReadBootSector(){
    return ReadSector(0, Data->BS.BootSectorBytes);
}

bool FATFileSystem::ReadSector(uint32_t lba, uint8_t* buffer){
    this->device->Seek(SeekPos::Set, lba*SectorSize);
    return this->device->Read(buffer, SectorSize) == SectorSize;
}

bool FATFileSystem::Initialize(BlockDevice* device) {
    this->device = device;

    if(!ReadBootSector()){
        Debug::Error(module_name,"[ReadBootSector] Failed to read bootsector!");
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
    uint32_t rootDirLBA;
    uint32_t rootDirSize;
    if (isFAT32) {
        DataSectionLba = Data->BS.BootSector.ReservedSectors + SectorsPerFat * Data->BS.BootSector.FatCount;
        rootDirLBA = ClusterToLba(Data->BS.BootSector.EBR32.RootdirCluster);
        rootDirSize = 0;
    } else {
        rootDirLBA = Data->BS.BootSector.ReservedSectors + SectorsPerFat * Data->BS.BootSector.FatCount;
        rootDirSize = sizeof(FATDirectoryEntry) * Data->BS.BootSector.DirEntryCount;
        uint32_t rootDirectorySectors = (rootDirSize + Data->BS.BootSector.BytesPerSector - 1) / Data->BS.BootSector.BytesPerSector;
        DataSectionLba = rootDirLBA + rootDirectorySectors;
    }

    // Initialize Root Directory
    Data->RootDirectory.Public.Handle = ROOT_DIRECTORY_HANDLE;
    Data->RootDirectory.Public.IsDirectory = true;
    Data->RootDirectory.Public.Position = 0;
    Data->RootDirectory.Public.Size =  sizeof(FATDirectoryEntry) * Data->BS.BootSector.DirEntryCount;
    Data->RootDirectory.Opened = true;
    Data->RootDirectory.FirstCluster = rootDirLBA;
    Data->RootDirectory.CurrentCluster = rootDirLBA;
    Data->RootDirectory.CurrentSectorInCluster = 0;


    if (!ReadSector(rootDirLBA, Data->RootDirectory.Buffer)){
        Debug::Error(module_name,"[ReadBootSector] [FAT_Initialize] Read root directory failed!!");
        return false;
    }

    Detect();

    for(int i = 0; i < MaxFileHandles; i++)
        Data->OpenedFiles[i].Opened = false;

    return true;    //TODO: ADD DETECTION HERE
}

File* FATFileSystem::Open(FileEntry* parent, FileOpenMode mode){
    
}

FileEntry* FATFileSystem::GetNextFileEntry(FileEntry* parent, FileEntry* previous){

}
