/*
 * Copyright (C) 2019-2022 Tobias Flaig.
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

/**
 * \file contactform.cpp
 * \brief A simple contact form application using nawa.
 */

#include <nawa/Exception.h>
#include <nawa/application.h>
#include <nawa/logging/Log.h>
#include <nawa/mail/Email/impl/SimpleEmail.h>
#include <nawa/mail/SmtpMailer.h>
#include <nawa/util/encoding.h>
#include <random>

using namespace std;
using namespace nawa;
using namespace nawa::mail;

namespace {
    Log logger;
}

int init(AppInit& appInit) {
    // set up logging
    logger.setAppname("Contact form app");

    return 0;
}

int handleRequest(Connection& connection) {

    // make sure cookies are a bit more secure
    connection.setCookiePolicy(Cookie().httpOnly(true));

    // we use session variables for a basic spam protection
    connection.session().start();
    // randVal is a random value that could be present from a previous request
    auto randVal = connection.session()["randVal"];

    // we will work with POST data quite often now, so we'll create a shortcut
    auto const& post = connection.request().post();

    // get a shortcut for the response stream
    auto& resp = connection.responseStream();

    // HTML header
    resp << "<!DOCTYPE html><head><title>Contact Form</title></head><body>";

    // if there is POST data, and randVal is not empty, process it
    if (randVal.has_value() && post.count("rand_val") == 1) {

        // check for correct rand_val
        bool randValid = false;
        try {
            if (any_cast<unsigned int>(randVal) == stoul(post["rand_val"])) {
                randValid = true;
            }
        } catch (...) {}

        if (!randValid) {
            resp << "<p>No spamming, please!</p>";
            return 0;
        }

        // check whether the user filled in all required fields
        if (post["name"].empty() || post["email"].empty() || post["subject"].empty() || post["message"].empty()) {
            resp << "<p>Please go back and fill in all required fields!</p></body></html>";
            return 0;
        }

        // create a SimpleEmail object from the form entries
        SimpleEmail email;
        EmailAddress from(post["name"], "contactform@example.com");
        EmailAddress to("The Admin", "admin@example.com");
        EmailAddress replyTo(post["email"]);
        email.headers()["From"] = from.get();
        email.headers()["To"] = to.get();
        email.headers()["Content-Type"] = "text/plain; charset=UTF-8";
        // apply Q-encoding to the header, just in case the user used special chars in the subject
        email.headers()["Subject"] = encoding::makeEncodedWord("[Contact Form] " + post["subject"]);
        email.quotedPrintableEncode(true);
        email.text() = "This contact form was sent via an example nawa application!\r\n\r\n"
                       "Name of the sender: " +
                       post["name"] + "\r\n"
                                      "Email of the sender: " +
                       post["email"] + "\r\n\r\n" +
                       post["message"];

        // now use SmtpMailer to send the email to your mailbox
        mail::SmtpMailer smtp("example.com", 587, mail::SmtpMailer::TlsMode::REQUIRE_STARTTLS,
                              true, "test@example.com", "12345");
        smtp.enqueue(std::make_shared<SimpleEmail>(email), to, std::make_shared<EmailAddress>(from));
        try {
            smtp.processQueue();
            resp << "<p>Message sent successfully!</p>";
        } catch (const Exception& e) {
            resp << "<p>Message could not be sent due to a technical problem :(</p>";
            NLOG_ERROR(logger, "Error sending email: " << e.getMessage())
            NLOG_DEBUG(logger, "Debug info: " << e.getDebugMessage())
        }

        resp << "</body></html>";

        return 0;
    }

    // generate a new random value for inclusion in our form, so we can check it later
    random_device rd;
    randVal = rd();
    connection.session().set("randVal", randVal);

    // and show the form!
    resp << "<p>Please fill in the following form in order to contact us! All fields are required.</p>\r\n"
            "<form name=\"contact\" method=\"post\" action=\"?\">"
            "<input type=\"hidden\" name=\"rand_val\" value=\""
         << any_cast<unsigned int>(randVal)
         << "\" />"
            "<p>Your name: <input type=\"text\" name=\"name\" /></p>"
            "<p>Email address: <input type=\"email\" name=\"email\" /></p>"
            "<p>Subject: <input type=\"text\" name=\"subject\" /></p>"
            "<p>Message: <textarea name=\"message\" rows=\"5\" cols=\"30\"></textarea></p>"
            "<p><input type=\"submit\" name=\"go\" value=\"Submit\" /></p>"
            "</form></body></html>";

    return 0;
}
