# - Try to find Cereal lib
#
# This sets the following variables:
# CEREAL_FOUND - True if Cereal was found.
# CEREAL_INCLUDE_DIRS - Directories containing the Cereal include files.


find_path(CEREAL_INCLUDE_DIRS cereal
	HINTS "$ENV{CMAKE_SOURCE_DIR}/include" "/usr/include" "$ENV{CMAKE_BINARY_DIR}/cereal/include")

set(CEREAL_INCLUDE_DIRS ${CEREAL_INCLUDE_DIRS})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Cereal DEFAULT_MSG CEREAL_INCLUDE_DIRS)

mark_as_advanced(CEREAL_INCLUDE_DIRS)

if(CEREAL_FOUND)
  MESSAGE(STATUS "Found Cereal: ${CEREAL_INCLUDE_DIRS}")
endif(CEREAL_FOUND)

