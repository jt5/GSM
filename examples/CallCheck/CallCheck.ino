/*
Пример входящего вызова.
Используется плата Ардуино, шилд «Cosmo GSM Connect».
Если номер входящего вызова совпал с указанным в скетче номером, ему в ответ отправляется СМС сообщение.
*/

#include <GSM.h>
#include <avr/pgmspace.h>

const char RemoteID[] PROGMEM =  "+79280000000"; //Шаблон номера, с которого ждём звонка и на который отправляем СМС
const char PIN[] PROGMEM =  "0000"; //ПИН-код! Поменяйте на свой!!! Иначе СИМ карта заблокируется!

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
	if (IncomingCall == 1) { //пришёл звонок!
		//Отбиваем
		while (GSM.TerminateCall()<0);
		IncomingCall = 0;
		if (strstr_P(CallerID, RemoteID)) {
			// Номер звонящего совпал с шаблоном
			while(GSM.SendSMS(RemoteID, "Hello from Arduino!") < 0);// SMS из SRAM памяти
		}
		//Ждём звонка
		GSM.WaitCall();
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