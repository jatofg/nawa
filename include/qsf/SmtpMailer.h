/**
 * \file SmtpMailer.h
 * \brief For establishing a connection to an SMTP server and sending emails.
 */

#ifndef QSF_SMTPMAILER_H
#define QSF_SMTPMAILER_H

#include <string>
#include <qsf/Email.h>

namespace Qsf {
    class SmtpMailer {
        struct QueueElem {
            std::shared_ptr<const Email> email;
            std::shared_ptr<const EmailAddress> from;
            EmailAddress to;
            ReplacementRules replacementRules;
        };

        std::string serverUrl;
        unsigned int serverPort;
        enum TlsMode {
            NONE,
            SMTPS,
            TRY_STARTTLS,
            REQUIRE_STARTTLS
        } tlsMode;
        bool verifyTlsCert;
        std::string authUsername;
        std::string authPassword;
        bool connected = false;
        std::vector<QueueElem> queue;
    public:
        /**
         * Construct an SmtpMailer object and optionally set the connection and authentication properties. Constructing
         * the object will not establish a connection to the SMTP server yet.
         * @param _serverUrl URL of the server to use.
         * @param _serverPort
         * @param _tlsMode
         * @param _verifyTlsCert
         * @param _authUsername
         * @param _authPassword
         */
        explicit SmtpMailer(std::string _serverUrl = "localhost", unsigned int _serverPort = 25, TlsMode _tlsMode = NONE,
                bool _verifyTlsCert = true, std::string _authUsername = "", std::string _authPassword = "");
        void setServer(std::string _serverUrl, unsigned int _serverPort = 25, TlsMode _tlsMode = NONE,
                bool _verifyTlsCert = true);
        void setAuth(std::string _authUsername, std::string _authPassword);
        // TODO send emails async?
        // TODO send multiple mails in one SMTP connection?
        // TODO attachments, MIME in general, ...
        // TODO signing and encryption?
        void enqueue(std::shared_ptr<Email> email, EmailAddress to = EmailAddress(), EmailAddress from = EmailAddress());
        void bulkEnqueue(std::shared_ptr<Email> email, std::vector<EmailAddress> recipients,
                EmailAddress from, ReplacementRules replacementRules = ReplacementRules());
        void clearQueue();
        void processQueue();
    };
}

#endif //QSF_SMTPMAILER_H
