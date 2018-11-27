//
// Created on 30/09/18. All rights reserved.
//

#include <unistd.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <dlfcn.h>
#include "RequestHandler.h"
#include "Request.h"
#include "Connection.h"

static Qsf::handleRequest_t* appHandleRequest;

bool Qsf::RequestHandler::response() {
    Qsf::Request request(*this);
    Qsf::Connection connection(request, config);

    // run application
    // TODO maybe do something with return value in future
    appHandleRequest(connection);

    // flush response
    connection.flush();

    return true;
}

void Qsf::RequestHandler::flush(Qsf::Connection& connection) {
    auto raw = connection.getRaw();
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
Qsf::Config Qsf::RequestHandler::config;

void Qsf::RequestHandler::setConfig(const Qsf::Config& cfg, void* appOpen) {
    config = cfg;
    postMax = 0;
    try {
        postMax = config.isSet({"post", "max_size"})
                      ? static_cast<size_t>(std::stoul(config[{"post", "max_size"}])) * 1024 : 0;
    }
    catch(std::invalid_argument& e) {
        std::cerr << "WARNING: Invalid value given for post/max_size given in the config file." << std::endl;
    }
    // raw_access is translated to an integer according to the macros defined in RequestHandler.h
    std::string rawPostStr = config[{"post", "raw_access"}];
    rawPostAccess = (rawPostStr == "never")
                   ? QSF_RAWPOST_NEVER : ((rawPostStr == "always") ? QSF_RAWPOST_ALWAYS : QSF_RAWPOST_NONSTANDARD);

    // load appHandleRequest function
    appHandleRequest = (Qsf::handleRequest_t*) dlsym(appOpen, "handleRequest");
    auto dlsymErr = dlerror();
    if(dlsymErr) {
        std::cerr << "Fatal Error: Could not load handleRequest function from application: " << dlsymErr << std::endl;
        exit(1);
    }
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
