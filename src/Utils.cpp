/**
 * \file Utils.cpp
 * \brief Implementation of the Utils class.
 */

/*
 * Copyright (C) 2019-2020 Tobias Flaig.
 *
 * This file is part of nawa.
 *
 * nawa is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * nawa is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with nawa.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <iomanip>
#include <unordered_map>
#include <nawa/Utils.h>
#include <fstream>
#include <nawa/Exception.h>
#include <nawa/Encoding.h>
#include <boost/algorithm/string.hpp>

using namespace nawa;
using namespace std;

namespace {
    unordered_map<string, string> contentTypeMap = {
            {"aac",   "audio/aac"},
            {"arc",   "application/x-freearc"},
            {"avi",   "video/x-msvideo"},
            {"azw",   "application/vnd.amazon.ebook"},
            {"bmp",   "image/bmp"},
            {"bz",    "application/x-bzip"},
            {"bz2",   "application/x-bzip2"},
            {"csh",   "application/x-csh"},
            {"css",   "text/css"},
            {"csv",   "text/csv"},
            {"deb",   "application/vnd.debian.binary-package"},
            {"doc",   "application/msword"},
            {"dot",   "application/msword"},
            {"docx",  "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
            {"dotx",  "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
            {"eot",   "application/vnd.ms-fontobject"},
            {"epub",  "application/epub+zip"},
            {"flv",   "video/x-flv"},
            {"f4v",   "video/mp4"},
            {"f4a",   "audio/mp4"},
            {"gif",   "image/gif"},
            {"gz",    "application/x-gzip"},
            {"htm",   "text/html"},
            {"html",  "text/html"},
            {"ico",   "image/vnd.microsoft.icon"},
            {"ics",   "text/calendar"},
            {"jar",   "application/java-archive"},
            {"java",  "text/plain"},
            {"jpg",   "image/jpeg"},
            {"jpeg",  "image/jpeg"},
            {"js",    "text/javascript"},
            {"json",  "application/json"},
            {"mid",   "audio/x-midi"},
            {"midi",  "audio/x-midi"},
            {"mjs",   "application/javascript"},
            {"mp3",   "audio/mpeg"},
            {"mpeg",  "video/mpeg"},
            {"mp4",   "application/mp4"},
            {"m4v",   "video/mp4"},
            {"m4a",   "audio/mp4"},
            {"mpkg",  "application/vnd.apple.installer+xml"},
            {"odp",   "application/vnd.oasis.opendocument.presentation"},
            {"otp",   "application/vnd.oasis.opendocument.presentation"},
            {"ods",   "application/vnd.oasis.opendocument.spreadsheet"},
            {"ots",   "application/vnd.oasis.opendocument.spreadsheet"},
            {"odt",   "application/vnd.oasis.opendocument.text"},
            {"ott",   "application/vnd.oasis.opendocument.text"},
            {"ogg",   "application/ogg"},
            {"ogx",   "application/ogg"},
            {"oga",   "audio/ogg"},
            {"ogv",   "video/ogg"},
            {"otf",   "font/otf"},
            {"png",   "image/png"},
            {"pdf",   "application/pdf"},
            {"ppt",   "application/vnd.ms-powerpoint"},
            {"pptx",  "application/vnd.openxmlformats-officedocument.presentationml.presentation"},
            {"rar",   "application/x-rar-compressed"},
            {"rtf",   "application/rtf"},
            {"sh",    "application/x-sh"},
            {"svg",   "image/svg+xml"},
            {"swf",   "application/x-shockwave-flash"},
            {"tar",   "application/x-tar"},
            {"tif",   "image/tiff"},
            {"tiff",  "image/tiff"},
            {"ttf",   "font/ttf"},
            {"txt",   "text/plain"},
            {"vsd",   "application/vnd.visio"},
            {"wav",   "audio/wav"},
            {"weba",  "audio/webm"},
            {"webm",  "video/webm"},
            {"webp",  "image/webp"},
            {"woff",  "font/woff"},
            {"woff2", "font/woff2"},
            {"xhtml", "application/xhtml+xml"},
            {"xls",   "application/vnd.ms-excel"},
            {"xlt",   "application/vnd.ms-excel"},
            {"xlsx",  "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
            {"xltx",  "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
            {"xml",   "application/xml"},
            {"xul",   "application/vnd.mozilla.xul+xml"},
            {"xz",    "application/x-xz"},
            {"zip",   "application/zip"},
            {"3gp",   "video/3gpp"},
            {"3g2",   "video/3gpp2"},
            {"7z",    "application/x-7z-compressed"}
    };

    /**
     * Check whether current locale is "C" and, if it is not, reset it to "C". This function might not be thread-safe,
     * but ideally, it should never have to update the locale. The app should not update the locale itself.
     */
