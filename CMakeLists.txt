cmake_minimum_required(VERSION 3.16)

project(torrent-client VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(src src/core src/gui)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBTORRENT REQUIRED libtorrent-rasterbar)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Charts Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Charts Core)

find_library(MAXMINDDB_LIBRARY maxminddb)

set(PROJECT_SOURCES
        src/main.cpp
        src/gui/mainwindow.cpp
        src/gui/mainwindow.h
        src/gui/mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(torrent-client
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}

        src/core/controllers/sessionmanager.h src/core/controllers/sessionmanager.cpp
        src/core/models/torrentstablemodel.h src/core/models/torrentstablemodel.cpp
        src/core/delegates/torrentitemdelegate.h src/core/delegates/torrentitemdelegate.cpp
        src/core/utils/torrent.h
        src/gui/dialogs/settingsdialog.h src/gui/dialogs/settingsdialog.cpp src/gui/dialogs/settingsdialog.ui
        src/core/utils/settingsvalues.h
        src/gui/dialogs/savetorrentdialog.h src/gui/dialogs/savetorrentdialog.cpp src/gui/dialogs/savetorrentdialog.ui
        src/core/utils/torrenthandle.h src/core/utils/torrenthandle.cpp
        src/core/utils/peer.h
        src/core/models/peertablemodel.h src/core/models/peertablemodel.cpp
        src/gui/widgets/propertieswidget.h src/gui/widgets/propertieswidget.cpp src/gui/widgets/propertieswidget.ui
        src/gui/widgets/peerswidget.h src/gui/widgets/peerswidget.cpp src/gui/widgets/peerswidget.ui
        src/gui/widgets/generalinfowidget.h src/gui/widgets/generalinfowidget.cpp src/gui/widgets/generalinfowidget.ui
        src/core/utils/torrentinfo.h
        src/core/utils/utils.h src/core/utils/utils.cpp
        src/core/net/metadatafetcher.h src/core/net/metadatafetcher.cpp
        src/core/models/trackertablemodel.h src/core/models/trackertablemodel.cpp
        src/core/utils/tracker.h
        src/gui/widgets/trackerswidget.h src/gui/widgets/trackerswidget.cpp src/gui/widgets/trackerswidget.ui
        src/gui/widgets/piecesbarwidget.h src/gui/widgets/piecesbarwidget.cpp src/gui/widgets/piecesbarwidget.ui
        src/gui/dialogs/addpeersdialog.h src/gui/dialogs/addpeersdialog.cpp src/gui/dialogs/addpeersdialog.ui
        src/gui/widgets/speedgraphwidget.h src/gui/widgets/speedgraphwidget.cpp src/gui/widgets/speedgraphwidget.ui
        src/core/models/filetreemodel.h src/core/models/filetreemodel.cpp src/core/models/filetreemodel.ui
        src/core/utils/file.h
        src/core/delegates/fileitemdelegate.h src/core/delegates/fileitemdelegate.cpp
        src/gui/widgets/filelistwidget.h src/gui/widgets/filelistwidget.cpp src/gui/widgets/filelistwidget.ui
        src/gui/dialogs/deletetorrentdialog.h src/gui/dialogs/deletetorrentdialog.cpp src/gui/dialogs/deletetorrentdialog.ui
        src/core/delegates/filestatusdelegate.h src/core/delegates/filestatusdelegate.cpp
        src/core/filters/categorysortfilter.h src/core/filters/categorysortfilter.cpp
        src/core/delegates/fileprioritydelegate.h src/core/delegates/fileprioritydelegate.cpp
        src/core/utils/category.h
        src/core/utils/dirs.h
        src/core/utils/priority.h
        src/gui/dialogs/torrentsettingsdialog.h src/gui/dialogs/torrentsettingsdialog.cpp src/gui/dialogs/torrentsettingsdialog.ui
        src/gui/dialogs/loadmagnetdialog.h src/gui/dialogs/loadmagnetdialog.cpp src/gui/dialogs/loadmagnetdialog.ui
        src/gui/dialogs/managepeersdialog.h src/gui/dialogs/managepeersdialog.cpp src/gui/dialogs/managepeersdialog.ui
        src/gui/widgets/applicationsettings.h src/gui/widgets/applicationsettings.cpp src/gui/widgets/applicationsettings.ui
        src/gui/widgets/torrentsettings.h src/gui/widgets/torrentsettings.cpp src/gui/widgets/torrentsettings.ui
        src/gui/widgets/connectionsettings.h src/gui/widgets/connectionsettings.cpp src/gui/widgets/connectionsettings.ui
        src/gui/widgets/basesettings.h
        src/resources/themes.qrc
        src/gui/widgets/advancedsettings.h src/gui/widgets/advancedsettings.cpp src/gui/widgets/advancedsettings.ui
        src/resources/icons.qrc
    )
    # Define target properties for Android with Qt 6 as:
    # set_property(TARGET torrent-client APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
    # ${CMAKE_CURRENT_SOURCE_DIR}/android)
    # For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(torrent-client SHARED
            ${PROJECT_SOURCES}
        )
        # Define properties for Android with Qt 5 after find_package() calls as:
        # set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(torrent-client
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(torrent-client PRIVATE Qt${QT_VERSION_MAJOR}::Widgets ${LIBTORRENT_LIBRARIES} ${MAXMINDDB_LIBRARY} Qt${QT_VERSION_MAJOR}::Charts)
target_include_directories(torrent-client PRIVATE ${LIBTORRENT_INCLUDE_DIRS} ${MAXMINDDB_INCLUDE_DIR})

target_include_directories(torrent-client PRIVATE src src/core src/gui)

# target_compile_options(torrent-client PRIVATE -fsanitize=address -g -O1 -fno-omit-frame-pointer -Wall -Wextra)
# target_link_options(torrent-client PRIVATE -fsanitize=address)

if (MSVC)
    target_compile_options(torrent-client PRIVATE /W4)
else ()
    target_compile_options(torrent-client PRIVATE -Wall -Wextra)
endif ()

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

# GoogleTest setup
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Test executable
add_executable(tests
    tests/test_utils.cpp
    src/core/utils/utils.cpp  # Include necessary source files for testing
)

target_link_libraries(tests
    GTest::gtest_main
    Qt${QT_VERSION_MAJOR}::Core
)

target_include_directories(tests PRIVATE src src/core)

if (MSVC)
    target_compile_options(tests PRIVATE /W4)
else ()
    target_compile_options(tests PRIVATE -Wall -Wextra)
endif ()

include(GoogleTest)
gtest_discover_tests(tests)
