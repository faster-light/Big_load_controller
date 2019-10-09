/*	Прошивка предназначена для платы BFDriver с контроллерами ESP32 и STM32F103C8T6
Это ESP часть прошивки. Здесь выполняется работа с дисплеем, работа по MQTT по WI-Fi
UART связь с контроллером STM32F103C8T6 работает по следующему протоколу:

Команда ESP->STM		Разъем на плате		Контакт STM
------------------------- DC LOAD ----------------------
10XXX				  P1				PA0
11XXX				  P2				PA1
12XXX				  P3				PA2
13XXX				  P4				PA3
14XXX				  P5				PA6
15XXX				  P6				PA7
16XXX				  P7				PB0
17XXX				  P8				PB1
18XXX				  P9				PB6
19XXX				  P10				PB7
------------------------- AC LOAD ----------------------
20XXX				  P15				PB15
21XXX				  P12				PB13
22XXX				  P13				PB12
23XXX				  P14				PB14

Где XXX - от 100 до 200 - режим диммирования
*/

#include <HardwareSerial.h>
HardwareSerial Serial2(2);

#include <WiFi.h>
#include <PubSubClient.h>
/*
const char *ssid = "KBM";                      // Имя вайфай точки доступа
const char *password = "23112311";                 // Пароль от точки доступа

const char *mqttServer = "m10.cloudmqtt.com";  // Имя сервера MQTT
const int mqttPort = 16029;                    // Порт для подключения к серверу MQTT
const char *mqttUser = "kzvcomct";             // Логин от сервера
const char *mqttPassword = "-mW311FSubSi";         // Пароль от сервера
*/

const char *ssid = "faster";
const char *password = "76137613";
const char *mqttServer = "192.168.0.41";
const int mqttPort = 1883;
const char *mqttUser = "pi";
const char *mqttPassword = "0611";


WiFiClient espClient;
PubSubClient client(espClient);

#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <XPT2046_Touchscreen.h>

#define _cs   22
#define _dc   21
#define _rst  5

#define TOUCH_CS_PIN 33
#define TOUCH_IRQ_PIN 35

Adafruit_ILI9341 tft = Adafruit_ILI9341(_cs, _dc, _rst);
XPT2046_Touchscreen touch(TOUCH_CS_PIN, TOUCH_IRQ_PIN);
TS_Point rawLocation;

int page_selected = 0; // По умолчанию 0 - страница управления нагрузкой, 1 - страница статуса системы

bool page_status_on = false;		// При переходе на страницу Статуса сделать TRUE
bool page_control_on = true;

bool connect_to_mqtt_and_wifi = false;

bool button_wifi_state = false;

int temp = 0;
bool atx_state = false;

int x = 0;
int y = 0;

bool button_1_state = false;
bool button_2_state = false;
bool button_3_state = false;
bool button_4_state = false;
bool button_5_state = false;
bool button_6_state = false;

const char *button_1_name = "HALL";
const char *button_2_name = "BATHROOM";
const char *button_3_name = "KITCHEN";
const char *button_4_name = "ROOM";
const char *button_5_name = "TABLE";
const char *button_6_name = "VENTING";

const char *topic_1 = "/home/hall/light";
const char *topic_2 = "/home/bathroom/light";
const char *topic_3 = "/home/kitchen/light";
const char *topic_4 = "/home/room/light";
const char *topic_5 = "/home/table/light";

const char *topic_13 = "/home/bathroom/ventilation";

