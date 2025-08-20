#include "FATFile.hpp"

constexpr const char* module_name = "FATFile";

FATFile::FATFile()
    :   Opened(false),
        isRootDirectory(false),
        position(),
        size(),
        FirstCluster(),
        CurrentCluster(),
        CurrentSectorInCluster()
{
}


bool FATFile::Open(FATFileSystem* fileSystem, uint32_t firstCluster, uint32_t size){
    
    this->isRootDirectory = false;
    this->position = 0;
    this->size = size;
    this->FirstCluster = firstCluster;
    this->CurrentCluster = this->FirstCluster;
    this->CurrentSectorInCluster = 0;

    if (!fileSystem->ReadSectorFromCluster(this->FirstCluster, CurrentSectorInCluster, this->Buffer)){
        Debug::Error(module_name, "Failed to open file!!");
        return false;
    }
    this->Opened = true;
}

bool FATFile::OpenFat1216RootDirectory(FATFileSystem* fileSystem, uint32_t rootDirectoryLba, uint32_t rootDirectorySize){
    this->isRootDirectory = true;
    this->position = 0;
    this->size = rootDirectorySize;
    this->FirstCluster = rootDirectoryLba;
    this->CurrentCluster = this->FirstCluster;
    this->CurrentSectorInCluster = 0;

    if (!fileSystem->ReadSector(this->FirstCluster, this->Buffer)){
        Debug::Error(module_name,"Read root directory failed!!");
        return false;
    }
}


bool FATFile::ReadFileEntry(FATDirectoryEntry* dirEntry){
    return Read(reinterpret_cast<uint8_t*>(dirEntry), sizeof(FATDirectoryEntry)) == sizeof(FATDirectoryEntry);
}

size_t FATFile::Read(uint8_t* data, size_t size) {
    
} 

bool FATFile::Seek(SeekPos pos, int rel) {
    uint32_t ClusterSize = fs->GetFatData().BS.BootSector.SectorsPerCluster * SectorSize;
    switch (pos)
    {
    case SeekPos::Set:
        CurrentCluster = FirstCluster;
        CurrentSectorInCluster = 0;
        position = 0;
        while(rel > 0){
            CurrentCluster = fs->GetNextCluster(CurrentCluster);
            position += ClusterSize;
            rel -= ClusterSize;

        }

        CurrentSectorInCluster = rel / SectorSize;
        position = rel % SectorSize;
        if (!fs->ReadSectorFromCluster(this->FirstCluster, CurrentSectorInCluster, this->Buffer)){
            Debug::Error(module_name, "Failed to open file!!");
            return;
        }
        break;
    case SeekPos::Current:
        
        break;
    case SeekPos::End:
        
        break;
    
    default:
        break;
    }
}

size_t FATFile::Size() {
    
}

size_t FATFile::Position() {
    
}

FileEntry FATFile::GetNextFileEntry(File* parent, const FileEntry& previous){

}
