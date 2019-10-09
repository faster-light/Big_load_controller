/*	Прошивка предназначена для платы BFDriver с контроллерами ESP32 и STM32F103C8T6
Это STM часть прошивки. Здесь выполняется работа с низковольтной и высоковольтной нагрузкой

Команда ESP->STM		Разъем на плате		Контакт STM
------------------------- DC LOAD ----------------------
10XXX				  P1				PA0
11XXX				  P2				PA1
12XXX				  P3				PA2
13XXX				  P4				PA3
14XXX				  P5				PA6
15XXX				  P6				PA7
16XXX				  P7				PB0
17XXX				  P8				PB1
18XXX				  P9				PB6
19XXX				  P10				PB7
------------------------- AC LOAD ----------------------
20XXX				  P15				PB15
21XXX				  P12				PB13
22XXX				  P13				PB12
23XXX				  P14				PB14

Где XXX - от 100 до 200 - режим диммирования
*/

char incomingBytes[5];

void setup() {
	Serial1.begin(9600);
	Serial.begin(115200);
	HardwareTimer pwmtimer2(2);
//	HardwareTimer pwmtimer3(3);
//	HardwareTimer pwmtimer4(4);

	pinMode(PA0, PWM);
	pinMode(PA1, PWM);
	pinMode(PA2, PWM);
	pinMode(PA3, PWM);

	pinMode(PA6, PWM);
	pinMode(PA7, PWM);
	pinMode(PB0, PWM);
	pinMode(PB1, PWM);

	pinMode(PB6, PWM);
	pinMode(PB7, PWM);

	pwmWrite(PA0, 0);
	pwmWrite(PA1, 0);
	pwmWrite(PA2, 0);
	pwmWrite(PA3, 0);

	pwmWrite(PA6, 0);
	pwmWrite(PA7, 0);
	pwmWrite(PB0, 0);
	pwmWrite(PB1, 0);

	pwmWrite(PB6, 0);
	pwmWrite(PB7, 0);

	pwmtimer2.setPeriod(20);
//	pwmtimer3.setPeriod(20);
//	pwmtimer4.setPeriod(20);

	pinMode(PB15, OUTPUT);
	pinMode(PB12, OUTPUT);
	pinMode(PB13, OUTPUT);
	pinMode(PB14, OUTPUT);

	digitalWrite(PB15, LOW);
	digitalWrite(PB12, LOW);
	digitalWrite(PB13, LOW);
	digitalWrite(PB14, LOW);

}

char inc = 0;

void loop() {
	//pwmWrite(PA2, 585);
	//	Serial.println("check");
	delay(100);

	if (Serial1.available()) {

		Serial1.readBytes(incomingBytes, 5);

		int a1 = incomingBytes[0] - '0';
		int a2 = incomingBytes[1] - '0';

		int a = a1 * 10 + a2 * 1;

		int v1 = incomingBytes[2] - '0';
		int v2 = incomingBytes[3] - '0';
		int v3 = incomingBytes[4] - '0';

		int v = v1 * 100 + v2 * 10 + v3;
		v = (v - 100)*14.4;


		Serial.print(a);
		Serial.print(" ");
		Serial.println(v);

		if (a == 10) {
			pwmWrite(PA0, v);
		}

		if (a == 11) {
			pwmWrite(PA1, v);
		}

		if (a == 12) {
			pwmWrite(PA2, v);
		}

		if (a == 13) {
			pwmWrite(PA3, v);
		}

		if (a == 14) {
			pwmWrite(PA6, v);
		}

		if (a == 15) {
			pwmWrite(PA7, v);
		}

		if (a == 16) {
			pwmWrite(PB0, v);
		}

		if (a == 17) {
			pwmWrite(PB1, v);
		}

		if (a == 18) {
			pwmWrite(PB6, v);
		}

		if (a == 19) {
			pwmWrite(PB7, v);
		}



		if (a == 20) {
			digitalWrite(PB15, v / 14.4);
		}

		if (a == 21) {
			digitalWrite(PB13, v / 14.4);
		}

		if (a == 22) {
			digitalWrite(PB12, v / 14.4);
		}

		if (a == 23) {
			digitalWrite(PB14, v / 14.4);
		}

	}

}
