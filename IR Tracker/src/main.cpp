#include <Arduino.h>
#include <Servo.h>

#define UP_IR 4
#define DOWN_IR 5
#define LEFT_IR 7
#define RIGHT_IR 6

#define SERVO_VERT_PIN 9
#define SERVO_HOR_PIN 10

#define SERVO_POS_INIT 90
#define SERVO_POS_MIN 0
#define SERVO_POS_MAX 180

Servo svVert;
Servo svHor;

bool senseFlag = false;

const int arrPins[] = {UP_IR, DOWN_IR, LEFT_IR, RIGHT_IR};
const int arrSize = sizeof(arrPins) / sizeof(arrPins[0]);

int posV = SERVO_POS_INIT;
int posH = SERVO_POS_INIT;

unsigned long servoLastMillisV = millis();
unsigned long servoLastMillisH = millis();
unsigned long servoDelay = 5;

// unsigned long loopMillis = millis();
// unsigned long loopDelay = 1000;

void moveServo(Servo &sv, int irOne, int irTwo, unsigned long &servoLastMillis, unsigned long &servoDelay, int &pos)
{
  if (millis() - servoLastMillis > servoDelay)
  {
    if (irOne < irTwo)
    {
      pos++;
      if (pos > SERVO_POS_MAX)
      {
        pos = SERVO_POS_MAX;
      }
    }

    if (irOne > irTwo)
    {
      pos--;
      if (pos < SERVO_POS_MIN)
      {
        pos = SERVO_POS_MIN;
      }
    }

    sv.write(pos);
    Serial.print("Servo Pos: ");
    Serial.println(pos);

    servoLastMillis = millis();
  }
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);

  svVert.attach(SERVO_VERT_PIN);
  svHor.attach(SERVO_HOR_PIN);

  svVert.write(SERVO_POS_INIT);
  svHor.write(SERVO_POS_INIT);

  for (int i = 0; i < arrSize; i++)
  {
    pinMode(arrPins[i], INPUT);
  }
}

void loop()
{
  // put your main code here, to run repeatedly:

  int upIr = digitalRead(UP_IR);
  int downIr = digitalRead(DOWN_IR);
  int leftIr = digitalRead(LEFT_IR);
  int rightIr = digitalRead(RIGHT_IR);

  if (upIr == 0 && downIr == 0 && leftIr == 0 && rightIr == 0)
  {
    senseFlag = true;
  }

  if (senseFlag)
  {
    moveServo(svVert, upIr, downIr, servoLastMillisV, servoDelay, posV);
    moveServo(svHor, leftIr, rightIr, servoLastMillisH, servoDelay, posH);

    if (upIr == 1 && downIr == 1 && leftIr == 1 && rightIr == 1)
    {
      senseFlag = false;
    }
  }

  // Serial.print("UP: ");
  // Serial.print(upIr);
  // Serial.print(" | Down: ");
  // Serial.print(downIr);
  // Serial.print(" | Left: ");
  // Serial.print(leftIr);
  // Serial.print(" | Right: ");
  // Serial.println(rightIr);

  // Serial.print("Flag: ");
  // Serial.println(senseFlag);
}
