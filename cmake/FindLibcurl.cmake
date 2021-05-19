# - Try to find Libcurl includes dirs and libraries
#
# Usage of this module as follows:
#
#     find_package(Libcurl)
#
# Variables used by this module, they can change the default behaviour and need
# to be set before calling find_package:
#
# Variables defined by this module:
#
#  Libevent_FOUND            System has Libevent, include and lib dirs found
#  Libevent_INCLUDE_DIR      The Libevent includes directories.
#  Libevent_LIBRARY          The Libevent library.

find_path(Libcurl_INCLUDE_DIR NAMES curl/curl.h PATHS /usr/local/include)
find_library(Libcurl_LIBRARY NAMES  libcurl curl PATHS /usr/local/lib)
message(STATUS "********Libcurl_INCLUDE_DIR is ${Libcurl_INCLUDE_DIR}")
message(STATUS "********Libcurl_LIBRARY is ${Libcurl_LIBRARY}")

if(Libcurl_INCLUDE_DIR AND Libcurl_LIBRARY)
    set(Libcurl_FOUND TRUE)
    mark_as_advanced(
        Libcurl_INCLUDE_DIR
        Libcurl_LIBRARY
    )
endif()

if(NOT Libcurl_FOUND)
    message(FATAL_ERROR "Libcurl doesn't exist")
endif()