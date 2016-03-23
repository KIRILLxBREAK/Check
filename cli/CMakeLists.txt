######################################################################################################################################
find_package(Qt5Widgets REQUIRED)	# Widgets finds its own dependencies (QtGui and QtCore).

# The Qt5Widgets_INCLUDES also includes the include directories for
# dependencies QtCore and QtGui
include_directories(${Qt5Widgets_INCLUDES})

add_definitions(${Qt5Widgets_DEFINITIONS})	# We need add -DQT_WIDGETS_LIB when using QtWidgets in Qt 5.

# Executables fail to build with Qt 5 in the default configuration
# without -fPIE. We add that here.
set(CMAKE_CXX_FLAGS "${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

#FIND_PACKAGE(Qt5Core QUIET)
#FIND_PACKAGE(Qt5Widgets QUIET)
#FIND_PACKAGE(Qt5Gui QUIET)
#if(NOT Qt5Core_FOUND AND NOT Qt5Widgets_FOUND AND NOT Qt5Gui_FOUND)
#	message(SEND_ERROR "Failed to find QT")
#	return()
#endif()
######################################################################################################################################

include_directories(${PROJECT_SOURCE_DIR}/lib/)
include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/externals/tinyxml/)

file(GLOB hdrs "*.h")
file(GLOB srcs "*.cpp")
file(GLOB mainfile "main.cpp")
list(REMOVE_ITEM srcs ${mainfile})

add_library(cli_objs OBJECT ${hdrs} ${srcs}) 
add_executable(cppcheck ${hdrs} ${mainfile} $<TARGET_OBJECTS:cli_objs> $<TARGET_OBJECTS:lib_objs> $<TARGET_OBJECTS:tinyxml_objs>)
if (HAVE_RULES)
    target_link_libraries(cppcheck pcre)
endif()

install(TARGETS cppcheck
    RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
    COMPONENT applications)

install(FILES ${cfgs}
   DESTINATION ${CMAKE_INSTALL_FULL_DATAROOTDIR}/${PROJECT_NAME}/
   COMPONENT headers)
