#ifndef SETTINGSVALUES_H
#define SETTINGSVALUES_H

struct SettingsValues
{
    static inline constexpr const char*const SESSION_DOWNLOAD_SPEED_LIMIT = "session/downloadSpeedLimit";
    static inline constexpr const char*const SESSION_UPLOAD_SPEED_LIMIT = "session/downloadUploadLimit";

    static inline constexpr const char*const GUI_LANGUAGE = "gui/language";
    static inline constexpr const char*const GUI_THEME    = "gui/theme";

    static inline constexpr const char*const SESSION_DEFAULT_SAVE_LOCATION = "session/defaultSavePath";

    static inline constexpr const char* const GUI_CUSTOM_THEME = "gui/customTheme";
    static inline constexpr const char* const TRANSFER_CONFIRM_DELETION = "transfer/deletion";
};

#endif // SETTINGSVALUES_H
