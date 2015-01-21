# HBM Device Scan (devscan) Client Examples
Implements an HBM Device Scan Client and some simple example programs using C++.

The Device Scan interface allows automatic discovery and monitoring reachability of devices connected to the network.

* Build Status: [![Build Status](https://travis-ci.org/HBM/devscan.svg?branch=master)](https://travis-ci.org/HBM/devscan)


## License
Copyright (c) 2014 Hottinger Baldwin Messtechnik. See the LICENSE file for license rights and limitations.


## Prerequisites

### Sources 
All sources of the example programs are to be found on the [HBM Device Scan Examples github project page](https://github.com/HBM/devscan "")

### Used Additional Sources
Some common source code from hbm is being used. It can be found on [HBM common code github project page](https://github.com/HBM "")

### Used Libraries
We try to use as much existing and proofed software as possible in order to keep implementation and testing effort as low as possible. All libraries used carry a generous license. See the licenses for details.

The open source project jsoncpp is being used as JSON composer and parser.

For Linux, simply install the jsoncpp development package of your distribution. For Windows, download the source from the [jsoncpp github project page](https://github.com/open-source-parsers/jsoncpp "") and put them beside the directory of this project.

In addition, we do use the Boost libraries. Refer to [boost](http://www.boost.org/ "") for details.
For Linux, simply install the Boost development packages of your distribution. For Windows, the projects are tailored to link against the prebuilt boost binaries from [boost](http://www.boost.org/ "").
Download and install the binaries and set the '`BOOST_ROOT`' environment variable to the installation directory.


### Build System
#### Linux
Under Linux the cmake build system is being used. Install it using your distribution package system. Create a sub directory inside the project directory. change into this subdirectory and call '`cmake ..`'. Execute '`make`' afterwards to build all libraries and executables.
Tested with gcc 4.8.2

```
git clone https://github.com/HBM/devscan.git

cd devscan/devscan/
cmake .
make
```

#### Windows
A solution for MSVC2010 is provided.


## Client Code
All code relevant for the client side is to be found here.
### Library
Code used by all example programs is put together to a library. It is responsible to receive HBM scan announcements, it notifies about relevant events and allows sending configuration requests.

### Tools
There are some example tools that make use of the Scan Client library.
## Download and Build


## Documentation

Doxygen http://www.doxygen.org/ is needed to create the documentation in html format.

In directory `devscan/devscan/` call
```
doxygen
```
Find the starting page of the documentation in client/doc/doxygen/html/index.html 

A generated version of the doxygen documentation is available on the `gh_pages` branch and can be accessed online:
* http://hbm.github.io/devscan/
* http://hbm.github.io/devscan/devscan/client/doc/doxygen/html/
