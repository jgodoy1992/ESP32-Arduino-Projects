#include <Arduino.h>
#include <Wire.h>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_now.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "PushButton.h"

#define WIDTH 128
#define HEIGHT 64
#define SCREEN_I2C_ADDR 0x3C
#define OLED_RST_PIN -1

#define MODE_PIN 13
#define LEFT_PIN 14
#define FORWARD_PIN 15
#define RIGHT_PIN 25
#define BACKWARD_PIN 26

Adafruit_SSD1306 screen(WIDTH, HEIGHT, &Wire, OLED_RST_PIN);

unsigned long lastTimeLoop = millis();
unsigned long loopDelay = 50;

//*******BUTTON INSTANCES**********/

PushButton modeBut(MODE_PIN, true, true);
PushButton leftBut(LEFT_PIN, true, true);
PushButton forBut(FORWARD_PIN, true, true);
PushButton rightBut(RIGHT_PIN, true, true);
PushButton backBut(BACKWARD_PIN, true, true);

/******DATA**************/
uint8_t broadcastAddress[] = {0xd8, 0xbc, 0x38, 0xf9, 0x58, 0x0c};

typedef struct struct_msg_out
{
    bool mode;
    bool left;
    bool forward;
    bool right;
    bool backward;
} struct_msg_out;

struct_msg_out outGoingData;

typedef struct struct_msg_inc
{
    double distanceLeft;
    double distanceFront;
    double distanceRight;
} struct_msg_inc;

struct_msg_inc recievedData;

esp_now_peer_info_t peerInfo;

bool modeState = true;

//***********FUCNTIONS***************/

///*****ON SENT DATA******** */
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    Serial.print("\r\nLast Packet Send Status: \t");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

//******************DISAPLY*************/

void displayDistance(double d1, double d2, double d3)
{

    screen.clearDisplay();

    screen.drawLine(42, 0, 42, HEIGHT, WHITE);
    screen.drawLine(84, 0, 84, HEIGHT, WHITE);

    screen.setTextSize(1);
    screen.setTextColor(WHITE);

    screen.setCursor(0, 20);
    screen.print("L (cm)");
    screen.setCursor(0, 30);
    screen.print(d1);

    screen.setCursor(45, 20);
    screen.print("F (cm)");
    screen.setCursor(45, 30);
    screen.print(d2);

    screen.setCursor(87, 20);
    screen.print("R (cm)");
    screen.setCursor(87, 30);
    screen.print(d3);

    screen.display();
}

void onDataRecieve(const uint8_t *mac, const uint8_t *incomingData, int len)
{
    memcpy(&recievedData, incomingData, sizeof(recievedData));
}

//********MAIN***********/

void setup()
{

    Serial.begin(115200);

    modeBut.init();
    leftBut.init();
    forBut.init();
    rightBut.init();
    backBut.init();

    screen.begin(SSD1306_SWITCHCAPVCC, SCREEN_I2C_ADDR);
    screen.clearDisplay();

    WiFi.mode(WIFI_STA);

    if (esp_now_init() != ESP_OK)
    {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    esp_now_register_send_cb(onDataSent);

    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        Serial.println("Failed to add peer");
        return;
    }

    esp_now_register_recv_cb(esp_now_recv_cb_t(onDataRecieve));
}

void loop()
{
    if (millis() - lastTimeLoop > loopDelay)
    {
        if (modeBut.isPressed())
        {
            modeState = !modeState;
        }

        outGoingData.mode = modeState;

        if (modeState)
        {
            screen.clearDisplay();
            outGoingData.left = leftBut.isPressed() == HIGH;
            outGoingData.forward = forBut.isPressed() == HIGH;
            outGoingData.right = rightBut.isPressed() == HIGH;
            outGoingData.backward = backBut.isPressed() == HIGH;
        }
        else
        {
            outGoingData.left = false;
            outGoingData.forward = false;
            outGoingData.right = false;
            outGoingData.backward = false;
            displayDistance(
                recievedData.distanceLeft, recievedData.distanceFront, recievedData.distanceRight);
        }

        esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&outGoingData, sizeof(outGoingData));

        if (result == ESP_OK)
        {
            Serial.println("Sent with success");
        }
        else
        {
            Serial.println("Error sendig the data");
        }

        lastTimeLoop = millis();
    }
}