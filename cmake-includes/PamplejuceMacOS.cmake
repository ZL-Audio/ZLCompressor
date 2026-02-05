if(CMAKE_OSX_ARCHITECTURES MATCHES "arm64")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11" CACHE STRING "Support macOS down to macOS 11")
    message(STATUS "arm64 macOS deployment target is macOS 11")
elseif (CMAKE_OSX_ARCHITECTURES MATCHES "x86_64")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Support macOS down to macOS 10.13")
    message(STATUS "x86-64 macOS deployment target is macOS 10.13")
endif ()

# By default we don't want Xcode schemes to be made for modules, etc
set(CMAKE_XCODE_GENERATE_SCHEME OFF)