//    inline void resetLocale() {
//        if(locale() != locale::classic()) {
//            locale::global(locale::classic());
//        }
//    }

    /**
     * Get the day of week as a string. This function is used instead of the %a specifier, as it is locale-independent,
     * and checking and setting the locale is not the best idea (not thread-safe).
     * @param dow Day of week, range [0-6], where 0 is Sun and 6 is Sat.
     * @return Abbreviated weekday string representation.
     */
    inline string getDayOfWeek(int dow) {
        string ret;
        switch (dow) {
            case 0:
                ret = "Sun";
                break;
            case 1:
                ret = "Mon";
                break;
            case 2:
                ret = "Tue";
                break;
            case 3:
                ret = "Wed";
                break;
            case 4:
                ret = "Thu";
                break;
            case 5:
                ret = "Fri";
                break;
            case 6:
                ret = "Sat";
                break;
            default:
                break;
        }
        return ret;
    }

    /**
     * Get the abbreviated month representation as a string, used instead of the %b specifier for the same reasons as
     * above.
     * @param mon Months since January (range [0-11]).
     * @return String representation.
     */
    inline string getMonth(int mon) {
        string ret;
        switch (mon) {
            case 0:
                ret = "Jan";
                break;
            case 1:
                ret = "Feb";
                break;
            case 2:
                ret = "Mar";
                break;
            case 3:
                ret = "Apr";
                break;
            case 4:
                ret = "May";
                break;
            case 5:
                ret = "Jun";
                break;
            case 6:
                ret = "Jul";
                break;
            case 7:
                ret = "Aug";
                break;
            case 8:
                ret = "Sep";
                break;
            case 9:
                ret = "Oct";
                break;
            case 10:
                ret = "Nov";
                break;
            case 11:
                ret = "Dec";
                break;
            default:
                break;
        }
        return ret;
    }

}

// doxygen bug, somehow doxygen does not like std::function
void
nawa::regex_replace_callback(std::string &s, const std::regex &rgx,
                             const std::function<std::string(const std::vector<std::string> &)> &fmt) {
    // how many submatches do we have to deal with?
    int marks = rgx.mark_count();
    // we want to iterate through all submatches (to collect them in a vector passed to fmt())
    vector<int> submatchList;
    for (int i = -1; i <= marks; ++i) {
        submatchList.push_back(i);
    }

    sregex_token_iterator begin(s.begin(), s.end(), rgx, submatchList), end;
    stringstream out;

    // prefixes and submatches (should) alternate
    int submatch = -1;
    vector<string> submatchVector;
    for (auto it = begin; it != end; ++it) {
        if (submatch == -1) {
            out << it->str();
            ++submatch;
        } else {
            submatchVector.push_back(it->str());
            if (submatch < marks) {
                ++submatch;
            } else {
                out << fmt(submatchVector);
                submatchVector.clear();
                submatch = -1;
            }
        }
    }
    s = out.str();
}

