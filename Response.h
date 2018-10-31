//
// Created on 30/10/18.
//

#ifndef QSF_RESPONSE_H
#define QSF_RESPONSE_H

#include <string>
#include <map>
#include <sstream>

namespace Qsf {
    class Response {
        std::string body;
        std::stringstream bodyStream;
        std::map<std::string, std::string> headers;
        // set body
        // operator << and >> overloads for stream handling (body)
        // add/remove/set headers
        // handling of html special chars (in some other [static] class?)
        // cookie setting
        // export of data to fcgi++ (must merge bodyStream into body) => getRaw (?)
        // constructor should set cookies (?)
        // automatic handling of session cookies
        void clearStream();
        void mergeStream();
    public:
        void setBody(std::string content);
        void setHeader(std::string key, std::string value);
        void unsetHeader(std::string key);
        std::string getRaw();
        Response& operator<<(std::ostream&(*f)(std::ostream&));
    };
}

// TODO add stream modifiers if needed

#endif //QSF_RESPONSE_H
