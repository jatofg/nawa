//
// Created on 30/09/18. All rights reserved.
//

#include <unistd.h>
#include "RequestHandler.h"
#include "Request.h"

bool Qsf::RequestHandler::response() {
    Qsf::Request request(*this);
    out << "Content-Type: text/html; charset=utf-8\r\n\r\n";

    out << "<!DOCTYPE html>\n"
           "<html><head><title>Test</title></head><body>"
           "<p>Hello World, GET test = "<< Fastcgipp::Encoding::HTML << request.get["test"]
           << Fastcgipp::Encoding::NONE << "</p>"
           "<p>Current uid: " << getuid() << " - Current gid: " << getgid() << "</p>"
           "</body></html>";

    return true;
}