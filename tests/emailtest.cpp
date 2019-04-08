//
// Created by tobias on 08/04/19.
//

#include <qsf/Email.h>
#include <qsf/Utils.h>
#include "app.h"

using namespace Qsf;
using namespace Qsf::Types;

int init(AppInit& appInit) {
    return 0;
}

int handleRequest(Connection& connection) {
    connection.setHeader("content-type", "text/plain");

    // part 1: simple email
    connection.response << "+++++ TEST 1: SimpleEmail +++++\r\n\r\n";

    SimpleEmail email1;
    email1.headers["From"] = "test@example.com";
    email1.headers["Subject"] = "Test mail";
    email1.text = "Test email 'm€ssage' =@#$%^&*()===";
    connection.response << email1.getRaw() << "\r\n\r\n";

    // part 2: MIME email
    connection.response << "+++++ TEST 2: MimeEmail +++++\r\n\r\n";

    MimeEmail email2;
    email2.headers["From"] = "test@example.com";
    email2.headers["Subject"] = "Test mail";

    // text part
    MimeEmail::MimePart textPart;
    textPart.applyEncoding = MimeEmail::MimePart::QUOTED_PRINTABLE;
    textPart.contentType = "text/plain; charset=utf-8";
    textPart.contentDisposition = "inline";
    textPart.data = "Test email 'm€ssage' =@#$%^&*()=== asjdflkasjdfoiwej sdflkawjefijwefijsldjf dsnvndvjnwkjenggfweg";

    // html part
    MimeEmail::MimePart htmlPart;
    htmlPart.applyEncoding = MimeEmail::MimePart::QUOTED_PRINTABLE;
    htmlPart.contentType = "text/html";
    htmlPart.contentDisposition = "inline";
    htmlPart.data = "<html><head><title>Bla</title></head>\n<body><p>T&auml;st</p></body></html>";

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
    connection.response << email2.getRaw();
}
