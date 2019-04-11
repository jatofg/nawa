/**
 * \file SmtpMailer.cpp
 * \brief Implementation of the SmtpMailer class.
 */

#include <qsf/SmtpMailer.h>
#include <qsf/Utils.h>
#include <curl/curl.h>
#include <qsf/UserException.h>

namespace {
    /**
     * Check for the existence of the obligatory "Date" header in the email, and set it if non-existent.
     * @param email Email to check and modify.
     * @param from EmailAddress object to set the From header from, if necessary.
     */
    void addMissingHeaders(std::shared_ptr<Qsf::Email> &email, const std::shared_ptr<Qsf::EmailAddress> &from) {
        if(!email->headers.count("Date")) {
            email->headers["Date"] = Qsf::make_smtp_time(time(nullptr));
        }
    }
}

Qsf::SmtpMailer::SmtpMailer(std::string _serverDomain, unsigned int _serverPort, Qsf::SmtpMailer::TlsMode _tlsMode,
                            bool _verifyTlsCert, std::string _authUsername, std::string _authPassword)
        : serverPort(_serverPort), tlsMode(_tlsMode), verifyTlsCert(_verifyTlsCert) {
    serverDomain = std::move(_serverDomain);
    authUsername = std::move(_authUsername);
    authPassword = std::move(_authPassword);
}

void Qsf::SmtpMailer::setServer(std::string _serverDomain, unsigned int _serverPort, Qsf::SmtpMailer::TlsMode _tlsMode,
                                bool _verifyTlsCert) {
    serverDomain = std::move(_serverDomain);
    serverPort = _serverPort;
    tlsMode = _tlsMode;
    verifyTlsCert = _verifyTlsCert;
}

void Qsf::SmtpMailer::setAuth(std::string _authUsername, std::string _authPassword) {
    authUsername = std::move(_authUsername);
    authPassword = std::move(_authPassword);
}

void Qsf::SmtpMailer::enqueue(std::shared_ptr<Email> email, EmailAddress to, std::shared_ptr<EmailAddress> from,
        ReplacementRules replacementRules) {
    bulkEnqueue(std::move(email), std::vector<EmailAddress>({std::move(to)}), std::move(from),
            std::move(replacementRules));
}

void Qsf::SmtpMailer::bulkEnqueue(std::shared_ptr<Email> email, std::vector<EmailAddress> recipients,
                                  std::shared_ptr<EmailAddress> from, ReplacementRules replacementRules) {
    addMissingHeaders(email, from);
    std::unique_ptr<ReplacementRules> rulesPtr;
    if(!replacementRules.empty()) {
        rulesPtr = std::make_unique<ReplacementRules>(replacementRules);
    }
    queue.push_back(QueueElem{.email=std::move(email), .from=std::move(from), .recipients=std::move(recipients),
                              .replacementRules=std::move(rulesPtr)});
}

void Qsf::SmtpMailer::clearQueue() {
    queue.clear();
}

void Qsf::SmtpMailer::processQueue() const {
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
            curl_easy_setopt(curl, CURLOPT_READDATA, fmemopen((void*)payload.c_str(), payload.length(), "r"));

            // perform sending
            res = curl_easy_perform(curl);

            // free memory
            curl_slist_free_all(recipients);

            // check for errors, throw exception if one happens
            if(res != CURLE_OK) {
                curl_easy_cleanup(curl);
                throw UserException("Qsf::SmtpMailer::processQueue()", 1,
                        std::string("CURL error: ") + curl_easy_strerror(res));
            }

        }

        curl_easy_cleanup(curl);

    }

}
