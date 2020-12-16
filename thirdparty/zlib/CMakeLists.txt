project(zlib)

# Create the list of source files
aux_source_directory( . source_files )

# Define _UNICODE (use wide-char encoding)
add_compile_definitions( _UNICODE _CRT_SECURE_NO_WARNINGS )

add_compile_options( /wd4127 /wd4131 /wd4244 /wd4245 /wd4267 /wd4996 )

fix_default_compiler_settings_()

# Add library build target
add_library(zlib STATIC ${source_files})

set_target_properties(zlib PROPERTIES DEBUG_POSTFIX d )
