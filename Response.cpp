//
// Created by tobias on 30/10/18.
//

#include <algorithm>
#include "Response.h"

void Qsf::Response::setBody(std::string content) {
    body = std::move(content);
    clearStream();
}

void Qsf::Response::setHeader(std::string key, std::string value) {
    // convert to lowercase
    // TODO check for correctness, maybe using regex (or waste of cpu time?)
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
    headers[key] = std::move(value);
}

void Qsf::Response::unsetHeader(std::string key) {
    headers.erase(key);
}

std::string Qsf::Response::getRaw() {
    mergeStream();
    std::stringstream raw;

    // include headers
    for(auto const &e: headers) {
        raw << e.first << ": " << e.second << "\r\n";
    }
    raw << "\r\n";

    raw << body;
    return raw.str();
}

void Qsf::Response::mergeStream() {
    body += bodyStream.str();
    clearStream();
}

void Qsf::Response::clearStream() {
    bodyStream.str(std::string());
    bodyStream.clear();
}

Qsf::Response& Qsf::Response::operator<<(std::ostream &(*f)(std::ostream &)) {
    bodyStream << f;
    return *this;
}
