Getting Started {#gettingstarted}
===

This tutorial will explain how to write a simple, working "Hello World!" 
NAWA application.

A NAWA app consists of:
- A config file in INI format with options concerning the NAWA 
environment. Options to set up your own app can be added, too, you can 
access (and alter) the options within your app.
- A shared object file containing the actual app.

## The config file

We will first have a look at the config file. You can find an example in 
the `config.ini` file in the top level of the repo.

The `[fastcgi]` section specifies how the app should communicate with the 
web server. In production environments, this should be done using the 
FastCGI protocol (however, NAWA can also open up a development web server, 
will be explained later). We recommend that you always use a UNIX socket, 
if possible. Set `mode` to `unix` and specify a path where the socket 
should be created in the `path` option. The socket will be created by 
NAWA, just make sure that:

- the path exists,
- the user starting the app has both write and read permissions on the 
  file (if the app is started by root and downgrades its permissions 
  according to the relevant options in the `privileges` section, this 
  doesn't matter, as the socket connection is established before the 
  privilege downgrade happens), and
- the web server has both read and write permissions on the socket 
  file, too.

In case this doesn't work for you, you can use a TCP socket. If the 
machine you're running the app on is connected to the Internet without 
an accordingly configured firewall, you should set the `listen` IP to 
`127.0.0.1`, so that only processes on the same machine can connect 
to the TCP socket. Use another interface on your own risk, there is no 
authentication whatsoever.

If you want to use the integrated development web server, you can 
configure it in the `[http]` section. We recommend that you only change 
the port, if desired, and leave the other settings as they are (the 
development web server should not be exposed to a public network or the 
internet). NAWA will then open a server on the port you've chosen 
(8080 by default), which can be accessed only from the local machine 
for safety reasons. If you want to make your app publicly available, 
please use the FastCGI request handler. Please also note that you have 
to change the `request_handler` setting in the `[system]` section to 
`http` in order to use the development web server instead of FastCGI.

In the next section, `[privileges]`, specify the system user and group 
your app should run at. NAWA will downgrade its privileges and continue 
running as the specified user and group after connecting to the socket. 
This will only work if the app is started as root. If you are not 
starting the app as root, make sure the starting user has all necessary 
permissions to create the socket.

In the `[system]` section, you can configure the request handler. 
If you want to use the integrated development web server instead of 
FastCGI (for testing purposes only), you can set `request_handler = http`.
You can also specify the number of threads used 
for request handling. The default values, `concurrency = hardware` 
and `threads = 1.0`, will make NAWA determine the number of threads 
according to your hardware's parallelism capabilities. If this doesn't 
lead to the expected result on your system, you can use 
`concurrency = fixed` to specify a fixed number of threads. 

The `[application]` section contains the path to the shared object file. 
Make sure the path points to your application file. We will see later 
how to create it.

The following sections, `[session]` and `[crypto]`, contain some options 
affecting the behavior of NAWA. The options are sufficiently explained 
in the config file itself, so we'll skip them for now.

## The application

Now, we'll have a look on how to create a simple NAWA application. In 
the end, you'll have a shared object file (such as `libmyapp.so`) that 
you can refer to in the config file.

To create a NAWA app, you can use any IDE for C++, such as CLion. 
Start by creating a new shared library project (C++14 recommended). 

For our simple example, you just need a single C++ file, which will 
probably be created automatically by your IDE.

For the start, you just need to include a single header file:

```cpp
#include <nawa/Application.h>
```

There are two required functions you need to implement in your app, 
`init` and `handleRequest`.

In the `init` function, you can alter the config (at least the options 
concerning your own app), set up static filters, and do any 
initialization you might want to do. If the return value is not 0, 
NAWA will terminate with an error, this can be useful, e.g., if the 
config file doesn't contain all necessary options needed by your app, 
or if your app encountered another problem during initialization.

The `init` function is run once before request handling starts, and 
request handling will not start until your `init` function has returned. 
So you should refrain from including long-running tasks. You can, 
however, start threads and processes that should run in the background.

However, if you initialize any data structures to use while handling 
requests, please be aware that the invocations of `handleRequest` might 
(and most probably will) run in multiple threads. Consider using 
thread-safe data structures :)

