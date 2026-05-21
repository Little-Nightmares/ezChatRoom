set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

# Include both Qt6, OpenSSL, and MinGW sysroot in the find root path
set(CMAKE_FIND_ROOT_PATH /opt/qt6-windows/6.2.4/mingw_64 /opt/openssl-windows /usr/x86_64-w64-mingw32)
set(CMAKE_PREFIX_PATH /opt/qt6-windows/6.2.4/mingw_64)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# OpenSSL static library (cross-compiled for Windows)
set(OPENSSL_ROOT_DIR /opt/openssl-windows)

# Use Linux-native Qt6 tools (moc, rcc) from the system Qt6 installation.
# The system Qt6 is version 6.2.4, matching the Windows target Qt6.
set(QT_HOST_PATH /usr/lib/x86_64-linux-gnu)

# Explicitly set host tools paths for cross-compilation
set(CMAKE_AUTOMOC_EXECUTABLE /usr/lib/qt6/libexec/moc)
set(CMAKE_AUTORCC_EXECUTABLE /usr/lib/qt6/libexec/rcc)
set(CMAKE_AUTOUIC_EXECUTABLE /usr/lib/qt6/libexec/uic)

# Qt6 QML tools
set(QT_QMLCACHEGEN_EXECUTABLE /usr/lib/qt6/libexec/qmlcachegen)
set(QT_QMLTYPEGEN_EXECUTABLE /usr/lib/qt6/libexec/qmltyperegistrar)
set(QT_QMLPLUGINDUMP_EXECUTABLE /usr/lib/qt6/libexec/qmlplugindump)

# Make sure Qt6 uses the host tools, not the Windows ones
set(QT6_HOST_INFO_CMAKE_DIR /usr/lib/x86_64-linux-gnu/cmake/Qt6HostInfo)
