/**
 * \file emailtest.cpp
 * \brief Tests for SmtpMailer. Replace the email addresses and login data with existing ones to test.
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

#include <nawa/Email.h>
#include <nawa/Utils.h>
#include <ctime>
#include <iomanip>
#include <nawa/SmtpMailer.h>
#include <nawa/Application.h>
#include <nawa/UserException.h>

using namespace nawa;
using namespace std;

int init(AppInit &appInit) {
    return 0;
}

int handleRequest(Connection &connection) {
    connection.setHeader("content-type", "text/plain; charset=utf-8");

    connection.response << "SMTP time: ";
    time_t currentTime = time(nullptr);
    tm ltime;
    localtime_r(&currentTime, &ltime);
    connection.response << put_time(&ltime, "%a, %e %b %Y %H:%M:%S %z") << "\r\n\r\n";

    // The replacement rules to apply
    auto replacementRules = make_shared<ReplacementRules>();
    replacementRules->insert({"Test", "T€st"});
    replacementRules->insert({"email", "émail"});

    EmailAddress from("John Doe", "johndoe@example.com");
    EmailAddress to("The Admin", "theadmin@example.com");

    // part 1: simple email
    connection.response << "+++++ TEST 1: SimpleEmail +++++\r\n\r\n";

    SimpleEmail email1;
    email1.headers["From"] = from.get();
    email1.headers["To"] = to.get();
    email1.headers["Content-Type"] = "text/plain; charset=utf-8";
    email1.headers["Subject"] = "Test mail";
    email1.text = "Test email 'm€ssage' =@#$%^&*()===";
    connection.response << email1.getRaw(replacementRules) << "\r\n\r\n";

    // part 2: MIME email
    connection.response << "+++++ TEST 2: MimeEmail +++++\r\n\r\n";

    MimeEmail email2;
    email2.headers["From"] = from.get();
    email2.headers["To"] = to.get();
    email2.headers["Subject"] = "Test email 2";

    // text part
    MimeEmail::MimePart textPart;
    textPart.applyEncoding = MimeEmail::MimePart::QUOTED_PRINTABLE;
    textPart.contentType = "text/plain; charset=utf-8";
    textPart.contentDisposition = "inline";
    textPart.data = "Test email 'm€ssage' =@#$%^&*()=== asjdflkasjdfoiwej sdflkawjefijwefijsldjf dsnvndvjnwkjenggfweg";

    // html part
    MimeEmail::MimePart htmlPart;
    htmlPart.applyEncoding = MimeEmail::MimePart::QUOTED_PRINTABLE;
    htmlPart.contentType = "text/html; charset=utf-8";
    htmlPart.contentDisposition = "inline";
    htmlPart.allowReplacements = true;
    htmlPart.data = "<html><head><title>Bla</title></head>\n<body><p>Test T&auml;st email</p></body></html>";

    // attachment
    MimeEmail::MimePart attachmentPart;
    attachmentPart.applyEncoding = MimeEmail::MimePart::BASE64;
    attachmentPart.contentType = "image/png; name=test.png";
    attachmentPart.contentDisposition = "attachment; filename=test.png";
    attachmentPart.data = get_file_contents("/home/tobias/Pictures/wireguard-loc.png");

    // create an alternative-type MIME container for text and html
    MimeEmail::MimePartList textAndHtml;
    textAndHtml.multipartType = MimeEmail::MimePartList::MultipartType::ALTERNATIVE;
    textAndHtml.mimeParts.emplace_back(textPart);
    textAndHtml.mimeParts.emplace_back(htmlPart);

    // add the text/html alternative-type container and the attachment to an outer mixed-type container
    email2.mimePartList.multipartType = MimeEmail::MimePartList::MultipartType::MIXED;
    email2.mimePartList.mimeParts.emplace_back(textAndHtml);
    email2.mimePartList.mimeParts.emplace_back(attachmentPart);

    // print the result
    connection.response << email2.getRaw(replacementRules);

    // if GET sendit=yes, then send it via SMTP localhost
    if (connection.request.get["sendit"] == "yes") {

        connection.response << "\r\n";

        // connect to an SMTP server - default is localhost:25 without TLS (good for use on live web/mail servers only)
        SmtpMailer smtp("example.com", 587, SmtpMailer::TlsMode::REQUIRE_STARTTLS,
                        true, "test@example.com", "12345");
        smtp.enqueue(make_shared<SimpleEmail>(email1), to, make_shared<EmailAddress>(from), replacementRules);
        smtp.enqueue(make_shared<MimeEmail>(email2), to, make_shared<EmailAddress>(from), replacementRules);

        try {
            smtp.processQueue();
            connection.response << "Mail sent successfully!";
        }
        catch (const UserException &e) {
            connection.response << "Error sending mail: " << e.what();
        }

    }

    return 0;

}
