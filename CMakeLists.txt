cmake_minimum_required(VERSION 3.5.0)
if (NOT DEFINED GUI_BASE_DIR)
	if (DEFINED ENV{GUI_BASE_DIR})
		set(GUI_BASE_DIR $ENV{GUI_BASE_DIR})
	else()
		set(GUI_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../plugin-GUI)
	endif()
endif()

set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "Build architecture for Mac OS X" FORCE)

set(COMMONLIB_NAME "OpenEphysZarr")

project(OE_COMMONLIB_${COMMONLIB_NAME})

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	set(LINUX 1)
	if(NOT CMAKE_BUILD_TYPE)
		set(CMAKE_BUILD_TYPE Debug)
	endif()
endif()

set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS
	"$<$<PLATFORM_ID:Windows>:JUCE_API=__declspec(dllimport)>"
	$<$<PLATFORM_ID:Windows>:_CRT_SECURE_NO_WARNINGS>
	$<$<PLATFORM_ID:Linux>:JUCE_DISABLE_NATIVE_FILECHOOSERS=1>
	$<$<CONFIG:Debug>:DEBUG=1>
	$<$<CONFIG:Debug>:_DEBUG=1>
	$<$<CONFIG:Release>:NDEBUG=1>
	)
	
set(SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Source)
file(GLOB_RECURSE SRC_FILES LIST_DIRECTORIES false "${SOURCE_PATH}/*.cpp" "${SOURCE_PATH}/*.c")
file(GLOB_RECURSE HEADER_FILES LIST_DIRECTORIES false "${SOURCE_PATH}/*.h??" "${SOURCE_PATH}/*.h")
list(APPEND SRC_FILES ${HEADER_FILES})
set(GUI_COMMONLIB_DIR ${GUI_BASE_DIR}/installed_libs)

set(CONFIGURATION_FOLDER $<$<CONFIG:Debug>:Debug>$<$<NOT:$<CONFIG:Debug>>:Release>)

list(APPEND CMAKE_PREFIX_PATH ${GUI_COMMONLIB_DIR} ${GUI_COMMONLIB_DIR}/${CONFIGURATION_FOLDER})

add_library(${COMMONLIB_NAME} SHARED ${SRC_FILES})

target_compile_features(${COMMONLIB_NAME} PUBLIC cxx_auto_type cxx_generalized_initializers cxx_std_17)
target_include_directories(${COMMONLIB_NAME} PUBLIC ${GUI_BASE_DIR}/JuceLibraryCode ${GUI_BASE_DIR}/JuceLibraryCode/modules ${GUI_BASE_DIR}/Plugins/Headers ${GUI_COMMONLIB_DIR}/include)

set(GUI_BIN_DIR ${GUI_BASE_DIR}/Build/${CONFIGURATION_FOLDER})

if (NOT CMAKE_LIBRARY_ARCHITECTURE)
	if (CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(CMAKE_LIBRARY_ARCHITECTURE "x64")
	else()
		set(CMAKE_LIBRARY_ARCHITECTURE "x86")
	endif()
endif()

install(TARGETS ${COMMONLIB_NAME}
	LIBRARY DESTINATION ${GUI_COMMONLIB_DIR}/${CONFIGURATION_FOLDER}/lib/${CMAKE_LIBRARY_ARCHITECTURE}
	ARCHIVE DESTINATION ${GUI_COMMONLIB_DIR}/${CONFIGURATION_FOLDER}/lib/${CMAKE_LIBRARY_ARCHITECTURE}
	RUNTIME DESTINATION ${GUI_COMMONLIB_DIR}/${CONFIGURATION_FOLDER}/bin/${CMAKE_LIBRARY_ARCHITECTURE} 
	)
install(FILES ${HEADER_FILES} DESTINATION ${GUI_COMMONLIB_DIR}/include)
	
#Libraries and compiler options
if(MSVC)
	add_compile_definitions(H5_BUILT_AS_DYNAMIC_LIB=1)
	target_link_libraries(${COMMONLIB_NAME} ${GUI_BIN_DIR}/open-ephys.lib)
	target_compile_options(${COMMONLIB_NAME} PRIVATE /sdl- /W0)
	
	install(TARGETS ${COMMONLIB_NAME} RUNTIME DESTINATION ${GUI_BIN_DIR}/shared CONFIGURATIONS ${CMAKE_CONFIGURATION_TYPES})
	
	list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/libs/windows)
elseif(LINUX)
	target_link_libraries(${COMMONLIB_NAME} GL X11 Xext Xinerama asound dl freetype pthread rt)
	set_property(TARGET ${COMMONLIB_NAME} APPEND_STRING PROPERTY LINK_FLAGS
		"-fvisibility=hidden -fPIC -rdynamic -Wl,-rpath,'$ORIGIN'")
	target_compile_options(${COMMONLIB_NAME} PRIVATE -fPIC -rdynamic)
	target_compile_options(${COMMONLIB_NAME} PRIVATE -O3) #enable optimization for linux debug
	
	install(TARGETS ${COMMONLIB_NAME} LIBRARY DESTINATION ${GUI_BIN_DIR}/shared)
	
	list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/libs/linux)
elseif(APPLE)
	set_target_properties(${COMMONLIB_NAME} PROPERTIES BUNDLE FALSE)
	set_property(TARGET ${COMMONLIB_NAME} APPEND_STRING PROPERTY LINK_FLAGS
	"-undefined dynamic_lookup")

	install(TARGETS ${COMMONLIB_NAME} DESTINATION $ENV{HOME}/Library/Application\ Support/open-ephys/shared-api8)
	list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/libs/macos)
endif()

#create filters for vs and xcode

foreach( src_file IN ITEMS ${SRC_FILES})
	get_filename_component(src_path "${src_file}" PATH)
	file(RELATIVE_PATH src_path_rel "${SOURCE_PATH}" "${src_path}")
	string(REPLACE "/" "\\" group_name "${src_path_rel}")
	source_group("${group_name}" FILES "${src_file}")
endforeach()


#copy the dependencies when installing
if (MSVC)
	install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/libs/windows/bin/ DESTINATION ${GUI_BIN_DIR}/shared CONFIGURATIONS ${CMAKE_CONFIGURATION_TYPES})
elseif(LINUX)
	install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/libs/linux/bin/ DESTINATION ${GUI_BIN_DIR}/shared)
elseif(APPLE)
	install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/libs/macos/bin/ DESTINATION $ENV{HOME}/Library/Application\ Support/open-ephys/shared-api8)
endif()

find_library(ZSTD_LIBRARY NAMES libzstd_static)
find_path(ZSTD_INCLUDE_DIR zstd.h)

target_link_libraries(${COMMONLIB_NAME} ${ZSTD_LIBRARY})
target_include_directories(${COMMONLIB_NAME} PRIVATE ${ZSTD_INCLUDE_DIR})