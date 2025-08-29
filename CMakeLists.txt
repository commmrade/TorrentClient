cmake_minimum_required(VERSION 3.16)

project(torrent-client VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(controllers widgets models delegates dialogs common)

find_package(spdlog REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBTORRENT REQUIRED libtorrent-rasterbar)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

find_library(MAXMINDDB_LIBRARY maxminddb)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(torrent-client
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        widgets/torrentwidget.h widgets/torrentwidget.cpp widgets/torrentwidget.ui
        controllers/sessionmanager.h controllers/sessionmanager.cpp
        models/torrentstablemodel.h models/torrentstablemodel.cpp
        delegates/torrentitemdelegate.h delegates/torrentitemdelegate.cpp
        common/torrent.h
        dialogs/settingsdialog.h dialogs/settingsdialog.cpp dialogs/settingsdialog.ui
        common/settingsvalues.h
        dialogs/savetorrentdialog.h dialogs/savetorrentdialog.cpp dialogs/savetorrentdialog.ui
        common/torrenthandle.h common/torrenthandle.cpp
        common/peer.h
        models/peertablemodel.h models/peertablemodel.cpp
        widgets/propertieswidget.h widgets/propertieswidget.cpp widgets/propertieswidget.ui
        widgets/peerswidget.h widgets/peerswidget.cpp widgets/peerswidget.ui
        widgets/generalinfowidget.h widgets/generalinfowidget.cpp widgets/generalinfowidget.ui
        common/torrentinfo.h
        common/utils.h common/utils.cpp
        common/metadatafetcher.h common/metadatafetcher.cpp
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET torrent-client APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(torrent-client SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(torrent-client
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(torrent-client PRIVATE Qt${QT_VERSION_MAJOR}::Widgets spdlog::spdlog ${LIBTORRENT_LIBRARIES} ${MAXMINDDB_LIBRARY})
target_include_directories(torrent-client PRIVATE ${LIBTORRENT_INCLUDE_DIRS} ${MAXMINDDB_INCLUDE_DIR})

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.torrent-client)
endif()
set_target_properties(torrent-client PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS torrent-client
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(torrent-client)
endif()
