#ifndef H_CQ_NCONF

#define H_CQ_NCONF

#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

struct CqNodeConfiguration
{
    bool configured = 0;
    char accessPoint[64] = "";
    char password[64] = "";
    char thumbprint[64] = "";
    char server[64] = "";
    uint16_t port = 0;
};

class CqNodeConfigurator
{
private:
    CqNodeConfiguration _configuration;
    ESP8266WebServer _server;

    bool _debug;

    void log(String message)
    {
        if (!_debug)
        {
            return;
        }

        Serial.println(message);
    }

public:
    CqNodeConfigurator(bool debug) : _server(80)
    {
        _debug = debug;

        if (_debug)
        {
            Serial.begin(9600);
        }

        log("Loading EEPROM");

        EEPROM.begin(259);
    }

    CqNodeConfigurator()
    {
        CqNodeConfigurator(false);
    }

    // this has to be called before Begin()
    void Clear()
    {
        log("Clearing configuration");

        EEPROM.put(0, _configuration);
        EEPROM.commit();

        log("Configuration cleared");
    }

    void Begin()
    {
        if (_configuration.configured)
        {
            log("Already configured");
            // connect to wifi here
            return;
        }

        log("Not configured");

        // start web server for configuration purposes

        String apName = "CQ-" + WiFi.macAddress();

        log("Starting AP: " + apName);

        IPAddress ipLocal(10, 11, 12, 13);
        IPAddress ipGateway(10, 11, 12, 12);
        IPAddress ipSubnet(255, 255, 255, 0);

        WiFi.softAPConfig(ipLocal, ipGateway, ipSubnet);

        while (!WiFi.softAP(apName))
        {
            delay(1000);
        }

        log("AP started, IP: " + WiFi.softAPIP().toString());
        log("Starting HTTP server");

        _server.on("/", [this]() {
            log("HTTP: / requested");

            String html = "<html><head> <title>CQ Device Configurator</title> <style>body{font-size: 16px; font-family: sans-serif;}</style></head><body> <form method=\"GET\" action=\"/\"> <table> <tr> <td>Access point name</td><td><input id=\"accessPointName\" type=\"text\"/></td></tr><tr> <td>Password</td><td><input id=\"password\" type=\"text\"/></td></tr><tr> <td>SSL thumbprint</td><td><input id=\"thumbprint\" type=\"text\"/></td></tr><tr> <td>Server hostname</td><td><input id=\"server\" type=\"text\"/></td></tr><tr> <td>Server port</td><td><input id=\"port\" type=\"text\"/></td></tr><tr> <td>&nbsp;</td><td><button type=\"button\">Submit</button></td></tr></table> </form></body></html>";
               
            _server.send(200, "text/html", html);
        });

        _server.begin();

        log("HTTP server started");

        while (!_configuration.configured)
        {
            _server.handleClient();
        }
    }

    CqNodeConfiguration GetConfiguration()
    {
        return _configuration;
    }
};

#endif