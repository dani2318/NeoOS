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

    Detect();

    // Initialize Root Directory
    FATFileEntry rootEntry;
    rootEntry.directoryEntry.FirstClusterLow = rootDirLBA & 0xFF;
    rootEntry.directoryEntry.FirstClusterHigh = rootDirLBA >> 16;
    rootEntry.directoryEntry.Size = sizeof(FATDirectoryEntry) * Data->BS.BootSector.DirEntryCount;

    Data->RootDirectory.Open(&rootEntry);


    for(int i = 0; i < MaxFileHandles; i++)
        Data->OpenedFiles[i] = FATFile();
    
    Data->LFNCount = 0;

    return true;
}

File* FATFileSystem::Open(FileEntry* parent, FileOpenMode mode){
    int handle = -1;

    for (int i = 0; i < MaxFileHandles && handle < 0; i++){
        if(!Data->OpenedFiles[i].isOpened()){
            handle = i;
        }
    }
    if(handle < 0){
        Debug::Error(module_name, "[FAT_OpenEntry] Run out of HANDLES!");
        return nullptr;
    }

    Data->OpenedFiles[handle].Open((FATFileEntry*)parent);

    return &Data->OpenedFiles[handle];
}

FileEntry* FATFileSystem::GetNextFileEntry(File* parent, const FileEntry& previous){

}