As we just want to create a simple, first example app, we'll ignore this 
function for now and just let this function return 0:

```cpp
int init(nawa::AppInit &appInit) {
    return 0;
}
```

The `handleRequest` function is called once for each request (not 
always in the same thread, of course, if concurrency is used). The 
`Connection` structure contains essentially all properties of the 
request and can be used to specify the response. The return value is 
currently unused, but this may change. Consider returning 0 until then, 
which should be safe.

We'll deal later with the powerful `Connection` object, as for now, 
our goal is just to send a simple, but effective "Hello World!" to 
the browser. Let's go:

```cpp
int handleRequest(nawa::Connection &connection) {
    connection.responseStream() << "Hello World!";
    return 0;
}
```

And that's it! `connection.responseStream()` is an `ostream`, its contents 
will only be used after the response is flushed, either when the 
`handleRequest` function returns, or when you call 
`connection.flushResponse()` manually. Instead of using the stream, you 
could also do `connection.setResponseBody("Hello World!")`.

You can find the full source code of this example in the 
`examples/helloworld.cpp` file.

Now, compile your app (as a library) and insert the correct path into your 
`config.ini` file. If you're using CMake, you can do it like this 
(assuming that nawa, especially its headers, are installed on your 
system):

```cmake
add_library(myapp SHARED myapp.cpp)
```

Do not link your library against NAWA when writing apps this way. On 
macOS, you will have to use a special linker flag to avoid linking 
errors (neither necessary nor working on Linux):

```cmake
set_target_properties(myapp PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
```

## Configuring the web server

If you use the integrated development web server (HTTP request handler), 
you can skip this section, as NAWA will automatically start a web server 
for you.

If you're using FastCGI, you'll also have to configure your web server to 
connect to the FastCGI socket and make your new website accessible in your 
browser. How this works differs depending on the server you use. In Apache2, 
you'll have to enable the `proxy_fcgi` module by running

`a2enmod proxy_fcgi`

and add a line like this to your server configuration, using the 
correct path or URL of your socket (for a vhost or as a global 
directive, e.g., in a file `/etc/apache2/conf-enabled/nawa.conf`):

`ProxyPass "/" "unix:/etc/nawa/sock.d/nawa.sock|fcgi://localhost/" enablereuse=on flushpackets=on`

Or, in case of a TCP socket:

`ProxyPass "/" "fcgi://127.0.0.1:8000/" enablereuse=on flushpackets=on`

After restarting your web server (`systemctl restart apache2`), you 
should be all set for running and accessing your app!

## Run your app!

In a terminal (assuming you're in the directory where the 
`config.ini` file is located), run:

`nawarun`

For more options (providing a path to the config file, 
command-line config overrides, no-config-file mode), run 
`nawarun -h` or `nawarun --help`.

Open your web browser and enjoy!

### Reloading the app

You can send a SIGHUP signal to reload the config. Your app will be reloaded 
as well, allowing you to hot-swap it to a new version without stopping 
request handling. The new app's init function will run before request handling 
switches to the new app.

Please note that reloading only works if the config as well as the application 
file are still accessible after downgrading privileges. The configuration of 
the request handler (especially the `[fastcgi]`, `[http]`, and `[system]` 
sections of the config file) cannot be updated without restart.

### Using systemd

The best way to keep a NAWA app running is to start it as a systemd 
service. That's easy, too, as NAWA already comes with the necessary 
systemd service file. If the path to your config file would be 
`/path/to/config.ini`, you would run (using systemd's escape format):

`systemctl start nawa@-path-to-config.ini`

To start your service automatically on boot:

`systemctl enable nawa@-path-to-config.ini`

To reload configuration and app (see above):

`systemctl reload nawa@-path-to-config.ini`

Of course, you can use `stop` to stop it, `restart` to restart it, and 
`disable` to stop automatically starting it on boot.

## Learn more

Apart from reading the other tutorials, having a look at the examples is a 
good way to explore NAWA's features. 
`examples/multipage.cpp` is a good place to start. 