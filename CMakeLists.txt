cmake_minimum_required(VERSION 3.25)
project(DS_Project)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Function to read .env file
function(read_env_file)
    if(EXISTS "${CMAKE_SOURCE_DIR}/.env")
        file(READ "${CMAKE_SOURCE_DIR}/.env" ENV_CONTENT)
        string(REGEX REPLACE "\n" ";" ENV_LINES "${ENV_CONTENT}")
        foreach(LINE ${ENV_LINES})
            if(LINE MATCHES "^[^#].*=")
                string(REGEX REPLACE "^([^=]+)=.*" "\\1" VAR_NAME "${LINE}")
                string(REGEX REPLACE "^[^=]+=(.*)" "\\1" VAR_VALUE "${LINE}")
                string(STRIP "${VAR_NAME}" VAR_NAME)
                string(STRIP "${VAR_VALUE}" VAR_VALUE)
                set(${VAR_NAME} "${VAR_VALUE}" PARENT_SCOPE)
            endif()
        endforeach()
    else()
        message(FATAL_ERROR ".env file not found. Please create a .env file based on .env.example")
    endif()
endfunction()

# Read environment variables from .env file
read_env_file()

# Set Qt installation path
if(NOT DEFINED QT_DIR)
    message(FATAL_ERROR "QT_DIR not found in .env file. Please check your .env file.")
endif()

set(Qt6_DIR "${QT_DIR}/lib/cmake/Qt6")

# Add resource files
set(RESOURCE_FILES
        resources.qrc
)

# Add data files
set(DATA_FILES
        "project code/Data/users.json"
        "project code/Data/members.json"
        "project code/Data/courts.json"
        "project code/Data/bookings.json"
        "project code/Data/waitlists.json"
        "project code/Data/remembered.json"
        "project code/Data/saved_cards.json"
        "project code/Data/classes.json"
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
        "translations/fitflexpro_fr.ts"
        "translations/fitflexpro_de.ts"
        "translations/fitflexpro_en.ts"
)

qt_add_translation(QM_FILES ${TS_FILES})

# Create a custom target for translations
add_custom_target(translations ALL DEPENDS ${QM_FILES})

# Group source files for organization
set(MAIN_SOURCES
        "project code/main/main.cpp"
        "project code/main/mainwindow.cpp"
        "project code/main/mainwindow.h"
)

set(SRC_SOURCES
        "project code/src/auth/authpage.cpp"
        "project code/src/auth/authpage.h"
        "project code/src/auth/imageslider.cpp"
        "project code/src/auth/imageslider.h"
        "project code/src/pages/mainpage.cpp"
        "project code/src/pages/mainpage.h"
        "project code/src/pages/homepage.cpp"
        "project code/src/pages/homepage.h"
        "project code/src/onboarding/onboardingpage.h"
        "project code/src/onboarding/onboardingpage.cpp"
        "project code/src/onboarding/splashscreen.h"
        "project code/src/onboarding/splashscreen.cpp"
        "project code/src/onboarding/languageselectionpage.h"
        "project code/src/onboarding/languageselectionpage.cpp"
        "project code/src/pages/Gym/staffhomepage.h"
        "project code/src/pages/Gym/staffhomepage.cpp"
        "project code/src/pages/Padel/addcourtpage.h"
        "project code/src/pages/Padel/addcourtpage.cpp"
        "project code/src/pages/Padel/BookingWindow.cpp"
        "project code/src/pages/Padel/BookingWindow.h"
)

set(SETTINGS_SOURCES
        "project code/Settings/settingspage.cpp"
        "project code/Settings/settingspage.h"
        "project code/Settings/developerpage.h"
        "project code/Settings/developerpage.cpp"
)

set(DATA_MANAGER_SOURCES
        "project code/DataManager/userdatamanager.cpp"
        "project code/DataManager/userdatamanager.h"
        "project code/DataManager/memberdatamanager.cpp"
        "project code/DataManager/memberdatamanager.h"
        "project code/DataManager/classdatamanager.cpp"
        "project code/DataManager/classdatamanager.h"
        "project code/DataManager/padeldatamanager.cpp"
        "project code/DataManager/padeldatamanager.h"
)

