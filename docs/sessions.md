Sessions {#sessionsmanual}
===

NAWA has a comfortable session system that allows you to save arbitrary 
objects. The visitors of your website are identified by using a cookie, 
and whenever one visitor requests a page, all their session variables 
are available. Therefore, the NAWA session system is similar to the 
one of PHP.

Of course, the session system is thread-safe.

## Configuration

The relevant config options are in the `[session]` section of the config 
file. The values are explained in the comments of the sample config file.

The most relevant options are:

- `cookie_secure`: When on, sessions will only work over secure 
connections (HTTPS), as the browser will be instructed not to send 
the session cookie over unencrypted connections.
- `cookie_expires`: When on, cookies will have an explicit expiry 
date (`keepalive` seconds after the last request), i.e., sessions 
do not necessarily end when the visitors close their browser. 
This might make a "cookie banner" necessary for complying with the GDPR.
- `autostart`: When on, you do not have to call 
`connection.session.start();` to start a session -- sessions will be 
created automatically, and session cookies sent with every request.

## Using sessions

Unless `autostart` is enabled, you have to start the session explicitly 
before being able to access session variables. `connection` refers to 
the `nawa::Connection` object you get as a parameter of your 
`handleRequest` function throughout this tutorial.

`connection.session.start();`

You can use `connection.session.established()` to check whether a 
session has already been started.

### Setting variables

Use the `nawa::Session::set()` function to set a variable. It takes 
two arguments: the key (a string), and the value (an arbitrary object).

Examples:

```cpp
connection.session.set("myInt", 5);
connection.session.set("myString", "Hello World!"); // see *
connection.session.set("myObject", obj);
```

\* Please note that "Hello World!" in this example is actually a 
`const char*`. As it doesn't make sense to store pointers in session 
variables (seriously, it doesn't, and will lead to segmentation faults 
-- better don't use pointers at all), and the session system is clever, 
it will be converted to a `std::string` automatically.

**Important!** As C++ is a statically typed language, you will have 
to state the exact type of the object when retrieving it. If the type 
of a literal is not obvious (e.g., when dealing with numeric types), 
use a cast to save the correct type.

### Accessing variables

To access a session variable with a certain key, use the `[]` operator. 
It will return an object of type `std::any`. Using this object, 
you can retrieve the original stored object by using `std::any_cast`:

```cpp
int mySessionInt = std::any_cast<int>(connection.session["myInt"]);
std::string mySessionString = std::any_cast<std::string>(connection.session["myString"]);
```

To just check whether a variable exists, you can use 
`connection.session.isSet("variable")` or 
`connection.session["variable"].has_value()`.

**Please note:** When trying to access a non-existent variable using 
the `[]` operator, a `std::any` object without value will be returned. 
Casting it will throw an exception of type `std::bad_any_cast`. 
Casting to the wrong type will throw 
this exception, too.

### Unsetting variables

To unset a session variable, use `connection.session.unset("variable")`.

### Terminating a session

You can use `connection.session.invalidate()` to delete the current 
session along with its data, see `nawa::Session::invalidate()`.

## Learn more

Read the docs of `nawa::Session` and 
have a look at the `tests/sessiontest.cpp` to see a working example.