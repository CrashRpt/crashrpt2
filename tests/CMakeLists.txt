project(Tests)

# Create the list of source files
aux_source_directory( . source_files )
file( GLOB header_files *.h )

list(APPEND source_files ${CMAKE_SOURCE_DIR}/reporting/CrashRpt/Utility.cpp)

# Enable usage of precompiled header
set(srcs_using_precomp ${source_files})
list(REMOVE_ITEM srcs_using_precomp ./stdafx.cpp )
add_msvc_precompiled_header(stdafx.h ./stdafx.cpp srcs_using_precomp )

# Define _UNICODE and UNICODE (use wide-char encoding)
add_compile_definitions( _UNICODE UNICODE )

add_compile_options( /wd4100 )

if(NOT CRASHRPT_BUILD_SHARED_LIBS)
  add_compile_definitions( CRASHRPT_LIB )
endif(NOT CRASHRPT_BUILD_SHARED_LIBS)

fix_default_compiler_settings_()

set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG")

# Add include dir
include_directories(
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/reporting/CrashRpt
  ${CMAKE_SOURCE_DIR}/thirdparty/wtl
)

# Add executable build target
add_executable(Tests ${source_files} ${header_files})

# Add input link libraries
target_link_libraries(Tests CrashRpt CrashRptProbe)

set_target_properties(Tests PROPERTIES DEBUG_POSTFIX d )

INSTALL(TARGETS Tests
  LIBRARY DESTINATION ${CRASHRPT_INSTALLDIR_BIN}
  ARCHIVE DESTINATION ${CRASHRPT_INSTALLDIR_LIB}
  RUNTIME DESTINATION ${CRASHRPT_INSTALLDIR_BIN}
)
