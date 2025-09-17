#ifndef DIRS_H
#define DIRS_H

namespace Dirs {
    inline constexpr const char* STATE = "state";
    inline constexpr const char* TORRENTS = "torrents";
    inline constexpr const char* METADATA = "metadata";
    inline constexpr const char* THEMES = "themes";
}

namespace FileEnding{
    inline constexpr const char* DOT_TORRENT = ".torrent";
    inline constexpr const char* DOT_FASTRESUME = ".fastresume";
}

#endif // DIRS_H
