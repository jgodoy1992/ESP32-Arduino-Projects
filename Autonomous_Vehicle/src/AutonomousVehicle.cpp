#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <esp_now.h>

#define ECHO_PIN_FRONT 18
#define TRIGG_PIN_FRONT 19
#define ECHO_PIN_LEFT 4
#define TRIGG_PIN_LEFT 5
#define ECHO_PIN_RIGHT 17
#define TRIGG_PIN_RIGHT 16

#define MOTORL_1 25
#define MOTORL_2 26
#define MOTORR_1 33
#define MOTORR_2 32

#define THRESH_HOLD 5

uint8_t broadcastAddress[] = {};

typedef struct struct_msg_inc
{
  bool mode;
  bool left;
  bool forward;
  bool right;
  bool backward;
} struct_msg_inc;

struct_msg_inc recievedData;

typedef struct struct_msg_out
{
  double distanceLeft;
  double distanceFront;
  double distanceRight;
} struct_msg_out;

struct_msg_out outGoingData;

esp_now_peer_info_t peerInfo;

volatile unsigned long pulseTimeBeginFront;
volatile unsigned long pulseTimeEndFront;
bool newDistanceAvailableFront = false;

volatile unsigned long pulseTimeBeginLeft;
volatile unsigned long pulseTimeEndLeft;
bool newDistanceAvailableLeft = false;

volatile unsigned long pulseTimeBeginRight;
volatile unsigned long pulseTimeEndRight;
bool newDistanceAvailableRight = false;

// double prevDistance = 400.0;

unsigned long lastTimeLoop = millis();
unsigned long loopDelay = 50;

//*****INTERRUPTS**********

void IRAM_ATTR echoFrontISR()
{
  if (digitalRead(ECHO_PIN_FRONT) == HIGH)
  {
    pulseTimeBeginFront = micros();
  }
  else
  {
    pulseTimeEndFront = micros();
    newDistanceAvailableFront = true;
  }
}

void IRAM_ATTR echoLeftISR()
{
  if (digitalRead(ECHO_PIN_LEFT) == HIGH)
  {
    pulseTimeBeginLeft = micros();
  }
  else
  {
    pulseTimeEndLeft = micros();
    newDistanceAvailableLeft = true;
  }
}

void IRAM_ATTR echoRightISR()
{
  if (digitalRead(ECHO_PIN_RIGHT) == HIGH)
  {
    pulseTimeBeginRight = micros();
  }
  else
  {
    pulseTimeEndRight = micros();
    newDistanceAvailableRight = true;
  }
}
//************GET DISTANCE***********

double getDistance(int trigPin, volatile unsigned long &pulseTimeBegin, volatile unsigned long &pulseTimeEnd, bool &newDistanceAvailable)
{

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  newDistanceAvailable = false;

  double durationMicros = pulseTimeEnd - pulseTimeBegin;
  double distance = durationMicros / 58.0;

  /*if(distance > 400) {return prevDistance;}

  distance = prevDistance * 0.3 + distance * 0.7;
  prevDistance = distance;*/

  return distance;
}

//*********MOTOR FUNCTIONS******

void stop()
{
  digitalWrite(MOTORL_1, LOW);
  digitalWrite(MOTORL_2, LOW);
  digitalWrite(MOTORR_1, LOW);
  digitalWrite(MOTORR_2, LOW);
}

void forward()
{
  digitalWrite(MOTORL_1, HIGH);
  digitalWrite(MOTORL_2, LOW);
  digitalWrite(MOTORR_1, HIGH);
  digitalWrite(MOTORR_2, LOW);
}

void left()
{
  digitalWrite(MOTORL_1, LOW);
  digitalWrite(MOTORL_2, LOW);
  digitalWrite(MOTORR_1, HIGH);
  digitalWrite(MOTORR_2, LOW);
}

void right()
{
  digitalWrite(MOTORL_1, HIGH);
  digitalWrite(MOTORL_2, LOW);
  digitalWrite(MOTORR_1, LOW);
  digitalWrite(MOTORR_2, LOW);
}

void backwards()
{
  digitalWrite(MOTORL_1, LOW);
  digitalWrite(MOTORL_2, HIGH);
  digitalWrite(MOTORR_1, LOW);
  digitalWrite(MOTORR_2, HIGH);
}

//*********AUTONOMOUS FUNCTION****/

void autonomousFunc(double dL, double dF, double dR)
{
  if (dF > THRESH_HOLD && dL > THRESH_HOLD && dR > THRESH_HOLD)
  {
    forward();
  }
  else if (dF <= THRESH_HOLD)
  {
    if ((dL <= THRESH_HOLD && dR <= THRESH_HOLD) || (dL == dR))
    {
      backwards();
    }
    else if (dL > dR)
    {
      left();
    }
    else if (dL < dR)
    {
      right();
    }
  }
  else if (dL <= THRESH_HOLD)
  {
    right();
  }
  else if (dR <= THRESH_HOLD)
  {
    left();
  }
}

///*****ON SENT DATA******** */
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  Serial.print("\r\nLast Packet Send Status: \t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

//***********ON RECIEDE DATA***************** */

void onDataRecieve(const uint8_t *mac, const uint8_t *incomingData, int len)
{
  memcpy(&recievedData, incomingData, sizeof(recievedData));

  Serial.println(recievedData.mode ? "Manual" : "Autonomous");
}

void manualMode()
{
  stop();

  if (recievedData.left)
  {
    left();
  }
  if (recievedData.forward)
  {
    forward();
  }
  if (recievedData.right)
  {
    right();
  }
  if (recievedData.backward)
  {
    backwards();
  }
}

//**************MAIN*************

void setup()
{
  // put your setup code here, to run once:

  Serial.begin(115200);

  pinMode(ECHO_PIN_FRONT, INPUT);
  pinMode(TRIGG_PIN_FRONT, OUTPUT);
  pinMode(ECHO_PIN_LEFT, INPUT);
  pinMode(TRIGG_PIN_LEFT, OUTPUT);
  pinMode(ECHO_PIN_RIGHT, INPUT);
  pinMode(TRIGG_PIN_RIGHT, OUTPUT);

  pinMode(MOTORL_1, OUTPUT);
  pinMode(MOTORL_2, OUTPUT);
  pinMode(MOTORR_1, OUTPUT);
  pinMode(MOTORR_2, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(ECHO_PIN_FRONT), echoFrontISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ECHO_PIN_LEFT), echoLeftISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ECHO_PIN_RIGHT), echoRightISR, CHANGE);

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
  // put your main code here, to run repeatedly:
  if (recievedData.mode)
  {
    manualMode();
  }
  else
  {
    if (millis() - lastTimeLoop > loopDelay)
    {

      double distanceLeft = getDistance(TRIGG_PIN_LEFT, pulseTimeBeginLeft, pulseTimeEndLeft, newDistanceAvailableLeft);
      double distanceFront = getDistance(TRIGG_PIN_FRONT, pulseTimeBeginFront, pulseTimeEndFront, newDistanceAvailableFront);
      double distanceRight = getDistance(TRIGG_PIN_RIGHT, pulseTimeBeginRight, pulseTimeEndRight, newDistanceAvailableRight);

      autonomousFunc(distanceLeft, distanceFront, distanceRight);

      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&outGoingData, sizeof(outGoingData));

      if (result == ESP_OK)
      {
        Serial.println("Sent with success");
      }
      else
      {
        Serial.println("Error sendig the data");
      }

      lastTimeLoop += loopDelay;
    }
  }
}
