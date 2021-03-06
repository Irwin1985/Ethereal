cmake_minimum_required(VERSION 2.8)

project(Ethereal C CXX)

set(CMAKE_CXX_STANDARD 11)
if(CMAKE_VERSION VERSION_LESS "3.1")
	add_compile_options(-std=c++11)
endif()

# add cmake_modules path
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/third_party/cmake_modules")

set(ETHEREAL_VERSION_MAJOR 0)
set(ETHEREAL_VERSION_MINOR 1)
set(ETHEREAL_VERSION_PATCH 2)
execute_process(COMMAND date "+%a %b %d, %Y at %H:%M:%S" OUTPUT_VARIABLE BUILD_DATE OUTPUT_STRIP_TRAILING_WHITESPACE)

set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_MACOSX_RPATH TRUE)

include(CheckCXXCompilerFlag)

find_library(gmp REQUIRED)
find_library(gmpxx REQUIRED)
find_package(MPFR REQUIRED)
if(NOT MPFR_FOUND)
	error("MPFR library is required but missing")
endif()
find_package(MPC REQUIRED)
if(NOT MPC_FOUND)
	error("MPC library is required but missing")
endif()
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
if(CMAKE_USE_PTHREADS_INIT)
	message("-- Using thread flags: -pthread")
	set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -pthread")
	set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -pthread")
else()
	message("-- Using thread flags: ${CMAKE_THREAD_LIBS_INIT}")
	set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${CMAKE_THREAD_LIBS_INIT}")
	set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_THREAD_LIBS_INIT}")
endif()

# For libGMP on macOS and BSD
if(APPLE OR ${CMAKE_SYSTEM_NAME} MATCHES ".*BSD.*")
	include_directories(/usr/local/include)
	link_directories(/usr/local/lib)
endif()

if(WIN32)
	error("Project not supported on Windows yet")
endif()

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES ".*BSD.*")
	add_definitions(-D_WITH_GETLINE)
endif()

if(DEFINED ENV{DEBUG})
	add_definitions(-DDEBUG_MODE)
	message("-- Running in debug mode")
else()
	if(NOT DEFINED ENV{SKIP_LTO})
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto")
	endif()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif()

check_cxx_compiler_flag(-march=native COMPILER_SUPPORTS_MARCH_NATIVE)
if(COMPILER_SUPPORTS_MARCH_NATIVE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
endif()

if(DEFINED ENV{PREFIX_DIR} AND NOT "$ENV{PREFIX_DIR}" STREQUAL "")
	set(CMAKE_INSTALL_PREFIX "$ENV{PREFIX_DIR}")
else()
	set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}")
endif()
add_definitions(-DBUILD_PREFIX_DIR=${CMAKE_INSTALL_PREFIX})
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib/ethereal")
message("-- Using PREFIX = ${CMAKE_INSTALL_PREFIX}")

# Finally! The sources!!!!!

# Set Config.hpp.in template
configure_file("${PROJECT_SOURCE_DIR}/src/Config.hpp.in" "${PROJECT_SOURCE_DIR}/src/Config.hpp" @ONLY)

# Includes
file(COPY "${CMAKE_SOURCE_DIR}/include" DESTINATION "${CMAKE_INSTALL_PREFIX}/")

# Ethereal Core
file(GLOB_RECURSE INCS RELATIVE "${PROJECT_SOURCE_DIR}" "include/*.hpp")
file(GLOB_RECURSE SRCS RELATIVE "${PROJECT_SOURCE_DIR}" "src/*.cpp")
list(REMOVE_ITEM SRCS "src/FE/Main.cpp" "src/VM/Main.cpp")

add_library(et-lib SHARED ${SRCS} ${INCS})
target_link_libraries(et-lib ${CMAKE_DL_LIBS})
target_link_libraries(et-lib mpfr gmp gmpxx)
set_target_properties(et-lib
    PROPERTIES
    OUTPUT_NAME et
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/ethereal"
    INSTALL_RPATH_USE_LINK_PATH TRUE
)
install(TARGETS et-lib
	LIBRARY
	  DESTINATION lib/ethereal
	  COMPONENT Libraries
	PUBLIC_HEADER
	  DESTINATION include/ethereal
)

add_executable(et-bin "${PROJECT_SOURCE_DIR}/src/FE/Main.cpp")
target_link_libraries(et-bin et-lib)
set_target_properties(et-bin
	PROPERTIES
	OUTPUT_NAME et
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
	INSTALL_RPATH_USE_LINK_PATH TRUE
)
install(TARGETS et-bin
	RUNTIME
	  DESTINATION bin
	  COMPONENT Binaries
)

# Libraries
file(GLOB_RECURSE mods RELATIVE "${PROJECT_SOURCE_DIR}" "modules/*.cpp")
foreach(m ${mods})
	get_filename_component(mod ${m} NAME_WE)
	get_filename_component(moddir ${m} DIRECTORY)
	string(SUBSTRING ${moddir} 8 -1 moddir)
	add_library(${mod} SHARED "${m}")
	target_link_libraries(${mod} mpc et-lib)
	if(${mod} STREQUAL "complex")
		target_link_libraries(${mod} mpc et-lib)
	endif()
	set_target_properties(${mod}
	    PROPERTIES
	    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/ethereal/${moddir}"
	    INSTALL_RPATH_USE_LINK_PATH TRUE
	)
	install(TARGETS ${mod}
		LIBRARY
		  DESTINATION lib/ethereal/${moddir}
		  COMPONENT Libraries
	)
endforeach()

# Playground
set(BINARY_LOC "${CMAKE_INSTALL_PREFIX}/bin/et")
configure_file("${PROJECT_SOURCE_DIR}/playground.et" "${CMAKE_BINARY_DIR}/artifacts/et-repl" @ONLY)
install(PROGRAMS "${CMAKE_BINARY_DIR}/artifacts/et-repl" DESTINATION "${CMAKE_INSTALL_PREFIX}/bin/")

# EPM
configure_file("${PROJECT_SOURCE_DIR}/tools/epm/epm.et" "${CMAKE_BINARY_DIR}/artifacts/epm" @ONLY)
install(PROGRAMS "${CMAKE_BINARY_DIR}/artifacts/epm" DESTINATION "${CMAKE_INSTALL_PREFIX}/bin/")
install(DIRECTORY "${CMAKE_SOURCE_DIR}/tools/epm/epm/" DESTINATION "${CMAKE_INSTALL_PREFIX}/include/ethereal/epm")
