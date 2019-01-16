#include "app.h"

#include <iostream>
#include "qsf/Encoding.h"
#include "qsf/Session.h"
#include "qsf/Types/Universal.h"
#include "qsf/Crypto.h"
#include "qsf/Application.h"

using namespace Qsf;

int init() {
//    std::cout << "It works!" << std::endl;
//
//    std::string encoded = R"(&lt;input type=&quot;text&quot; value=&quot;t&auml;&Aopf;&#x1D538;&#120120;st&quot;&gt;)";
//    std::string decoded = R"(<input type="text" value="tä𝔸𝔸𝔸st">)";
//
//    std::string encodedDecoded = Encoding::htmlDecode(encoded);
//    if(decoded == encodedDecoded) {
//        std::cout << "yay!" << std::endl;
//    }
//    else {
//        std::cout <<  Encoding::htmlEncode(encodedDecoded) << std::endl;
//    }
//
//    std::string forUrl = "bla bla bla!??xyzäßédsfsdf ";
//    auto urlEncoded = Encoding::urlEncode(forUrl);
//    std::string reDecoded = Encoding::urlDecode(urlEncoded);
//    if(forUrl == reDecoded) {
//        std::cout << "yay2!" << std::endl;
//    }
//    else {
//        std::cout <<  Encoding::htmlEncode(reDecoded) << std::endl;
//    }
//
//    std::cout << urlEncoded << std::endl;
//
//    Types::Universal u1(std::string("test"));
//    Types::Universal u2;
//    u2 = 5;
//
//    std::cout << "u1 = " << u1.get<std::string>() << std::endl;
//
//    std::cout << "u2 = " << u2.get<int>() << std::endl;
//
//    u2.unset();
//
//    u2 = u1;
//    u1.unset();
//    Types::Universal u3 = u2;
//    u2 = std::string("test2xyz");
//
//    std::cout << "u2 = " << u2.get<std::string>() << std::endl;
//    std::cout << "u3 = " << u3.get<std::string>() << std::endl;
//    //std::cout << "u1 = " << u1.get<std::string>() << std::endl;
//
//    std::string md5test = "Hello wórld!";
//    std::cout << "MD5 of " << md5test << " is: " << Crypto::md5(md5test, true) << std::endl;
//    std::string hwHash = Crypto::passwordHash(md5test);
//    std::cout << "Its hash is: " << hwHash << std::endl;
//    std::cout << "Another one: " << Crypto::passwordHash(md5test, 10) << std::endl;
//    std::cout << "Is 'Hello world!' correct? " << (Crypto::passwordVerify("Hello world!", hwHash) ? "yes" : "no") << std::endl;
//    std::cout << "Is '" << md5test << "' correct? "  << (Crypto::passwordVerify("Hello wórld!", hwHash) ? "yes" : "no") << std::endl;

    return 0;
}

int handleRequest(Connection &connection) {

    // start a session
    //connection.session.start();

    connection.setCookie("TEST", Cookie("test"));
    Cookie policy;
    policy.httpOnly = true;
    connection.setCookiePolicy(policy);

    std::string encoded = R"(&lt;input type=&quot;text&quot; value=&quot;t&auml;&Aopf;&#x1D538;&#120120;st&quot;&gt;)";
    std::string decoded = R"(<input type="text" value="tä𝔸𝔸𝔸st">)";

    connection.response << "<!DOCTYPE html>\n"
                  "<html><head><title>Test</title></head><body>"
                  "<p>Hello World! HTML string: " << Encoding::htmlEncode(decoded, true) << "</p>"
                  "<p>Client IP: " << Encoding::htmlEncode(connection.request.env["remoteAddress"]) << "</p>";
    
    // alternative: connection.session["test"].isSet()
    if(connection.session.isSet("test")) {
        connection.response << "<p>Session available! Value: " << connection.session["test"].get<std::string>() << "</p>";
        connection.session.invalidate();
        connection.session.start();
        connection.session.set("test", Types::Universal(std::string("noch viel mehr blub")));
        
    }
    else {
        connection.session.set("test", Types::Universal(std::string("bla bla blub")));
        connection.response << "<p>There was no session yet, but now there should be one!" << "</p>";
    }

    std::string encodedDecoded = Encoding::htmlDecode(encoded);
    if(decoded == encodedDecoded) {
        connection.response << "<p>yay!</p>";
    }
    else {
        connection.response << "<p>" << Encoding::htmlEncode(encodedDecoded) <<"</p>";
    }

    connection.flushResponse();

    // wait a few secs, then print more
    //std::this_thread::sleep_for(std::chrono::seconds(3));

    connection.response << "<p>Hello World 2!</p>"
                  "</body></html>";

//    connection.flushResponse();

    return 0;
}
