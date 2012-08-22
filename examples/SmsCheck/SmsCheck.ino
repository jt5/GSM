/*
Пример разбора СМС-сообщения.
Используется плата Ардуино, шилд «Cosmo GSM Connect».
Все входящие СМС разбираютс и в зависимости от текста сообщений выполняются логические действия.
*/

#include <GSM.h>
#include <avr/pgmspace.h>

const char RemoteID[] PROGMEM =  "+79280000000"; //Шаблон номера, с которого ждём звонка и на который отправляем СМС
const char PIN[] PROGMEM =  "0000"; //ПИН-код! Поменяйте на свой!!! Иначе СИМ карта заблокируется!
char SMStxt[162]; // Буфер для чтения текста СМС
String strSMS;
unsigned long interval = 60000; //Интервал проверки состояния GPRS соединения
unsigned long currentMillis;
unsigned long previousMillis;

void setup() {
    delay(4000);
    while (GSM.Init(PIN)<0);
    GSM.NewSMSindic();
    GSM.WaitCall();    
    GSM.WaitSMS();
    previousMillis=millis();
}

void loop() {        
	// Начало отсчёта 60 сек.       
	currentMillis = millis();
	if (GSM.CheckSMS()== 1) { //Есть новое СМС
		GSM.ReadSMS(NewSMS_index, (char*) SMStxt); //
		// Удалить СМСки (накапливать не будем!)
		GSM.DeleteAllSMS();
        strSMS = String(SMStxt);
		if(strSMS == "1") {
			while(GSM.SendSMS(RemoteID, "It is 1") < 0);
		}
		if(strSMS == "2") {
			while(GSM.SendSMS(RemoteID, "It is 2") < 0);
		}
		//GSM.WriteStr(SMStxt);
		//GSM.WriteStr_P(PSTR("\r\n"));
		GSM.WaitCall();
		GSM.WaitSMS();
		// Начало отсчёта 60 сек.       
		currentMillis = previousMillis = millis();   
	}

    if(currentMillis - previousMillis > interval) { // Пора проверить состояние GPRS подключения!
		if ((GSM.CheckStatus() != 1)) { // GPRS соединение не установлено!
			while (GSM.Init(PIN)<0); // Делаем инициализацию
		}
		//Ждём звонка
		GSM.WaitCall();
		// Начало отсчёта 60 сек.       
		currentMillis = previousMillis = millis();   
	}
}