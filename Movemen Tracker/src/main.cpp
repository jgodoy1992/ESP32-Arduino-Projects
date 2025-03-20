#include <Arduino.h>
#include <Servo.h>

#define ECHO_PIN 2
#define TRIG_PIN 3

#define LEFT_IR 5
#define RIGHT_IR 4

#define THRESHOLD_DIST 5

#define SERVO_PIN 10
#define SERVO_INIT_POS 90

unsigned long lastTimeTrig = millis();
unsigned long trigDelay = 60;

volatile unsigned long pulseEchoBegin;
volatile unsigned long pulseEchoEnd;
volatile bool newDistance = false;

unsigned long lastTimeSrv = millis();
unsigned long srvDelay = 50;

Servo srv;

int pos = SERVO_INIT_POS;

void echoISR()
{
  if (digitalRead(ECHO_PIN) == HIGH)
  {
    pulseEchoBegin = micros();
  }
  else
  {
    pulseEchoEnd = micros();
    newDistance = true;
  }
}

void servo_pos(Servo &srv, int &pos, int lIr, int rIr)
{
  if (millis() - lastTimeSrv > srvDelay)
  {
    if (rIr < lIr)
    {
      pos += 5;
      if (pos >= 180)
      {
        pos = 179;
      }
    }
    if (rIr > lIr)
    {
      pos -= 5;
      if (pos <= 0)
      {
        pos = 1;
      }
    }

    srv.write(pos);

    lastTimeSrv += srvDelay;
  }
}

void triggerUS()
{
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
}

double getDistance()
{
  double durationMicros = pulseEchoEnd - pulseEchoBegin;
  double distance = durationMicros / 58.0;

  return distance;
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(LEFT_IR, INPUT);
  pinMode(RIGHT_IR, INPUT);

  srv.attach(SERVO_PIN);
  srv.write(SERVO_INIT_POS);

  attachInterrupt(digitalPinToInterrupt(ECHO_PIN), echoISR, CHANGE);
}

void loop()
{
  // put your main code here, to run repeatedly:

  if (millis() - lastTimeTrig > trigDelay)
  {
    lastTimeTrig += trigDelay;
    triggerUS();
  }

  if (newDistance)
  {
    newDistance = false;
    double distance = getDistance();

    if (distance <= 9)
    {
      int leftRead = digitalRead(LEFT_IR);
      int rightRead = digitalRead(RIGHT_IR);

      servo_pos(srv, pos, leftRead, rightRead);

      Serial.print("Distance: ");
      Serial.print(distance);
      Serial.print(" CM || ");
      Serial.print("Left sensor: ");
      Serial.print(leftRead);
      Serial.print(" || Right sensor: ");
      Serial.println(rightRead);
    }
    else
    {
      // srv.write(SERVO_INIT_POS);
      Serial.print("Distance: ");
      Serial.print(distance);
      Serial.print(" CM || ");
      Serial.print("Left sensor: ");
      Serial.print("No reading");
      Serial.print(" || Right sensor: ");
      Serial.println("No reading");
    }
  }
}
