cmake_minimum_required(VERSION 2.8.9)

project (dukluv)

include(duktape.cmake)

find_package(PkgConfig)
PKG_CHECK_MODULES(LIBUV libuv>=1.7)

include_directories(
  ${LIBUV_INCLUDE_DIRS}
)
link_directories(
  ${LIBUV_LIBRARY_DIRS}
)

set( CMAKE_VERBOSE_MAKEFILE on )

add_library(duv SHARED
  src/duv.c
  src/refs.c
  src/utils.c
  src/loop.c
  src/req.c
  src/handle.c
  src/timer.c
  src/thread.c
  src/stream.c
  src/tcp.c
  src/pipe.c
  src/tty.c
  src/fs.c
  src/misc.c
  src/utils.c
  src/uv_schema.c
  src/callbacks.c
  src/miniz.c
)

add_library(dschema SHARED src/schema.c)

add_executable(dukluv src/main.c)

target_link_libraries(dukluv duv dschema duktape ${LIBUV_LIBRARIES})

# shared object generation
set_target_properties(dukluv PROPERTIES PUBLIC_HEADER src/duv.h)
set_target_properties(dschema PROPERTIES PUBLIC_HEADER src/schema.h)

set(CMAKE_INSTALL_NAME_DIR ${LIB_INSTALL_DIR})
set(BIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/bin" CACHE STRING "Directory where lib will install")
set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}" CACHE STRING "Directory where lib will install")
set(INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/include" CACHE PATH "The directory the headers are installed in")
set(PROJECT_DESCRIPTION "DukTape libuv bindings")
set(PROJECT_VERSION "0.1")
set(PKG_CONFIG_LIBDIR "\${prefix}/lib")
set(PKG_CONFIG_REQUIRES "libuv")
set(PKG_CONFIG_INCLUDEDIR "\${prefix}/include/duv")
set(PKG_CONFIG_LIBS "-L\${libdir} -lduv -ldschema -lduktape")
set(PKG_CONFIG_CFLAGS "-I\${includedir}")

configure_file( "${CMAKE_CURRENT_SOURCE_DIR}/pkg-config.pc.cmake" "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc")

install(TARGETS duv dschema duktape DESTINATION ${LIB_INSTALL_DIR})
install(TARGETS dukluv DESTINATION ${BIN_INSTALL_DIR})
install(FILES src/refs.h src/duv.h lib/duktape/src-separate/duk_config.h lib/duktape/src-separate/duktape.h DESTINATION ${INCLUDE_INSTALL_DIR}/duv)
install(FILES "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc" DESTINATION lib/pkgconfig)




