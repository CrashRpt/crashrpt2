project(WTLDemo)

# Create the list of source files
aux_source_directory( . source_files )
file( GLOB header_files *.h )

# Enable usage of precompiled header
set(srcs_using_precomp ${source_files})
list(REMOVE_ITEM srcs_using_precomp ./stdafx.cpp)
add_msvc_precompiled_header(stdafx.h ./stdafx.cpp srcs_using_precomp)

# Include resource file
list(APPEND source_files ./WTLDemo.rc )

# Define _UNICODE and UNICODE (use wide-char encoding)
add_compile_definitions( _UNICODE UNICODE )

fix_default_compiler_settings_()

# Add include dir
include_directories( ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/thirdparty/wtl )

# Add executable build target
add_executable(WTLDemo WIN32 ${source_files} ${header_files})

# Add input link libraries
target_link_libraries(WTLDemo CrashRpt)

# Add "d" postfix to output file name in Debug
set_target_properties(WTLDemo PROPERTIES DEBUG_POSTFIX d )
set_target_properties(WTLDemo PROPERTIES COMPILE_FLAGS "/Od /Zi /fp:except" LINK_FLAGS "/DEBUG")

# Add post-build event
IF(CMAKE_CL_64)
SET(AMD_SUFFIX "amd64")
ENDIF()

ADD_CUSTOM_COMMAND(
    TARGET WTLDemo
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} ARGS -E copy "\"${CMAKE_SOURCE_DIR}/demos/WTLDemo/dummy.ini\" \"${CMAKE_BINARY_DIR}/bin/\""
    COMMAND ${CMAKE_COMMAND} ARGS -E copy "\"${CMAKE_SOURCE_DIR}/demos/WTLDemo/dummy.log\" \"${CMAKE_BINARY_DIR}/bin/\""
    COMMAND ${CMAKE_COMMAND} ARGS -E copy "\"${CMAKE_SOURCE_DIR}/lang_files\\crashrpt_lang_EN.ini\" \"${CMAKE_BINARY_DIR}/bin/crashrpt_lang.ini\""
)

if( NOT DBGHELP_USE_INSTALLED_SDK )
  ADD_CUSTOM_COMMAND(
      TARGET WTLDemo
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} ARGS -E copy "\"${CMAKE_SOURCE_DIR}/thirdparty/dbghelp/bin/${AMD_SUFFIX}/dbghelp.dll\" \"${CMAKE_BINARY_DIR}/bin/\""
  )
endif()

INSTALL(TARGETS WTLDemo LIBRARY DESTINATION ${CRASHRPT_INSTALLDIR_BIN}
             ARCHIVE DESTINATION ${CRASHRPT_INSTALLDIR_LIB}
             RUNTIME DESTINATION ${CRASHRPT_INSTALLDIR_BIN})

INSTALL(FILES ${CMAKE_SOURCE_DIR}/demos/WTLDemo/dummy.ini DESTINATION ${CRASHRPT_INSTALLDIR_BIN})
INSTALL(FILES ${CMAKE_SOURCE_DIR}/demos/WTLDemo/dummy.log DESTINATION ${CRASHRPT_INSTALLDIR_BIN})
INSTALL(FILES "${CMAKE_SOURCE_DIR}/lang_files/crashrpt_lang_EN.ini" DESTINATION ${CRASHRPT_INSTALLDIR_BIN} RENAME crashrpt_lang.ini)
