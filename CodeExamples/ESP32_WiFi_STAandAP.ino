/*
 Name:		ESP32_WiFi.ino
 Created:	2/7/2020 9:59:34 AM
 Author:	giltal
*/

#include <GeneralLib.h>
#include <Adafruit_FT6206.h>
#include "PCF8574.h"
#include <Wire.h>
#include <RTClib.h>
#include "graphics.h"

// WiFi part
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>

WebServer server(80);
void handle_OnConnect();
void wifiSetup_page();
void toggleIO_page();
void handle_NotFound();

bool wifiSetupPageVisited = false;
bool toggleIOpageVisited = false;

const char* ssid = " ";	// "Your-SSID";
const char* password = " ";		// "Your-PASSWORD";

// WiFi - Until here

#define SPEAKER_PIN 5
#define MP_BUTTON_1	27
#define MP_BUTTON_2	33
#define TOUCH_1_PIN	15
#define TOUCH_2_PIN	32
#define JOY_X_PIN	36
#define JOY_Y_PIN	39


PCF8574 pcf8574(0x39);
Adafruit_FT6206 ts = Adafruit_FT6206();

#define _262K_COLORS
//#define _8_COLORS

#if defined(_262K_COLORS)
ILI9488SPI_264KC lcd(480, 320);
#else
ILI9488SPI_8C lcd(480, 320);
#endif

// the setup function runs once when you press reset or power the board
void setup()
{
	// Setup the IOs
	pinMode(34, INPUT); // Touch pannel interrupt
	pinMode(36, ANALOG); // Joystick X
	pinMode(39, ANALOG); // Joystick Y
	pinMode(MP_BUTTON_1, INPUT);
	pinMode(MP_BUTTON_2, INPUT);
	// Setup the speaker
	pinMode(SPEAKER_PIN, OUTPUT);
	ledcSetup(0, 2000, 8);
	ledcAttachPin(SPEAKER_PIN, 0);

	Serial.begin(115200);

	// Initialize the LCD screen
#if defined(_262K_COLORS)
	lcd.init();
#else
	if (!lcd.init(18, 19, 23, 5, 40000000L, singleFrameBuffer))
	{
		printf("Cannot initialize LCD!");
		while (1);
	}
#endif

	// Setup the touch pannel
	delay(250);

	if (!ts.begin(20))
	{
		Serial.println("Unable to start touchscreen.");
	}
	else
	{
		Serial.println("Touchscreen started.");
	}
#define WIFI_TIMEOUT	20 // 10 seconds
	// WiFi Part
	WiFi.mode(WIFI_AP_STA);
	if (!WiFi.softAP("ESP32_AP", "12345678"))
	{
		Serial.println("Failed to init WiFi AP");
	}
	else
	{
		Serial.println("IP address of AP is:");
		Serial.println((WiFi.softAPIP()));
	}

	int timeOutCounter = 0;

	printf("Connecting to: ");
	printf("%s\n", ssid);
	WiFi.begin(ssid, password);
	while ((WiFi.status() != WL_CONNECTED) && (timeOutCounter < WIFI_TIMEOUT))
	{
		delay(500);
		Serial.print(".");
		timeOutCounter++;
	}
	if (timeOutCounter != WIFI_TIMEOUT)
	{
		printf("\nWiFi connected.\n");
		Serial.println(WiFi.localIP());
		Serial.println(WiFi.macAddress());
	}
	else
	{
		printf("WiFi: cannot connect to: %s\n", ssid);
	}
	if (!MDNS.begin("esp32"))
	{
		printf("Error setting up MDNS responder!\n");
	}
	else
		printf("mDNS responder started\n");
	MDNS.addService("http", "tcp", 80);
	// Web pages setup
	server.on("/", handle_OnConnect);
	server.on("/wifiSetupSelected", wifiSetup_page);
	server.on("/ioPageSelected", toggleIO_page);//toggleIO_page
	server.onNotFound(handle_NotFound);
	server.begin();
}

