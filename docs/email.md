Email {#emailmanual}
===

NAWA allows you to easily send emails right from your app via SMTP.

## Creating and sending a simple email

Create a `nawa::SimpleEmail` object:

```cpp
nawa::SimpleEmail email;
```

Create `nawa::EmailAddress` objects for the sender, recipient, and optionally 
the Reply-To address:

```cpp
EmailAddress from(connection.request.post["name"], "contactform@example.com");
EmailAddress to("The Admin", "admin@example.com");
EmailAddress replyTo(connection.request.post["email"]);
```

Set the necessary headers (at least From, To, and Subject are generally considered 
to be necessary):

```cpp
email.headers["From"] = from.get();
email.headers["To"] = to.get();
email.headers["Content-Type"] = "text/plain; charset=UTF-8";
email.headers["Subject"] = Encoding::makeEncodedWord("[Contact Form] " + post["subject"]);
```

In the last line, so-called Q-encoding is applied to the subject, just in case it 
contains any illegal characters (as input from the user is used).
You can use the following option to automatically encode the email, to make 
sure it doesn't contain any illegal characters which would violate the email 
standards:

```cpp
email.quotedPrintableEncode = true;
```

And of course, don't forget to set the actual text/body of the email:

```cpp
email.text = "This is an example email!\r\n"
             "Don't forget that according to the standard,\r\n"
             "you have to use Windows-style CRLF line breaks!";
```

Now, create a `nawa::SmtpMailer` object with the credentials of your SMTP 
server:

```cpp
SmtpMailer smtp("example.com", 587, SmtpMailer::TlsMode::REQUIRE_STARTTLS,
                 true, "test@example.com", "12345");
```

Have a look at the `nawa::SmtpMailer` documentation for details.
Add your email to the queue:

```cpp
smtp.enqueue(std::make_shared<SimpleEmail>(email), to, std::make_shared<EmailAddress>(from));
```

Use `std::make_shared` as this function requires the `nawa::Email` object and 
sender (From) address to be passed as shared pointers. You can optionally provide 
a list of replacement rules as an additional parameter to this function, which is 
useful for sending personalized messages to a large number of recipients without 
having to create an Email object for each of them. See `SmtpMailer::enqueue` 
for details.

For sending the same email to multiple recipients, you can also use the function 
`SmtpMailer::bulkEnqueue`.

And finally, send the email:

```cpp
smtp.processQueue();
```

You should, however, catch `nawa::Exception` here, as this function 
establishes a connection to the SMTP server, which may fail. Errors may also 
occur during sending.

If you should need to send multiple emails, add all of them to the queue before 
calling `processQueue`, so that one SMTP connection is used for all of them.

Learning by example is the best way to learn, so have a look at 
`examples/contactform.cpp` for a practical example. The documentation of the classes 
used in this example might contain additional features not mentioned in this tutorial, 
so make sure to read it as well :)

## MIME emails

For sending complex emails with multiple MIME parts (such as text and HTML, 
or emails containing attachments), you can use the `nawa::MimeEmail` class:

```cpp
nawa::MimeEmail email2;
email2.headers["From"] = from.get();
email2.headers["To"] = to.get();
email2.headers["Subject"] = "A MIME email";
```

As you see, setting the headers works just like with simple emails. Concerning 
the content, however, 
they work differently: You'll have to create MIME parts using 
`nawa::MimeEmail::MimePart`. These parts have to be added to one or more 
`nawa::MimeEmail::MimePartList` objects. MIME part lists allow nesting, meaning 
that you can also add MIME part lists to a MIME part list.

The top-level MIME part list is the one in `nawa::MimeEmail::mimePartList`, 
i.e., already part of the `nawa::MimeEmail` object. Adding your MIME part lists 
and MIME parts there will add them to the email.

In the end, you can enqueue and send your MIME email just like a simple email.

As MIME parts and MIME part lists have a lot of options, make sure to read the 
example in `tests/emailtest.cpp` and the documentation of the classes mentioned 
in this tutorial.