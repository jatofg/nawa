//
// Created by tobias on 08/04/19.
//

#include <qsf/Email.h>
#include <qsf/Utils.h>
#include <ctime>
#include <iomanip>
#include <qsf/SmtpMailer.h>
#include "app.h"

using namespace Qsf;

int init(AppInit& appInit) {
    return 0;
}

int handleRequest(Connection& connection) {
    connection.setHeader("content-type", "text/plain; charset=utf-8");

    connection.response << "SMTP time: ";
    time_t currentTime = time(nullptr);
    tm ltime;
    localtime_r(&currentTime, &ltime);
    connection.response << std::put_time(&ltime, "%a, %e %b %Y %H:%M:%S %z") << "\r\n\r\n";
    
    // The replacement rules to apply
    ReplacementRules replacementRules;
    replacementRules.insert({"Test", "T€st"});
    replacementRules.insert({"email", "émail"});

    EmailAddress from("QSF Test", "qsftest@tobias-flaig.de");
    EmailAddress to("Tobias Flaig", "me@tobias-flaig.de");

    // part 1: simple email
    connection.response << "+++++ TEST 1: SimpleEmail +++++\r\n\r\n";

    SimpleEmail email1;
    email1.headers["From"] = from.get();
    email1.headers["To"] = to.get();
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
    textAndHtml.multipartType = MimeEmail::MimePartList::ALTERNATIVE;
    textAndHtml.mimeParts.emplace_back(textPart);
    textAndHtml.mimeParts.emplace_back(htmlPart);

    // add the text/html alternative-type container and the attachment to an outer mixed-type container
    email2.mimePartList.multipartType = MimeEmail::MimePartList::MIXED;
    email2.mimePartList.mimeParts.emplace_back(textAndHtml);
    email2.mimePartList.mimeParts.emplace_back(attachmentPart);

    // print the result
    connection.response << email2.getRaw(replacementRules);

    // if GET sendit=yes, then send it via SMTP localhost
    if(connection.request.get["sendit"] == "yes") {

        connection.response << "\r\n";

        // connect to an SMTP server - default is localhost:25 without TLS (good for use on live web/mail servers only)
        SmtpMailer smtp("s4.quicktools.org", 587, SmtpMailer::REQUIRE_STARTTLS, true, "qsftest@tobias-flaig.de", "875EMKJK4xLc5ur");
        smtp.enqueue(std::make_shared<MimeEmail>(email2), to, std::make_shared<EmailAddress>(from), replacementRules);

        try {
            smtp.processQueue();
            connection.response << "Mail sent successfully!";
        }
        catch(const UserException &e) {
            connection.response << "Error sending mail: " << e.what();
        }

    }

}