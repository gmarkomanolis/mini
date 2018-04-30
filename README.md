MinI
====

MINI is a tracing library that provides minimal instrumentation of MPI applications. Its purpose is to minimize the instrumentation overhead and provide the executed instructions for all the computation phases and detailed communication pattern between the MPI processes.

Download
--------

git clone git://github.com/gmarkomanolis/mini.git


Compiling
---------

The compiling of the MinI tool is simple.

You can use the bash script:
* ./compile.sh

or

* gcc -shared -fPIC -c mini.c
* gcc -fPIC -shared -o libmini.so mini.o


* Copy the libmini.so file in the folder that you want this library to be located


Link with Mini
---------------

Use the following libraries

-lpapi -lmini -lmpi_f77 -lmpi

Be sure that the $LD_LIBRARY_PATH is updated with all the mentioned libraries

For example you can compile NAS benchmarks by adding the decaring the FMPI_LIB variable in make.def file as follows

FMPI_LIB = -L/path/to/mini/library -lmini -lpapi -lmpi_f77 -lmpi


Execution
---------

First of all in order to execute the instrumented application initially you shoud create the following folder (on each participated node or only once if you use a global file system) in the same path as the one the binary is located:

* mkdir ti_traces

You should declare to your .bashrc file which PAPI metric you like to measure

* export MINI_METRIC=PAPI_TOT_INS

and declare that we do not want to measure the time (no calibration):

* export MINI_TIME=0

Use MINI_TIME=1 if you would like Time-Independent traces with the durations of the computaiton phases.

Now you can execute your applications and the traces will be located in the folder ti_traces.


The same information are here: https://github.com/gmarkomanolis/mini/wiki

Changelog
---------


15/08/2013
Version 1.0 released


Contact
-------
For questions, bugs, problems contact at george@markomanolis.com
