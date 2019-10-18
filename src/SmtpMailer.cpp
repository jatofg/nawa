/**
 * \file SmtpMailer.cpp
 * \brief Implementation of the SmtpMailer class.
 */

/*
 * Copyright (C) 2019 Tobias Flaig.
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

#include <nawa/SmtpMailer.h>
#include <nawa/Utils.h>
#include <curl/curl.h>
#include <nawa/UserException.h>
#include <random>
#include <nawa/Crypto.h>

namespace {
    /**
     * Check for the existence of the obligatory "Date" and "From" headers in the email, and set them if non-existent.
     * Also adds a "Message-ID" header if necessary and possible.
     * @param email Email to check and modify.
     * @param from EmailAddress object to set the From header from, if necessary.
     */
    void addMissingHeaders(std::shared_ptr<nawa::Email> &email, const std::shared_ptr<nawa::EmailAddress> &from) {
        if(!email->headers.count("Date")) {
            email->headers["Date"] = nawa::make_smtp_time(time(nullptr));
        }
        if(!email->headers.count("From") && !from->address.empty()) {
            email->headers["From"] = from->get();
        }
        unsigned long atPos;
        if(!email->headers.count("Message-ID") && !from->address.empty()
                && (atPos = from->address.find_last_of('@')) != std::string::npos) {
            std::stringstream mid;
            std::stringstream base;
            std::random_device rd;
            timespec mtime;
            clock_gettime(CLOCK_REALTIME, &mtime);
            base << mtime.tv_sec << mtime.tv_nsec << from->address << rd();
            mid << '<' << nawa::Crypto::md5(base.str(), true) << '@' << from->address.substr(atPos+1) << '>';
            email->headers["Message-ID"] = mid.str();
        }
    }
}

nawa::SmtpMailer::SmtpMailer(std::string _serverDomain, unsigned int _serverPort, nawa::SmtpMailer::TlsMode _tlsMode,
                            bool _verifyTlsCert, std::string _authUsername, std::string _authPassword)
        : serverPort(_serverPort), tlsMode(_tlsMode), verifyTlsCert(_verifyTlsCert) {
    serverDomain = std::move(_serverDomain);
    authUsername = std::move(_authUsername);
    authPassword = std::move(_authPassword);
}

void nawa::SmtpMailer::setServer(std::string _serverDomain, unsigned int _serverPort, nawa::SmtpMailer::TlsMode _tlsMode,
                                bool _verifyTlsCert) {
    serverDomain = std::move(_serverDomain);
    serverPort = _serverPort;
    tlsMode = _tlsMode;
    verifyTlsCert = _verifyTlsCert;
}

void nawa::SmtpMailer::setAuth(std::string _authUsername, std::string _authPassword) {
    authUsername = std::move(_authUsername);
    authPassword = std::move(_authPassword);
}

void nawa::SmtpMailer::enqueue(std::shared_ptr<Email> email, EmailAddress to, std::shared_ptr<EmailAddress> from,
        ReplacementRules replacementRules) {
    bulkEnqueue(std::move(email), std::vector<EmailAddress>({std::move(to)}), std::move(from),
            std::move(replacementRules));
}

void nawa::SmtpMailer::bulkEnqueue(std::shared_ptr<Email> email, std::vector<EmailAddress> recipients,
                                  std::shared_ptr<EmailAddress> from, ReplacementRules replacementRules) {
    addMissingHeaders(email, from);
    std::unique_ptr<ReplacementRules> rulesPtr;
    if(!replacementRules.empty()) {
        rulesPtr = std::make_unique<ReplacementRules>(replacementRules);
    }
    queue.push_back(QueueElem{.email=std::move(email), .from=std::move(from), .recipients=std::move(recipients),
                              .replacementRules=std::move(rulesPtr)});
}

void nawa::SmtpMailer::clearQueue() {
    queue.clear();
}

void nawa::SmtpMailer::processQueue() const {
    CURL *curl;
    CURLcode res = CURLE_OK;

    curl = curl_easy_init();
    if(curl) {

        // authentication, if requested
        if(!authUsername.empty()) {
            curl_easy_setopt(curl, CURLOPT_USERNAME, authUsername.c_str());
            if(!authPassword.empty())
                curl_easy_setopt(curl, CURLOPT_PASSWORD, authPassword.c_str());
        }

        // build URL for curl
        {
            std::stringstream curlUrl;
            if(tlsMode == SMTPS) {
                curlUrl << "smtps://";
            }
            else {
                curlUrl << "smtp://";
            }
            curlUrl << serverDomain << ":" << serverPort;
            curl_easy_setopt(curl, CURLOPT_URL, curlUrl.str().c_str());
        }

        // set up TLS
        if(tlsMode == REQUIRE_STARTTLS) {
            curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
        }
        else if(tlsMode == TRY_STARTTLS) {
            curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_TRY);
        }
        if(tlsMode != NONE && !verifyTlsCert) {
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        }

//        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        FILE* devNull = fopen("/dev/null", "wb");
        curl_easy_setopt(curl, CURLOPT_STDERR, devNull);

        // iterate queue
        for(const auto &mail: queue) {

            // set sender
            curl_easy_setopt(curl, CURLOPT_MAIL_FROM, mail.from->get(false).c_str());

            // set recipients
            curl_slist *recipients = nullptr;
            for(const auto &to: mail.recipients) {
                recipients = curl_slist_append(recipients, to.get(false).c_str());
            }
            curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

            // specify how to read the mail data
            std::string payload = mail.email->getRaw(*mail.replacementRules);
            // fmemopen will create a FILE* to read from the string (curl expects that, unfortunately)
            FILE* payloadFile = fmemopen((void*)payload.c_str(), payload.length(), "r");
            curl_easy_setopt(curl, CURLOPT_READDATA, (void*)payloadFile);
            curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

            // perform sending
            res = curl_easy_perform(curl);

            // free memory
            curl_slist_free_all(recipients);
            fclose(payloadFile);

            // check for errors, throw exception if one happens
            if(res != CURLE_OK) {
                curl_easy_cleanup(curl);
                throw UserException("nawa::SmtpMailer::processQueue()", 1,
                        std::string("CURL error: ") + curl_easy_strerror(res));
            }

        }

        curl_easy_cleanup(curl);
        fclose(devNull);

    }

}
