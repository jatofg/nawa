NAWA
===

NAWA is a framework for web applications written in C++(17). It aims to 
make writing native web apps nearly as easy as writing them 
in specialized scripting languages (such as PHP). It provides the 
necessary features, such as sessions, request filters (for providing 
static elements such as images), and accessing and controlling all 
aspects of requests and responses (GET, POST, cookies, ...). Scroll on 
for a list of features.

NAWA communicates with the web server via FastCGI and currently uses 
the fastcgi++ lite library for efficient request handling.

It also comes 
with an integrated development web server (HTTP Request Handler) which can 
serve your application without depending on dedicated web server software
(still somehow experimental and unfit for production use).

## News
**New in v0.8**

The configuration can now be reloaded without restarting and apps (and request handling 
functions) can be hot-swapped. This allows you to update your app without interrupting 
request handling. Reloading can be done via systemd or by sending a SIGHUP signal to 
nawarun. If you are using NAWA as a library, take a look at 
`nawa::RequestHandler::reconfigure()`.

Almost all data members of classes have become methods, therefore, it is necessary to 
update existing apps before upgrading. Please note that the project has been extensively 
restructured, therefore it might also be necessary to correct the includes of existing apps.

Additionally, this release adds support for macOS.

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
    connection.responseStream() << "Hello World!";
    return 0;
}
```

You want to learn more? 
[Read the full introduction tutorial.](https://jatofg.github.io/nawa/v0.8/docs/gettingstarted.html) 

If you don't like the IoC-style approach, you can also use NAWA as a library. 
[An introduction can be found here.](https://jatofg.github.io/nawa/v0.8/docs/aslibrarymanual.html)

### Warning!

This project is still in an early state (version 0.8!). It might, of 
course, still contain bugs. Use it at your own risk.

If you use it, please report any bugs and wishes, so that this project 
might reach a higher version number at some point :)

Also, the ABI and API can still change at any time, making 
modifications to your apps (or, at least, recompilation) necessary. When such 
changes happen, the version number will be increased, and nawarun will refuse to 
load apps compiled against an older version.

## Main Features

* Request Handling Backends ([learn more](https://jatofg.github.io/nawa/v0.8/docs/gettingstarted.html))
  * FastCGI (serve requests through web server software)
  * HTTP (development web server, not production-ready yet)
* Access to request data ([learn more](https://jatofg.github.io/nawa/v0.8/docs/environmentmanual.html))
  * GET, POST, COOKIE variables
  * Request headers
  * Request details (document root, request method, client IP, ...)
  * Environment information (server software, server IP, ...)
* Defining the response ([learn more](https://jatofg.github.io/nawa/v0.8/docs/gettingstarted.html))
  * Set response body directly or using a stream
  * Set response headers
  * Set cookies
  * Default options for cookie security
  * Flush response prematurely
  * Respond with files from disk (incl. automatic setting of relevant response headers)
  * Automatically generate and send error pages
* Sessions ([learn more](https://jatofg.github.io/nawa/v0.8/docs/sessionsmanual.html))
  * Full-featured session management
  * Session security options
  * Start sessions automatically (optional)
  * Set and get session variables of arbitrary types
* Request Filters ([learn more](https://jatofg.github.io/nawa/v0.8/docs/filtersmanual.html))
  * Filter requests based on path, file extensions, or regular expressions
  * Serve filtered requests with files from disk (forward filter)
  * Block filtered requests with error pages
  * Require HTTP authentication for filtered requests
* Email ([learn more](https://jatofg.github.io/nawa/v0.8/docs/emailmanual.html))
  * Create emails with headers
  * MIME email and attachment support
  * Automatic quoted-printable or base64 encoding (if desired)
  * Q-encoding for subjects
  * Replacement rules for personalizing mass emails
  * Sending emails via SMTP
* Cryptographic functions
  * Password hashing (bcrypt, Argon2)
  * Classic hashing (SHA-1, SHA-2, MD5)
* Encoding and decoding functions
  * HTML entities
  * URL encoding
  * Base64, quoted-printable, Q-encoding, encoded-word
  * Punycode for internationalized domain names
* Utility functions
  * Regular expressions with callback
  * Convert strings to uppercase and lowercase
  * Generate error pages
  * Deduct content types
  * HTTP and SMTP time formatting
  * String and UNIX path splitting and merging
  * Line ending conversion
  * Load files into strings
  * Parse headers and cookies from raw HTTP
* Core features
  * Multi-threaded request handling (configurable)
  * Hot-swapping of apps and request handling functions
  * Reload configuration without interrupting request handling

## Example

A NAWA app consists of:

- A simple config file with properties concerning the behavior of the 
application, and
- A shared object file containing the application itself.

You can find an introduction containing a simple "Hello World!" 
NAWA app here: 
[Getting started.](https://jatofg.github.io/nawa/v0.8/docs/gettingstarted.html)

The path to the object file is included in the config file, and NAWA 
will take care of setting everything up and starting the app. You 
can run it like this:

`nawarun /path/to/config.ini` (or just `nawarun` if the file is named 
`config.ini` and located in the current working directory).

Run `nawarun -h` or `nawarun --help` for information on command line 
options (config overrides, starting without config file).

If systemd is present on your system, a systemd service file will 
also be installed on your system. You can start an app as a 
daemon using the path to its ini file (/path/to/config.ini):

`sudo systemctl start nawa@-path-to-config.ini`

For automatically starting an app on boot, use `systemctl enable`.

## Building

NAWA has been tested especially on Linux, but also runs fine 
on macOS (both as a library and using nawarun) *(1)*. It will 
probably also run on other BSD derivates, but this has not 
been tested.

Windows is not supported and never will be.

**Please note:** All commands in the following instructions should be 
run as an unprivileged user. Commands which must run as root are 
prepended with `sudo`.

*(1) ~~Please note that due to a bug in Apple Clang, building will only 
succeed with gcc, which can be installed using homebrew. Other dependencies 
may also be installed using homebrew.~~  Compiling with Apple Clang works fine 
on macOS 12, dependencies still have to be installed from homebrew.*

### Requirements

**For running NAWA apps via nawarun / applications using libnawa:**

* OpenSSL >= 1.1.1
* libcurl4 >= 7.58.0
* libboost-system >= 1.62.0
* libboost-thread >= 1.62.0
* libargon2 >= 0~20161029-1.1 (only if built with Argon2 support)

**For building (additionally):**

* libssl (OpenSSL) development files >= 1.1.1
* (lib)boost, (lib)boost-system, (lib)boost-thread development files >= 1.62
* libcurl4 development files >= 7.58.0
* libargon2 development files >= 0~20161029-1.1  (only if building with Argon2 support)
* git
* cmake >= 3.13
* gcc-c++/g++ or clang compiler with C++17 support

For building the docs, doxygen must be installed. However, you can also 
[access the docs online](https://jatofg.github.io/nawa/v0.8/docs/).

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
* Build the *nawa_static* static library: `-DBuildStaticLib=ON`
* Do not build nawarun: `-DBuildNawarun=OFF`
* Disable building the examples: `-DBuildExamples=OFF`
* Disable building docs: `-DBuildDocs=OFF` (disabled automatically if doxygen is not installed)
* Disable Argon2 support: `-DEnableArgon2=OFF`

Build NAWA (nawarun and the shared library, tests, examples, and docs, depending on your 
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
* punycode: MIT License, see `libs/punycode/LICENSE` for details

It automatically downloads the following libraries during build configuration:
* fastcgilite (also LGPL-licensed)
* cpp-netlib (Boost Software License)