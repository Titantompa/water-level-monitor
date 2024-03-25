#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <TaskScheduler.h>
#include <NeoPixelBus.h>

#pragma region Constants Typedefs

#define UPPER_SENSOR_PIN 30
#define LOWER_SENSOR_PIN 31

#define PIXEL_PIN 33

// This is the refresh interval in milliseconds for the sensor reading and LED update.
#define REFRESH_INTERVAL_MS 1000

// This is the pulse length in milliseconds for the flashing effect. I e half a cycle.
#define FLASHING_PULSE_MS 250

const static RgbColor Black(0, 0, 0);
const static RgbColor White(255, 255, 255);
const static RgbColor Red(255, 0, 0);
const static RgbColor Green(0, 255, 0);
const static RgbColor Blue(0, 0, 255);
const static RgbColor Yellow(255, 255, 0);
const static RgbColor Purple(255, 0, 255);

typedef NeoGrbFeature MyPixelColorFeature;
typedef NeoEsp8266DmaWs2812xMethod MyPixelColorMethod;

#pragma endregion

#pragma region Tasks

class ProbeSensorsTask : public Task
{
  private:
    NeoPixelBus<MyPixelColorFeature, MyPixelColorMethod> &_neoPixel;

    void execute()
    {
      bool lowerSensor = digitalRead(LOWER_SENSOR_PIN) == LOW;
      bool upperSensor = digitalRead(LOWER_SENSOR_PIN) == LOW;
      bool flashing = false;

      if(!lowerSensor && !upperSensor)
      {
        Serial.println("No sensors triggered.");
        flashing = true;
        _neoPixel.ClearTo(Red);
      }
      else if(lowerSensor && !upperSensor)
      {
        Serial.println("Lower sensor triggered.");
        _neoPixel.ClearTo(Green);
      }
      else if(!lowerSensor && upperSensor)
      {
        Serial.println("Only upper sensor triggered.");
        flashing = true;
        _neoPixel.ClearTo(Purple);
      }
      else if(lowerSensor && upperSensor)
      {
        Serial.println("Both sensors triggered.");
        _neoPixel.ClearTo(Yellow);
      }

      if(flashing && (millis() / FLASHING_PULSE_MS) & 1)
      {
        _neoPixel.ClearTo(Black);
      }

      _neoPixel.Show();
    }

  public:
    ProbeSensorsTask(Scheduler &scheduler, NeoPixelBus<MyPixelColorFeature, MyPixelColorMethod> &strip)
    : Task(REFRESH_INTERVAL_MS, TASK_FOREVER, [this]{execute();}, &scheduler, false),  _neoPixel(strip)
  {

  }
};

#pragma endregion

#pragma region Globals

NeoPixelBus<MyPixelColorFeature, MyPixelColorMethod> NeoPixel(1, PIXEL_PIN);

Scheduler TaskScheduler;

ProbeSensorsTask Refresher(TaskScheduler, NeoPixel);

#pragma endregion

void setup()
{
  // Turn off WiFi, to save power and preserve the ether
  WiFi.mode(WIFI_OFF);

  pinMode(PIXEL_PIN, OUTPUT);
  pinMode(LOWER_SENSOR_PIN, INPUT_PULLUP);
  pinMode(UPPER_SENSOR_PIN, INPUT_PULLUP);

  Serial.begin(115200);

  // NOTE: THIS MUST BE COMMENTED OUT WHEN NOT CONNECTED TO A COMPUTER!
  while (!Serial && millis() < 10000)
  {
    delay(100);
  }

  NeoPixel.Begin();
  NeoPixel.ClearTo(Black);
  NeoPixel.Show();

  TaskScheduler.enable();
  Refresher.enable();
}

void loop()
{
  // put your main code here, to run repeatedly:
  TaskScheduler.execute();
}