void wifi_status_update(int status) {
	tft.setTextSize(2);

	tft.setTextColor(ILI9341_BLACK);
	tft.setCursor(140, 35);
	tft.print("Disconnected");
	tft.setCursor(135, 35);
	tft.print("Connection to");
	tft.setCursor(205 - 5 * strlen(ssid), 55);
	tft.print(ssid);
	tft.setCursor(140, 35);
	tft.print("Connected to");


	switch (status) {
	case 0:
		tft.setCursor(140, 35);
		tft.setTextColor(ILI9341_RED);
		tft.print("Disconnected");
		break;
	case 1:
		tft.setCursor(135, 35);
		tft.setTextColor(ILI9341_YELLOW);
		tft.print("Connection to");
		tft.setCursor(205 - 5 * strlen(ssid), 55);
		tft.setTextColor(ILI9341_CYAN);
		tft.print(ssid);
		break;
	case 2:
		tft.setCursor(140, 35);
		tft.setTextColor(ILI9341_GREEN);
		tft.print("Connected to");
		tft.setCursor(205 - 5 * strlen(ssid), 55);
		tft.setTextColor(ILI9341_CYAN);
		tft.print(ssid);
		break;
	}
}

void mqtt_status_update(int status) {
	tft.setTextSize(2);

	tft.setTextColor(ILI9341_BLACK);
	tft.setCursor(140, 80);
	tft.print("Disconnected");
	tft.setCursor(135, 80);
	tft.print("Connection to");
	tft.setCursor(205 - 5.5*strlen(mqttServer), 100);
	tft.print(mqttServer);
	tft.setCursor(140, 80);
	tft.print("Connected to");


	switch (status) {
	case 0:
		tft.setCursor(140, 80);
		tft.setTextColor(ILI9341_RED);
		tft.print("Disconnected");
		break;
	case 1:
		tft.setCursor(135, 80);
		tft.setTextColor(ILI9341_YELLOW);
		tft.print("Connection to");
		tft.setCursor(205 - 5.5*strlen(mqttServer), 100);
		tft.setTextColor(ILI9341_CYAN);
		tft.print(mqttServer);
		break;
	case 2:
		tft.setCursor(140, 80);
		tft.setTextColor(ILI9341_GREEN);
		tft.print("Connected to");
		tft.setCursor(205 - 5.5*strlen(mqttServer), 100);
		tft.setTextColor(ILI9341_CYAN);
		tft.print(mqttServer);
		break;
	}
}

void button_wifi_drow(bool state) {

	tft.setTextSize(2);
	tft.setTextColor(ILI9341_BLACK);

	tft.setCursor(40, 190);
	tft.print("disconnect from wifi");

	tft.setCursor(67, 190);
	tft.print("connect to wifi");

	if (state) {

		tft.setTextColor(ILI9341_RED);

		tft.drawLine(20, 183, 300, 183, ILI9341_RED);
		tft.drawLine(20, 213, 300, 213, ILI9341_RED);

		tft.setCursor(40, 190);
		tft.print("disconnect from wifi");

	}
	else {

		tft.setTextColor(ILI9341_CYAN);

		tft.drawLine(20, 183, 300, 183, ILI9341_CYAN);
		tft.drawLine(20, 213, 300, 213, ILI9341_CYAN);

		tft.setCursor(67, 190);
		tft.print("connect to wifi");

	}
}

void print_on_tft(const char *word, int x, int y, int color) {

	tft.setCursor(x, y);

	switch (color) {
	case 0:
		tft.setTextColor(ILI9341_WHITE);
		break;
	case 1:
		tft.setTextColor(ILI9341_CYAN);
		break;
	case 2:
		tft.setTextColor(ILI9341_RED);
		break;
	case 3:
		tft.setTextColor(ILI9341_BLACK);
		break;
	}

	tft.print(word);
}

void print_on_tft_int(int word, int x, int y, int color) {

	tft.setCursor(x, y);

	switch (color) {
	case 0:
		tft.setTextColor(ILI9341_WHITE);
		break;
	case 1:
		tft.setTextColor(ILI9341_CYAN);
		break;
	case 2:
		tft.setTextColor(ILI9341_RED);
		break;
	case 3:
		tft.setTextColor(ILI9341_BLACK);
		break;
	}

	tft.print(word);
}

