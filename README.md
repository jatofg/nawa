NAWA
===

NAWA is a framework for web applications written in C++(14). It aims to 
make writing a natively running web app nearly as easy as writing it 
in a specialized scripting language (such as PHP). It provides the 
necessary features, such as sessions, static filters (for providing 
static elements such as images), and accessing and controlling all 
aspects of requests and responses (GET, POST, cookies, ...).

NAWA communicates with the web server via FastCGI and currently uses 
the fastcgi++ lite library for efficient request handling.

You want to learn more? 
[Have a look at the manual](https://www.tobiasflaig.eu/nawa/docs/gettingstarted.html) 
to see how easy it is.

### Warning!

This project is in a very early state (version 0.1!). It might contain 
serious bugs that could compromise the security of your system, put 
your house on fire, or kill your cat (nooo, not the cat!). So use it 
on your own risk, and better don't use it on a production system 
yet. But you SHOULD test it (e.g., on a VM or container), report 
any bugs, and help to make this project more stable! :)

Also, it means that the ABI and API can change at any time, making 
modifications to your apps (or, at least, recompilation) necessary.

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

## Building

NAWA has been tested on Linux only so far, but it might also run on BSD derivates and macOS. 
Windows is not supported and will never be. 

### Requirements

The requirements marked with * are also required for running nawa, the 
others for building only.

- libfastcgi++* (lite) (see next subsection)
- libssl-dev >= 1.1.1 (OpenSSL* is probably already present on your 
system)
- libboost-dev >= 1.65.1.0
- libcurl4*, libcurl4-openssl-dev >= 7.58.0
- libargon2*, libargon2-0-dev >= 0~20161029-1.1

Git, CMake, gcc, and other basic tools for building software 
are required, too.

For building the docs, doxygen must be installed. However, you can also 
[access the docs online](https://www.tobiasflaig.eu/nawa/docs/).

### Clone, build, and install fastcgi++

Run the following commands to build, run, and install fastcgi++ lite 
(which is an optimized version of the fastcgi++ library by eddic). The 
original library will work, too, with a few caveats (such as an 
inconsistent logging style).

`git clone https://github.com/jatofg/fastcgipp.git fastcgipp`

`mkdir fastcgipp/build`

`cd fastcgipp/build`

`cmake -DCMAKE_BUILD_TYPE=RELEASE ..`

`make`

Note: If you want to build the fastcgi++ docs, too, run 
`make doc` now.

`sudo make install`

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

And last, install it to the target directories so that the apps 
can find it:

`sudo make install`

## Binary packages

Binary packages for Debian and Ubuntu might be provided in the future, 
as soon as this project reaches a more stable state. Currently, there 
are not even releases available.