string nawa::hex_dump(const string &in) {
    stringstream rets;
    rets << hex << setfill('0');
    for (char c: in) {
        rets << setw(2) << (int) (unsigned char) c;
    }
    return rets.str();
}

string nawa::to_lowercase(string s) {
    transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

string nawa::to_uppercase(string s) {
    transform(s.begin(), s.end(), s.begin(), ::toupper);
    return s;
}

string nawa::generate_error_page(unsigned int httpStatus) {
    string errorStr;
    string explanation;
    switch (httpStatus) {
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
        default:
            errorStr = "Unknown Error";
    }

    stringstream ep;
    ep << "<!DOCTYPE html><html><head><title>" << httpStatus << ' ' << errorStr << "</title></head><body><h1>"
       << errorStr << "</h1><p>" << explanation << "</p></body></html>";

    return ep.str();
}

string nawa::get_file_extension(const string &filename) {
    try {
        return filename.substr(filename.find_last_of('.') + 1);
    }
    catch (out_of_range &) {}

    return string();
}

string nawa::content_type_by_extension(string extension) {
    auto ext = to_lowercase(move(extension));
    if (contentTypeMap.count(ext) == 1) {
        return contentTypeMap.at(ext);
    }
    return "application/octet-stream";
}

string nawa::make_http_time(time_t time1) {
    stringstream httpTime;
    tm gmt;
    gmtime_r(&time1, &gmt);
    httpTime << getDayOfWeek(gmt.tm_wday) << put_time(&gmt, ", %d ") << getMonth(gmt.tm_mon);
    httpTime << put_time(&gmt, " %Y %H:%M:%S GMT");

    return httpTime.str();
}

time_t nawa::read_http_time(const string &httpTime) {
    istringstream timeStream(httpTime);
    tm timeStruct;
    timeStream >> get_time(&timeStruct, "%a, %d %b %Y %H:%M:%S GMT");

    // timegm will interpret the tm as UTC and convert it to a time_t
    return timegm(&timeStruct);
}

string nawa::make_smtp_time(time_t time1) {
    stringstream smtpTime;
    tm ltime;
    localtime_r(&time1, &ltime);
    smtpTime << getDayOfWeek(ltime.tm_wday) << put_time(&ltime, ", %e ") << getMonth(ltime.tm_mon);
    smtpTime << put_time(&ltime, " %Y %H:%M:%S %z");

    return smtpTime.str();
}

time_t nawa::read_smtp_time(const string &smtpTime) {
    string smtpTimeM = smtpTime;
    tm timeStruct;

    // there seems to be a bug in get_time, %e parsing with leading space does not work, so this fails for
    // days of month < 10:
    //timeStream >> get_time(&timeStruct, "%a, %e %b %Y %H:%M:%S %z");

    // dirty hack
    if (smtpTimeM.length() > 5 && smtpTimeM[5] == ' ') {
        smtpTimeM[5] = '0';
    }
    istringstream timeStream(smtpTimeM);
    timeStream >> get_time(&timeStruct, "%a, %d %b %Y %H:%M:%S %z");

    // timegm will create a time_t, but does not honor the time zone, unfortunately (not part of tm)
    time_t unixTime = timegm(&timeStruct);

    // so we'll have to add/subtract the difference manually
    if (smtpTimeM.length() > 30) {
        int tzAdjust = smtpTimeM[26] == '-' ? 1 : -1;
        int tzH = stoi(smtpTimeM.substr(27, 2));
        int tzM = stoi(smtpTimeM.substr(29, 2));
        unixTime += tzAdjust * (tzH * 3600 + tzM * 60);
    }

    // mktime will interpret the tm as local time and convert it to a time_t
    return unixTime;
}

vector<string> nawa::split_string(string str, char delimiter, bool ignoreEmpty) {
    vector<string> ret;
    for (size_t pos = 0; !str.empty();) {
        pos = str.find_first_of(delimiter);
        auto token = str.substr(0, pos);
        if (!ignoreEmpty || !token.empty()) {
            ret.push_back(str.substr(0, pos));
        }
        if (pos < str.length()) {
            str = str.substr(pos + 1);
        } else {
            break;
        }
    }
    return ret;
}

string nawa::merge_path(const vector<string> &path) {
    if (path.empty()) {
        return "/";
    }
    stringstream stringPath;
    for (auto const &e: path) {
        stringPath << '/' << e;
    }
    return stringPath.str();
}

vector<string> nawa::split_path(const string &pathString) {
    // remove query string
    string rawPath = pathString.substr(0, pathString.find('?'));
    return split_string(rawPath, '/', true);
}

string nawa::convert_line_endings(const string &in, const string &ending) {
    stringstream ret;
    for (const auto &c: in) {
        if (c == '\n') ret << ending;
        else if (c != '\r') ret << c;
    }
    return ret.str();
}

string nawa::get_file_contents(const string &path) {
    // open file as binary
    ifstream f(path, ifstream::binary);

    // throw exception if file cannot be opened
    if (!f) {
        throw Exception(__PRETTY_FUNCTION__, 1, "Cannot open file for reading");
    }

    // get file size
    f.seekg(0, ios::end);
    long fs = f.tellg();
    f.seekg(0);

    // load to string
    string ret(static_cast<unsigned long>(fs), '\0');
    f.read(&ret[0], fs);

    return ret;
}

string nawa::string_replace(string input, const unordered_map<char, char> &patterns) {
    for (const auto &[key, val]: patterns) {
        replace(input.begin(), input.end(), key, val);
    }
    return input;
}

string nawa::string_replace(string input, const unordered_map<string, string> &patterns) {
    for (const auto &[key, val]: patterns) {
        for (size_t pos = input.find(key); pos != string::npos;) {
            input.replace(pos, key.length(), val);
            pos = input.find(key, pos + val.length());
        }
    }
    return input;
}

unordered_multimap<string, string> nawa::split_query_string(const string &queryString) {
    string qs;
    size_t qmrkPos = queryString.find_first_of('?');
    unordered_multimap<string, string> ret;
    if (qmrkPos != string::npos && queryString.length() > qmrkPos) {
        qs = queryString.substr(qmrkPos + 1);
    } else if (qmrkPos == string::npos) {
        qs = queryString;
    }
    auto pairs = split_string(qs, '&', true);
    for (auto const &p: pairs) {
        size_t eqPos = p.find_first_of('=');
        string k = p.substr(0, eqPos);
        string v = (eqPos < p.length() - 1) ? Encoding::urlDecode(p.substr(eqPos + 1)) : "";
        ret.insert({k, v});
    }
    return ret;
}

unordered_map<string, string> nawa::parse_headers(string rawHeaders) {
    unordered_map<string, string> ret;
    // filter out carriage returns
    boost::erase_all(rawHeaders, "\r");
    // split
    auto lines = split_string(rawHeaders, '\n', true);
    for (auto const &line: lines) {
        auto colonPos = line.find_first_of(':');
        if (line.length() < colonPos + 2) {
            continue;
        }
        auto key = to_lowercase(line.substr(0, colonPos));
        auto val = line.substr(colonPos + 1);
        boost::trim_left(val);
        ret[key] = val;
    }
    return ret;
}

unordered_multimap<std::string, std::string> nawa::parse_cookies(const string &rawCookies) {
    unordered_multimap<std::string, std::string> ret;
    // split by ;
    auto cookies = split_string(rawCookies, ';', true);
    for (auto c: cookies) {
        // remove whitespaces
        boost::trim(c);
        // key and value
        auto eqPos = c.find_first_of('=');
        if (c.length() < eqPos + 2) {
            continue;
        }
        auto key = c.substr(0, eqPos);
        auto val = c.substr(eqPos + 1);
        ret.insert({key, val});
    }
    return ret;
}
