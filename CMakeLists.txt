# Project
project(lastmoid)
 
# Find the required Libaries
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
 
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
   ${QT_QTTEST_INCLUDE_DIR}
   )
 
# We add our source code here
set(lastmoid_SRCS lastmoid.cpp barlabel.cpp)
 
# Now make sure all files get to the right place
kde4_add_ui_files(lastmoid_SRCS lastmoidConfig.ui)
kde4_add_plugin(plasma_applet_lastmoid ${lastmoid_SRCS})
target_link_libraries(plasma_applet_lastmoid ${KDE4_KDEUI_LIBS} ${QT_QTNETWORK_LIBRARY} ${QT_QTXML_LIBRARY}  ${KDE4_PLASMA_LIBS} )
 
install(TARGETS plasma_applet_lastmoid
        DESTINATION ${PLUGIN_INSTALL_DIR})
 
install(FILES plasma-applet-lastmoid.desktop
        DESTINATION  ${SERVICES_INSTALL_DIR})

install(FILES
    lastmoid.svg
    DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/widgets)

