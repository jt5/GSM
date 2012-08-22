/*
Пример мониторинга температуры.
Используется плата Ардуино, шилд «Cosmo GSM Connect».
В скетче задается температурный интервал, при его достижении отправляется тревожное СМС-сообщение.
При возвращении показаний датчика в нормальный температурный диапазон — отправляется соответствующее СМС уведомление.
Также есть возможность получения показания с датчиков по входящему на шилд вызову.
*/

#include <GSM.h>
#include <avr/pgmspace.h>

const char RemoteID[] PROGMEM =  "+79280000000"; // Шаблон номера, с которого ждём звонка и на который отправляем СМС
const char PIN[] PROGMEM =  "0000"; // ПИН-код!

unsigned long interval = 60000; // Интервал проверки состояния GPRS соединения; 60 секунд
unsigned long currentMillis;
unsigned long previousMillis;

#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 8 // Номер линии, к которой подключены датчики температуры
#define TEMPERATURE_PRECISION 9
boolean alarm = false;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress sensor_1, sensor_2; // Используем 2 температурных датчика

unsigned long previousMillisTemp = 0;
unsigned long intervalTemp = 1500; // Интервал опроса датчиков; 1,5 секунды

String textMessage;
char s[32];
float tempS1;
float tempS2;

int maxValue = 30;


void setup() {
  sensors.begin();
  if (!sensors.getAddress(sensor_1, 0)) {}; 
  if (!sensors.getAddress(sensor_2, 1)) {};
  while (GSM.Init(PIN)<0);
  GSM.NewSMSindic();
  GSM.WaitCall();    
  GSM.WaitSMS();
  previousMillis=millis();
}

void loop() {        
  // Начало отсчёта 60 сек.       
  currentMillis = millis();
  
  // Проверяем входящие вызововы
  if (IncomingCall == 1) { //пришёл звонок!
    while (GSM.TerminateCall()<0);
    IncomingCall = 0;
    if (strstr_P(CallerID, RemoteID)) { // Номер звонящего совпал с шаблоном
      sensors.requestTemperatures();
      tempS1 = sensors.getTempC(sensor_1);
      tempS2 = sensors.getTempC(sensor_2);
      sendData(0, tempS1, tempS2, alarm);
    }
    GSM.WaitCall();
    currentMillis = previousMillis = millis();
  }
  
  // Пора проверить состояние GPRS подключения
  if(currentMillis - previousMillis > interval) {
    if ((GSM.CheckStatus() != 1)) { // GPRS соединение не установлено!
      while (GSM.Init(PIN)<0); // Делаем инициализацию
    }
    GSM.WaitCall();
    currentMillis = previousMillis = millis();
  }

  // Считываем показания датчиков температуры
  if(currentMillis - previousMillisTemp > intervalTemp) {
    previousMillisTemp = currentMillis;
    
    sensors.requestTemperatures();
    tempS1 = sensors.getTempC(sensor_1);
    tempS2 = sensors.getTempC(sensor_2);
    
    if(tempS1 > maxValue && !alarm) {
      sendData(1, tempS1, tempS2, true);
    }
    if(tempS1 < maxValue && alarm) {
      sendData(2, tempS1, tempS2, false);
    }
    
  }
 
}


void sendData(int flag, float temp1, float temp2, boolean param) {
  if(flag == 0) {
    textMessage = "Temperature: ";
  }
  if(flag == 1) {
    textMessage = "Alarm! ";
  }
  if(flag == 2) {
    textMessage = "Now temperature is OK ";
  }
  textMessage += "Sensor1 = ";
  textMessage += dtostrf(temp1, 2, 2, s);
  textMessage += " Sensor2 = ";
  textMessage += dtostrf(temp2, 2, 2, s);
  char Out[textMessage.length()+1];
  textMessage.toCharArray(Out,(textMessage.length())+1);
  while(GSM.SendSMS(RemoteID, Out) < 0);
  alarm = param;
}