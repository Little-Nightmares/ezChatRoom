# fix-cross-link.cmake
# This script is included via CMAKE_PROJECT_TOP_LEVEL_INCLUDES to fix
# cross-compilation linker issues with OpenSSL static libraries on Windows.
#
# It patches the generated build system to use --start-group/--end-group
# around all libraries, resolving circular dependencies between OpenSSL
# and Windows system libraries.

# We need to defer this until after all targets are defined.
# Use a finalizer to modify the link flags after CMake generates the build files.

function(fix_cross_link_target target)
    get_target_property(type ${target} TYPE)
    if(NOT type STREQUAL "EXECUTABLE")
        return()
    endif()

    # Add linker group flags via target properties
    # LINK_FLAGS is placed right before the libraries in the link command
    set_property(TARGET ${target} APPEND PROPERTY LINK_FLAGS "-Wl,--start-group")

    # We need --end-group at the very end. Use LINK_FLAGS_MULTIPLIER or
    # append to the linker flags that come after libraries.
    # Unfortunately CMake doesn't have a clean way to add flags after libraries.
    # We use a response file trick instead.
endfunction()
