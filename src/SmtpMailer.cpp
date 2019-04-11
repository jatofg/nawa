/**
 * \file SmtpMailer.cpp
 * \brief Implementation of the SmtpMailer class.
 */

#include <qsf/SmtpMailer.h>
#include <qsf/Utils.h>

namespace {
    /**
     * Check for the existence of the obligatory "From" and "Date" headers in the email, and set them if non-existent.
     * @param email Email to check and modify.
     * @param from EmailAddress object to set the From header from, if necessary.
     */
    void addMissingHeaders(std::shared_ptr<Qsf::Email> &email, const std::shared_ptr<Qsf::EmailAddress> &from) {
        if(!email->headers.count("From")) {
            email->headers["From"] = from->name.empty() ? from->address : from->name + " <" + from->address + ">";
        }
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
    std::unique_ptr<ReplacementRules> rulesPtr;
    addMissingHeaders(email, from);
    if(!replacementRules.empty()) {
        rulesPtr = std::make_unique<ReplacementRules>(replacementRules);
    }
    queue.push_back(QueueElem{.email=std::move(email), .from=std::move(from), .to=std::move(to),
                              .replacementRules=std::move(rulesPtr)});
}

void Qsf::SmtpMailer::bulkEnqueue(std::shared_ptr<Email> email, const std::vector<EmailAddress> &recipients,
                                  const std::shared_ptr<EmailAddress> &from) {
    addMissingHeaders(email, from);
    for(const auto &to: recipients) {
        queue.push_back(QueueElem{.email=email, .from=from, .to=to});
    }
}

void Qsf::SmtpMailer::clearQueue() {
    queue.clear();
}
