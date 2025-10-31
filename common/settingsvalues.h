#ifndef SETTINGSVALUES_H
#define SETTINGSVALUES_H

struct SettingsNames
{
    static inline constexpr const char *const SESSION_DOWNLOAD_SPEED_LIMIT =
        "session/downloadSpeedLimit";
    static inline constexpr const char *const SESSION_UPLOAD_SPEED_LIMIT =
        "session/downloadUploadLimit";

    static inline constexpr const char *const GUI_LANGUAGE     = "gui/language";
    static inline constexpr const char *const GUI_THEME        = "gui/theme";
    static inline constexpr const char *const GUI_CUSTOM_THEME = "gui/customTheme";

    static inline constexpr const char *const SESSION_DEFAULT_SAVE_LOCATION =
        "session/defaultSavePath";

    static inline constexpr const char* const LOGS_ENABLED = "logs/enabled";
    static inline constexpr const char* const LOGS_PATH = "logs/path";
    static inline constexpr const char* const LOGS_MAX_SIZE = "logs/maxSize";

    static inline constexpr const char *const TRANSFER_CONFIRM_DELETION = "transfer/deletion";

    static inline constexpr const char *const DESKTOP_EXIT_BEH = "desktop/exitBehaviour";

    static inline constexpr const char *const DESKTOP_SHOW_TRAY   = "desktop/showInTray";
    static inline constexpr const char *const DESKTOP_SHOW_NOTIFS = "desktop/showNotifications";

    static inline constexpr const char *const LISTENING_PORT = "listening/port";
    static inline constexpr const char* const LISTENING_PROTOCOL = "listening/protocol";

    static inline constexpr const char* const LIMITS_MAX_NUM_OF_CONNECTIONS = "limits/mNumOfCon";
    static inline constexpr const char* const LIMITS_MAX_NUM_OF_CONNECTIONS_PT = "limits/mNumOfConPT"; // per torrent

    static inline constexpr const char* const PRIVACY_DHT_ENABLED = "privacy/dht";
    static inline constexpr const char* const PRIVACY_PEEREX_ENABLED = "privacy/peerex";
    static inline constexpr const char* const PRIVACY_LOCAL_PEER_DESC = "privacy/localDiscovery";

    static inline constexpr const char* const DATA_TORRENTS_HEADER = "data/torrentsTable";
};

struct SettingsValues
{
    static inline constexpr int SESSION_DOWNLOAD_SPEED_LIMIT = 0;
    static inline constexpr int SESSION_UPLOAD_SPEED_LIMIT   = 0;

    static inline constexpr int GUI_LANGUAGE_ENGLISH = 0;
    static inline constexpr int GUI_LANGUAGE_RUSSIAN = 1;

    static inline constexpr int GUI_THEME_DARK   = 0;
    static inline constexpr int GUI_THEME_LIGHT  = 1;
    static inline constexpr int GUI_THEME_CUSTOM = 2;

    static inline constexpr bool TRANSFER_CONFIRM_DELETION_DEFAULT = true;

    static inline constexpr unsigned int LOGS_MAX_SIZE_DEFAULT = 64 * 1024;
    static inline constexpr bool LOGS_ENABLED_DEFAULT = true;

    static inline constexpr int DESKTOP_EXIT_BEH_CLOSE   = 0;
    static inline constexpr int DESKTOP_EXIT_BEH_TO_TRAY = 1;

    static inline constexpr bool DESKTOP_SHOW_TRAY_DEFAULT   = true;
    static inline constexpr bool DESKTOP_SHOW_NOTIFS_DEFAULT = true;

    static inline constexpr unsigned short LISTENING_PORT_DEFAULT = 6881;
    static inline constexpr int LISTENING_PROTOCOL_TCP_AND_UTP = 0;
    static inline constexpr int LISTENING_PROTOCOL_TCP = 1;
    static inline constexpr int LISTENING_PROTOCOL_UTP = 2;

    static inline constexpr int LIMITS_MAX_NUM_OF_CONNECTIONS_DEFAULT = 200;
    static inline constexpr int LIMITS_MAX_NUM_OF_CONNECTIONS_PT_DEFAULT = 100;

    static inline constexpr bool PRIVACY_DHT_ENABLED_DEFAULT = true;
    static inline constexpr bool PRIVACY_PEEREX_ENABLED_DEFAULT = true;
    static inline constexpr bool PRIVACY_LOCAL_PEER_DESC_DEFAULT = true;
};

#endif // SETTINGSVALUES_H
