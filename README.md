mysql-hyperloglog128
=================

An experimental MySQL plugin for [the HyperLogLog Algorithm](http://en.wikipedia.org/wiki/HyperLogLog), supporting the hashing ([MurmurHash3](https://en.wikipedia.org/wiki/MurmurHash)) and default dimensions set in [Algebird](https://github.com/twitter/algebird), forked from the original Amir Tuval [mysql-hyperloglog](https://github.com/amirtuval/mysql-hyperloglog).

It adds several aggregate functions to mysql, that allows you to estimate the distinct count of large datasets, in addition to estimate the cardinality of previously stored HLL data structures pre-computed by Algebird in [Spark](http://spark.apache.org), which is where the real power of the plugin resides, serving as a replacement for count(distinct).


Thanks
======
Thanks to the original code by Amir Tuval.
The excellent [cpp-hyperloglog](https://github.com/hideo55/cpp-HyperLogLog) project is used for the actual HyperLogLog implementation. Thanks, Amir Tuval and Hideaki Ohno!

Usage
=====

The plugin includes 4 aggregate functions:

HLL_CREATE - Given a list of values, this function will return the HLL string computed from these values.  
HLL_COMPUTE - Given a list of values, this function will return an integer representing the estimated distinct count of these values.  
HLL_MERGE - Given a list of HLL strings, this function will return the HLL string that is the combination of all the hll strings.  
HLL_MERGE_COMPUTE - Given a list of HLL strings, this function will return an integer representing the estimated distinct count of these values.
  
HyperLogLog stores its data as a byte vector. An HLL string is the base64 representation of the byte vector.  
In this implementation, the plugin uses hyperloglog with a 12 bit width, resulting in a 4096 bytes vector. Base64 string of that is ~5200 chars long.
  
[Here's a more detailed example](sql/example.sql).

**NOTE:** Each of the above functions can accept any number of arguments. Each value will be treated as an additional value added.  
So you can issue the following statement, for example:

```sql
mysql> select hll_compute(1,2,4,3,4,2,1);
+----------------------------+
| hll_compute(1,2,4,3,4,2,1) |
+----------------------------+
|                          4 |
+----------------------------+
1 row in set (0.01 sec)
```

The interesting use case this fork is addressing is being able to read HLL structure stored from Spark to compute in MySQL its estimated size.

```scala
import com.twitter.algebird.Operators._
import com.twitter.algebird.HLL
import com.twitter.algebird.HyperLogLog._
import com.twitter.algebird.HyperLogLogMonoid
import javax.xml.bind.DatatypeConverter

val b = 12
def hllToB64(hll: HLL): String = {
  val header = s"${b}|"
  val data = hll.toDenseHLL.v.array
  val s = new String(DatatypeConverter.printBase64Binary(data))
  header ++ s
}

val hll = new HyperLogLogMonoid(b)
val data = for (i <- 1 to 100000) yield i
val myHLL = hll.sum( data.map(hll.toHLL(_)) )
val query = s"INSERT INTO test_hll values('100000k-sample', '${hllToB64(myHLL)}');"
updateMySql(query)
```

This naïve example simply generates an HLL for values between 1 and 100000, which should be replaced by your data keys in your dataset. The result is stored in a table like:

| key          | hll         |
|:------------:|:-----------:|
|100000k-sample| 12|AQgEAQ...|

where the HLL is b64 encoded, showing the nr of bits in the HLL (12).

The only method missing in this example is the one enabling you to write to MySQL database. Write your own method, or take a look to different implementation like the one in [Alvin Alexander blog](http://alvinalexander.com/scala/scala-jdbc-connection-mysql-sql-select-example).

To read the HLL structure from MySql:

```sql
SELECT key, hll_merge_compute(hll) estimated_count FROM test_hll GROUP BY key;
```

and the result should by an estimate of the "100000k-sample" key that is slightly above 100K.

| key          | estimated_count |
|:------------:|:---------------:|
|100000k-sample| 99810           |


Installation
============

Compilation
-----------

The project uses [CMake](http://www.cmake.org/) as a build tool, and
the code is platform independent, so it should compile fine on most
platforms (linux, windows, mac).
There might be some issues around compiling cmake that are beyond the
scope of this guide to resolve. Please consult the documentation/relevant
forums for it. For example, CMake has some issues with compiling 64 bit
projects on windows.

**Mac OS X**
Tested on Mac OS X 10.11.4 and 10.10.

**Linux**
Tested on ubuntu precise (12.04) 64 bit and CentOS 6, but should work
pretty much the same on most linux distros.
  
**Prerequisites:** Make sure you have the cmake, build-essential and
libmysqlclient128-dev packages installed.

Run the following from the project's root directory

```bash
git submodule update --init
cmake .
make
```
  
**NOTE:** You may need to tell cmake where to find mysql header files.
The default is "/usr/include/mysql", but if they are located in another
directory on your machine, add "-DMYSQL_INCLUDE_DIR={DIR}" to the
cmake command line.

After a successful compilation, you will have the libmysqlhll.so binary
under the libmysqlhll/ directory.

Once installed the plugin in the `plugin` directory of MySQL, you
must run the following commands in the console:

```sql
drop function if exists hll_create;
drop function if exists hll_create_legacy;
drop function if exists hll_compute;
drop function if exists hll_merge;
drop function if exists hll_merge_compute;

create aggregate function hll_create returns string soname 'libmysqlhll128.so';
create aggregate function hll_create_legacy returns string soname 'libmysqlhll128.so';
create aggregate function hll_compute returns int soname 'libmysqlhll128.so';
create aggregate function hll_merge returns string soname 'libmysqlhll128.so';
create aggregate function hll_merge_compute returns int soname 'libmysqlhll128.so';
```

MySQL Installation
------------------

Once the binary is compiled (.so on linux, .dll on windows, .dylib on mac), you have to copy it to the MySQL plugins dir. If you are not sure where that is, you can check by running this command 
  
`show variables like '%plugin%';`  
  
in mysql client (On linux, it is usually /usr/lib/mysql/plugin/).

After that login to mysql as root, and run [the functions installation script](sql/udf.sql). Replace .so with .dll if you are on windows.


Customization
=============

If you would like to change the bit width of the HyperLogLog algorithm, you can change it by editing the [constants.hpp file](libmysqlhll128/mysqlhll.cpp) before compiling the project.  
The default is 12, resulting in 2**12(4096) bytes of storage. This can go as high as 16 to get better accuracy with more storage, or you can lower it to save storage.
