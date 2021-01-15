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

#include <nawa/SmtpMailer.h>
#include <nawa/Utils.h>
#include <curl/curl.h>
#include <nawa/Exception.h>
#include <random>
#include <nawa/Crypto.h>

using namespace nawa;
using namespace std;

namespace {
    /**
     * Check for the existence of the obligatory "Date" and "From" headers in the email, and set them if non-existent.
     * Also adds a "Message-ID" header if necessary and possible.
     * @param email Email to check and modify.
     * @param from EmailAddress object to set the From header from, if necessary.
     */
    void addMissingHeaders(shared_ptr<Email> &email, const shared_ptr<EmailAddress> &from) {
        if (!email->headers.count("Date")) {
            email->headers["Date"] = make_smtp_time(time(nullptr));
        }
        if (!email->headers.count("From") && !from->address.empty()) {
            email->headers["From"] = from->get();
        }
        unsigned long atPos;
        if (!email->headers.count("Message-ID") && !from->address.empty()
            && (atPos = from->address.find_last_of('@')) != string::npos) {
            stringstream mid;
            stringstream base;
            random_device rd;
            timespec mtime;
            clock_gettime(CLOCK_REALTIME, &mtime);
            base << mtime.tv_sec << mtime.tv_nsec << from->address << rd();
            mid << '<' << Crypto::md5(base.str(), true) << '@' << from->address.substr(atPos + 1) << '>';
            email->headers["Message-ID"] = mid.str();
        }
    }
}

SmtpMailer::SmtpMailer(string serverDomain, unsigned int serverPort, SmtpMailer::TlsMode serverTlsMode,
                       bool verifyServerTlsCert, string authUsername, string authPassword,
                       long connectionTimeout)
        : serverDomain(move(serverDomain)), serverPort(serverPort), serverTlsMode(serverTlsMode),
          verifyServerTlsCert(verifyServerTlsCert),
          authUsername(move(authUsername)), authPassword(move(authPassword)), connectionTimeout(connectionTimeout) {}

void
SmtpMailer::setServer(string domain, unsigned int port, SmtpMailer::TlsMode tlsMode, bool verifyTlsCert) {
    serverDomain = move(domain);
    serverPort = port;
    serverTlsMode = tlsMode;
    verifyServerTlsCert = verifyTlsCert;
}

void SmtpMailer::setAuth(string username, string password) {
    authUsername = move(username);
    authPassword = move(password);
}

void SmtpMailer::setConnectionTimeout(long timeout) {
    connectionTimeout = timeout;
}

void SmtpMailer::enqueue(shared_ptr<Email> email, EmailAddress to, shared_ptr<EmailAddress> from,
                         shared_ptr<ReplacementRules> replacementRules) {
    bulkEnqueue(move(email), vector<EmailAddress>({move(to)}), move(from),
                move(replacementRules));
}

void SmtpMailer::bulkEnqueue(shared_ptr<Email> email, vector<EmailAddress> recipients,
                             shared_ptr<EmailAddress> from, shared_ptr<ReplacementRules> replacementRules) {
    addMissingHeaders(email, from);
    queue.push_back(QueueElem{.email=move(email), .from=move(from), .recipients=move(recipients),
            .replacementRules=move(replacementRules)});
}

void SmtpMailer::clearQueue() {
    queue.clear();
}

void SmtpMailer::processQueue() const {
    CURL *curl;
    CURLcode res = CURLE_OK;

    curl = curl_easy_init();
    if (curl) {

        // authentication, if requested
        if (!authUsername.empty()) {
            curl_easy_setopt(curl, CURLOPT_USERNAME, authUsername.c_str());
            if (!authPassword.empty())
                curl_easy_setopt(curl, CURLOPT_PASSWORD, authPassword.c_str());
        }

        // build URL for curl
        {
            stringstream curlUrl;
            if (serverTlsMode == TlsMode::SMTPS) {
                curlUrl << "smtps://";
            } else {
                curlUrl << "smtp://";
            }
            curlUrl << serverDomain << ":" << serverPort;
            curl_easy_setopt(curl, CURLOPT_URL, curlUrl.str().c_str());
        }

        // set up TLS
        if (serverTlsMode == TlsMode::REQUIRE_STARTTLS) {
            curl_easy_setopt(curl, CURLOPT_USE_SSL, (long) CURLUSESSL_ALL);
        } else if (serverTlsMode == TlsMode::TRY_STARTTLS) {
            curl_easy_setopt(curl, CURLOPT_USE_SSL, (long) CURLUSESSL_TRY);
        }
        if (serverTlsMode != TlsMode::NONE && !verifyServerTlsCert) {
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        }

//        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        FILE *devNull = fopen("/dev/null", "wb");
        curl_easy_setopt(curl, CURLOPT_STDERR, devNull);

        // connection timeout
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, connectionTimeout);

        // iterate queue
        for (const auto &mail: queue) {

            // set sender
            curl_easy_setopt(curl, CURLOPT_MAIL_FROM, mail.from->get(false).c_str());

            // set recipients
            curl_slist *recipients = nullptr;
            for (const auto &to: mail.recipients) {
                recipients = curl_slist_append(recipients, to.get(false).c_str());
            }
            curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

            // specify how to read the mail data
            string payload = mail.email->getRaw(mail.replacementRules);
            // fmemopen will create a FILE* to read from the string (curl expects that, unfortunately)
            FILE *payloadFile = fmemopen((void *) payload.c_str(), payload.length(), "r");
            curl_easy_setopt(curl, CURLOPT_READDATA, (void *) payloadFile);
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
