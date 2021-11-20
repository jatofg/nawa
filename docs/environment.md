Environment {#environmentmanual}
===

The `nawa::Request` object `connection.request()` (which can be accessed 
through `connection`, the `nawa::Connection` object you get as a parameter 
of your `handleRequest` function) allows you to access information about 
the request (such as POST, GET, and COOKIE data) and the environment.

This manual page will also show you how to set cookies, HTTP headers, 
and the HTTP status.

## Request Path and environment

The request path is probably the most important information you want to 
access in your application, as it allows you to respond with the correct 
page. The function `nawa::request::Env::getRequestPath()`, accessible as 
`connection.request().env().getRequestPath()`, returns a vector of strings 
containing all elements of the request URI (without query string).

If the user requested the URI "/dir1/dir2/page", the vector would contain 
the elements {"dir1", "dir2", "page"}.

For an example concerning the usage of the request path, see 
`examples/multipage.cpp`.

Other environment variables and request headers can be accessed as strings 
by using the `[]` operator. For example, to access the server hostname, use 
`connection.request().env()["host"]`. In the following section, you'll find 
the full list of environment variables that can be accessed this way.

### List of environment variables

All of the following environment variables are available in the FastCGI 
request handler. All except for those marked with (\*) are also available 
in the HTTP request handler.

- `DOCUMENT_ROOT`: HTTP root directory (\*)
- `SCRIPT_NAME`: Filename of script relative to http root (\*)
- `REQUEST_METHOD`: HTTP request method, one of:
    - ERROR
    - HEAD
    - GET
    - POST
    - PUT
    - DELETE
    - TRACE
    - OPTIONS
    - CONNECT
- `REQUEST_URI`: The request URI, including query string
- `SERVER_ADDR`: Server IP address
- `REMOTE_ADDR`: Client IP address
- `SERVER_PORT`: Port of the web server
- `REMOTE_PORT`: Port used by the client
- `HTTPS`: String should be "on" when the page has been accessed through 
  a secure (HTTPS) connection, otherwise undefined (most probably empty).
- `SERVER_NAME`: The server's FQDN (fully qualified domain name). (\*)
- `SERVER_SOFTWARE`: The web server software, such as "Apache" or 
  "NAWA Development Web Server".
- `BASE_URL`: The URL of the current request, starting with `http://` or 
  `https://`, and including the hostname (and port, if necessary), but not 
  the request URI.
- `FULL_URL_WITH_QS`: The full URL of the current request, starting with 
  `http://` or `https://`, including the query string.
- `FULL_URL_WITHOUT_QS`: Same as `FULL_URL_WITH_QS`, but without the query 
  string.

When using the FastCGI request handler, additional FastCGI parameters are 
accessible using their original key, unless they start with `HTTP_` (then, they 
are interpreted as HTTP request headers, see next section).

### Availability of request headers

Most HTTP request headers are available as environment 
variables (with lowercase keys), for example:

- `host`: Requested server hostname
- `user-agent`: User agent string (of the client's web browser)
- `accept`: Content types accepted by the client
- `accept-charset`: Character sets accepted by the client
- `accept-language`: Content languages preferred and understood by the client.
- `authorization`: HTTP authorization string
- `referer`: Referral URL
- `content-type`: Content type of data from client
- `if-modified-since`: Unix timestamp sent by the client (to indicate that 
  a full response is only necessary if the requested content has been 
  modified since this date)
  
All request headers are available when using the HTTP request handler. 
When using the FastCGI request handler, all request headers which are 
available as FastCGI parameters with standardized keys (e.g., `HTTP_MY_HEADER` 
for the header `my-header`) are accessible this way. Depending on 
your web server software, you might need to manually map custom headers to 
FastCGI parameters. Refer to your web server's docs for instructions.

## GET, POST, and COOKIE

You can access the GET, POST, and COOKIE variables through the `[]` 
operator of `connection.request().get()`, `connection.request().post()`, and 
`connection.request().cookie()`, respectively. If the variable exists more than 
once (which is possible), the operator will only access one definition 
(usually the first one). Example:

```cpp
std::string firstName = connection.request().post()["first_name"];
```

To find out how many definitions exist for the name "variable", use  
`connection.request().post().count("variable")`.

If there are multiple definitions, you can use 
`connection.request().post().getVector("variable")` to get a vector of strings. 
Alternatively, you can get a multimap of all key-value pairs via 
`connection.request().post().getMultimap()`. You can also iterate through the 
GET, POST, or COOKIE data, for example:

```cpp
for (auto const &e: connection.request().post()) {
    connection.responseStream() << "<p>POST[" << e.first << "] = " << e.second << "</p>";
}
```

To find out if anything has been submitted via POST at all, you can just use 
`connection.request().post()` as a boolean value:

```cpp
if (connection.request().post()) {
    // do something
}
```

All of the above also applies to GET and COOKIE variables. A detailed 
documentation of the methods is available here: `nawa::request::GPC`. 
An example can be found in `examples/contactform.cpp`.

### Special methods for POST data

POST is not limited to key-value pairs, the browser might also send files, 
or something completely else. You can access the content type (as a string) 
via `connection.request().post().getContentType()`.

If files have been submitted (content type `multipart/form-data`), 
you can access them through `connection.request().post().getFile("key")` 
(see `nawa::request::Post::getFile()`). For multiple files with the same name, 
`nawa::request::Post::getFileVector()` can be used, or the full multimap can 
be accessed via `nawa::request::Post::getFileMultimap()`.

Files are represented as `nawa::File` objects, have a look at the documentation 
of that class to see how to retrieve the file and its meta data.

To access the raw POST data as a string, you can use  
`connection.request().post().getRawHttp()` depending on the config value 
`raw_access` in the `[post]` section. By default, the value is 
`nonstandard`, meaning you can access the raw data only if it doesn't 
contain standard content types (key-value pairs or files). You can, 
however, set it to `always`, but keep in mind that this leads to a higher 
memory consumption (as the POST data is always copied). If you don't need 
this feature at all, consider setting it to `never`.

For an example on advanced POST features such as file handling, have a look 
at `tests/gpctest.cpp`.

### Setting cookies

For setting cookies, i.e., sending them with your response and requesting 
the browser to save them and send them back with its requests, there is 
the method `connection.setCookie()` (`nawa::Connection::setCookie()`).

You can use this method in the following way to create a cookie 
*with default attributes* (since v0.2, see below):

```cpp
connection.setCookie("myCookie", "cookie_content");
```

A cookie does not only contain a value, but a lot of attributes. To set 
a cookie with custom attributes, you can create a `nawa::Cookie` object 
and set the attributes accordingly, if desired. 

All attributes, except for the 
actual content string, have default values that are determined by 
the cookie policy, which you can set by creating a `nawa::Cookie` 
object with the desired attributes, and passing it to 
`connection.setCookiePolicy()` (`nawa::Connection::setCookiePolicy()`) -- 
all cookies created for the current response from that point will have 
these default attributes, unless they are overridden.

The default values for the `expires`, `maxAge`, `domain`, `path`, and 
`sameSite` attributes are used only if the corresponding members of the 
`nawa::Cookie` object are not set. For the 
`secure` and `httpOnly` attributes, the strongest value (`true`) wins 
(i.e., these attributes are set if either the actual Cookie object or the 
cookie policy demands this).

After defining your `nawa::Cookie` object, pass it to 
`connection.setCookie()`:

```cpp
nawa::Cookie myCookie("cookie_content");
// set desired attributes ...
connection.setCookie("myCookie", myCookie);
```

**Please note:** This method won't have any effect after the response 
has been flushed at least once (using `connection.flushResponse()`).

#### Unsetting cookies

You can remove a cookie from the cookie map, so it won't be sent with 
your request, by using `connection.unsetCookie("key")` 
(`nawa::Connection::unsetCookie()`). Please note that this won't remove 
a cookie from the user's browser, it will just undo the `setCookie` 
operation (only works before flushing the response, of course).

Actually removing a cookie from the visitor's browser is not specified in 
HTTP. To delete the contents of a cookie, set the cookie to an empty string. 
You can also try to set the expiry date to a date in the past, which might 
delete the cookie in some browsers.

## Setting HTTP status and response headers

To set the HTTP status, e.g. to 404 ("Not found"), use  
`connection.setStatus(uint status)` (`nawa::Connection::setStatus()`).

HTTP response headers can be set via `connection.setHeader(string key, string value)` 
(`nawa::Connection::setHeader()`), and unset via 
`connection.unsetHeader(string key)` (`nawa::Connection::unsetHeader()`).

Both methods can only be used before flushing the response.