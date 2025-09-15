#ifndef SETTINGSVALUES_H
#define SETTINGSVALUES_H

struct SettingsValues {
    static inline constexpr const char* SESSION_DOWNLOAD_SPEED_LIMIT = "session/downloadSpeedLimit";
    static inline constexpr const char* SESSION_UPLOAD_SPEED_LIMIT = "session/downloadUploadLimit";

    static inline constexpr const char* GUI_LANGUAGE = "gui/language";
    static inline constexpr const char* GUI_THEME = "gui/theme";

    static inline constexpr const char* SESSION_DEFAULT_SAVE_LOCATION = "session/defaultSavePath";

    static inline constexpr const char* GUI_CUSTOM_THEME = "gui/customTheme";
};

#endif // SETTINGSVALUES_H

