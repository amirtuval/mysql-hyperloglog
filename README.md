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
In its default implementation, the plugin uses hyperloglog with a 12 bit width, resulting in a 4096 bytes vector. Base64 string of that is ~5500 chars long.
  
[Here's a more detailed example](sql/example.sql).

Installation
============

Compilation
-----------

The project uses [CMake](http://www.cmake.org/) as a build tool, and the code is platform independent, so it should compile fine on most platforms (linux, windows, mac).  

**TODO:** Add specific instructions for the major platforms.

MySQL Installation
------------------

Once the binary is compiled (.so on linux, .dll on windows), you have to copy it to the MySQL plugins dir.  
If you are not sure where that is, you can check by running this command 
  
`show variables like '%plugin%';`  
  
in mysql client (On linux, it is usually /usr/lib/mysql/plugin/).

After that login to mysql as root, and run [the functions installation script](sql/udf.sql). Replace .so with .dll if you are on windows.



