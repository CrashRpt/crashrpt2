project(libjpeg)

# Create the list of source files
aux_source_directory( . source_files )

# Define _UNICODE (use wide-char encoding)
add_compile_definitions( _UNICODE _CRT_SECURE_NO_WARNINGS )

add_compile_options( /wd4100 /wd4127 )

fix_default_compiler_settings_()

# Add library build target
add_library(libjpeg STATIC ${source_files})

set_target_properties(libjpeg PROPERTIES DEBUG_POSTFIX d )
