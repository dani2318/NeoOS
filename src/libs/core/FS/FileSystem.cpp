#include "FileSystem.hpp"
#include <core/Debug.hpp>

#define module_name "FileSystem"


File* FileSystem::Open(const char* path, FileOpenMode mode){
    char name[MAX_PATH_SIZE];

    // ignore leading slash
    if (path[0] == '/')
        path++;


    File* root = this->RootDirectory();

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

        Debug::Info(module_name,"Searching for: %s", name);

        // find directory entry in current directory
        FileEntry* entry = root->ReadFileEntry();
        while(entry != nullptr){
            if(strcmp(entry->Name(), name) == 0){
                //TODO: Close root & release
                root = entry->Open(isLast ? mode : FileOpenMode::Read);
                entry = nullptr;
                if(isLast) return root;
            }else{
                root->FreeFileEntry(entry);
                entry = root->ReadFileEntry();
            }
            // check if directory
            if (entry->Type() == FileType::Directory)
            {
                Debug::Error(module_name," %s not a director", name);
                return NULL;
            }
        }
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
