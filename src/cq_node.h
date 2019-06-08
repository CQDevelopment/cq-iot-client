#ifndef H_CQ_NODE

#define H_CQ_NODE

#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsClient.h>

struct CqNodeConfiguration
{
    bool configured = false;
    char ssid[128] = "";
    char password[128] = "";
    char server[128] = "";
    uint16_t port = 0;
};

typedef void (*SwitchSetFunctionPtr)(uint8_t index, bool state);

typedef void (*SwitchGetFunctionPtr)(uint8_t index);

typedef void (*SensorGetFunctionPtr)(uint8_t index);

class CqNode
{
private:
    CqNodeConfiguration _configuration;
    ESP8266WebServer _server;
    WebSocketsClient _client;

    char _chipId[8];
    bool _connected = false;
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

    void receive(WStype_t type, uint8_t *payload, size_t length)
    {
        if (type == WStype_CONNECTED)
        {
            log("Connected to socket server, sending registration");
            _connected = true;

            char buffer[3];

            itoa(SwitchCount, buffer, 10);
            String switchCount = (String)buffer;

            itoa(SensorCount, buffer, 10);
            String sensorCount = (String)buffer;

            itoa(PushCount, buffer, 10);
            String pushCount = (String)buffer;

            _client.sendTXT("register," + (String)_chipId + "," + switchCount + ',' + sensorCount + ',' + pushCount);

            log("Registration sent");

            return;
        }

        if (type == WStype_DISCONNECTED)
        {
            log("Disconnected from socket server");
            _connected = false;

            return;
        }

        if (type == WStype_TEXT)
        {
            log("Payload received: " + String((char *)payload));

            return;
        }
    }

    void connect()
    {
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

        log("Connecting to server");
        _client.setReconnectInterval(5000);
        _client.enableHeartbeat(5000, 2500, 2);

        _client.onEvent([this](WStype_t type, uint8_t *payload, size_t length) {
            this->receive(type, payload, length);
        });

        _client.begin(_configuration.server, _configuration.port, "/");

        log("Starting client loop");

        while (true)
        {
            _client.loop();

            if (!_connected)
            {
                log("Not connected, waiting...");
                delay(1000);
            }
        }
    }

public:
    uint8_t SwitchCount = 0;
    uint8_t SensorCount = 0;
    uint8_t PushCount = 0;

    SwitchGetFunctionPtr SwitchGetFunction;
    SwitchSetFunctionPtr SwitchSetFunction;
    SensorGetFunctionPtr SensorGetFunction;

    CqNode(bool debug) : _server(80)
    {
        _debug = debug;

        if (_debug)
        {
            Serial.begin(9600);
        }

        log("Initialising EEPROM");

        EEPROM.begin(512);

        sprintf(_chipId, "%x", ESP.getChipId());

        log("Chip ID: " + (String)_chipId);
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
        connect();
    }
};

#endif