#include <Arduino.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "PushButton.h"

#define BUZZ_PIN 8

#define BTN_HOUR 2
#define BTN_STUDY 3
#define BTN_REST 4
#define BTN_START 5
#define BTN_RST 6

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

PushButton hourBtn(BTN_HOUR, true, true);
PushButton studyBtn(BTN_STUDY, true, true);
PushButton restBtn(BTN_REST, true, true);
PushButton startBtn(BTN_START, true, true);
PushButton rstBtn(BTN_RST, true, true);

int totalHours = 1;
int studyMinutes = 25;
int restMinutes = 5;

int totalBlocks = 0;
int currentBlock = 0;

bool isRunning = false;
bool isResting = false;

void showSetupScreen()
{
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Pomodoro Timer Setup:");
  display.print("Hours: ");
  display.println(totalHours);
  display.print("Study: ");
  display.print(studyMinutes);
  display.println(" min");
  display.print("Rest: ");
  display.print(restMinutes);
  display.println(" min");
  display.display();
}

void showProgress(int elapsedSec, int totalSec, bool isStudy)
{
  display.clearDisplay();

  display.setCursor(0, 0);
  display.print(isStudy ? "Study Time" : "Rest Time");
  display.setCursor(0, 10);
  display.print("Block ");
  display.print(currentBlock + 1);
  display.print(" of ");
  display.println(totalBlocks);

  int barWidth = map(elapsedSec, 0, totalSec, 0, 100);
  display.drawRect(10, 30, 100, 10, WHITE);      // Outline
  display.fillRect(10, 30, barWidth, 10, WHITE); // Fill

  int minutesLeft = (totalSec - elapsedSec) / 60;
  int secondsLeft = (totalSec - elapsedSec) % 60;
  display.setCursor(0, 50);
  display.print("Time Left: ");
  display.print(minutesLeft);
  display.print("m ");
  display.print(secondsLeft);
  display.print("s");

  display.display();
}

void resetProgram()
{
  isRunning = false;
  totalHours = 1;
  studyMinutes = 25;
  restMinutes = 5;
  showSetupScreen();
}

void buzz(int duration)
{
  tone(BUZZ_PIN, 1000);
  delay(duration);
  noTone(BUZZ_PIN);
}

void countDown(int minutes, bool isStudy)
{
  for (int m = 0; m < minutes; m++)
  {
    for (int s = 0; s < 60; s++)
    {
      showProgress(m * 60 + s, minutes * 60, isStudy);
      delay(1000);

      if (rstBtn.isPressed())
      {
        resetProgram();
        return;
      }
    }
  }
}

void handleSetupInput()
{

  if (hourBtn.isPressed())
  {
    totalHours++;
    if (totalHours > 15)
    {
      totalHours = 1;
    }
    showSetupScreen();
    delay(200);
  }

  if (studyBtn.isPressed())
  {
    studyMinutes++;
    if (studyMinutes > 45)
    {
      studyMinutes = 25;
    }
    showSetupScreen();
    delay(200);
  }

  if (restBtn.isPressed())
  {
    restMinutes += 5;
    if (restMinutes > 15)
    {
      restMinutes = 5;
    }
    showSetupScreen();
    delay(200);
  }

  if (startBtn.isPressed())
  {
    isRunning = true;
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Starting Pomodoro...");
    display.display();
    delay(1000);
  }

  if (rstBtn.isPressed())
  {
    totalHours = 1;
    studyMinutes = 25;
    restMinutes = 5;
    showSetupScreen();
    delay(200);
  }
}

void setup()
{
  // put your setup code here, to run once:

  hourBtn.init();
  studyBtn.init();
  restBtn.init();
  startBtn.init();
  rstBtn.init();

  pinMode(BUZZ_PIN, OUTPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  showSetupScreen();
}

void loop()
{
  // put your main code here, to run repeatedly:
  if (!isRunning)
  {
    handleSetupInput();
  }
  else
  {

    if (isRunning)
    {
      totalBlocks = (totalHours * 60) / (studyMinutes + restMinutes);

      for (currentBlock = 0; currentBlock < totalBlocks; currentBlock++)
      {
        isResting = true;
        countDown(studyMinutes, true);
        if (!isRunning)
          return;
        buzz(1000);

        isResting = true;
        countDown(restMinutes, false);
        if (!isRunning)
          return;
        buzz(2000);
      }
    }

    display.clearDisplay();
    display.setCursor(0, 20);
    display.setTextSize(2);
    display.print("All done!");
    display.setTextSize(1);
    display.setCursor(0, 50);
    display.print("Great work :)");
    display.display();
    isRunning = false;
  }
}
