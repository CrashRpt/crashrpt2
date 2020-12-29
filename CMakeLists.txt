# Preserve backwards compatibility
cmake_minimum_required (VERSION 3.15)
project (CrashRpt)

# CrashRpt version number
set (CRASHRPT_VER 1500)

# Check supported generators
if(NOT MSVC OR ${MSVC_VERSION} LESS 1600)
  message(FATAL_ERROR "This version of Visual Studio is not supported: ${CMAKE_GENERATOR}.")
endif(NOT MSVC OR ${MSVC_VERSION} LESS 1600)

# Build options
option(CRASHRPT_BUILD_SHARED_LIBS "If set (default), CrashRpt modules are built as dynamic-link libraries, otherwise as static libs." ON)
option(CRASHRPT_LINK_CRT_AS_DLL "If set (default), CrashRpt modules link C run-time (CRT) as multi-threaded dynamic libraries, otherwise as multi-threaded static libs." ON)
option(CRASHRPT_BUILD_DEMOS "If set (default), CrashRpt builds the demo projects." ON)
option(CRASHRPT_BUILD_TESTS "If set (default), CrashRpt builds the test projects." ON)
option(CRASHRPT_INSTALL_PDB "If set (default), CrashRpt also installs PDB files." ON)

add_compile_options( /W4 /wd4456 /wd4458 /MP /Oy- /EHsc )

if(MSVC_VERSION LESS 1910 )
  set( DBGHELP_USE_INSTALLED_SDK True )
else()
  # Windows 10 SDK not guaranteed to be available, so header dependencies may be missing
  set( DBGHELP_USE_INSTALLED_SDK False )
endif()

if(MSVC_VERSION LESS 1900 )
  SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} /DEBUG /OPT:NOREF /OPT:NOICF" )
  SET( CMAKE_SHARED_LINKER_FLAGS  "${CMAKE_SHARED_LINKER_FLAGS} /DEBUG /OPT:NOREF /OPT:NOICF" )
else()
  SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} /DEBUG:FULL /OPT:NOREF /OPT:NOICF" )
  SET( CMAKE_SHARED_LINKER_FLAGS  "${CMAKE_SHARED_LINKER_FLAGS} /DEBUG:FULL /OPT:NOREF /OPT:NOICF" )
endif()


# Set output directory for executable files
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR}/bin)

# Set output directory for DLL files
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR}/bin)

# Set output directory for LIB files
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR}/lib)

# Set installation directories
IF(CMAKE_CL_64)
  SET(AMD_INSTALL_SUFFIX "x64/")
  SET(AMD_3RD_SUFFIX "amd64/")
ENDIF()

SET(CRASHRPT_INSTALLDIR_BIN ${CMAKE_INSTALL_PREFIX}/bin/${AMD_INSTALL_SUFFIX})
SET(CRASHRPT_INSTALLDIR_LIB ${CMAKE_INSTALL_PREFIX}/lib/${AMD_INSTALL_SUFFIX})

# Precompiled headers stuff (CMake doesn't have a standard command for enabling precompiled headers,
# so we have to use a macro)
MACRO(ADD_MSVC_PRECOMPILED_HEADER PrecompiledHeader PrecompiledSource SourcesVar)
  IF(MSVC)
    GET_FILENAME_COMPONENT(PrecompiledBasename ${PrecompiledHeader} NAME_WE)
    SET(PrecompiledBinary "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${PrecompiledBasename}.pch")
    SET(Sources ${${SourcesVar}})

    SET_SOURCE_FILES_PROPERTIES(${PrecompiledSource}
                                PROPERTIES COMPILE_FLAGS "/Yc\"${PrecompiledHeader}\" /Fp\"${PrecompiledBinary}\""
                                           OBJECT_OUTPUTS "${PrecompiledBinary}")
    SET_SOURCE_FILES_PROPERTIES(${Sources}
                                PROPERTIES COMPILE_FLAGS "/Yu\"${PrecompiledBinary}\" /FI\"${PrecompiledBinary}\" /Fp\"${PrecompiledBinary}\""
                                           OBJECT_DEPENDS "${PrecompiledBinary}")
    # Add precompiled header to SourcesVar
    LIST(APPEND ${SourcesVar} ${PrecompiledSource})
  ENDIF(MSVC)
