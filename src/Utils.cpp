/**
 * \file Utils.cpp
 * \brief Implementation of the Utils class.
 */

/*
 * Copyright (C) 2019 Jan Flaig.
 *
 * This file is part of QSF.
 *
 * QSF is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * QSF is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with QSF.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <iomanip>
#include <unordered_map>
#include <qsf/Utils.h>
#include <fstream>
#include <qsf/UserException.h>

namespace {
    std::unordered_map<std::string, std::string> contentTypeMap = {
            {"aac", "audio/aac"},
            {"arc", "application/x-freearc"},
            {"avi", "video/x-msvideo"},
            {"azw", "application/vnd.amazon.ebook"},
            {"bmp", "image/bmp"},
            {"bz", "application/x-bzip"}, {"bz2", "application/x-bzip2"},
            {"csh", "application/x-csh"},
            {"css", "text/css"},
            {"csv", "text/csv"},
            {"deb", "application/vnd.debian.binary-package"},
            {"doc", "application/msword"}, {"dot", "application/msword"},
            {"docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
            {"dotx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
            {"eot", "application/vnd.ms-fontobject"},
            {"epub", "application/epub+zip"},
            {"flv", "video/x-flv"}, {"f4v", "video/mp4"}, {"f4a", "audio/mp4"},
            {"gif", "image/gif"},
            {"gz", "application/x-gzip"},
            {"htm", "text/html"}, {"html", "text/html"},
            {"ico", "image/vnd.microsoft.icon"},
            {"ics", "text/calendar"},
            {"jar", "application/java-archive"}, {"java", "text/plain"},
            {"jpg", "image/jpeg"}, {"jpeg", "image/jpeg"},
            {"js", "text/javascript"}, {"json", "application/json"},
            {"mid", "audio/x-midi"}, {"midi", "audio/x-midi"},
            {"mjs", "application/javascript"},
            {"mp3", "audio/mpeg"},
            {"mpeg", "video/mpeg"}, {"mp4", "application/mp4"}, {"m4v", "video/mp4"}, {"m4a", "audio/mp4"},
            {"mpkg", "application/vnd.apple.installer+xml"},
            {"odp", "application/vnd.oasis.opendocument.presentation"}, {"otp", "application/vnd.oasis.opendocument.presentation"},
            {"ods", "application/vnd.oasis.opendocument.spreadsheet"}, {"ots", "application/vnd.oasis.opendocument.spreadsheet"},
            {"odt", "application/vnd.oasis.opendocument.text"}, {"ott", "application/vnd.oasis.opendocument.text"},
            {"ogg", "application/ogg"}, {"ogx", "application/ogg"}, {"oga", "audio/ogg"}, {"ogv", "video/ogg"},
            {"otf", "font/otf"},
            {"png", "image/png"},
            {"pdf", "application/pdf"},
            {"ppt", "application/vnd.ms-powerpoint"},
            {"pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation"},
            {"rar", "application/x-rar-compressed"},
            {"rtf", "application/rtf"},
            {"sh", "application/x-sh"},
            {"svg", "image/svg+xml"},
            {"swf", "application/x-shockwave-flash"},
            {"tar", "application/x-tar"},
            {"tif", "image/tiff"}, {"tiff", "image/tiff"},
            {"ttf", "font/ttf"},
            {"txt", "text/plain"},
            {"vsd", "application/vnd.visio"},
            {"wav", "audio/wav"},
            {"weba", "audio/webm"}, {"webm", "video/webm"}, {"webp", "image/webp"},
            {"woff", "font/woff"}, {"woff2", "font/woff2"},
            {"xhtml", "application/xhtml+xml"},
            {"xls", "application/vnd.ms-excel"}, {"xlt", "application/vnd.ms-excel"},
            {"xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
            {"xltx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
            {"xml", "application/xml"},
            {"xul", "application/vnd.mozilla.xul+xml"},
            {"xz", "application/x-xz"},
            {"zip", "application/zip"},
            {"3gp", "video/3gpp"}, {"3g2", "video/3gpp2"},
            {"7z", "application/x-7z-compressed"}
    };
}

void
Qsf::regex_replace_callback(std::string &s, const std::regex &rgx, std::function<std::string(const std::vector<std::string>&)> fmt) {
    // how many submatches do we have to deal with?
    int marks = rgx.mark_count();
    // we want to iterate through all submatches (to collect them in a vector passed to fmt())
    std::vector<int> submatchList;
    for(int i = -1; i <= marks; ++i) {
        submatchList.push_back(i);
    }

    std::sregex_token_iterator begin(s.begin(), s.end(), rgx, submatchList), end;
    std::stringstream out;

    // prefixes and submatches (should) alternate
    int submatch = -1;
    std::vector<std::string> submatchVector;
    for(auto it = begin; it != end; ++it) {
        if(submatch == -1) {
            out << it->str();
            ++submatch;
        }
        else {
            submatchVector.push_back(it->str());
            if(submatch < marks) {
                ++submatch;
            }
            else {
                out << fmt(submatchVector);
                submatchVector.clear();
                submatch = -1;
            }
        }
    }
    s = out.str();
}

std::string Qsf::hex_dump(const std::string &in) {
    std::stringstream rets;
    rets << std::hex << std::setfill('0');
    for(char c: in) {
        rets << std::setw(2) << (int)(unsigned char)c;
    }
    return rets.str();
}

std::string Qsf::to_lowercase(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

std::string Qsf::to_uppercase(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::toupper);
    return s;
}

std::string Qsf::generate_error_page(unsigned int httpStatus) {
    std::string errorStr;
    std::string explanation;
    switch(httpStatus) {
        case 400:
            errorStr = "Bad Request";
            explanation = "The server cannot process your request.";
            break;
        case 401:
            errorStr = "Unauthorized";
            explanation = "The necessary credentials have not been provided.";
            break;
        case 403:
            errorStr = "Forbidden";
            explanation = "You do not have the necessary permissions to view this page.";
            break;
        case 404:
            errorStr = "Not Found";
            explanation = "The requested URL was not found on this server.";
            break;
        case 405:
            errorStr = "Method Not Allowed";
            explanation = "The used request method is not supported for the requested resource.";
            break;
        case 406:
            errorStr = "Not Applicable";
            explanation = "The requested function is unable to produce a resource that satisfies your browser's Accept header.";
            break;
        case 408:
            errorStr = "Request Timeout";
            explanation = "A timeout occurred while waiting for your request.";
            break;
        case 409:
            errorStr = "Conflict";
            explanation = "The request cannot be processed due to a conflict on the underlying resource.";
            break;
        case 410:
            errorStr = "Gone";
            explanation = "The requested resource is no longer available.";
            break;
        case 415:
            errorStr = "Unsupported Media Type";
            explanation = "Your browser has requested a media type that cannot be provided by this resource.";
            break;
        case 418:
            errorStr = "I'm a teapot";
            explanation = "I cannot brew coffee for you.";
            break;
        case 429:
            errorStr = "Too Many Requests";
            break;
        case 451:
            errorStr = "Unavailable For Legal Reasons";
            break;
        case 500:
            errorStr = "Internal Server Error";
            explanation = "The server encountered an internal error and is unable to fulfill your request.";
            break;
        case 501:
            errorStr = "Not Implemented";
            explanation = "The server is not able to fulfill your request.";
            break;
        case 503:
            errorStr = "Service Unavailable";
            explanation = "This service is currently unavailable. Please try again later.";
            break;
            // TODO maybe add others
        default:
            errorStr = "Unknown Error";
    }

    std::stringstream ep;
    ep << "<!DOCTYPE html><html><head><title>" << httpStatus << ' ' << errorStr << "</title></head><body><h1>"
       << errorStr << "</h1><p>" << explanation << "</p></body></html>";

    return ep.str();
}

std::string Qsf::get_file_extension(const std::string &filename) {
    try {
        return filename.substr(filename.find_last_of('.') + 1);
    }
    catch(std::out_of_range&) {}

    return std::string();
}

std::string Qsf::content_type_by_extension(std::string extension) {
    auto ext = to_lowercase(std::move(extension));
    if(contentTypeMap.count(ext) == 1) {
        return contentTypeMap.at(ext);
    }
    return "application/octet-stream";
}

std::string Qsf::make_http_time(time_t time) {
    std::stringstream httpTime;
    httpTime << std::put_time(gmtime(&time), "%a, %d %b %Y %H:%M:%S GMT");
    return httpTime.str();
}

time_t Qsf::read_http_time(std::string httpTime) {
    std::istringstream timeStream(httpTime);
    tm timeStruct;
    timeStream >> std::get_time(&timeStruct, "%a, %d %b %Y %H:%M:%S GMT");
    return mktime(&timeStruct);
}

std::vector<std::string> Qsf::split_string(std::string str, char delimiter, bool ignoreEmpty) {
    std::vector<std::string> ret;
    for(size_t pos = 0; !str.empty();) {
        pos = str.find_first_of(delimiter);
        auto token = str.substr(0, pos);
        if(!ignoreEmpty || !token.empty()) {
            ret.push_back(str.substr(0, pos));
        }
        if(pos < str.length()) {
            str = str.substr(pos + 1);
        }
        else {
            break;
        }
    }
    return ret;
}

std::string Qsf::merge_path(const std::vector<std::string> &path) {
    std::stringstream stringPath;
    for(auto const &e: path) {
        stringPath << '/' << e;
    }
    return stringPath.str();
}

std::vector<std::string> Qsf::split_path(const std::string &pathString) {
    auto ret = split_string(pathString, '/', true);
    return ret;
}

std::string Qsf::convert_line_endings(const std::string &in, const std::string &ending) {
    std::stringstream ret;
    for(const auto &c: in) {
        if(c == '\n') ret << ending;
        else if(c != '\r') ret << c;
    }
    return ret.str();
}

std::string Qsf::get_file_contents(const std::string &path) {
    // open file as binary
    std::ifstream f(path, std::ifstream::binary);

    // throw exception if file cannot be opened
    if(!f) {
        throw Qsf::UserException("Qsf::Utils::file_get_contents", 1, "Cannot open file for reading");
    }

    // get file size
    f.seekg(0, std::ios::end);
    long fs = f.tellg();
    f.seekg(0);

    // load to string
    std::string ret(static_cast<unsigned long>(fs) + 1, '\0');
    f.read(&ret[0], fs);
    
    return ret;
}
