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
           "<p>Hello World, GET test = "<< Fastcgipp::Encoding::HTML << request.get["test"]  << Fastcgipp::Encoding::NONE << "</p>"
           "<p>POST test = " << Fastcgipp::Encoding :: HTML << request.post["test"] << Fastcgipp::Encoding::NONE << "</p>"
           "<form name=\"testform\" method=\"post\" action=\"\"><p><input type=\"test\" name=\"test\" />"
           "<input type=\"submit\" name=\"submit\" value=\"Send\" /></p></form>"
           "</body></html>";

    return true;
}

size_t Qsf::RequestHandler::postMax = 0;

void Qsf::RequestHandler::setPostMax(size_t pm) {
    postMax = pm;
}

Qsf::RequestHandler::RequestHandler() : Fastcgipp::Request<char>(postMax) {

}