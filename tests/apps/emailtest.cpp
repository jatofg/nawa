/**
 * \file emailtest.cpp
 * \brief Tests for SmtpMailer. Replace the email addresses and login data with existing ones to test.
 */

/*
 * Copyright (C) 2019-2021 Tobias Flaig.
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

#include <ctime>
#include <iomanip>
#include <nawa/Exception.h>
#include <nawa/application.h>
#include <nawa/mail/Email/impl/MimeEmail.h>
#include <nawa/mail/Email/impl/SimpleEmail.h>
#include <nawa/mail/SmtpMailer.h>
#include <nawa/util/encoding.h>
#include <nawa/util/utils.h>

using namespace nawa;
using namespace nawa::mail;
using namespace std;

int init(AppInit& appInit) {
    return 0;
}

int handleRequest(Connection& connection) {
    auto& resp = connection.responseStream();

    connection.setHeader("content-type", "text/plain; charset=utf-8");

    resp << "SMTP time: ";
    time_t currentTime = time(nullptr);
    tm ltime;
    localtime_r(&currentTime, &ltime);
    resp << put_time(&ltime, "%a, %e %b %Y %H:%M:%S %z") << "\r\n\r\n";

    // The replacement rules to apply
    auto replacementRules = make_shared<ReplacementRules>();
    replacementRules->insert({"Test", "T€st"});
    replacementRules->insert({"email", "émail"});

    EmailAddress from("John Doe", "johndoe@example.com");
    EmailAddress to("The Admin", "theadmin@example.com");

    // part 1: simple email
    resp << "+++++ TEST 1: SimpleEmail +++++\r\n\r\n";

    SimpleEmail email1;
    email1.headers()["From"] = from.get();
    email1.headers()["To"] = to.get();
    email1.headers()["Content-Type"] = "text/plain; charset=utf-8";
    email1.headers()["Subject"] = encoding::makeEncodedWord("Test mail");
    email1.text() = "Test email 'm€ssage' =@#$%^&*()===";
    resp << email1.getRaw(replacementRules) << "\r\n\r\n";

    // part 2: MIME email
    resp << "+++++ TEST 2: MimeEmail +++++\r\n\r\n";

    MimeEmail email2;
    email2.headers()["From"] = from.get();
    email2.headers()["To"] = to.get();
    email2.headers()["Subject"] = "Test email 2";

    // text part
    MimeEmail::MimePart textPart;
    textPart.applyEncoding() = MimeEmail::MimePart::ApplyEncoding::QUOTED_PRINTABLE;
    textPart.contentType() = "text/plain; charset=utf-8";
    textPart.contentDisposition() = "inline";
    textPart.partData() = "Test email 'm€ssage' =@#$%^&*()=== asjdflkasjdfoiwej sdflkawjefijwefijsldjf dsnvndvjnwkjenggfweg";

    // html part
    MimeEmail::MimePart htmlPart;
    htmlPart.applyEncoding() = MimeEmail::MimePart::ApplyEncoding::QUOTED_PRINTABLE;
    htmlPart.contentType() = "text/html; charset=utf-8";
    htmlPart.contentDisposition() = "inline";
    htmlPart.allowReplacements() = true;
    htmlPart.partData() = "<html><head><title>Bla</title></head>\n<body><p>Test T&auml;st email</p></body></html>";

    // attachment
    MimeEmail::MimePart attachmentPart;
    attachmentPart.applyEncoding() = MimeEmail::MimePart::ApplyEncoding::BASE64;
    attachmentPart.contentType() = "image/png; name=test.png";
    attachmentPart.contentDisposition() = "attachment; filename=test.png";
    try {
        attachmentPart.partData() = utils::getFileContents("/home/tobias/Pictures/testimage.png");
    } catch (Exception const& e) {
        resp << "!!! Specified image file could not be loaded: " << e.getMessage() << " !!!\r\n\r\n";
    }

    // create an alternative-type MIME container for text and html
    MimeEmail::MimePartList textAndHtml;
    textAndHtml.multipartType() = MimeEmail::MimePartList::MultipartType::ALTERNATIVE;
    textAndHtml.mimeParts().emplace_back(textPart);
    textAndHtml.mimeParts().emplace_back(htmlPart);

    // add the text/html alternative-type container and the attachment to an outer mixed-type container
    email2.mimePartList().multipartType() = MimeEmail::MimePartList::MultipartType::MIXED;
    email2.mimePartList().mimeParts().emplace_back(textAndHtml);
    email2.mimePartList().mimeParts().emplace_back(attachmentPart);

    // print the result
    resp << email2.getRaw(replacementRules);

    // if GET sendit=yes, then send it via SMTP localhost
    if (connection.request().get()["sendit"] == "yes") {

        resp << "\r\n";

        // connect to an SMTP server - default is localhost:25 without TLS (good for use on live web/mail servers only)
        mail::SmtpMailer smtp("example.com", 587, mail::SmtpMailer::TlsMode::REQUIRE_STARTTLS,
                              true, "test@example.com", "12345");
        smtp.enqueue(make_shared<SimpleEmail>(email1), to, make_shared<EmailAddress>(from), replacementRules);
        smtp.enqueue(make_shared<MimeEmail>(email2), to, make_shared<EmailAddress>(from), replacementRules);

        try {
            smtp.processQueue();
            resp << "Mail sent successfully!";
        } catch (Exception const& e) {
            resp << "Error sending mail: " << e.getDebugMessage();
        }
    }

    return 0;
}
