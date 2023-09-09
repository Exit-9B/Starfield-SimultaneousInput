cmake_minimum_required(VERSION 3.19)

project(
	LibStarfield
	LANGUAGES CXX
)

find_package(spdlog CONFIG REQUIRED)

file(
	GLOB_RECURSE
	HEADER_FILES
	LIST_DIRECTORIES false
	CONFIGURE_DEPENDS
	"${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
)

file(
	GLOB_RECURSE
	SOURCE_FILES
	LIST_DIRECTORIES false
	CONFIGURE_DEPENDS
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library("${PROJECT_NAME}" STATIC)

add_library("${PROJECT_NAME}::${PROJECT_NAME}" ALIAS "${PROJECT_NAME}")

target_sources(
	"${PROJECT_NAME}"
	PUBLIC
	${HEADER_FILES}
	${SOURCE_FILES}
	.clang-format
)

target_compile_features(
	"${PROJECT_NAME}"
	PUBLIC
	cxx_std_20
)

target_include_directories(
	"${PROJECT_NAME}"
	PUBLIC
	"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
	"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_link_libraries(
	"${PROJECT_NAME}"
	PUBLIC
	spdlog::spdlog
	Version.lib
)
