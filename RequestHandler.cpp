//
// Created on 30/09/18. All rights reserved.
//

#include <unistd.h>
#include <iostream>
#include "RequestHandler.h"
#include "Request.h"

bool Qsf::RequestHandler::response() {
    Qsf::Request request(*this);
    out << "Content-Type: text/html; charset=utf-8\r\n\r\n";

    out << "<!DOCTYPE html>\n"
           "<html><head><title>Test</title></head><body>"
           "<p>Hello World, GET test = "<< Fastcgipp::Encoding::HTML << request.get["test"]  << Fastcgipp::Encoding::NONE << "</p>"
           "<p>POST test = " << Fastcgipp::Encoding :: HTML << request.post["test"] << Fastcgipp::Encoding::NONE << "</p>"
           "<form name=\"testform\" method=\"post\" action=\"\" enctype=\"multipart/form-data\">"
           "<p>Text input: <input type=\"test\" name=\"test\"> <br>"
           "File upload: <input type=\"file\" name=\"testfile\" id=\"testfile\"><br>"
           "<input type=\"submit\" name=\"submit\" value=\"Send\" /></p></form>"
           "<p>Server Address: " << Fastcgipp::Encoding::HTML << request.env["serverAddress"] << Fastcgipp::Encoding::NONE << "</p>"
           "<p>Client Address: " << Fastcgipp::Encoding::HTML << request.env["remoteAddress"] << Fastcgipp::Encoding::NONE << "</p>"
           "<p>Raw Post: " << Fastcgipp::Encoding::HTML << request.post.getRaw() << Fastcgipp::Encoding::NONE << "</p>"
           "</body></html>";

    auto uploadedFiles = request.post.getFileVector("testfile");
    if(uploadedFiles.size() >= 1) {
        uploadedFiles[0].writeFile("/home/tinyp/" + uploadedFiles[0].filename);
        std::cout << uploadedFiles[0].filename << std::endl;
    }

    return true;
}

size_t Qsf::RequestHandler::postMax = 0;

void Qsf::RequestHandler::setPostMax(size_t pm) {
    postMax = pm;
}

Qsf::RequestHandler::RequestHandler() : Fastcgipp::Request<char>(postMax) {

}