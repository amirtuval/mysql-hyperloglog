include_directories (${MYSQLHLL_SOURCE_DIR}/libmysqlhll/cpp-hyperloglog/include ${MYSQLHLL_SOURCE_DIR}/libmysqlhll/base64) 

add_library (mysqlhll SHARED mysqlhll.cxx)
