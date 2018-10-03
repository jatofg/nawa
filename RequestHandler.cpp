//
// Created on 30/09/18. All rights reserved.
//

#include "RequestHandler.h"
#include "QsfRequest.h"

bool RequestHandler::response() {
    QsfRequest::Env qre(*this);
    out << "Content-Type: text/html; charset=utf-8\r\n\r\n";

    out << "<!DOCTYPE html>\n"
           "<html><head><title>Test</title></head><body>"
           "<p>Hello World äöü é</p>"
           "</body></html>";

    return true;
}