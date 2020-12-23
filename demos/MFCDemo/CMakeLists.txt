project(MFCDemo)

# Create the list of source files
aux_source_directory( . source_files )
file( GLOB header_files *.h )

# Enable usage of precompiled header
set(srcs_using_precomp ${source_files})
list(REMOVE_ITEM srcs_using_precomp ./stdafx.cpp)
add_msvc_precompiled_header(stdafx.h ./stdafx.cpp srcs_using_precomp)

# Use MFC
add_compile_definitions( _AFXDLL )
if (CRASHRPT_LINK_CRT_AS_DLL)
  # Use MFC as a shared lib
  SET(CMAKE_MFC_FLAG 2)
else(CRASHRPT_LINK_CRT_AS_DLL)
  # Use MFC as a static lib
  SET(CMAKE_MFC_FLAG 1)
endif(CRASHRPT_LINK_CRT_AS_DLL)

# Include resource file
list(APPEND source_files ./MFCDemo.rc )

# Define _UNICODE and UNICODE (use wide-char encoding)
add_compile_definitions( _UNICODE UNICODE )

fix_default_compiler_settings_()

# Add include dir
include_directories(${CMAKE_SOURCE_DIR}/include)

# Add executable build target
add_executable(MFCDemo WIN32 ${source_files} ${header_files})

# Workaround for WinMain linker error during Unicode build
# https://gitlab.kitware.com/cmake/cmake/-/issues/21202
target_link_options(MFCDemo PRIVATE "/entry:wWinMainCRTStartup")

# Add input link libraries
target_link_libraries(MFCDemo CrashRpt)

# Add "d" postfix to output file name in Debug
set_target_properties(MFCDemo PROPERTIES DEBUG_POSTFIX d )

# Add post-build event
IF(CMAKE_CL_64)
SET(AMD_SUFFIX "amd64")
ENDIF()

ADD_CUSTOM_COMMAND(
    TARGET MFCDemo
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} ARGS -E copy "\"${CMAKE_SOURCE_DIR}/lang_files\\crashrpt_lang_EN.ini\" \"${CMAKE_BINARY_DIR}/bin/crashrpt_lang.ini\""
)

if( NOT DBGHELP_USE_INSTALLED_SDK )
  ADD_CUSTOM_COMMAND(
      TARGET MFCDemo
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} ARGS -E copy "\"${CMAKE_SOURCE_DIR}/thirdparty/dbghelp/bin/${AMD_SUFFIX}/dbghelp.dll\" \"${CMAKE_BINARY_DIR}/bin/\""
  )
endif()

INSTALL(TARGETS MFCDemo LIBRARY DESTINATION ${CRASHRPT_INSTALLDIR_BIN}
             ARCHIVE DESTINATION ${CRASHRPT_INSTALLDIR_LIB}
             RUNTIME DESTINATION ${CRASHRPT_INSTALLDIR_BIN})
