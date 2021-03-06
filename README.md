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
**v0.6 is here!**
A brand-new HTTP request handler can open up a development web server, allowing you 
to test your app directly in your browser without setting up a local web server and 
communicating via FastCGI. In addition, you can now build NAWA without Argon2 support 
and accessing request headers has become easier and consistent.

Current (known) limitations of the HTTP request handler:
* Some HTTP status codes might be unsupported
* Flushes may be out of order
* `nawa::Request::Env::getAcceptLanguages()` does not work yet (you can access the 
  HTTP header directly instead)

**Attention!** All environment variables have been renamed in v0.6. Please make 
sure to adapt your apps properly 
(see [Environment Docs](https://www.tobiasflaig.eu/nawa/0.6/docs/environmentmanual.html)).

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
[Read the full introduction tutorial.](https://www.tobiasflaig.eu/nawa/0.6/docs/gettingstarted.html) 

If you don't like the IoC-style approach, you can also use NAWA as a library. 
[An introduction can be found here.](https://www.tobiasflaig.eu/nawa/0.6/docs/aslibrarymanual.html)

### Warning!

This project is still in an early state (version 0.6!). It might, of 
course, still contain bugs. Use it at your own risk.

If you use it, please report any bugs and wishes, so that this project 
might reach a higher version number at some point :)

Also, the ABI and API can still change at any time, making 
modifications to your apps (or, at least, recompilation) necessary. When such 
changes happen, the version number will be increased, and nawarun will refuse to 
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
NAWA app here: 
[Getting started.](https://www.tobiasflaig.eu/nawa/0.6/docs/gettingstarted.html)

The path to the object file is included in the config file, and NAWA 
will take care of setting everything up and starting the app. You 
can run it like this:

`nawarun config.ini`

If systemd is present on your system, a systemd service file will 
also be installed on your system. You can start an app as a 
daemon using the path to its ini file (/path/to/config.ini):

`sudo systemctl start nawa@-path-to-config.ini`

For automatically starting an app on boot, use `systemctl enable`.

## Building

NAWA has been tested on Linux only so far, but it might also run on BSD derivates 
and macOS. 
Windows is not supported and will never be.

**Please note:** All commands in the following instructions should be 
run as an unprivileged user. Commands which must run as root are 
prepended with `sudo`.

### Requirements

**For running NAWA apps via nawarun / applications using libnawa:**

* OpenSSL >= 1.1.1
* libcurl4 >= 7.58.0
* libboost-system >= 1.67
* libboost-thread >= 1.67
* libargon2 >= 0~20161029-1.1 (only if built with Argon2 support)

**For building (additionally):**

* libssl (OpenSSL) development files >= 1.1.1
* (lib)boost, (lib)boost-system, (lib)boost-thread development files >= 1.67
* libcurl4 development files >= 7.58.0
* libargon2 development files >= 0~20161029-1.1  (only if building with Argon2 support)
* git
* cmake >= 3.13
* gcc-c++/g++ or clang compiler with C++17 support

For building the docs, doxygen must be installed. However, you can also 
[access the docs online](https://www.tobiasflaig.eu/nawa/0.6/docs/).

### Clone, build, and install NAWA

To build nawarun, pull the source code, open a terminal, and 
switch to the NAWA directory. If you haven't pulled the source code 
yet, do it:

`git clone https://github.com/jatofg/nawa.git nawa`

`cd nawa`

First, create a directory for the build files:

`mkdir build`

`cd build`

Create the build configuration:

`cmake -DCMAKE_BUILD_TYPE=RELEASE ..`

**Note: You can pass some arguments to cmake to influence the build configuration:** 
* Build the tests as well: `-DBuildTests=ON`
* Build NAWA as shared library: `-DBuildSharedLib=ON`
* Disable building the examples: `-DBuildExamples=OFF`
* Disable building docs: `-DBuildDocs=OFF` (disabled automatically if doxygen is not installed)
* Disable Argon2 support: `-DEnableArgon2=OFF`

Build nawarun (and tests, the shared library, examples, docs, depending on your 
cmake arguments):

`make`

And last, install it to the target directories so that the apps 
can find it (this will also install the headers needed for app development):

`sudo make install`

### Uninstalling

In the build directory, run:

`sudo xargs rm < install_manifest.txt`

Uninstalling before installing a new version is recommended to avoid ancient 
relics remaining on your system.

## Binary packages

Binary packages are not available yet, just build it yourself :)

## Third-party licenses

This project includes the following libraries (in the libs directory) 
with different (less strict) licenses:
* base64: see `libs/base64/LICENSE` for details
* inih: New BSD License, see `libs/inih/LICENSE.txt` for details
* libbcrypt: CC0, see `libs/libbcrypt/COPYING` for details

It automatically downloads the following libraries during build configuration:
* fastcgilite (also LGPL-licensed)
* cpp-netlib (Boost Software License)