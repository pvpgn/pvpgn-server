add_library(tinycdb 
	cdb_find.cpp cdb_findnext.cpp cdb.h cdb_hash.cpp cdb_init.cpp cdb_int.h
	cdb_make_add.cpp cdb_make.cpp cdb_make_find.cpp cdb_make_put.cpp 
	cdb_seek.cpp cdb_seq.cpp cdb_unpack.cpp)
add_executable(bncdb cdb.cpp)
target_link_libraries(bncdb tinycdb common compat )

install(TARGETS bncdb DESTINATION ${BINDIR})
