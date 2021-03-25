/**
 * \file SmtpMailer.cpp
 * \brief Implementation of the SmtpMailer class.
 */

/*
 * Copyright (C) 2019-2021 Tobias Flaig.
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

#include <curl/curl.h>
#include <nawa/Exception.h>
#include <nawa/mail/EmailAddress.h>
#include <nawa/mail/SmtpMailer.h>
#include <nawa/util/crypto.h>
#include <nawa/util/utils.h>
#include <random>

using namespace nawa;
using namespace std;

namespace {
    /**
     * Check for the existence of the obligatory "Date" and "From" headers in the email, and set them if non-existent.
     * Also adds a "Message-ID" header if necessary and possible.
     * @param email Email to check and modify.
     * @param from EmailAddress object to set the From header from, if necessary.
     */
    void addMissingHeaders(shared_ptr<mail::Email>& email, shared_ptr<mail::EmailAddress> const& from) {
        if (!email->headers().count("Date")) {
            email->headers()["Date"] = utils::makeSmtpTime(time(nullptr));
        }
        if (!email->headers().count("From") && !from->address().empty()) {
            email->headers()["From"] = from->get();
        }
        unsigned long atPos;
        if (!email->headers().count("Message-ID") && !from->address().empty() && (atPos = from->address().find_last_of('@')) != string::npos) {
            stringstream mid;
            stringstream base;
            random_device rd;
            timespec mtime;
            clock_gettime(CLOCK_REALTIME, &mtime);
            base << mtime.tv_sec << mtime.tv_nsec << from->address() << rd();
            mid << '<' << crypto::md5(base.str(), true) << '@' << from->address().substr(atPos + 1) << '>';
            email->headers()["Message-ID"] = mid.str();
        }
    }

    /**
     * An element of the sending queue.
     */
    struct QueueElem {
        std::shared_ptr<mail::Email const> email;
        std::shared_ptr<mail::EmailAddress const> from;
        std::vector<mail::EmailAddress> recipients;
        std::shared_ptr<mail::ReplacementRules> replacementRules;
    };
}// namespace

struct mail::SmtpMailer::Data {
    std::string serverDomain;
    unsigned int serverPort;
    TlsMode serverTlsMode;
    bool verifyServerTlsCert;
    std::string authUsername;
    std::string authPassword;
    long connectionTimeout;
    std::vector<QueueElem> queue;

    Data(string serverDomain, unsigned int serverPort, TlsMode serverTlsMode, bool verifyServerTlsCert,
         string authUsername, string authPassword, long connectionTimeout) : serverDomain(move(serverDomain)),
                                                                             serverPort(serverPort),
                                                                             serverTlsMode(serverTlsMode),
                                                                             verifyServerTlsCert(verifyServerTlsCert),
                                                                             authUsername(move(authUsername)),
                                                                             authPassword(move(authPassword)),
                                                                             connectionTimeout(connectionTimeout) {}
};

NAWA_DEFAULT_DESTRUCTOR_IMPL_WITH_NS(mail, SmtpMailer)

mail::SmtpMailer::SmtpMailer(string serverDomain, unsigned int serverPort, SmtpMailer::TlsMode serverTlsMode,
                             bool verifyServerTlsCert, string authUsername, string authPassword,
                             long connectionTimeout) {
    data = make_unique<Data>(move(serverDomain), serverPort, serverTlsMode, verifyServerTlsCert, move(authUsername),
                             move(authPassword), connectionTimeout);
}

void mail::SmtpMailer::setServer(string domain, unsigned int port, SmtpMailer::TlsMode tlsMode, bool verifyTlsCert) {
    data->serverDomain = move(domain);
    data->serverPort = port;
    data->serverTlsMode = tlsMode;
    data->verifyServerTlsCert = verifyTlsCert;
}

void mail::SmtpMailer::setAuth(string username, string password) {
    data->authUsername = move(username);
    data->authPassword = move(password);
}

void mail::SmtpMailer::setConnectionTimeout(long timeout) {
    data->connectionTimeout = timeout;
}

void mail::SmtpMailer::enqueue(shared_ptr<Email> email, EmailAddress to, shared_ptr<EmailAddress> from,
                               shared_ptr<ReplacementRules> replacementRules) {
    bulkEnqueue(move(email), vector<EmailAddress>({move(to)}), move(from),
                move(replacementRules));
}

void mail::SmtpMailer::bulkEnqueue(shared_ptr<Email> email, vector<EmailAddress> recipients,
                                   shared_ptr<EmailAddress> from, shared_ptr<ReplacementRules> replacementRules) {
    addMissingHeaders(email, from);
    data->queue.push_back(QueueElem{.email = move(email), .from = move(from), .recipients = move(recipients), .replacementRules = move(replacementRules)});
}

void mail::SmtpMailer::clearQueue() {
    data->queue.clear();
}

void mail::SmtpMailer::processQueue() const {
    CURL* curl;
    CURLcode res = CURLE_OK;

    curl = curl_easy_init();
    if (curl) {

        // authentication, if requested
        if (!data->authUsername.empty()) {
            curl_easy_setopt(curl, CURLOPT_USERNAME, data->authUsername.c_str());
            if (!data->authPassword.empty())
                curl_easy_setopt(curl, CURLOPT_PASSWORD, data->authPassword.c_str());
        }

        // build URL for curl
        {
            stringstream curlUrl;
            if (data->serverTlsMode == TlsMode::SMTPS) {
                curlUrl << "smtps://";
            } else {
                curlUrl << "smtp://";
            }
            curlUrl << data->serverDomain << ":" << data->serverPort;
            curl_easy_setopt(curl, CURLOPT_URL, curlUrl.str().c_str());
        }

        // set up TLS
        if (data->serverTlsMode == TlsMode::REQUIRE_STARTTLS) {
            curl_easy_setopt(curl, CURLOPT_USE_SSL, (long) CURLUSESSL_ALL);
        } else if (data->serverTlsMode == TlsMode::TRY_STARTTLS) {
            curl_easy_setopt(curl, CURLOPT_USE_SSL, (long) CURLUSESSL_TRY);
        }
        if (data->serverTlsMode != TlsMode::NONE && !data->verifyServerTlsCert) {
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        }

        //        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        FILE* devNull = fopen("/dev/null", "wb");
        curl_easy_setopt(curl, CURLOPT_STDERR, devNull);

        // connection timeout
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, data->connectionTimeout);

        // iterate queue
        for (const auto& mail : data->queue) {

            // set sender
            curl_easy_setopt(curl, CURLOPT_MAIL_FROM, mail.from->get(false).c_str());

            // set recipients
            curl_slist* recipients = nullptr;
            for (const auto& to : mail.recipients) {
                recipients = curl_slist_append(recipients, to.get(false).c_str());
            }
            curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

            // specify how to read the mail data
            string payload = mail.email->getRaw(mail.replacementRules);
            // fmemopen will create a FILE* to read from the string (curl expects that, unfortunately)
            FILE* payloadFile = fmemopen((void*) payload.c_str(), payload.length(), "r");
            curl_easy_setopt(curl, CURLOPT_READDATA, (void*) payloadFile);
            curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

            // perform sending
            res = curl_easy_perform(curl);

            // free memory
            curl_slist_free_all(recipients);
            fclose(payloadFile);

            // check for errors, throw exception if one happens
            if (res != CURLE_OK) {
                curl_easy_cleanup(curl);
                throw Exception(__PRETTY_FUNCTION__, 1,
                                string("CURL error: ") + curl_easy_strerror(res));
            }
        }

        curl_easy_cleanup(curl);
        fclose(devNull);
    }
}
