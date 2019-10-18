nawa
===

nawa is a framework for web applications written in C++(14). It aims to 
make writing a natively running web app nearly as easy as writing it 
in, for example, PHP. nawa communicates with the web server via 
FastCGI and currently uses the eddic/fastcgipp library for request 
handling.

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
- Session management: nawa will manage session cookies by itself, if 
you want, and allows you to store and receive session-specific data 
without having to care about session management yourself
- Cryptographic functions and other useful functions to make your life 
easier are included (e.g., SHA-1/2 hasing, password hashing, hex 
encoding, base64 encoding/decoding, date/time format conversions, 
default page generation, content type guessing, string splitting)
- Set up request filters to forward files and block or authenticate 
requests.
- Request handling may take place using multiple threads exploit  
multi-core CPUs.

More features will come. For the full documentation, have a look at:
[TBA]

## Example

A nawa app consists of:

- A simple config file with properties concerning the behavior of the 
application, and
- A shared object file containing the application itself.

For a simple nawa app:
[TBA]

The path to the object file is included in the config file, and NAWA 
will take care of setting everything up and starting the app. You 
can run it like this:

`$ nawarun config.ini`

If systemd is present on your system, a systemd service file will 
also be installed on your system. You can start an app as a 
daemon using the path to its ini file (/path/to/config.ini):

`# systemctl start nawa@-path-to-config.ini`

For automatically starting an app on boot, use `systemctl enable`.

## Building

nawa has been tested on Linux only so far, but it might also run on BSD derivates and macOS. 
Windows is not supported and will never be. 
Before building, make sure that you have the dependencies installed:

- libfastcgi++ (pull from eddic/fastcgipp)
- libssl-dev (should be available in your distro) >= 1.1.1
- libboost-dev >= 1.65.1.0
- libcurl4-openssl-dev >= 7.58.0
- libargon2-0-dev >= 0~20161029-1.1

The documentation will be built if doxygen is installed.

For actually running applications with nawarun, libfastcgi++, libcurl4 
(>= 7.58.0), and libargon2 (>= 0~20161029-1.1) have to be installed.

To build nawarun, pull the source code, open a terminal, and 
switch to the nawa directory.

First, create a directory for the build files:

`$ mkdir build`<br>
`$ cd build`

Then, build nawa:

`$ cmake --build ..`

And last, install it to the target directories so that the apps 
can find it:

`$ sudo make install`

## Binary packages

Debian and Ubuntu repositories will soon by available here: [TBA]
