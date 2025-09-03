#pragma once

enum class FileType{
    File,
    Directory
};
enum FileOpenMode{
    Read,
    Write,
    Append
};
class FileEntry {
    public:
        virtual const char* Name() = 0;
        virtual const FileType Type() = 0;
        virtual File* Open(FileOpenMode mode) = 0;
};