void touch_check_page_status()
{
	rawLocation = touch.getPoint();
	y = (rawLocation.x - 200) / 15.85;
	x = (rawLocation.y - 200) / 10.97;

	if ((x < 300) and (x > 20) and (y > 183) and (y < 213)) {

		button_wifi_state = !button_wifi_state;
		button_wifi_drow(button_wifi_state);
		connect_to_mqtt_and_wifi = true;

		if (!button_wifi_state) {
			WiFi.disconnect();
			client.disconnect();
			connect_to_mqtt_and_wifi = false;

			wifi_status_update(0);

			mqtt_status_update(0);

		}

	}

	if ((x < 231) and (x > 90) and (y > 6) and (y < 30)) {

		page_selected = 0;
		page_control_on = true;
	}

}

void page_status_draw_first() {

	tft.fillScreen(ILI9341_BLACK);

	if (WiFi.status() != WL_CONNECTED)
		wifi_status_update(0);
	else
		wifi_status_update(2);

	if (client.connected())
		mqtt_status_update(2);
	else
		mqtt_status_update(0);



	button_wifi_drow(button_wifi_state);

	tft.setTextSize(2);
	print_on_tft("STATUS SYSTEM", 90, 10, 1);
	print_on_tft("Wi-Fi", 10, 35, 0);

	print_on_tft("MQTT", 10, 80, 0);

	print_on_tft("Temperature", 10, 130, 0);
	print_on_tft_int(temp, 200, 130, 0);

	print_on_tft("Power supply", 10, 150, 0);
	if (atx_state)
		print_on_tft("Full start", 200, 150, 1);
	else
		print_on_tft("Standby", 200, 150, 0);

	tft.setTextSize(1);
	print_on_tft("Created by faster_light", 150, 230, 0);

}

void page_conrol_draw_first() {

	tft.fillScreen(ILI9341_BLACK);

	tft.setTextSize(2);
	print_on_tft("CONTROL PAGE", 90, 10, 1);

	button_1_drow(button_1_state);
	button_2_drow(button_2_state);
	button_3_drow(button_3_state);
	button_4_drow(button_4_state);
	button_5_drow(button_5_state);
	button_6_drow(button_6_state);
}

void touch_check_page_control()
{
	rawLocation = touch.getPoint();
	y = (rawLocation.x - 200) / 15.85;
	x = (rawLocation.y - 200) / 10.97;


	if ((x < 140) and (x > 30) and (y > 50) and (y < 80)) {

		button_1_state = !button_1_state;
		button_1_drow(button_1_state);

	}

	if ((x < 290) and (x > 180) and (y > 50) and (y < 80)) {

		button_2_state = !button_2_state;
		button_2_drow(button_2_state);

		if (button_2_state) {
			client.publish(topic_2, "100");

			if(!client.connected())
				Serial2.print(11200);

		}
		else {
			client.publish(topic_2, "0");
			Serial2.print(11150);

		}
	}

	if ((x < 140) and (x > 30) and (y > 90) and (y < 120)) {

		button_3_state = !button_3_state;
		button_3_drow(button_3_state);
		if (button_3_state) {
			client.publish(topic_3, "100");
			Serial2.print(12200);

		}
		else {
			client.publish(topic_3, "0");
			Serial2.print(12150);

		}
	}

	if ((x < 290) and (x > 180) and (y > 90) and (y < 120)) {

		button_4_state = !button_4_state;
		button_4_drow(button_4_state);

	}

	if ((x < 140) and (x > 30) and (y > 130) and (y < 160)) {

		button_5_state = !button_5_state;
		button_5_drow(button_5_state);

	}

	if ((x < 290) and (x > 180) and (y > 130) and (y < 160)) {

		button_6_state = !button_6_state;
		button_6_drow(button_6_state);

	}

	if ((x < 231) and (x > 90) and (y > 6) and (y < 30)) {

		page_selected = 1;
		page_status_on = true;
	}

}

