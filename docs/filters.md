Filters and Sending Files {#filtersmanual}
===

Usually, a website also contains static elements such as images, templates, 
stylesheets, and so on. The ForwardFilter feature of NAWA can deal with 
this (and send static elements automatically) and keep this out of your 
main application logic (in `handleRequest`).

Additionally, BlockFilters can block access to certain paths and 
AuthFilters authenticate access.

Filters are defined in the `init` function of your app.

## Enable filtering

In your `int init(AppInit &appInit)` function, you can use the 
`nawa::AppInit` object to enable filtering and to define filters.

Enable static filtering by setting:

```cpp
appInit.accessFilters.filtersEnabled = true;
```

## Filter conditions

There are three criteria that can be used for filtering. A filter matches 
if and only if **all** of the defined conditions match. A filter without 
any conditions matches all requests.

- Request path: You define a path. Only requests under this path match.
- File extension: Only files with the given file extension match.
- Regex: Only URIs matched by a regular expression match.

As an example, we use a `ForwardFilter`, but `BlockFilter`s and 
`AuthFilter`s work equally.

```cpp
nawa::ForwardFilter myFilter;
```

To filter only URIs starting with `/static/images`:

```cpp
myFilter.pathFilter = {"static", "images"};
```

To filter only files with file extension `.jpeg`:

```cpp
myFilter.extensionFilter = "jpeg";
```

To filter basing on a regular expression, you have to enable the regex 
filter explicitly, and then assign a regular expression:

```cpp
myFilter.regexFilterEnabled = true;
myFilter.regexFilter.assign(R"(/test(/images)?(/[A-Za-z0-9_\-]*\.?[A-Za-z]{2,4})?)");
```

The above example would match everything that is not in `/test`, 
`/test/images`, and some more things. Please note that regex filters are 
expensive (in CPU cycles) and create a lot of overhead on every request. 
Use them only if your goal cannot be achieved without.

You can also invert your filter, i.e., the filter matches when **none** of 
your conditions matches. An inverted filter without conditions matches no 
requests.

```cpp
myFilter.invert = true;
```

## Forward filters

A `nawa::ForwardFilter` maps all requests with a matching URI to files on 
the file system and send these files to the user. The `basePath` is the 
directory where the files are located. With `basePathExtension`, you 
specify if the URI should be attached to the base path (`BY_PATH`, e.g., 
the request URI `/static/images/image.jpeg` would be mapped to 
`{basePath}/static/images/image.jpeg`), or only the filename (`BY_FILENAME`, 
default, e.g., the request URI `/static/images/image.jpeg` would be mapped 
to `{basePath}/image.jpeg`).

Example for a forward filter mapping to :

```cpp
nawa::ForwardFilter imageFilter;
imageFilter.pathFilter = {"static", "images"};
imageFilter.extensionFilter = "png";
imageFilter.basePath = "/var/www/multipage/images";
imageFilter.basePathExtension = nawa::ForwardFilter::BY_FILENAME; // could be skipped, default option anyway
appInit.accessFilters.forwardFilters.push_back(imageFilter); // add the filter to appInit
```

To become active, the filter has to be added to the corresponding 
vector in the `nawa::AppInit` object (see last line of the example).

## Block filters

A `nawa::BlockFilter` responds to all requests with matching URIs with 
a standardized error page. This can be useful to respond with a 
"404 Not Found" to all URIs outside of the scope of your app, for example. 
The `status` attribute can be used to specify the HTTP status code that 
should be used (the message on the error page will be adapted to it).

Example for a simple BlockFilter that blocks every request outside 
of `/app`:

```cpp
nawa::BlockFilter blockFilter;
blockFilter.pathFilter = {"app"};
blockFilter.invert = true;
blockFilter.status = 404;
appInit.accessFilters.blockFilters.push_back(blockFilter);
```

## Auth filters

A `nawa::AuthFilter` requests an HTTP basic authentication for every 
matching request. If the authentication is not successful, an error 
page (status code 403) will be sent.

The `authFunction` is a function that determines whether the auth was 
successful. The `authName` is the description of the auth that might be 
shown to the users by their web browser. If `useSessions` is true, NAWA 
will create a session variable containing the user name. See 
`nawa::AuthFilter` for details.

Example for an auth filter that authenticates everything under `/secret` 
and requires the user to provide the username "user" and the password 
"super_secret":

```cpp
nawa::AuthFilter authFilter;
authFilter.pathFilter = {"secret"};
authFilter.authName = "Top secret area!";
authFilter.authFunction = [](std::string user, std::string password) -> bool {
    return (user == "user" && password == "super_secret");
};
appInit.accessFilters.authFilters.push_back(authFilter);
```

For a working example with static filters, see `examples/multipage.cpp`.

## Manually sending files

To respond to a request with a file from disk manually, you can use the 
`connection.sendFile()` function, have a look at: 
`nawa::Connection::sendFile()`