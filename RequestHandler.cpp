//
// Created on 30/09/18. All rights reserved.
//

#include <unistd.h>
#include <iostream>
#include <chrono>
#include <thread>
#include "RequestHandler.h"
#include "Request.h"
#include "Response.h"

bool Qsf::RequestHandler::response() {
    Qsf::Request request(*this);
    Qsf::Response response(request);

    response.setCookie("TEST", Cookie("test"));
    Cookie policy;
    policy.httpOnly = true;
    response.setCookiePolicy(policy);

    response << "<!DOCTYPE html>\n"
                "<html><head><title>Test</title></head><body>"
                "<p>Hello World!</p>";

    response.flush();

    // wait a few secs, then print more
    //std::this_thread::sleep_for(std::chrono::seconds(3));

    response << "<p>Hello World 2!</p>"
                "</body></html>";

    response.flush();

    //out << raw;
    //std::cout << raw << std::flush;

    return true;
}

void Qsf::RequestHandler::flush(Qsf::Response& response) {
    auto raw = response.getRaw();
    dump(raw.c_str(), raw.size());
}


//bool Qsf::RequestHandler::response() {
//    Qsf::Request request(*this);
//    out << "Content-Type: text/html; charset=utf-8\r\n\r\n";
//
//    out << "<!DOCTYPE html>\n"
//           "<html><head><title>Test</title></head><body>"
//           "<p>Hello World, GET test = "<< Fastcgipp::Encoding::HTML << request.get["test"]  << Fastcgipp::Encoding::NONE << "</p>"
//           "<p>POST test = " << Fastcgipp::Encoding :: HTML << request.post["test"] << Fastcgipp::Encoding::NONE << "</p>"
//           "<form name=\"testform\" method=\"post\" action=\"\" enctype=\"multipart/form-data\">"
//           "<p>Text input: <input type=\"test\" name=\"test\"> <br>"
//           "File upload: <input type=\"file\" name=\"testfile\" id=\"testfile\"><br>"
//           "<input type=\"submit\" name=\"submit\" value=\"Send\" /></p></form>"
//           "<p>Server Address: " << Fastcgipp::Encoding::HTML << request.env["serverAddress"] << Fastcgipp::Encoding::NONE << "</p>"
//           "<p>Client Address: " << Fastcgipp::Encoding::HTML << request.env["remoteAddress"] << Fastcgipp::Encoding::NONE << "</p>"
//           "<p>Post Content Type: " << Fastcgipp::Encoding::HTML << request.post.getContentType() << Fastcgipp::Encoding::NONE << "<br>"
//           "Raw Post: " << Fastcgipp::Encoding::HTML << request.post.getRaw() << Fastcgipp::Encoding::NONE << "</p>"
//           "</body></html>";
//
//    auto uploadedFiles = request.post.getFileVector("testfile");
////    if(uploadedFiles.size() >= 1) {
////        uploadedFiles[0].writeFile("/home/tinyp/" + uploadedFiles[0].filename);
////        std::cout << uploadedFiles[0].filename << std::endl;
////    }
//
//    return true;
//}

size_t Qsf::RequestHandler::postMax = 0;
uint Qsf::RequestHandler::rawPostAccess = 1;

void Qsf::RequestHandler::setPostConfig(size_t pm, uint rpa) {
    postMax = pm;
    rawPostAccess = rpa;
}

Qsf::RequestHandler::RequestHandler() : Fastcgipp::Request<char>(postMax) {

}

bool Qsf::RequestHandler::inProcessor() {
    postContentType = environment().contentType;
    if(rawPostAccess == QSF_RAWPOST_NEVER) {
        return false;
    }
    else if (rawPostAccess == QSF_RAWPOST_NONSTANDARD &&
            (postContentType == "multipart/form-data" || postContentType == "application/x-www-form-urlencoded")) {
        return false;
    }
    auto postBuffer = environment().postBuffer();
    rawPost = std::string(postBuffer.data(), postBuffer.size());
    return false;
}
