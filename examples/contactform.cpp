/**
 * \file contactform.cpp
 * \brief A simple contact form application using nawa.
 */

/*
 * Copyright (C) 2019-2020 Tobias Flaig.
 *
 * This file is part of nawa.
 *
 * nawa is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * nawa is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with nawa.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <nawa/Application.h>
#include <random>
#include <nawa/SmtpMailer.h>
#include <nawa/Encoding.h>

using namespace std;
using namespace nawa;

int init(AppInit& appInit) {

    return 0;
}

int handleRequest(Connection& connection) {

    // make sure cookies are a bit more secure
    Cookie policy;
    policy.httpOnly = true;
    connection.setCookiePolicy(policy);

    // we use session variables for a basic spam protection
    connection.session.start();
    // randVal is a random value that could be present from a previous request
    auto randVal = connection.session["randVal"];

    // we will work with POST data quite often now, so we'll create a shortcut
    auto const &post = connection.request.post;

    // HTML header
    connection.response << "<!DOCTYPE html><head><title>Contact Form</title></head><body>";

    // if there is POST data, and randVal is not empty, process it
    if(randVal.has_value() && post.count("rand_val") == 1) {

        // check for correct rand_val
        bool randValid = false;
        try {
            if(any_cast<unsigned int>(randVal) == stoul(post["rand_val"])) {
                randValid = true;
            }
        }
        catch(...) {}
        
        if(!randValid) {
            connection.response << "<p>No spamming, please!</p>";
            
            return 0;
        }

        // check whether the user filled in all required fields
        if(!post.count("name") || !post.count("email") || !post.count("subject")
            || !post.count("message")) {

            connection.response << "<p>Please go back and fill in all required fields!</p></body></html>";

            return 0;
        }

        // create a SimpleEmail object from the form entries
        SimpleEmail email;
        EmailAddress from(post["name"], "contactform@example.com");
        EmailAddress to("The Admin", "admin@example.com");
        EmailAddress replyTo(post["email"]);
        email.headers["From"] = from.get();
        email.headers["To"] = to.get();
        email.headers["Content-Type"] = "text/plain; charset=UTF-8";
        // apply Q-encoding to the header, just in case the user used special chars in the subject
        email.headers["Subject"] = Encoding::makeEncodedWord("[Contact Form] " + post["subject"]);
        email.quotedPrintableEncode = true;
        email.text = "This contact form was sent via an example nawa application!\r\n\r\n"
                     "Name of the sender: " + post["name"] + "\r\n"
                     "Email of the sender: " + post["email"] + "\r\n\r\n" + post["message"];

        // now use SmtpMailer to send the email to your mailbox
        SmtpMailer smtp("example.com", 587, SmtpMailer::TlsMode::REQUIRE_STARTTLS,
                true, "test@example.com", "12345");
        smtp.enqueue(std::make_shared<SimpleEmail>(email), to, std::make_shared<EmailAddress>(from));

        connection.response << "<p>Message sent successfully!</p></body></html>";

        return 0;
    }

    // generate a new random value for inclusion in our form, so we can check it later
    random_device rd;
    randVal = rd();
    connection.session.set("randVal", randVal);

    // and show the form!
    connection.response << "<p>Please fill in the following form in order to contact us! All fields are required.</p>\r\n"
                           "<form name=\"contact\" method=\"post\" action=\"?\">"
                           "<input type=\"hidden\" name=\"rand_val\" value=\"" << any_cast<unsigned int>(randVal) << "\" />"
                           "<p>Your name: <input type=\"text\" name=\"name\" /></p>"
                           "<p>Email address: <input type=\"email\" name=\"email\" /></p>"
                           "<p>Subject: <input type=\"text\" name=\"subject\" /></p>"
                           "<p>Message: <textarea name=\"message\" rows=\"5\" cols=\"30\"></textarea></p>"
                           "<p><input type=\"submit\" name=\"go\" value=\"Submit\" /></p>"
                           "</form></body></html>";
    
    return 0;
}