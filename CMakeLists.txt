cmake_minimum_required(VERSION 3.25)
project(DS_Project)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Set Qt installation path
set(Qt6_DIR "C:/Qt/6.10.0/msvc2022_64/lib/cmake/Qt6")

# Add resource files
set(RESOURCE_FILES
    resources.qrc
)

# Add data files
set(DATA_FILES
    "project code/Data/users.json"
    "project code/Data/remembered.json"
)

# Find required Qt6 components
find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        LinguistTools
        REQUIRED)

# Set up translations
set(TS_FILES
    translations/fitflexpro_fr.ts
    translations/fitflexpro_de.ts
)

qt_add_translation(QM_FILES ${TS_FILES})

# Create a custom target for translations
add_custom_target(translations ALL DEPENDS ${QM_FILES})

# Group source files for organization
set(START_APP_SOURCES
        "project code/Start app/authpage.cpp"
        "project code/Start app/authpage.h"
        "project code/Start app/mainpage.cpp"
        "project code/Start app/mainpage.h"
        "project code/Start app/imageslider.cpp"
        "project code/Start app/imageslider.h"
        "project code/Start app/homepage.cpp"
        "project code/Start app/homepage.h"
        "project code/Start app/settingspage.cpp"
        "project code/Start app/settingspage.h"
        "project code/Start app/developerpage.h"
        "project code/Start app/developerpage.cpp"
        "project code/Start app/onboardingpage.h"
        "project code/Start app/onboardingpage.cpp"
        "project code/Start app/splashscreen.h"
        "project code/Start app/splashscreen.cpp"
        "project code/Start app/languageselectionpage.h"
        "project code/Start app/languageselectionpage.cpp"

)

set(DATA_MANAGER_SOURCES
        "project code/DataManager/userdatamanager.cpp"
        "project code/DataManager/userdatamanager.h"
)

set(MODEL_SOURCES
        "project code/Model/user.cpp"
        "project code/Model/user.h"
)

set(UTIL_SOURCES
        "project code/Util/ThemeManager.cpp"
        "project code/Util/ThemeManager.h"
        "project code/Util/TopPanel.cpp"
        "project code/Util/TopPanel.h"
        "project code/Util/UIUtils.cpp"
        "project code/Util/UIUtils.h"
        "project code/Util/LanguageManager.h"
        "project code/Util/LanguageManager.cpp"
        "project code/Util/LanguageSelector.h"
        "project code/Util/LanguageSelector.cpp"
)

# Define project sources
set(PROJECT_SOURCES
        "project code/main/main.cpp"
        ${START_APP_SOURCES}
        ${DATA_MANAGER_SOURCES}
        ${MODEL_SOURCES}
        ${UTIL_SOURCES}
        ${RESOURCE_FILES}
        ${QM_FILES}
)

# Create executable
add_executable(DS_Project ${PROJECT_SOURCES})

# Copy data files to build directory
foreach(DATA_FILE ${DATA_FILES})
    add_custom_command(TARGET DS_Project POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/${DATA_FILE}"
        "$<TARGET_FILE_DIR:DS_Project>/${DATA_FILE}"
    )
endforeach()

# Include directories
target_include_directories(DS_Project PRIVATE
        "${CMAKE_SOURCE_DIR}/project code/Start app"
        "${CMAKE_SOURCE_DIR}/project code/DataManager"
        "${CMAKE_SOURCE_DIR}/project code/Model"
        "${CMAKE_SOURCE_DIR}/project code/Util"
)

# Link Qt libraries
target_link_libraries(DS_Project PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
)

# Windows-specific DLL handling
if(WIN32)
    set(DEBUG_SUFFIX "d")
    if(NOT CMAKE_BUILD_TYPE MATCHES "Debug")
        set(DEBUG_SUFFIX "")
    endif()
    
    # Set the absolute path to Qt installation
    set(QT_INSTALL_PATH "C:/Qt/6.10.0/msvc2022_64")
    
    # Create the destination directory if it doesn't exist
    add_custom_command(TARGET DS_Project POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:DS_Project>")
    
    # Copy the required DLLs
    foreach(QT_LIB Core Gui Widgets)
        add_custom_command(TARGET DS_Project POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:DS_Project>")
    endforeach()
    
    # Create platforms directory and copy the qwindowsd.dll
    add_custom_command(TARGET DS_Project POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:DS_Project>/plugins/platforms/"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
            "$<TARGET_FILE_DIR:DS_Project>/plugins/platforms/")
endif()