include_directories (${MYSQLHLL_SOURCE_DIR}/libmysqlhll) 

link_directories (${MYSQLHLL_BINARY_DIR}/libmysqlhll) 

add_executable (libmysqlhll-test test.cxx) 

target_link_libraries (libmysqlhll-test libmysqlhll) 
