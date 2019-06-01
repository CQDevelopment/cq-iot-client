#ifndef H_CQ_NODE

#define H_CQ_NODE

#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

struct CqNodeConfiguration
{
    bool configured = false;
    char ssid[128] = "";
    char password[128] = "";
    char server[128] = "";
    uint16_t port = 0;
};

class CqNode
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

    void awaitConfiguration()
    {
        if (_configuration.configured)
        {
            log("Already configured");

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

            String html = "<html><head><title>CQ Device Configurator</title><style>body { font-size: 16px; font-family: sans-serif; }</style></head><body><form method=\"POST\" action=\"/post\"><table><tr><td>SSID</td><td><input name=\"ssid\" type=\"text\" /></td></tr><tr><td>Password</td><td><input name=\"password\" type=\"password\" /></td></tr><tr><td>Server</td><td><input name=\"server\" type=\"text\" /></td></tr><tr><td>Port</td><td><input name=\"port\" type=\"number\" /></td></tr><tr><td>&nbsp;</td><td><button type=\"submit\">Submit</button></td></tr></table></form></body></html>";

            _server.send(200, "text/html", html);
        });

        _server.on("/post", [this]() {
            log("HTTP: /post requested");

            log("Setting configuration");

            _configuration.configured = true;
            strcpy(_configuration.ssid, _server.arg("ssid").c_str());
            strcpy(_configuration.password, _server.arg("password").c_str());
            strcpy(_configuration.server, _server.arg("server").c_str());
            _configuration.port = atoi(_server.arg("port").c_str());

            EEPROM.put(0, _configuration);
            EEPROM.commit();

            String html = "<html><head><title>CQ Device Configurator</title><style>body { font-size: 16px; font-family: sans-serif; }</style></head><body>Device configured, please close this window.</body></html>";

            _server.send(200, "text/html", html);
        });

        _server.begin();

        log("HTTP server started");

        while (!_configuration.configured)
        {
            _server.handleClient();
        }

        log("Stopping HTTP server in 5 seconds");
        delay(5000);

        _server.stop();

        log("HTTP server stopped");

        WiFi.softAPdisconnect(true);

        log("AP stopped");
    }

public:
    CqNode(bool debug) : _server(80)
    {
        _debug = debug;

        if (_debug)
        {
            Serial.begin(9600);
        }

        log("Initialising EEPROM");

        EEPROM.begin(512);
    }

    CqNode()
    {
        CqNode(false);
    }

    // this has to be called before Begin()
    void ClearConfiguration()
    {
        log("Clearing configuration");

        EEPROM.put(0, _configuration);
        EEPROM.commit();

        log("Configuration cleared");
    }

    void Begin()
    {
        log("MAC: " + WiFi.macAddress());

        EEPROM.get(0, _configuration);

        awaitConfiguration();

        log("Configuration loaded...");
        log("SSID:     " + (String)_configuration.ssid);
        log("Password: " + (String)_configuration.password);
        log("Server:   " + (String)_configuration.server);
        log("Port:     " + (String)_configuration.port);

        log("Connecting to wireless network");

        WiFi.begin(_configuration.ssid, _configuration.password);

        while (WiFi.status() != WL_CONNECTED)
        {
            Serial.println("Waiting...");

            delay(1000);
        }

        log("Connected to wireless network...");
        log("IP:      " + WiFi.localIP().toString());
        log("Netmask: " + WiFi.subnetMask().toString());
        log("Gateway: " + WiFi.gatewayIP().toString());
        log("DNS:     " + WiFi.dnsIP().toString());
    }

    CqNodeConfiguration GetConfiguration()
    {
        return _configuration;
    }
};

#endif