void button_1_drow(bool state) {

	tft.setTextSize(2);

	if (!state) {

		tft.setTextColor(ILI9341_RED);

		tft.drawLine(30, 50, 140, 50, ILI9341_RED);
		tft.drawLine(30, 80, 140, 80, ILI9341_RED);

		tft.setCursor(85 - 5.75 * strlen(button_1_name), 59);
		tft.print(button_1_name);

	}
	else {

		tft.setTextColor(ILI9341_CYAN);

		tft.drawLine(30, 50, 140, 50, ILI9341_CYAN);
		tft.drawLine(30, 80, 140, 80, ILI9341_CYAN);

		tft.setCursor(85 - 5.75 * strlen(button_1_name), 59);
		tft.print(button_1_name);

	}
}

void button_2_drow(bool state) {

	tft.setTextSize(2);

	if (!state) {

		tft.setTextColor(ILI9341_RED);

		tft.drawLine(180, 50, 290, 50, ILI9341_RED);
		tft.drawLine(180, 80, 290, 80, ILI9341_RED);

		tft.setCursor(235 - 5.75 * strlen(button_2_name), 59);
		tft.print(button_2_name);

	}
	else {

		tft.setTextColor(ILI9341_CYAN);

		tft.drawLine(180, 50, 290, 50, ILI9341_CYAN);
		tft.drawLine(180, 80, 290, 80, ILI9341_CYAN);

		tft.setCursor(235 - 5.75 * strlen(button_2_name), 59);
		tft.print(button_2_name);

	}
}

void button_3_drow(bool state) {

	tft.setTextSize(2);

	if (!state) {

		tft.setTextColor(ILI9341_RED);

		tft.drawLine(30, 90, 140, 90, ILI9341_RED);
		tft.drawLine(30, 120, 140, 120, ILI9341_RED);

		tft.setCursor(85 - 5.75 * strlen(button_3_name), 99);
		tft.print(button_3_name);

	}
	else {

		tft.setTextColor(ILI9341_CYAN);

		tft.drawLine(30, 90, 140, 90, ILI9341_CYAN);
		tft.drawLine(30, 120, 140, 120, ILI9341_CYAN);

		tft.setCursor(85 - 5.75 * strlen(button_3_name), 99);
		tft.print(button_3_name);

	}

}

void button_4_drow(bool state) {

	tft.setTextSize(2);

	if (!state) {

		tft.setTextColor(ILI9341_RED);

		tft.drawLine(180, 90, 290, 90, ILI9341_MAGENTA);
		tft.drawLine(180, 120, 290, 120, ILI9341_MAGENTA);

		tft.setCursor(235 - 5.75 * strlen(button_4_name), 99);
		tft.print(button_4_name);

	}
	else {

		tft.setTextColor(ILI9341_CYAN);

		tft.drawLine(180, 90, 290, 90, ILI9341_MAGENTA);
		tft.drawLine(180, 120, 290, 120, ILI9341_MAGENTA);

		tft.setCursor(235 - 5.75 * strlen(button_4_name), 99);
		tft.print(button_4_name);

	}
}

void button_5_drow(bool state) {

	tft.setTextSize(2);

	if (!state) {

		tft.setTextColor(ILI9341_RED);

		tft.drawLine(30, 130, 140, 130, ILI9341_RED);
		tft.drawLine(30, 160, 140, 160, ILI9341_RED);

		tft.setCursor(85 - 5.75 * strlen(button_5_name), 139);
		tft.print(button_5_name);

	}
	else {

		tft.setTextColor(ILI9341_CYAN);

		tft.drawLine(30, 130, 140, 130, ILI9341_CYAN);
		tft.drawLine(30, 160, 140, 160, ILI9341_CYAN);

		tft.setCursor(85 - 5.75 * strlen(button_5_name), 139);
		tft.print(button_5_name);

	}
}

