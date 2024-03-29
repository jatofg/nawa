/*
 * Copyright (C) 2019-2022 Tobias Flaig.
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

/**
 * \file SmtpMailer.h
 * \brief For establishing a connection to an SMTP server and sending emails.
 */

#ifndef NAWA_SMTPMAILER_H
#define NAWA_SMTPMAILER_H

#include <nawa/internal/macros.h>
#include <nawa/mail/Email/Email.h>
#include <nawa/mail/EmailAddress.h>
#include <string>

namespace nawa::mail {
    class SmtpMailer {
        NAWA_PRIVATE_DATA()

    public:
        /**
         * How TLS should be used when connecting to an SMTP server.
         * - NONE: Use an unencrypted connection.
         * - SMTPS: Use SMTPS (SMTP over TLS).
         * - TRY_STARTTLS: Try to switch to a TLS connection by using the STARTTLS command, if that fails, use an
         * unencrypted connection. This might be a security risk, better don't use it.
         * - REQUIRE_STARTTLS: Use the STARTTLS command to establish an encrypted connection, abort if not possible.
         */
        enum class TlsMode {
            NONE,
            SMTPS,
            TRY_STARTTLS,
            REQUIRE_STARTTLS
        };

        NAWA_DEFAULT_DESTRUCTOR_DEF(SmtpMailer);

        /**
         * Construct an SmtpMailer object and optionally set the connection and authentication properties. Constructing
         * the object will not establish a connection to the SMTP server yet.
         * @param serverDomain Domain name or IP address of the SMTP server to use. IPv6 addresses have to be enclosed
         * in brackets. This value will be used to assemble the SMTP(S) URL and will not be checked for validity.
         * @param serverPort Port of the SMTP server.
         * @param serverTlsMode How TLS should be used, see TlsMode struct.
         * @param verifyServerTlsCert Whether to verify the validity of the SMTP server's TLS certificate, if TLS is used
         * (highly recommended).
         * @param authUsername Username for authentication.
         * @param authPassword Password for authentication.
         * @param connectionTimeout Timeout for SMTP connection attempts in milliseconds.
         */
        explicit SmtpMailer(std::string serverDomain = "localhost", unsigned int serverPort = 25,
                            TlsMode serverTlsMode = TlsMode::NONE, bool verifyServerTlsCert = true,
                            std::string authUsername = "",
                            std::string authPassword = "", long connectionTimeout = 10000);

        /**
         * Set the connection properties. This will not establish a connection to the SMTP server yet.
         * @param domain Domain name or IP address of the SMTP server to use. IPv6 addresses have to be enclosed
         * in brackets. This value will be used to assemble the SMTP(S) URL and will not be checked for validity.
         * @param port Port of the SMTP server.
         * @param tlsMode How TLS should be used, see TlsMode struct.
         * @param verifyTlsCert Whether to verify the validity of the SMTP server's TLS certificate, if TLS is used
         * (highly recommended).
         */
        void setServer(std::string domain, unsigned int port = 25, TlsMode tlsMode = TlsMode::NONE,
                       bool verifyTlsCert = true);

        /**
         * Set the authentication parameters for the SMTP connection.
         * @param username Username for authentication.
         * @param password Password for authentication.
         */
        void setAuth(std::string username, std::string password);

        /**
         * Set the timeout for SMTP connection attempts.
         * @param timeout Timeout for SMTP connection attempts in milliseconds.
         */
        void setConnectionTimeout(long timeout);

        /**
         * Add an email to the sending queue. The email will be sent upon calling processQueue().
         * @param email The Email object representing the email to be enqueued. This function will expect a shared_ptr
         * to the email object, this allows efficient memory management: If the same email shall be sent to a lot of
         * recipients, the same shared_ptr object can be passed to all calls to enqueue(), which means that only the
         * pointer will be copied, but the Email object has to be kept in memory only once. To personalize the emails,
         * you can instead use the replacement rules - one rule set can be saved for each recipient. Please note that
         * this function might modify your Email object (it will set the obligatory "Date" and "From" headers, if they
         * are not there yet, as well as the "Message-ID" header, if possible).
         * @param to The recipient of the email (envelope). Please note that this will not modify the headers of your
         * email (as shown in the email app of the recipient), those have to be set inside of the Email object. This is
         * the address the mail will be actually sent to.
         * @param from The sender of the email (envelope, also known as the return-path). It should be an email address
         * that the SMTP server "owns", i.e., has the permission to send emails from this address (the spam filter of
         * the recipient normally uses this address to classify the email). This address should be set in all cases,
         * except for those noted in RFC 5321, section 4.5.5. This function will take a shared_ptr, too, as the
         * sender of mass emails is usually always the same, so you can reuse the same object for all recipients.
         * While this address may be different from the one in the "From" header of the email (which is required),
         * this function will set the "From" header from this address in case it doesn't exist in the email yet.
         * @param replacementRules An optional set of replacement rules. It is a map with a string as key (the text
         * to be replaced) and another string as value (the text to replace it with). This set is saved and evaluated
         * for each recipient individually, so this is a relatively memory-efficient way to personalize emails.
         */
        void enqueue(std::shared_ptr<Email> email, EmailAddress to,
                     std::shared_ptr<EmailAddress> from,
                     std::shared_ptr<ReplacementRules> replacementRules = std::shared_ptr<ReplacementRules>());

        /**
         * This function will enqueue an email for a list of recipients. This improves efficiency, but it doesn't
         * allow the application of replacement rules for each recipient individually. Replacement rules can still be
         * used, but the replacements will be the same for all recipients (if you need personalization, you
         * should call enqueue() for each recipient and pass individual rule sets; the email object can be the same
         * nevertheless, thanks to the shared_ptr).
         * @param email The Email object (for comments, look at the docs for enqueue()).
         * @param recipients The list of recipients as EmailAddress objects (envelope, see enqueue() docs).
         * @param from The sender of the email (envelope, see enqueue() docs).
         * @param replacementRules An optional set of replacement rules (see enqueue() docs). They will be applied
         * to the email once and do not offer personalization to individual recipients.
         */
        void bulkEnqueue(std::shared_ptr<Email> email, std::vector<EmailAddress> recipients,
                         std::shared_ptr<EmailAddress> from,
                         std::shared_ptr<ReplacementRules> replacementRules = std::shared_ptr<ReplacementRules>());

        /**
         * Clear the email queue.
         */
        void clearQueue();

        /**
         * Process the queue, i.e., establish an SMTP connection and send all emails in the queue. This function will
         * not modify (and therefore not clear) the queue. In case of errors, a nawa::Exception with error code 1 will
         * be thrown.
         */
        void processQueue() const;
    };
}// namespace nawa::mail

#endif//NAWA_SMTPMAILER_H
