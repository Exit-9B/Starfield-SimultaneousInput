cmake_minimum_required(VERSION 3.24)

project(
	SimultaneousInput
	VERSION 1.0.1
	LANGUAGES CXX
)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 20)

cmake_host_system_information(
	RESULT Starfield_PATH
	QUERY WINDOWS_REGISTRY "HKLM/SOFTWARE/Microsoft/Windows/CurrentVersion/Uninstall/Steam App 1716740"
	VALUE "InstallLocation"
)

set(Starfield_PATH ${Starfield_PATH} CACHE PATH "Installed path of Starfield.")
set(GAME_DIR ${Starfield_PATH})

add_library(${PROJECT_NAME} SHARED)

file(
	GLOB_RECURSE
	HEADER_FILES
	LIST_DIRECTORIES false
	CONFIGURE_DEPENDS
	"${PROJECT_SOURCE_DIR}/src/*.h"
)

source_group(
	TREE ${PROJECT_SOURCE_DIR}/src
	PREFIX "Header Files"
	FILES
	${HEADER_FILES}
)

target_sources(${PROJECT_NAME} PRIVATE ${HEADER_FILES})

file(
	GLOB_RECURSE
	SOURCE_FILES
	LIST_DIRECTORIES false
	CONFIGURE_DEPENDS
	"${PROJECT_SOURCE_DIR}/src/*.cpp"
)

source_group(
	TREE ${PROJECT_SOURCE_DIR}/src
	PREFIX "Source Files"
	FILES ${SOURCE_FILES}
)

target_sources(${PROJECT_NAME} PRIVATE ${SOURCE_FILES})

target_sources(
	${PROJECT_NAME}
	PRIVATE
	.clang-format
	.editorconfig
	vcpkg.json
)

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/Plugin.h.in
	${CMAKE_CURRENT_BINARY_DIR}/src/Plugin.h
	@ONLY
)

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in
	${CMAKE_CURRENT_BINARY_DIR}/version.rc
	@ONLY
)

target_sources(
	${PROJECT_NAME}
	PRIVATE
	${CMAKE_CURRENT_BINARY_DIR}/version.rc
)

target_include_directories(
	${PROJECT_NAME}
	PRIVATE
	${CMAKE_CURRENT_BINARY_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}/src
)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	target_compile_options(
		${PROJECT_NAME}
		PRIVATE
		"/sdl" # Enable Additional Security Checks
		"/utf-8" # Set Source and Executable character sets to UTF-8
		"/Zi" # Debug Information Format

		"/permissive-" # Standards conformance
		"/Zc:preprocessor" # Enable preprocessor conformance mode

		"/wd4200" # nonstandard extension used : zero-sized array in struct/union

		"$<$<CONFIG:DEBUG>:>"
		"$<$<CONFIG:RELEASE>:/Zc:inline;/JMC-;/Ob3>"
	)

	target_link_options(
		${PROJECT_NAME}
		PRIVATE
		"$<$<CONFIG:DEBUG>:/INCREMENTAL;/OPT:NOREF;/OPT:NOICF>"
		"$<$<CONFIG:RELEASE>:/INCREMENTAL:NO;/OPT:REF;/OPT:ICF;/DEBUG:FULL>"
	)
endif()

add_subdirectory(external/LibStarfield LibStarfield EXCLUDE_FROM_ALL)

find_package(spdlog CONFIG REQUIRED)

target_link_libraries(
	${PROJECT_NAME}
	LibStarfield::LibStarfield
	spdlog::spdlog
)

if(CMAKE_GENERATOR MATCHES "Visual Studio")
	option(CMAKE_VS_INCLUDE_INSTALL_TO_DEFAULT_BUILD "Include INSTALL target to default build." OFF)
endif()

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX "${GAME_DIR}/Data" CACHE PATH
		"Install path prefix (e.g. Starfield Data directory or Mod Organizer virtual directory)."
		FORCE
	)
endif()

install(
	FILES
	"$<TARGET_FILE:${PROJECT_NAME}>"
	"$<TARGET_PDB_FILE:${PROJECT_NAME}>"
	DESTINATION "SFSE/Plugins"
	COMPONENT SFSEPlugin
)

set(CPACK_GENERATOR "7Z")
set(CPACK_PACKAGE_FILE_NAME ${PROJECT_NAME}-${PROJECT_VERSION})
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY FALSE)
include(CPack)
