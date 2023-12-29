#include <Arduino.h>

const int maxValue = 120;
const int minValue = 0;
const int delayMs = 10;
const int blockTimeMs = 30000;

const int lights[8] = {1, 3, 5, 7, 9, 11, 13, 15}; // Od skrajny dół do skrajna góra
const int DUSK_SENSOR = 28;                        // Czujnik zmierzchu. Wysoki - ciemno, niski - jasno
const int DOWN_MOTION_SENSOR = 20;                 // Czujnik ruchu dół. Wysoki - ruch, niski - brak ruchu
const int UP_MOTION_SENSOR = 21;                   // Czujnik ruchu góra. Wysoki - ruch, niski - brak ruchu

bool states[8] = {false, false, false, false, false, false, false, false};
int upNextStepOn;
int upNextStepOff;
int downNextStepOn;
int downNextStepOff;
bool upLightOnInProgress;
bool upLightOffInProgress;
bool upLightOn;
bool downLightOnInProgress;
bool downLightOffInProgress;
bool downLightOn;

bool blockLightDown;
bool blockLightUp;
unsigned long blockStartTime;

bool anyStateOn();
void nextLightUpOn();
void nextLightUpOff();
void nextLightDownOn();
void nextLightDownOff();
void allOff();
void upOff();
void releaseBlocksIfNeeded();

void setup()
{
  upNextStepOn = 0;
  upNextStepOff = 0;
  downNextStepOn = 0;
  downNextStepOff = 0;
  blockStartTime = 0;
  upLightOnInProgress = false;
  upLightOffInProgress = false;
  upLightOn = false;
  downLightOnInProgress = false;
  downLightOffInProgress = false;
  downLightOn = false;
  blockLightDown = false;
  blockLightUp = false;

  pinMode(LED_BUILTIN, OUTPUT);
  for (int i = 0; i < 8; i++)
  {
    pinMode(lights[i], OUTPUT);
  }

  pinMode(DUSK_SENSOR, INPUT);
  pinMode(DOWN_MOTION_SENSOR, INPUT);
  pinMode(UP_MOTION_SENSOR, INPUT);
}

void loop()
{
  if (digitalRead(DUSK_SENSOR) == HIGH) // Ciemno
  {
    if (digitalRead(UP_MOTION_SENSOR) == HIGH && !blockLightUp)
    {
      digitalWrite(LED_BUILTIN, HIGH);
      if (!upLightOn)
      {
        if (!blockLightDown)
        {
          blockLightDown = true;
          blockStartTime = millis();
        }

        upLightOn = true;
        upLightOnInProgress = true;
      }
    }
    else
    {
      digitalWrite(LED_BUILTIN, LOW);
      if (upLightOn)
      {
        upLightOn = false;
        upLightOffInProgress = true;
      }
    }

    if (digitalRead(DOWN_MOTION_SENSOR) == HIGH && !blockLightDown)
    {
      digitalWrite(LED_BUILTIN, HIGH);
      if (!downLightOn)
      {
        if (!blockLightUp)
        {
          blockLightUp = true;
          blockStartTime = millis();
        }
        downLightOn = true;
        downLightOnInProgress = true;
      }
    }
    else
    {
      digitalWrite(LED_BUILTIN, LOW);
      if (downLightOn)
      {
        downLightOn = false;
        downLightOffInProgress = true;
      }
    }
    if (upLightOnInProgress)
    {
      nextLightUpOn();
    }
    if (upLightOffInProgress)
    {
      nextLightUpOff();
    }
    if (downLightOnInProgress)
    {
      nextLightDownOn();
    }
    if (downLightOffInProgress)
    {
      nextLightDownOff();
    }
  }
  else
  {
    blockLightDown = false;
    blockLightUp = false;
    blockStartTime = 0;
    if (anyStateOn())
    {
      allOff();
      upLightOn = false;
      upLightOnInProgress = false;
      downLightOn = false;
      downLightOffInProgress = false;
      upNextStepOn = 0;
      upNextStepOff = 0;
      downNextStepOn = 7;
      downNextStepOff = 7;
    }
  }

  releaseBlocksIfNeeded();
  delay(1);
}

