Environment {#environmentmanual}
===

The `nawa::Request` object `connection.request` (which can be accessed 
through `connection`, the `nawa::Connection` object you get as a parameter 
of your `handleRequest` function) allows you to access information about 
the request (such as POST, GET, and COOKIE data) and the environment.

## Request Path and environment

The request path is probably the most important information you want to 
access in your application, as it allows you to respond with the correct 
page. The function `nawa::Request::Env::getRequestPath()`, accessible as 
`connection.request.env.getRequestPath()`, returns a vector of strings 
containing all elements of the request URI (without query string).

If the user requested the URI "/dir1/dir2/page", the vector would contain 
the elements {"dir1", "dir2", "page"}.

For an example concerning the usage of the request path, see 
`examples/multipage.cpp`.

The function `connection.request.env.getAcceptLanguages()` 
(`nawa::Request::Env::getAcceptLanguages()`) returns a vector containing 
all languages accepted by the client (as strings in the format used by 
their web browser). This might be useful for internationalization.

Other environment variables can be accessed as strings by using the `[]` 
operator. For example, to access the server hostname, use 
`connection.request.env["host"]`. In the following section, you'll find 
the full list of environment variables that can be accessed this way.

### List of environment variables

- `host`: Server hostname
- `userAgent`: User agent string (of the client's web browser)
- `acceptContentTypes`: Content types accepted by the client
- `acceptCharsets`: Character sets accepted by the client
- `authorization`: HTTP authorization string
- `referer`: Referral URL
- `contentType`: Content type of data from client
- `root`: HTTP root directory
- `scriptName`: Filename of script relative to http root
- `requestMethod`: HTTP request method, one of:
    - ERROR
    - HEAD
    - GET
    - POST
    - PUT
    - DELETE
    - TRACE
    - OPTIONS
    - CONNECT
- `requestUri`: The request URI, including query string
- `serverAddress`: Server IP address
- `remoteAddress`: Client IP address
- `serverPort`: Port of the web server
- `remotePort`: Port used by the client
- `ifModifiedSince`: Unix timestamp sent by the client (to indicate that 
a full response is only necessary if the requested content has been 
modified since this date)

## Accessing GET, POST, and more

Coming soon! Until then:

- in the `examples/contactform.cpp` you can learn how to access post data 
(GET is equivalent), and 
- in the `tests/nawatest.cpp` you'll find some hints about cookie 
processing.