soru
===

soru is a framework for web applications written in C++(14). It aims to 
make writing a natively running web app nearly as easy as writing it 
in, for example, PHP. soru communicates with the web server via 
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
- Session management: soru will manage session cookies by itself, if 
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

A soru app consists of:

- A simple config file with properties concerning the behavior of the 
application, and
- A shared object file containing the application itself.

For a simple soru app:
[TBA]

The path to the object file is included in the config file, and QSF 
will take care of setting everything up and starting the app. You 
can run it like this:

`$ sorun config.ini`

Of course, it is also possible to start it as a systemd service.

## Building

soru has been tested on Linux only so far, but it might also run on BSD derivates and macOS. 
Windows is not supported and will never be. 
Before building, make sure that you have the dependencies installed:

- libfastcgi++ (pull from eddic/fastcgipp)
- libssl-dev (should be available in your distro)
- libboost-dev < 67
- libcurl4-openssl-dev
- libargon2-0-dev 

For actually running applications with sorun, only libfastcgi++ 
is required (and the others have to be installed, too...)

To build qsfrunner, pull the source code, open a terminal, and 
switch to the soru directory.

First, create a directory for the build files:

`$ mkdir build`<br>
`$ cd build`

Then, build soru:

`$ cmake --build ..`

And last, install it to the target directories so that the apps 
can find it:

`$ sudo make install`

## Binary packages

Debian and Ubuntu repositories will soon by available here: [TBA]
