cmake_minimum_required(VERSION 3.20)
project(SuperUser)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define preprocessor macros for Unicode support
add_definitions(-DUNICODE -D_UNICODE)

# Enable threading support
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

# Add the source and resource files
set(SOURCES
    cpp/main.cpp
    cpp/allocator.cpp
    cpp/superuser.cpp
    cpp/connector.cpp
    cpp/pressortext.cpp
    cpp/pressorfile.cpp
    cpp/depressorfile.cpp
    cpp/downloaddata.cpp
    cpp/uploaddata.cpp
    cpp/cpu_info.cpp
    cpp/winsys.cpp
    cpp/button_subclass.cpp
    cpp/cpu_info_plugger.cpp
    cpp/components.cpp
    cpp/deletedata.cpp
    cpp/font_utils.cpp
    cpp/signup.cpp
    cpp/events.cpp
    cpp/saveuser.cpp
    cpp/prevnext.cpp
    cpp/fireonboot.cpp
    cpp/lessbusycore.cpp
    cpp/video_converter.cpp
    cpp/retrievedata.cpp
    cpp/audio_converter.cpp
    cpp/image_converter.cpp
    resource.rc
)

set(HEADERS
    h/signup.h
    h/superuser.h
    h/allocator.h
    h/connector.h
    h/pressortext.h
    h/pressorfile.h
    h/depressorfile.h
    h/downloaddata.h
    h/uploaddata.h
    h/cpu_info.h
    h/winsys.h
    h/font_utils.h
    h/components.h
    h/cpu_info_plugger.h
    h/events.h
    h/lessbusycore.h
    h/button_subclass.h
    h/prevnext.h
    h/deletedata.h
    h/saveuser.h
    h/fireonboot.h
    h/audio_converter.h
    h/video_converter.h
    h/retrievedata.h
    h/image_converter.h 
    h/resource.h
)

# Set the project to be a Windows GUI application (hides console window)
set(CMAKE_WIN32_EXECUTABLE TRUE)

# Locate FFmpeg in MSYS2
find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET libavcodec libavformat libavutil libswscale libswresample)

# Locate libcurl (this will add libcurl support)
find_package(CURL REQUIRED)

# Find OpenMP
find_package(OpenMP REQUIRED)

# Create the executable
add_executable(SuperUser ${SOURCES} ${HEADERS})

# Include FFmpeg and libcurl directories
target_include_directories(SuperUser PRIVATE ${FFMPEG_INCLUDE_DIRS} ${CURL_INCLUDE_DIRS})

# Link necessary libraries.
target_link_libraries(SuperUser
    PRIVATE
    PkgConfig::FFMPEG    # Link FFmpeg libraries via PkgConfig
    pdh                  # PDH library
    z                    # zlib
    lzma                 # lzma
    kernel32             # Windows API
    user32
    gdi32
    advapi32
    shlwapi             # Shlwapi for path functions (e.g., PathRemoveFileSpec)
    wininet              # WinINet for HTTP functions
    comctl32             # Common controls for Windows UI
    comdlg32             # Common dialogs
    riched20             # RichEdit control
    swresample           # libswresample for audio conversion
    CURL::libcurl        # Link libcurl
    ws2_32               # Winsock 2 (needed for networking)
    winmm                # Windows Multimedia API
    pthread              # POSIX threads (only for MinGW)
    ${CMAKE_SOURCE_DIR}/C:/msys64/mingw64/lib/libsoxr.a  # Link libsoxr
    OpenMP::OpenMP_CXX   # Link OpenMP
)

# Ensure <filesystem> works
if(MINGW)
    target_link_libraries(SuperUser PRIVATE stdc++fs)  # Link stdc++fs for MinGW
endif()

# Optimize for smaller executable size
target_compile_options(SuperUser PRIVATE
    -Os                # Optimize for size
    -ffunction-sections
    -fdata-sections
)

# Force static linking for all libraries
target_link_options(SuperUser PRIVATE
    -static
    -static-libgcc
    -static-libstdc++  # Statically link libstdc++
    -static -lgomp     # Force static linking for OpenMP
    -s                 # Strip debug symbols (optional)
)

# Set the output binary directory
set_target_properties(SuperUser PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# For MinGW: define _WIN32_WINNT for RichEdit and other Windows headers
if(MINGW)
    target_compile_definitions(SuperUser PRIVATE _WIN32_WINNT=0x0601)  # Target Windows 7+
endif()