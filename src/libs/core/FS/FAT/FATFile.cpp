#include "FATFile.hpp"

bool FATFile::Open(FATFileEntry* fileEntry){
    this->Position = 0;
    this->Size = fileEntry->directoryEntry.Size;
    this->FirstCluster = fileEntry->directoryEntry.FirstClusterLow + ((uint32_t)fileEntry->directoryEntry.FirstClusterHigh << 16);
    this->CurrentCluster = this->FirstCluster;
    this->CurrentSectorInCluster = 0;

    if (!ReadSector(rootDirLBA, Data->RootDirectory.Buffer)){
        Debug::Error(module_name,"[ReadBootSector] [FAT_Initialize] Read root directory failed!!");
        return false;
    }
}
