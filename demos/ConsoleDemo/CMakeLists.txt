project(ConsoleDemo)

# Create the list of source files
aux_source_directory( . source_files )
file( GLOB header_files *.h )

# Define _UNICODE (use wide-char encoding)
add_definitions(-D_UNICODE )

fix_default_compiler_settings_()

# Add include dir
include_directories(${CRASHRPT_SRC}/include 
          ${CRASHRPT_SRC}/reporting/CrashRpt
          ${CRASHRPT_SRC}/thirdparty/wtl 
          ${CRASHRPT_SRC}/thirdparty/zlib
          ${CRASHRPT_SRC}/thirdparty/minizip  
          ${CRASHRPT_SRC}/thirdparty/jpeg 
          ${CRASHRPT_SRC}/thirdparty/libpng 
          ${CRASHRPT_SRC}/thirdparty/tinyxml )

# Add executable build target
add_executable(ConsoleDemo ${source_files} ${header_files})

# Add input link libraries
target_link_libraries(ConsoleDemo CrashRpt)

set_target_properties(ConsoleDemo PROPERTIES DEBUG_POSTFIX d )

INSTALL(TARGETS ConsoleDemo LIBRARY DESTINATION ${CRASHRPT_INSTALLDIR_BIN}
             ARCHIVE DESTINATION ${CRASHRPT_INSTALLDIR_LIB}
             RUNTIME DESTINATION ${CRASHRPT_INSTALLDIR_BIN})