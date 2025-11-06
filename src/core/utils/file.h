#ifndef FILE_H
#define FILE_H
#include <QString>

struct File
{
    int           id;
    bool          isEnabled;
    QString       filename;
    std::uint64_t filesize;   // in bytes
    std::uint64_t downloaded; // in bytes
    int           priority;
};

enum class FileFields
{
    STATUS,
    FILENAME,
    PROGRESS,
    DOWNLOADED,
    FILESIZE,
    PRIORITY,
    ID = Qt::UserRole + 1
};
// [^] Fuck.txt 50% 100 MB 190 MB High

constexpr int FILE_FIELD_COUNT = (int)FileFields::PRIORITY + 1;

#endif // FILE_H
