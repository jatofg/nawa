/**
 * \file SmtpMailer.cpp
 * \brief Implementation of the SmtpMailer class.
 */

#include <qsf/SmtpMailer.h>

Qsf::SmtpMailer::SmtpMailer(std::string _serverUrl, unsigned int _serverPort, Qsf::SmtpMailer::TlsMode _tlsMode,
                            bool _verifyTlsCert, std::string _authUsername, std::string _authPassword)
        : serverPort(_serverPort), tlsMode(_tlsMode), verifyTlsCert(_verifyTlsCert) {
    serverUrl = std::move(_serverUrl);
    authUsername = std::move(_authUsername);
    authPassword = std::move(_authPassword);
}

void Qsf::SmtpMailer::setServer(std::string _serverUrl, unsigned int _serverPort, Qsf::SmtpMailer::TlsMode _tlsMode,
                                bool _verifyTlsCert) {
    serverUrl = std::move(_serverUrl);
    serverPort = _serverPort;
    tlsMode = _tlsMode;
    verifyTlsCert = _verifyTlsCert;
}

void Qsf::SmtpMailer::setAuth(std::string _authUsername, std::string _authPassword) {
    authUsername = std::move(_authUsername);
    authPassword = std::move(_authPassword);
}

void Qsf::SmtpMailer::enqueue(std::shared_ptr<Email> email, EmailAddress to, EmailAddress from) {
    queue.push_back(QueueElem{.email=email,.from=std::make_shared<EmailAddress>(from),.to=std::move(to)});
}

void Qsf::SmtpMailer::bulkEnqueue(std::shared_ptr<Email> email, std::vector<EmailAddress> recipients,
                                  EmailAddress from, ReplacementRules replacementRules) {

}

void Qsf::SmtpMailer::clearQueue() {

}
