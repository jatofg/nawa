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
#include "qsf/Utils.h"

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
