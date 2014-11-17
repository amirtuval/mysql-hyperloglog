# rename to '.so' to '.dll' on windows

drop function if exists hll_create;
drop function if exists hll_create_legacy;
drop function if exists hll_compute;
drop function if exists hll_merge;
drop function if exists hll_merge_compute;

create aggregate function hll_create returns string soname 'libmysqlhll.so';
create aggregate function hll_create_legacy returns string soname 'libmysqlhll.so';
create aggregate function hll_compute returns int soname 'libmysqlhll.so';
create aggregate function hll_merge returns string soname 'libmysqlhll.so';
create aggregate function hll_merge_compute returns int soname 'libmysqlhll.so';
