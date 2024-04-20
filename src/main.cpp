#include <Arduino.h>

///////////////////////////////////////////////////////////////

#pragma region INCLUDES SECTION

  #include <Adafruit_Sensor.h>
  #include <DHT.h>
  #include <DHT_U.h>
  #include <LiquidCrystal_I2C.h>

#pragma endregion

///////////////////////////////////////////////////////////////

#pragma region DEFINE SECTION

  #define TASK_DELAY 2000

  #define DHT_PIN 25
  #define DHT_TYPE DHT11

  #define I2C_SDA 21
  #define I2C_SCL 22

  #define QUEUE_LEN 5

#pragma endregion

///////////////////////////////////////////////////////////////

#pragma region DECLARE SECTION FOR GLOBAL VARIABLE

  DHT_Unified dht(DHT_PIN,DHT_TYPE);
  sensor_t sensor;
  sensors_event_t event;

  LiquidCrystal_I2C lcd(0x27,16, 2);

  QueueHandle_t q_temp;
  QueueHandle_t q_humd;

#pragma endregion

///////////////////////////////////////////////////////////////

#pragma region FUNCTION PROTOTYPE SECTION
  
  void printSensorInfo();

  void setTempAndHumid(void* parameter);

  void printTemp2Console(void* parameter);
  void printTemp2LCD(void* parameter);

  void printHumid2Console(void* parameter);
  void printHumid2LCD(void* parmetrer);

#pragma endregion

///////////////////////////////////////////////////////////////

#pragma region SETUP SECTION

  void setup() {
    
    Serial.begin(115200);
    dht.begin();

    printSensorInfo();

    q_temp = xQueueCreate(QUEUE_LEN, sizeof(float)); // initialize the Temperature's queue
    q_humd = xQueueCreate(QUEUE_LEN, sizeof(float)); // initialize the Humidity's queue

    xTaskCreatePinnedToCore(setTempAndHumid, "setTempAndHumid", 2024, NULL, 2, NULL, 0);
    // xTaskCreatePinnedToCore(printTemp2Console, "printTemp2Console", 2024, NULL, 1, NULL,0);
    xTaskCreatePinnedToCore(printTemp2LCD, "printTemp2LCD", 2024, NULL, 1, NULL,0);
    // xTaskCreatePinnedToCore(printHumid2Console, "printHumid2Console", 2024, NULL, 1, NULL,0);
    xTaskCreatePinnedToCore(printHumid2LCD, "printHumid2LCD", 2024, NULL, 1, NULL,0);

    lcd.init(I2C_SDA, I2C_SCL); // initialize the lcd to use user defined I2C pins
    lcd.backlight();
    lcd.clear();
  }
#pragma endregion

void loop() {
  // put your main code here, to run repeatedly:
}

///////////////////////////////////////////////////////////////

#pragma region FUNCTION IMPLEMENTATION SECTION

  void printSensorInfo(){
    dht.temperature().getSensor(&sensor);
    Serial.println(F("------------------------------------"));
    Serial.println(F("Temperature Sensor"));
    Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
    Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
    Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
    Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("°C"));
    Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("°C"));
    Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("°C"));
    Serial.println(F("------------------------------------"));
    // Print humidity sensor details.
    dht.humidity().getSensor(&sensor);
    Serial.println(F("Humidity Sensor"));
    Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
    Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
    Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
    Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("%"));
    Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("%"));
    Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("%"));
    Serial.println(F("------------------------------------"));
  }

  void setTempAndHumid(void* parmeter){
    float temp = 0;
    float humid = 0;
    while (1)
    {
      vTaskDelay(TASK_DELAY/portTICK_PERIOD_MS);
      dht.temperature().getEvent(&event);
      if(isnan(event.temperature)){
        Serial.println(F("Error reading temperature!"));

        continue;
      }
      temp = event.temperature;
      xQueueSend(q_temp, &temp, TASK_DELAY/portTICK_PERIOD_MS);

      dht.humidity().getEvent(&event);
      if(isnan(event.relative_humidity)){
        Serial.println(F("Error reading humidity!"));

        continue;
      }
      
      humid = event.relative_humidity;
      // Serial.print("In setTemperature, Temperature = ");
      // Serial.println(temp);
      xQueueSend(q_humd, &humid, TASK_DELAY/portTICK_PERIOD_MS);

    }
    
  }

  void printTemp2Console(void* parameter){

    float temp = 0;
    while(true){
      vTaskDelay(TASK_DELAY/portTICK_PERIOD_MS);

      xQueueReceive(q_temp, &temp, TASK_DELAY/portTICK_PERIOD_MS);
      Serial.print(F("Temperature: "));
      Serial.print(temp);
      Serial.println(F("°C"));
      // lcd.clear();
      // lcd.setCursor(0,0);
      // lcd.print(F("Temp:"));
      // lcd.setCursor(6,0);
      // lcd.print(temperature);
      // lcd.setCursor(15,0);
      // lcd.print("C");
   
    }
  }

  void printTemp2LCD(void* parameter){
    float temp = 0;
    while(true){
      vTaskDelay(TASK_DELAY/portTICK_PERIOD_MS);

      xQueueReceive(q_temp, &temp, TASK_DELAY/portTICK_PERIOD_MS);
      lcd.setCursor(0,0);
      lcd.print("Temp:");
      lcd.setCursor(7,0);
      lcd.print(temp);
      lcd.setCursor(15,0);
      lcd.print("C");
   
    }
  }
  
  void printHumid2Console(void* parameter){

    float humid = 0;

    while(true){
      vTaskDelay(TASK_DELAY/portTICK_PERIOD_MS);

      xQueueReceive(q_humd, &humid, TASK_DELAY/portTICK_PERIOD_MS);
      Serial.print(F("Humidity: "));
      Serial.print(humid);
      Serial.println(F("%"));
      // lcd.clear();
      // lcd.setCursor(0,1);
      // lcd.print(F("Humid:"));
      // lcd.setCursor(7,1);
      // lcd.print(humidity);
      // lcd.setCursor(15,1);
      // lcd.print("%");
    
    }
  }

  void printHumid2LCD(void* parameter){
  
    float humid = 0;
    while(true){
      vTaskDelay(TASK_DELAY/portTICK_PERIOD_MS);

      xQueueReceive(q_humd, &humid, TASK_DELAY/portTICK_PERIOD_MS);
      lcd.setCursor(0,1);
      lcd.print("Humid:");
      lcd.setCursor(7,1);
      lcd.print(humid);
      lcd.setCursor(15,1);
      lcd.print("%");
   
    }
  }

#pragma endregion