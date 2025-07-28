#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <QObject>
#include <fstream>
#include <vector>
#include <libtorrent/libtorrent.hpp>
#include <QTimer>
#include <QStandardPaths>
#include <QDir>


constexpr const char* SESSION_FILENAME = ".session";

class SessionManager : public QObject
{
    Q_OBJECT

    std::unique_ptr<lt::session> m_session;
    // QList<lt::torrent_handle> m_torrentHandles;
    QHash<QString, lt::torrent_handle> m_torrentHandles;

    QTimer m_alertTimer;
    QTimer m_resumeDataTimer;
public:
    explicit SessionManager(QObject *parent = nullptr);
    ~SessionManager();

    void addTorrentByFilename(QStringView filepath);
    void addTorrentByMagnet(QString magnetURI);
private:
    void eventLoop();

    void loadResumes();
    void addTorrent(lt::add_torrent_params params);
signals:
};

inline std::vector<char> readFile(const char *filename)
{
    std::ifstream file(filename, std::ios_base::binary);
    file.unsetf(std::ios_base::skipws);
    return {std::istream_iterator<char>{file}, std::istream_iterator<char>{}};
}

inline void writeTorrentFile(const lt::torrent_info& ti) {
    auto basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    auto stateDirPath = basePath + QDir::separator() + "torrents";

    auto stateFilePath = stateDirPath + QDir::separator() + QString{lt::aux::to_hex(ti.info_hashes().get_best().to_string()).c_str()} + ".torrent";
    QFile file{stateFilePath};

    if (file.open(QIODevice::WriteOnly)) {
        lt::create_torrent ci{ti};
        auto buf = ci.generate_buf();
        file.write(buf.data(), buf.size());

        file.flush();
        file.close();
    }
}

inline void saveResumeData(const lt::torrent_info& ti, const std::vector<char> buf) {
    auto basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    auto stateDirPath = basePath + QDir::separator() + "state";
    auto stateFilePath = stateDirPath + QDir::separator() + QString{lt::aux::to_hex(ti.info_hashes().get_best().to_string()).c_str()} + ".fastresume";
    QFile file{stateFilePath};
    if (file.open(QIODevice::WriteOnly)) {
        file.write(buf.data(), buf.size());

        file.flush();
        file.close();
    }
}

#endif // SESSIONMANAGER_H
