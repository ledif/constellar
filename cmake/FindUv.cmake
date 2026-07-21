# Locate the uv Python package manager (https://docs.astral.sh/uv)
#
# Sets:
#   UV_EXECUTABLE - full path to the uv executable
#   Uv_FOUND      - whether uv was located
#
# Also defines the Uv::uv imported executable target.

find_program(UV_EXECUTABLE NAMES uv DOC "Path to the uv executable")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Uv REQUIRED_VARS UV_EXECUTABLE)

if(Uv_FOUND AND NOT TARGET Uv::uv)
    add_executable(Uv::uv IMPORTED)
    set_target_properties(Uv::uv PROPERTIES IMPORTED_LOCATION ${UV_EXECUTABLE})
endif()

mark_as_advanced(UV_EXECUTABLE)