set(MODEL_SOURCES
        "project code/Model/System/user.cpp"
        "project code/Model/System/user.h"
        "project code/Model/Gym/member.cpp"
        "project code/Model/Gym/member.h"
        "project code/Model/Gym/class.cpp"
        "project code/Model/Gym/class.h"
        "project code/Model/Gym/subscription.cpp"
        "project code/Model/Gym/subscription.h"
        "project code/Model/Padel/Court.cpp"
        "project code/Model/Padel/Court.h"
        "project code/Model/Padel/Booking.cpp"
        "project code/Model/Padel/Booking.h"
        "project code/src/pages/Gym/availableclassesscreen.cpp"
        "project code/src/pages/Gym/availableclassesscreen.h"
)

set(CORE_SOURCES
        "project code/Core/AppInitializer.h"
        "project code/Core/AppInitializer.cpp"
)

set(THEME_SOURCES
        "project code/Theme/ThemeManager.cpp"
        "project code/Theme/ThemeManager.h"
)

set(UI_SOURCES
        "project code/UI/TopPanel.cpp"
        "project code/UI/TopPanel.h"
        "project code/UI/UIUtils.cpp"
        "project code/UI/UIUtils.h"
        "project code/UI/leftsidebar.cpp"
        "project code/UI/leftsidebar.h"
        "project code/UI/Widgets/Notifications/Notifications.cpp"
        "project code/UI/Widgets/Notifications/Notifications.h"
        "project code/UI/Widgets/Revenue/revenue.cpp"
        "project code/UI/Widgets/Revenue/revenue.h"
)

set(LANGUAGE_SOURCES
        "project code/Language/LanguageManager.h"
        "project code/Language/LanguageManager.cpp"
        "project code/Language/LanguageSelector.h"
        "project code/Language/LanguageSelector.cpp"
)

set(SUBSCRIPTION_SOURCES
        "project code/Subscription/subscriptionpage.cpp"
        "project code/Subscription/subscriptionpage.h"
        "project code/Subscription/subscriptionstatuspage.cpp"
        "project code/Subscription/subscriptionstatuspage.h"
        "project code/Subscription/newusersubscriptionview.cpp"
)

set(PAYMENT_SOURCES
        "project code/Payment/paymentpage.cpp"
        "project code/Payment/paymentpage.h"
)
set(STAFF_SOURCES
        "project code/staff/addmemberpage.cpp"
        "project code/staff/addmemberpage.h"
        "project code/staff/retrievepage.cpp"
        "project code/staff/retrievepage.h"
        "project code/staff/searchmemberpage.cpp"
        "project code/staff/searchmemberpage.h"
)
# Define project sources
set(PROJECT_SOURCES
        ${MAIN_SOURCES}
        ${SRC_SOURCES}
        ${SETTINGS_SOURCES}
        ${DATA_MANAGER_SOURCES}
        ${MODEL_SOURCES}
        ${CORE_SOURCES}
        ${THEME_SOURCES}
        ${UI_SOURCES}
        ${LANGUAGE_SOURCES}
        ${SUBSCRIPTION_SOURCES}
        ${PAYMENT_SOURCES}
        ${STAFF_SOURCES}
        ${RESOURCE_FILES}
        ${QM_FILES}
        "project code/src/pages/Gym/availableclassesscreen.cpp"
        "project code/src/pages/Gym/availableclassesscreen.h"
        "project code/UI/Stylesheets/Gym/staffHomePageStyle.h"
        "project code/UI/Stylesheets/System/clockStyle.h"
        "project code/UI/Stylesheets/System/notificationStyle.h"
        "project code/UI/Widgets/Notifications/NotificationManager.h"
        "project code/UI/Widgets/Notifications/NotificationManager.cpp"
        "project code/UI/Stylesheets/System/mainpageStyle.h"
        "project code/Model/System/timeLogic.cpp"
        "project code/Model/System/timeLogic.h"
        "project code/UI/Widgets/Clock/Clock.cpp"
        "project code/UI/Widgets/Clock/Clock.h"
        "project code/UI/Stylesheets/System/homepageStyle.h"
        "project code/UI/Stylesheets/System/leftsidebarStyle.h"
        "project code/UI/Stylesheets/Padel/BookingWindowStyle.h"
)

