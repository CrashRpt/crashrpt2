project(libpng)

# Create the list of source files
aux_source_directory( . source_files )

# Define _UNICODE (use wide-char encoding)
add_compile_definitions( _UNICODE _CRT_SECURE_NO_WARNINGS )

add_compile_options( /wd4206 /wd4245 /wd4267 /wd4310 )

# Add include dir
include_directories( ${CMAKE_SOURCE_DIR}/thirdparty/zlib )

fix_default_compiler_settings_()

# Add library build target
add_library(libpng STATIC ${source_files})

# Add linker input libraries
target_link_libraries(libpng zlib)

set_target_properties(libpng PROPERTIES DEBUG_POSTFIX d )
