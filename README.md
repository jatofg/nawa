NAWA
===

NAWA is a framework for web applications written in C++(17). It aims to 
make writing a natively running web app nearly as easy as writing it 
in a specialized scripting language (such as PHP). It provides the 
necessary features, such as sessions, static filters (for providing 
static elements such as images), and accessing and controlling all 
aspects of requests and responses (GET, POST, cookies, ...).

NAWA communicates with the web server via FastCGI and currently uses 
the fastcgi++ lite library for efficient request handling.

## News
2020-07-31: v0.5 is now available! Apart from various bug fixes, it uses an entirely 
new, modular request handler which paves the way for planned new features 
(direct use of HTTP instead of FastCGI, hot swapping of apps, multiple 
request handling functions). Building and development is now easier, as the FastCGI 
library does not need to be built separately anymore, and its headers are not required 
anymore for app development. CMake 3.11 is now required.

## Example

The following example is the complete C++ source code of a NAWA app that 
answers every request with the "Hello World!". It's that simple.

```cpp
#include <nawa/Application.h>

using namespace nawa;

int init(AppInit &appInit) {
    return 0;
}

int handleRequest(Connection &connection) {
    connection.response << "Hello World!";
    return 0;
}
```

You want to learn more? 
[Read the full introduction tutorial.](https://www.tobiasflaig.eu/nawa/docs/gettingstarted.html) 

### Warning!

This project is still in an early state (version 0.5!). It might, of 
course, still contain bugs. Use it on your own risk.

If you use it, please report any bugs and wishes, so that this project 
might reach a higher version number at some point :)

Also, the ABI and API can still change at any time, making 
modifications to your apps (or, at least, recompilation) necessary. When such 
changes happen, the version number will be increased, so that NAWA will refuse to 
load apps compiled against an older version.

## Features (selection)

- Receive GET, POST, COOKIE, and environment data (such as headers) 
from the request
- Get the POST data in the format you want it: get key-value pairs, 
receive and store submitted files, and/or access the raw POST data
- Set status, headers, and cookies for the response
- Set defaults, for example for cookie parameters, by using the 
configuration engine
- Read and store configuration files in the .ini format
- Set the response body, either at once or using C++ streams
- Flush the response before it is complete (given that this is 
supported by your web server)
- Session management: NAWA will manage session cookies by itself, if 
you want, and allows you to store and receive session-specific data 
without having to care about session management yourself
- Cryptographic functions and other useful functions to make your life 
easier are included (e.g., SHA-1/2 hasing, password hashing, hex 
encoding, base64 encoding/decoding, date/time format conversions, 
default page generation, content type guessing, string splitting)
- Support for sending emails through SMTP.
- Set up request filters to forward files and block or authenticate 
requests.
- Request handling may take place using multiple threads to exploit  
multi-core CPUs.

## Example

A NAWA app consists of:

- A simple config file with properties concerning the behavior of the 
application, and
- A shared object file containing the application itself.

You can find an introduction containing a simple "Hello World!" 
NAWA app here:<br>
https://www.tobiasflaig.eu/nawa/docs/gettingstarted.html

The path to the object file is included in the config file, and NAWA 
will take care of setting everything up and starting the app. You 
can run it like this:

`nawarun config.ini`

If systemd is present on your system, a systemd service file will 
also be installed on your system. You can start an app as a 
daemon using the path to its ini file (/path/to/config.ini):

`sudo systemctl start nawa@-path-to-config.ini`

For automatically starting an app on boot, use `systemctl enable`.

### Using Docker

You can also use the Docker images available in the GitHub Package 
Registry. As they are built automatically, they should always be 
up to date. See the [packages](https://github.com/jatofg/nawa/packages) 
section to learn how to install them, and [the docs](https://www.tobiasflaig.eu/nawa/docs/usingdocker.html) 
for using and building them.

To install them, you need a GitHub account and access 
token (click "Learn more" in the Packages section for more 
information). After logging in with docker, use the command on 
the package page to pull the image.

## Building

NAWA has been tested on Linux only so far, but it might also run on BSD derivates 
and macOS. 
Windows is not supported and will never be.

**Please note:** All commands in the following instructions should be 
run as an unprivileged user. Commands which must run as root are 
prepended with `sudo`.

### Requirements

The requirements marked with * are also required for running nawa, the 
others for building only.

- libssl-dev >= 1.1.1 (OpenSSL* is probably already present on your 
system)
- libboost-dev >= 1.65.1.0
- libcurl4*, libcurl4-openssl-dev >= 7.58.0
- libargon2*, libargon2-0-dev >= 0~20161029-1.1

Git, CMake, gcc, and other basic tools for building software 
are required, too.

For building the docs, doxygen must be installed. However, you can also 
[access the docs online](https://www.tobiasflaig.eu/nawa/docs/).

### Clone, build, and install NAWA

To build nawarun, pull the source code, open a terminal, and 
switch to the NAWA directory. If you haven't pulled the source code 
yet, do it:

`git clone https://github.com/jatofg/nawa.git nawa`

`cd nawa`

First, create a directory for the build files:

`mkdir build`

`cd build`

Then, build NAWA:

`cmake -DCMAKE_BUILD_TYPE=RELEASE ..`

`make`

**Note: If you want to build the tests as well, pass the `-DBuildTests=ON` argument 
to cmake.** You can disable building the examples and docs using the arguments 
`-DBuildExamples=OFF` resp. `-DBuildDocs=OFF`.

And last, install it to the target directories so that the apps 
can find it (this will also install the headers needed for app development):

`sudo make install`

### Uninstalling

In the build directory, run:

`sudo xargs rm < install_manifest.txt`

## Binary packages

Binary packages for Debian and Ubuntu might be provided in future, 
as soon as this project reaches a more stable state. Currently, there 
are not even releases available.

## Third-party licenses

This project includes the following libraries (in the libs directory) 
with different (less strict) licenses:
- base64: see `libs/base64/LICENSE` for details
- inih: New BSD License, see `libs/inih/LICENSE.txt` for details
- libbcrypt: CC0, see `libs/libbcrypt/COPYING` for details