# Define absolute source data directory path
set(SOURCE_DATA_DIR "${CMAKE_SOURCE_DIR}")

# Add compile definition to force using the source directory for data files
add_compile_definitions(SOURCE_DATA_DIR="${SOURCE_DATA_DIR}")
add_compile_definitions(FORCE_SOURCE_DIR=1)

# Create executable
add_executable(DS_Project ${PROJECT_SOURCES})

# Create required directories only once during configure time
file(MAKE_DIRECTORY
        "${CMAKE_SOURCE_DIR}/project code/Data"
        "${CMAKE_SOURCE_DIR}/project code/UsersPhoto"
)

# Create required directories in build directory for plugins
add_custom_command(
        TARGET DS_Project
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:DS_Project>/plugins/platforms"
        "$<TARGET_FILE_DIR:DS_Project>/plugins/imageformats"
        COMMENT "Creating required directories for Qt plugins"
)

# Include directories
target_include_directories(DS_Project PRIVATE
        "${CMAKE_SOURCE_DIR}/project code/src"
        "${CMAKE_SOURCE_DIR}/project code/src/auth"
        "${CMAKE_SOURCE_DIR}/project code/src/pages"
        "${CMAKE_SOURCE_DIR}/project code/src/onboarding"
        "${CMAKE_SOURCE_DIR}/project code/Settings"
        "${CMAKE_SOURCE_DIR}/project code/DataManager"
        "${CMAKE_SOURCE_DIR}/project code/Model"
        "${CMAKE_SOURCE_DIR}/project code/Core"
        "${CMAKE_SOURCE_DIR}/project code/Theme"
        "${CMAKE_SOURCE_DIR}/project code/UI"
        "${CMAKE_SOURCE_DIR}/project code/Language"
        "${CMAKE_SOURCE_DIR}/project code/Subscription"
        "${CMAKE_SOURCE_DIR}/project code/Payment"
        "${CMAKE_SOURCE_DIR}/project code/staff"
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

    # Set the absolute path to Qt installation from .env file
    set(QT_INSTALL_PATH "${QT_DIR}")

    # Create a single custom command that copies all required DLLs
    # Only copies if they don't exist or have been modified (timestamp comparison)
    add_custom_command(TARGET DS_Project POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${QT_INSTALL_PATH}/bin/Qt6Core${DEBUG_SUFFIX}.dll"
            "${QT_INSTALL_PATH}/bin/Qt6Gui${DEBUG_SUFFIX}.dll"
            "${QT_INSTALL_PATH}/bin/Qt6Widgets${DEBUG_SUFFIX}.dll"
            "$<TARGET_FILE_DIR:DS_Project>/"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
            "$<TARGET_FILE_DIR:DS_Project>/plugins/platforms/"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${QT_INSTALL_PATH}/plugins/imageformats/qjpeg${DEBUG_SUFFIX}.dll"
            "${QT_INSTALL_PATH}/plugins/imageformats/qgif${DEBUG_SUFFIX}.dll"
            "${QT_INSTALL_PATH}/plugins/imageformats/qico${DEBUG_SUFFIX}.dll"
            "${QT_INSTALL_PATH}/plugins/imageformats/qsvg${DEBUG_SUFFIX}.dll"
            "$<TARGET_FILE_DIR:DS_Project>/plugins/imageformats/"
            COMMENT "Copying Qt DLLs and plugins"
    )
endif()