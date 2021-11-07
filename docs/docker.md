Using Docker {#usingdocker}
===

**Please note that the Docker images are currently mostly untested
and only used as a CI. They are no longer available from the GitHub 
package repository.**

**This manual page is no longer actively maintained.**

~~Docker images are built automatically by GitHub and are available 
[here](https://github.com/jatofg/nawa/packages).~~

~~To install them, you need a GitHub account and access 
token (click "Learn more" on the package page for more 
information). After logging in with docker, use the command on 
the package page to pull the image.~~

To try out NAWA with the Hello World example app, you can start a 
container like this:

`docker run -p 127.0.0.1:8000:8000 docker.pkg.github.com/jatofg/nawa/nawa`

The IP address after "-p" specifies the interface for Docker to publish the 
external port (127.0.0.1 means that Docker, and therefore NAWA, 
will only accept FastCGI connections from localhost). You should 
always set the interface IP address, as you don't want that the whole 
world is able to connect to your NAWA app via FastCGI -- only the 
web server should be able to do that. The port between the colons 
is the external listening port on your machine -- connect to this 
port via FastCGI to access the app (you can choose any port). 
The third element is the internal port in the container, as used by 
NAWA. This port must be 8000, unless you change it in the config file.

Next, set up your web server to connect to NAWA via FastCGI on 
the external listening port, e.g., 8000. This works exactly like 
without Docker. To learn how to do that, 
have a look to the [Getting Started](@ref gettingstarted) tutorial.

Then, you should be able to admire the "Hello World!" message in 
your web browser.

## Using your own app
In the [Getting Started](@ref gettingstarted) tutorial, you can 
learn how to write your first own app with NAWA. 

However, there 
are certain options in the config file that should be set in a 
certain way so they'll work with the Docker image, so you should 
copy the `[fastcgi]` and `[privileges]` section from the file 
`docker/nawa/config.ini` (in the source code/GitHub repo) or from 
here: 

```ini
[fastcgi]
; Docker has to communicate with the host via TCP
mode = tcp
; Listen to all interfaces, so the port can be accessed by the outer world
listen = all
; Internal port to use with the -p option of Docker
port = 8000
; Irrelevant options for Docker
path = /nawa/nawa.sock
permissions =
owner = nawa
group = nawa
; SO_REUSEADDR not necessary for Docker
reuseaddr = off

[privileges]
; user NAWA should run as (can be either nawa or root in Docker)
user = nawa
; group NAWA should run as (can be either nawa or root in Docker)
group = nawa
```

Also, 
please note that the your directory containing the app has to be 
addressed as `/nawa`, so the application section might be:

```ini
[application]
; Path to application to load
path = /nawa/myapp.so
```

To use your app in the 
NAWA docker container, just create a directory with the config file 
(which must be named "config.ini" for use with Docker), your actual 
app (the `.so` file), and all assets your app needs. Then, mount 
this directory as `/nawa` in your container:

`docker run -p 127.0.0.1:8000:8000 -v /path/to/my/nawaApp:/nawa docker.pkg.github.com/jatofg/nawa/nawa`

## Building the Docker image

If your app uses external libraries, you might need to modify 
the Dockerfile and add all libraries you need for your app to work 
to the `apt-get install` section.

You could also modify the image contents to include your actual 
app instead of the "Hello World" example.

Then, you can build the image simply by switching to the top level 
directory of the NAWA sources and running (change the tag as you 
like):

`docker build -t mynawaimage .`