ENDMACRO(ADD_MSVC_PRECOMPILED_HEADER)

# Modifies CMake's default compiler/linker settings.
macro(fix_default_compiler_settings_)
  if (MSVC)
    # For MSVC, CMake sets certain flags to defaults we want to override.
    # This replacement code is taken from sample in the CMake Wiki at
    # https://gitlab.kitware.com/cmake/community/-/wikis/FAQ#dynamic-replace.
    foreach(flag_var
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
      if(${flag_var} MATCHES "/MD" AND NOT CRASHRPT_LINK_CRT_AS_DLL)
        string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
      endif()
    endforeach()
  endif(MSVC)
endmacro()

fix_default_compiler_settings_()

# Other CMakeLists are located in project subdirectories

if( DBGHELP_USE_INSTALLED_SDK )
  set( DBGHELP_INCLUDE_DIR )
  set( DBGHELP_IMPORT_LIB )
else()
  set( DBGHELP_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/thirdparty/dbghelp/include" )
  if( CMAKE_CL_64 )
    set(DBGHELP_IMPORT_LIB ${CMAKE_SOURCE_DIR}/thirdparty/dbghelp/lib/amd64/dbghelp.lib)
  else()
    set(DBGHELP_IMPORT_LIB ${CMAKE_SOURCE_DIR}/thirdparty/dbghelp/lib/dbghelp.lib)
  endif()
endif()

IF(CRASHRPT_BUILD_DEMOS)
add_subdirectory("demos/ConsoleDemo")
add_subdirectory("demos/WTLDemo")
add_subdirectory("demos/MFCDemo")
ENDIF()

add_subdirectory("reporting/crashrpt")
add_subdirectory("reporting/crashsender")

add_subdirectory("processing/crashrptprobe")
add_subdirectory("processing/crprober")

IF(CRASHRPT_BUILD_TESTS)
  add_subdirectory("tests")
ENDIF()

add_subdirectory("thirdparty/tinyxml")
add_subdirectory("thirdparty/jpeg")
add_subdirectory("thirdparty/libpng")
add_subdirectory("thirdparty/minizip")
add_subdirectory("thirdparty/zlib")
add_subdirectory("thirdparty/libogg")
add_subdirectory("thirdparty/libtheora")

# Copy data in binary directory

if( NOT DBGHELP_USE_INSTALLED_SDK )
  MESSAGE("Copying DLLs in bin directory...")
  if(NOT CMAKE_CL_64)
    FILE(COPY ${CMAKE_SOURCE_DIR}/thirdparty/dbghelp/bin/dbghelp.dll DESTINATION ${CMAKE_BINARY_DIR}/bin/)
  ELSE()
    FILE(COPY ${CMAKE_SOURCE_DIR}/thirdparty/dbghelp/bin/amd64/dbghelp.dll DESTINATION ${CMAKE_BINARY_DIR}/bin/)
  ENDIF()
  INSTALL(FILES ${CMAKE_SOURCE_DIR}/thirdparty/dbghelp/bin/${AMD_3RD_SUFFIX}/dbghelp.dll DESTINATION ${CRASHRPT_INSTALLDIR_BIN})
endif()

MESSAGE("Copying test data files...")
FILE(COPY ${CMAKE_SOURCE_DIR}/lang_files/crashrpt_lang_EN.ini DESTINATION ${CMAKE_BINARY_DIR}/bin/)
FILE(RENAME ${CMAKE_BINARY_DIR}/bin/crashrpt_lang_EN.ini ${CMAKE_BINARY_DIR}/bin/crashrpt_lang.ini)

FILE(COPY ${CMAKE_SOURCE_DIR}/tests/dummy.ini DESTINATION ${CMAKE_BINARY_DIR}/bin/)
FILE(COPY ${CMAKE_SOURCE_DIR}/tests/dummy.log DESTINATION ${CMAKE_BINARY_DIR}/bin/)

# Installation

INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/include/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include/)
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/demos/ DESTINATION ${CMAKE_INSTALL_PREFIX}/demos/)
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/lang_files/ DESTINATION ${CMAKE_INSTALL_PREFIX}/lang_files/)
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/docs/ DESTINATION ${CMAKE_INSTALL_PREFIX}/docs/)