bool anyStateOn()
{
  for (size_t i = 0; i < 8; i++)
  {
    if (states[i])
    {
      return true;
    }
  }
  return false;
}

void nextLightUpOn()
{
  if (states[upNextStepOn] == false)
  {
    for (int i = minValue; i <= maxValue; i++)
    {
      analogWrite(lights[upNextStepOn], i);
      delay(delayMs);
    }
    states[upNextStepOn] = true;
  }

  if (upNextStepOn == 7)
  {
    upNextStepOn = 0;
    upLightOnInProgress = false;
  }
  else
  {
    upNextStepOn += 1;
  }
}

void nextLightUpOff()
{
  if (states[upNextStepOff] == true)
  {
    for (int i = maxValue; i >= minValue; i--)
    {
      analogWrite(lights[upNextStepOff], i);
      delay(delayMs);
    }
    states[upNextStepOff] = false;
  }

  if (upNextStepOff == 7)
  {
    upNextStepOff = 0;
    upLightOffInProgress = false;
  }
  else
  {
    upNextStepOff += 1;
  }
}

void nextLightDownOn()
{
  if (states[downNextStepOn] == false)
  {
    for (int i = minValue; i <= maxValue; i++)
    {
      analogWrite(lights[downNextStepOn], i);
      delay(delayMs);
    }
    states[downNextStepOn] = true;
  }

  if (downNextStepOn == 0)
  {
    downNextStepOn = 7;
    downLightOnInProgress = false;
  }
  else
  {
    downNextStepOn -= 1;
  }
}

void nextLightDownOff()
{
  if (states[downNextStepOff] == true)
  {
    for (int i = maxValue; i >= minValue; i--)
    {
      analogWrite(lights[downNextStepOff], i);
      delay(delayMs);
    }
    states[downNextStepOff] = false;
  }

  if (downNextStepOff == 0)
  {
    downNextStepOff = 7;
    downLightOffInProgress = false;
  }
  else
  {
    downNextStepOff -= 1;
  }
}

void allOff()
{
  for (int i = maxValue; i >= minValue; i--)
  {
    for (int j = 0; j < 8; j++)
    {
      if (states[j] == true)
      {
        analogWrite(lights[j], i);
      }
    }
    delay(delayMs);
  }

  for (int j = 0; j < 8; j++)
  {
    states[j] = false;
  }
}

void upOff()
{
  for (int i = maxValue; i >= minValue; i--)
  {
    analogWrite(1, i);
    delay(delayMs);
  }
  for (int i = maxValue; i >= minValue; i--)
  {
    analogWrite(3, i);
    delay(delayMs);
  }
  for (int i = maxValue; i >= minValue; i--)
  {
    analogWrite(5, i);
    delay(delayMs);
  }
  for (int i = maxValue; i >= minValue; i--)
  {
    analogWrite(7, i);
    delay(delayMs);
  }
  for (int i = maxValue; i >= minValue; i--)
  {
    analogWrite(9, i);
    delay(delayMs);
  }
  for (int i = maxValue; i >= minValue; i--)
  {
    analogWrite(11, i);
    delay(delayMs);
  }
  for (int i = maxValue; i >= minValue; i--)
  {
    analogWrite(13, i);
    delay(delayMs);
  }
  for (int i = maxValue; i >= minValue; i--)
  {
    analogWrite(15, i);
    delay(delayMs);
  }
}

void releaseBlocksIfNeeded()
{
  if (!blockLightDown && !blockLightUp)
  {
    return;
  }

  unsigned long currentMillis = millis();

  if (blockStartTime > currentMillis)
  {
    blockStartTime = currentMillis;
    return;
  }

  if (currentMillis - blockStartTime > blockTimeMs)
  {
    blockStartTime = 0;
    blockLightDown = false;
    blockLightUp = false;
  }
}