void button_6_drow(bool state) {

	tft.setTextSize(2);

	if (!state) {

		tft.setTextColor(ILI9341_RED);

		tft.drawLine(180, 130, 290, 130, ILI9341_MAGENTA);
		tft.drawLine(180, 160, 290, 160, ILI9341_MAGENTA);

		tft.setCursor(235 - 5.75 * strlen(button_6_name), 139);
		tft.print(button_6_name);

	}
	else {

		tft.setTextColor(ILI9341_CYAN);

		tft.drawLine(180, 130, 290, 130, ILI9341_MAGENTA);
		tft.drawLine(180, 160, 290, 160, ILI9341_MAGENTA);

		tft.setCursor(235 - 5.75 * strlen(button_6_name), 139);
		tft.print(button_6_name);

	}

	//	tft.drawLine(90, 10, 231, 10, ILI9341_RED);
	//	tft.drawLine(90, 23, 231, 23, ILI9341_RED);
}

void wifi_and_mqtt_connect() {

	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED) {

		if (page_selected == 1) {
			wifi_status_update(1);
		}

		delay(500);
		Serial.println("Connecting to WiFi..");
	}

	Serial.println("Connected to the WiFi network");


	if (page_selected == 1) {
		wifi_status_update(2);
	}


	client.setServer(mqttServer, mqttPort);
	client.setCallback(callback);

	while (!client.connected()) {
		Serial.println("Connecting to MQTT...");

		if (page_selected == 1) {
			mqtt_status_update(1);
		}



		if (client.connect("BFDriver_2", mqttUser, mqttPassword)) {

			Serial.println("connected");


			if (page_selected == 1) {
				mqtt_status_update(2);
			}

		}
		else {

			Serial.print("failed with state ");
			Serial.print(client.state());

			if (page_selected == 1) {
				mqtt_status_update(0);
			}

			delay(2000);

		}
	}

	//	client.publish("/home/kitchen/state", "0");
	client.subscribe(topic_1);
	client.subscribe(topic_2);
	client.subscribe(topic_3);
	client.subscribe(topic_4);
	client.subscribe(topic_5);

	client.subscribe(topic_13);

	connect_to_mqtt_and_wifi = false;
}

void setup() {

	Serial.begin(115200);
	Serial2.begin(9600);

	tft.begin();
	tft.setRotation(1);
	tft.fillScreen(ILI9341_BLACK);

	touch.begin();
	touch.setRotation(2);

}

void callback(char* topic, byte* payload, unsigned int length) {


	if (String(topic) == topic_3) {

		int v = 0;

		for (int i = 0; i < length; i++) {
			v = v + (payload[i] - '0') * pow(10, length - i - 1);
			Serial.println(pow(10, length - i - 1));
		}

//		int v1 = payload[0] - '0';
//		int v2 = payload[1] - '0';
//		int v3 = payload[2] - '0';

//		int v = v1 * 100 + v2 * 10 + v3;

		Serial2.println(12100 + v);
		Serial.println(12100 + v);
	}

	//	Serial.print("Message:");
//		for (int i = 0; i < length; i++) {
//			Serial.print((char)payload[i]);
//		}


}

void loop() {

	if (page_selected == 0) {

		if (page_control_on) {
			page_conrol_draw_first();
			page_control_on = false;
		}

		if (touch.touched()) {
			touch_check_page_control();
			delay(200);
		}
	}

	if (page_selected == 1) {

		if (page_status_on) {
			page_status_draw_first();
			page_status_on = false;
		}

		if (touch.touched()) {
			touch_check_page_status();
			delay(200);
		}

		if (connect_to_mqtt_and_wifi) {
			wifi_and_mqtt_connect();
		}
	}

	if (!client.connected() and (button_wifi_state))
		wifi_and_mqtt_connect();

	client.loop();
	delay(10);

}