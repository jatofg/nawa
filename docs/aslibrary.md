NAWA as a library {#aslibrarymanual}
===

If you don't like the IoC-style approach of NAWA (using nawarun and applications 
as shared libraries), you can also use NAWA's new request handler as a library 
(since v0.5). This way, you have full control over your executable and still create 
a web app in a very easy way (even if not *as* easy as with nawarun).

Basically, you will have to provide a `nawa::Config` object (created manually or 
from an `.ini` file) and a function for handling requests (which will be called when 
a new request arrives). Access filters can optionally be provided as well.

Of course, your application will have to take care of things like privilege 
management itself (e.g., downgrade its privileges if started as root to enhance 
security).

In your request handling function itself, of course, you can use all features 
of NAWA just like in "classic" NAWA apps.

A working "Hello World" example with comments explaining everything you need to 
know to get started, can be found in `examples/aslibrary.cpp`.