cmake_minimum_required(VERSION 2.8)
project(MYSQLHLL128)

include_directories(${MYSQL_INCLUDE_DIR})

set(MAIN
        main.cpp
        hyperloglog128.cpp
        hyperloglog128.h
        murmurhash3.c
        murmurhash3.h
        serializedhyperloglog.h
        base64.h)

set(SOURCE_FILES
        mysqlhll.cpp
        hyperloglog128.cpp
        hyperloglog128.h
        murmurhash3.c
        murmurhash3.h
        serializedhyperloglog.cpp
        base64.h)

add_library (mysqlhll128 SHARED
        mysqlhll.cpp
        serializedhyperloglog.cpp
        hyperloglog128.cpp
        hyperloglog128.h
        murmurhash3.c
        murmurhash3.h
        base64.h
        main.cpp
        serializedhyperloglog.h)

add_executable(hll128 ${MAIN})
