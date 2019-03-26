/**
 * \file SmtpMailer.h
 * \brief For establishing a connection to an SMTP server and sending emails.
 */

#ifndef QSF_SMTPMAILER_H
#define QSF_SMTPMAILER_H

#include <string>
#include <qsf/Types/MimeEmail.h>

namespace Qsf {
    class SmtpMailer {
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
        void enqueue(Types::MimeEmail email, Types::EmailAddress to = Types::EmailAddress(),
                Types::EmailAddress from = Types::EmailAddress());
        void bulkEnqueue(Types::MimeEmail email, std::vector<Types::EmailAddress> recipients,
                std::unordered_map<std::string, std::string> replacementRules = std::unordered_map<std::string, std::string>());
        void clearQueue();
        void processQueue();
    };
}

#endif //QSF_SMTPMAILER_H
