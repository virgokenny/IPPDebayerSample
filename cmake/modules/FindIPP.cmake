# - Find Intel IPP
# Find the IPP libraries
# Options:
#
#   IPP_STATIC: true if using static linking
#   IPP_MULTI_THREADED: true if using multi-threaded static linking
#
# This module defines the following variables:
#
#   IPP_FOUND       : True if IPP_INCLUDE_DIR are found
#   IPP_INCLUDE_DIR : where to find ipp.h, etc.
#   IPP_INCLUDE_DIRS: set when IPP_INCLUDE_DIR found
#   IPP_LIBRARIES   : the library to link against.

include(FindPackageHandleStandardArgs)

# Define search paths based on user input and environment variables
set(IPP_SEARCH_DIR ${IPP_ROOT_DIR} $ENV{IPP_INSTALL_DIR} $ENV{IPPROOT})

set(IPP_DEFAULT_SEARCH_DIR 
    "C:/Program Files (x86)/IntelSWTools/compilers_and_libraries/windows/ipp"
    "/opt/intel/ipp")

# Find header file dir
find_path(IPP_INCLUDE_DIR ipp.h
  HINTS ${IPP_INCLUDE_DIR} ${IPP_SEARCH_DIR}
  PATHS ${IPP_DEFAULT_SEARCH_DIR}
  PATH_SUFFIXES include)

##################################
# Set version strings
##################################

if(IPP_INCLUDE_DIR)
    file(READ "${IPP_INCLUDE_DIR}/ippversion.h" _ipp_version_file)
    string(REGEX REPLACE ".*#define IPP_VERSION_MAJOR  ([0-9]+).*" "\\1"
        IPP_VERSION_MAJOR "${_ipp_version_file}")
    string(REGEX REPLACE ".*#define IPP_VERSION_MINOR  ([0-9]+).*" "\\1"
        IPP_VERSION_MINOR "${_ipp_version_file}")
    string(REGEX REPLACE ".*#define IPP_VERSION_UPDATE ([0-9]+).*" "\\1"
        IPP_VERSION_UPDATE "${_ipp_version_file}")
    set(IPP_VERSION "${IPP_VERSION_MAJOR}.${IPP_VERSION_MINOR}.${IPP_VERSION_UPDATE}")
endif()

# Find libraries

# Set the target architecture
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(IPP_ARCHITECTURE "intel64")
else()
  set(IPP_ARCHITECTURE "ia32")
endif()

# Handle suffix
set(_IPP_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})

if(WIN32)
    set(CMAKE_FIND_LIBRARY_SUFFIXES .lib)
else()
    if(IPP_STATIC)
        set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
    else()
        set(CMAKE_FIND_LIBRARY_SUFFIXES .so)
    endif()
endif()

if(IPP_STATIC)
    if(IPP_MULTI_THREADED)
        set(IPP_LIBNAME_SUFFIX _t)
    else()
        set(IPP_LIBNAME_SUFFIX _l)
    endif()
else()
    set(IPP_LIBNAME_SUFFIX "")
endif()

macro(find_ipp_library IPP_COMPONENT)
  string(TOLOWER ${IPP_COMPONENT} IPP_COMPONENT_LOWER)

  find_library(IPP_LIB_${IPP_COMPONENT} ipp${IPP_COMPONENT_LOWER}${IPP_LIBNAME_SUFFIX}
               HINTS ${IPP_LIB_DIR} ${IPP_SEARCH_DIR}
               PATHS ${IPP_DEFAULT_SEARCH_DIR}
               PATH_SUFFIXES lib/${IPP_ARCHITECTURE})
  if(IPP_LIB_${IPP_COMPONENT} AND EXISTS "${IPP_LIB_${IPP_COMPONENT}}")
    list(APPEND IPP_LIBRARY "${IPP_LIB_${IPP_COMPONENT}}")  
    set(IPP_ipp${IPP_COMPONENT_LOWER}_FOUND TRUE)
  else()
    set(IPP_ipp${IPP_COMPONENT_LOWER}_FOUND FALSE)
  endif()
endmacro()

# IPP components
set(IPP_SEARCH_COMPOMPONENTS CORE AC CC CH CP CV DC DI GEN I J R M S SC SR VC VM)

# Find each component
foreach(_comp ${IPP_SEARCH_COMPOMPONENTS})
    string(TOLOWER ${_comp} IPP_COMPONENT_LOWER)
    if(";${IPP_FIND_COMPONENTS};" MATCHES ";ipp${IPP_COMPONENT_LOWER};")
        find_ipp_library(${_comp})
    endif()
endforeach()

#  # Search for the libraries
## Core
#find_ipp_library(CORE)
## Audio Coding
#find_ipp_library(AC)
## Color Conversion
#find_ipp_library(CC)
## String Processing
#find_ipp_library(CH)
## Cryptography
#find_ipp_library(CP)
## Computer Vision
#find_ipp_library(CV)
## Data Compression
#find_ipp_library(DC)
## Data Integrity
#find_ipp_library(DI)
## Generated Functions
#find_ipp_library(GEN)
## Image Processing
#find_ipp_library(I)
## Image Compression
#find_ipp_library(J)
## Realistic Rendering and 3D Data Processing
#find_ipp_library(R)
## Small Matrix Operations
#find_ipp_library(M)
## Signal Processing
#find_ipp_library(S)
## Speech Coding
#find_ipp_library(SC)
## Speech Recognition
#find_ipp_library(SR)
## Video Coding
#find_ipp_library(VC)
## Vector Math
#find_ipp_library(VM)

#set(IPP_LIBRARY
#    ${IPP_LIB_CORE}
#    ${IPP_LIB_AC}
#    ${IPP_LIB_CC}
#    ${IPP_LIB_CH}
#    ${IPP_LIB_CP}
#    ${IPP_LIB_CV}
#    ${IPP_LIB_DC}
#    ${IPP_LIB_DI}
#    ${IPP_LIB_GEN}
#    ${IPP_LIB_I}
#    ${IPP_LIB_J}
#    ${IPP_LIB_R}
#    ${IPP_LIB_M}
#    ${IPP_LIB_S}
#    ${IPP_LIB_SC}
#    ${IPP_LIB_SR}
#    ${IPP_LIB_VC}
#    ${IPP_LIB_VM})

set(CMAKE_FIND_LIBRARY_SUFFIXES ${_IPP_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES})

find_package_handle_standard_args(IPP 
    REQUIRED_VARS IPP_INCLUDE_DIR IPP_LIBRARY
    HANDLE_COMPONENTS
    VERSION_VAR IPP_VERSION)

if (IPP_FOUND)
    set(IPP_INCLUDE_DIRS ${IPP_INCLUDE_DIR})
    set(IPP_LIBRARIES ${IPP_LIBRARY})
endif()