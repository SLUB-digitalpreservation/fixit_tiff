cmake_minimum_required (VERSION 2.6)
project (fixit_tiff)
include_directories("${PROJECT_SOURCE_DIR}")
include (CheckSymbolExists)

file( GLOB fixit_tiff_SOURCES 
  "${PROJECT_SOURCE_DIR}/*.c"
  )
#
# Make a version file containing the current version from git.
#
execute_process( COMMAND git rev-list HEAD --count OUTPUT_VARIABLE REPO_REVISION OUTPUT_STRIP_TRAILING_WHITESPACE )
execute_process( COMMAND git rev-parse --abbrev-ref HEAD OUTPUT_VARIABLE BUILD_BRANCH OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process( COMMAND git rev-parse HEAD OUTPUT_VARIABLE BUILD_REV_ID OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process( COMMAND git describe --long --tags --dirty --always OUTPUT_VARIABLE BUILD_REV_ID_SHORT OUTPUT_STRIP_TRAILING_WHITESPACE)
add_definitions(-DVERSION="${BUILD_BRANCH}")
add_definitions(-DREPO_REVISION="${REPO_REVISION}")
add_definitions(-std=gnu11)
if(WIN32)
  SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
  SET(BUILD_SHARED_LIBRARIES OFF)
  SET(CMAKE_EXE_LINKER_FLAGS "-static")
  add_definitions(-DPCRE_STATIC)
  add_definitions(-D_POSIX_C_SOURCE=200809L)
  # FIXME: needed for MingW only
  add_definitions(-D_GNU_SOURCE=1)
endif(WIN32)

add_executable(fixit_tiff fixit_tiff.c ${fixit_tiff_SOURCES})
set(CMAKE_EXTRA_INCLUDE_FILES tiff.h)


## libtiff5
find_package(TIFF REQUIRED)
if(TIFF_FOUND)
  # Confirm required API is available
  #INCLUDE(CheckFunctionExists)
  SET(CMAKE_REQUIRED_LIBRARIES ${TIFF_LIBRARIES})
  #CHECK_FUNCTION_EXISTS(TIFFOpen HAVE_TIFFOPEN)
  #IF(NOT HAVE_TIFFOPEN)
  #  SET(TIFF_FOUND) # ReSET to NOT found for TIFF library
  #  MESSAGE(FATAL_ERROR "Failed to link with libtiff - TIFFOpen function not found")
  #ENDIF()

  # CHECK_FUNCTION_EXISTS(TIFFMergeFieldInfo HAVE_TIFFMERGEFIELDINFO)
  #IF(NOT HAVE_TIFFMERGEFIELDINFO)
  #  SET(TIFF_FOUND) # ReSET to NOT found for TIFF library
  #  MESSAGE(FATAL_ERROR "Failed to link with libtiff - TIFFMergeFieldInfo function not found. libtiff 3.6.0 Beta or later required. Please upgrade or use an older version of libgeotiff")
  #ENDIF()
  INCLUDE_DIRECTORIES(${TIFF_INCLUDE_DIR})
  ADD_DEFINITIONS(-DHAVE_TIFF=1)

  CHECK_SYMBOL_EXISTS(TIFFTAG_IMAGELAYER "tiff.h" HAVE_TIFFTAG_IMAGE_LAYER)
  IF(NOT HAVE_TIFFTAG_IMAGE_LAYER)
    MESSAGE(STATUS "old tiff library detected, -DTIFFTAG_IMAGELAYER=34732 added")
    ADD_DEFINITIONS(-DTIFFTAG_IMAGELAYER=34732)
  ENDIF()

  CHECK_SYMBOL_EXISTS(TIFF_UINT64_T "tiff.h" HAVE_TIFF_UINT64_T)
  IF(NOT HAVE_TIFF_UINT64_T)
    MESSAGE(STATUS "old tiff library detected, -Duint64='unsigned long long' added")

    if(SIZEOF_UNSIGNED_LONG EQUAL 8)
      set(TIFF_UINT64_T "unsigned long")
      set(TIFF_UINT64_FORMAT "%lu")
    elseif(SIZEOF_UNSIGNED_LONG_LONG EQUAL 8)
      set(TIFF_UINT64_T "unsigned long long")
      if (MINGW)
        set(TIFF_UINT64_FORMAT "%I64u")
      else()
        set(TIFF_UINT64_FORMAT "%llu")
      endif()
    endif()
  ENDIF()

  CHECK_SYMBOL_EXISTS(TIFF_INT64_T "tiff.h" HAVE_TIFF_INT64_T)
  IF(NOT HAVE_TIFF_INT64_T)
    MESSAGE(STATUS "old tiff library detected, -Dint64='long long' added")
    if(SIZEOF_SIGNED_LONG EQUAL 8)
      set(TIFF_INT64_T "signed long")
      set(TIFF_INT64_FORMAT "%ld")
    elseif(SIZEOF_SIGNED_LONG_LONG EQUAL 8)
      set(TIFF_INT64_T "signed long long")
      if (MINGW)
        set(TIFF_INT64_FORMAT "%I64d")
      else()
        set(TIFF_INT64_FORMAT "%lld")
      endif()
    endif()

  ENDIF()

else(TIFF_FOUND)
  MESSAGE(FATAL_ERROR "No TIFF library found, add $TIFF_INCLUDE_DIR manually")
endif(TIFF_FOUND)

TARGET_LINK_LIBRARIES(fixit_tiff ${TIFF_LIBRARIES})

install( TARGETS fixit_tiff
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
)
install(DIRECTORY 
  ../examples
  DESTINATION share/fixit_tiff/
)

file( GLOB README_FILES "../README*" "../FAQ" "../LICENSE" "../TODO" "../BUGS")
install(FILES ${README_FILES} DESTINATION share/fixit_tiff/)
