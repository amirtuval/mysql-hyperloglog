mysql-hyperloglog
=================

A MySQL plugin for [the HyperLogLog Algorithm](http://en.wikipedia.org/wiki/HyperLogLog).  
When installed, the plugin adds several aggregate functions to mysql, that allows you to estimate the distinct count of large datasets.  
You can use it as a replacement for count(distinct). However, the real power of the plugin is by storing the hyperloglog result in an [aggregate table](http://en.wikipedia.org/wiki/Aggregate_(data_warehouse)), which allows you to get an estimate of the distinct count from the aggregate.



**Note:** An estimated count is just like it sounds - an estimate, and therefore, not 100% accurate. You can expect several percentages of difference between the actual count and the estimated count, so it might not be suitable for all use cases.  
Hyperloglog trades space for accuracy, so you can increase the accuracy by allowing HLL to store more data. More on that later.

Thanks
======

The excellent [cpp-hyperloglog](https://github.com/hideo55/cpp-HyperLogLog) project is used for the actual HyperLogLog implementation.  
Thanks, Hideaki Ohno.

Usage
=====

The plugin includes 4 aggregate functions:

HLL_CREATE - Given a list of values, this function will return the HLL string computed from these values.  
HLL_COMPUTE - Given a list of values, this function will return an integer representing the estimated distinct count of these values.  
HLL_MERGE - Given a list of HLL strings, this function will return the HLL string that is the combination of all the hll strings.  
HLL_MERGE_COMPUTE - Given a list of HLL strings, this function will return an integer representing the estimated distinct count of these values.
  
HyperLogLog stores its data as a byte vector. An HLL string is the base64 representation of the byte vector.  
In its default implementation, the plugin uses hyperloglog with a 10 bit width, resulting in a 1024 bytes vector. Base64 string of that is ~1400 chars long.
  
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

Installation
============

Compilation
-----------

The project uses [CMake](http://www.cmake.org/) as a build tool, and the code is platform independent, so it should compile fine on most platforms (linux, windows, mac).  
There might be some issues around compiling cmake that are beyond the scope of this guide to resolve. Please consult the documentation/relevant forums for it. For example, CMake has some issues with compiling 64 bit projects on windows.  

**Linux**
  
Tested on ubuntu precise (12.04) 64 bit, but should work pretty much the same on most linux distros.
  
**Prerequisites:** Make sure you have the cmake, build-essential and libmysqlclient-dev packages installed.

Run the following from the project's root directory

```bash
git submodule update --init
cmake .
make
```
  
**NOTE:** You may need to tell cmake where to find mysql header files. The deault is "/usr/include/mysql", but if they are located in another directory on your machine, add "-DMYSQL_INCLUDE_DIR={DIR}" to the cmake command line.  

After a successful compilation, you will have the libmysqlhll.so binary under the libmysqlhll/ directory.
  
**Windows**
  
Tested on Windows 7 with Visual Studio 2010 proffessional, but should work pretty much the same on most windows machines.  
  
**Prerequisites**: Make sure you have CMake installed.

Run the following from the project's root directory, in a Visual Studio command prompt (open either 32 or 64 bit command prompt, depending on your chosen target CPU):

```
git submodule update --init
cmake .
msbuild mysqlhll.sln /p:Configuration=Release
```
  
**NOTE:** You may need to tell cmake where to find mysql header files. The deault is "C:\Program Files\MySQL\MySQL Server 5.6\include", but if they are located in another directory on your machine, add "-DMYSQL_INCLUDE_DIR={DIR}" to the cmake command line.  

After that, mysqlhll.dll file will be located under libmysqlhll\Release.

  
MySQL Installation
------------------

Once the binary is compiled (.so on linux, .dll on windows), you have to copy it to the MySQL plugins dir.  
If you are not sure where that is, you can check by running this command 
  
`show variables like '%plugin%';`  
  
in mysql client (On linux, it is usually /usr/lib/mysql/plugin/).

After that login to mysql as root, and run [the functions installation script](sql/udf.sql). Replace .so with .dll if you are on windows.


Customization
=============

If you would like to change the bit width of the HyperLogLog algorithm, you can change it by editing the [constants.hpp file](libmysqlhll/constants.hpp) before compiling the project.  
The default is 12, resulting in 2**12(4096) bytes of storage. This can go as high as 16 to get better accuracy with more storage, or you can lower it to save storage.