// the loop function runs over and over again until power down or reset
void loop()
{
	int numOfClientsConnected;
	while (1)
	{
		numOfClientsConnected = WiFi.softAPgetStationNum();
		// If WiFi is not connected and we have a user connected to our AP, trying to connect keeps changing the WiFi channel and will cause disconnectation of the user from our AP
		if ((numOfClientsConnected > 0) && (WiFi.status() != WL_CONNECTED))
		{
			WiFi.disconnect();
		}
		if ((numOfClientsConnected == 0) && (WiFi.status() != WL_CONNECTED))
		{
			WiFi.begin(ssid, password);
		}
		if ((numOfClientsConnected > 0) || (WiFi.status() == WL_CONNECTED))
		{
			server.handleClient();
			if (wifiSetupPageVisited)
			{
				if (server.args() >= 2)
				{ // Arguments were received
					wifiSetupPageVisited = false;

					String ssidName = server.arg(0);
					String ssidPassword = server.arg(1);

					Serial.println(server.args());
					Serial.println((const char*)ssidName.c_str());
					Serial.println((const char*)ssidPassword.c_str());
					WiFi.disconnect();
					delay(1000);
					int timeOutCounter = 0;

					printf("Connecting to: ");
					printf("%s\n", ssidName.c_str());

					WiFi.begin(ssidName.c_str(), ssidPassword.c_str());
					
					while ((WiFi.status() != WL_CONNECTED) && (timeOutCounter < WIFI_TIMEOUT))
					{
						delay(500);
						Serial.print(".");
						timeOutCounter++;
					}
					if (timeOutCounter != WIFI_TIMEOUT)
					{
						printf("\nWiFi connected.\n");
						Serial.println(WiFi.localIP());
						Serial.println(WiFi.macAddress());
					}
					else
					{
						printf("WiFi: cannot connect to: %s\n", ssid);
					}
				}
			}
			if (toggleIOpageVisited)
			{
				toggleIOpageVisited = false;
				if (server.args() > 0)
				{ // Arguments were received
					toggleIOpageVisited = false;

					String ioState = server.arg(0);

					Serial.println(server.args());
					Serial.println((const char*)ioState.c_str());
				}
			}
		}
	}
}

const char mainMenuPage[]PROGMEM = R"rawliteral(
<!DOCTYPE html> 
<html>
<head><meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
<title>ESP32 WebPage</title>
<style>
html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}
body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}
.button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 10px;}
.button-on {background-color: #1abc9c;}
.button-on:active {background-color: #16a085;}
p {font-size: 14px;color: #888;margin-bottom: 10px;}
</style>
</head>
<body>
<meta charset="utf-8">
<html lang="he">
<h1>Intel's Makers Demo Web Page</h1>
<a class="button button-on" href="/wifiSetupSelected">WiFi Setup</a>
<a class="button button-on" href="/ioPageSelected">IO Control</a>
</body>
<href="/">
</html>)rawliteral";

const char setupWiFiHTML[]PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
<style>
html {font-family: Arial; display: inline-block; text-align: center;}
.button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 10px;}
.boxStyle {  padding: 12px 20px;  margin: 8px 0;  box-sizing: border-box;  border: 2px solid red;  border-radius: 10px; font-size: 20px;text-align: center;}
</style>
</head>
<body>
<form action="/" method="POST">
Access Point Name:<br>
<input type="text" class="boxStyle" name="AccessPoint" value=""><br>
Password:<br>
<input type="text" class="boxStyle" name="Password" value=""><br>
<input type="submit" class="button" value="OK">
</form>
</body>
<href="/">
</html>
)rawliteral";

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem;}
    p {font-size: 3.0rem;}
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
    .switch {position: relative; display: inline-block; width: 120px; height: 68px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 34px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 68px}
    input:checked+.slider {background-color: #2196F3}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
    .button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 10px;}
  </style>
</head>
<body>
<h2>ESP Web Server</h2>
<h4>IO #1</h4>
<label class="switch"><input type="checkbox" onchange="toggleCheckbox(this)" id="1" checked><span class="slider"></span></label>
<h4>IO #2</h4>
<label class="switch"><input type="checkbox" onchange="toggleCheckbox(this)" id="2" checked><span class="slider"></span></label>
<h4>IO #3</h4>
<label class="switch"><input type="checkbox" onchange="toggleCheckbox(this)" id="3" checked><span class="slider"></span></label>
<h4><a class="button button-on" href="/">Home</a></h4>
<script>function toggleCheckbox(element) {
	var request = new XMLHttpRequest();
	var strOn	= "IO " + element.id + " ON";
	var strOff	= "IO " + element.id + " OFF";
	if(element.checked){ request.open("POST", "", true); request.setRequestHeader("Content-type", " ");request.send(strOn);}
	else { request.open("POST", "", true); request.setRequestHeader("Content-type", " ");request.send(strOff);}
	die();
}</script>
</body>
<href="/">
</html>
)rawliteral";

void handle_OnConnect()
{
	toggleIOpageVisited = false;
	server.send(200, "text/html", mainMenuPage);
}

void wifiSetup_page()
{
	wifiSetupPageVisited = true;
	toggleIOpageVisited = false;
	server.send(200, "text/html", setupWiFiHTML);
}

void toggleIO_page()
{
	toggleIOpageVisited = true;
	wifiSetupPageVisited = false;
	server.send(200, "text/html", index_html);
}

void handle_NotFound()
{
	server.send(404, "text/plain", "Not found");
}