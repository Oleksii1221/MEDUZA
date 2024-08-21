#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h> 
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "quaternion.h"

using namespace imu;

Adafruit_MPU6050 mpu;
sensors_event_t a, g, temp;

uint32_t lastSendTime = 0;

double deltaX, deltaY, deltaZ;
double angleX, angleY, angleZ;

Quaternion orientation, qDeviation;
Vector<3> eulerAngles;


#define CHANNEL 1

/** This is all the data about the peer **/
esp_now_peer_info_t slave;

/** The all important data! **/
float data[2] = {1,2};

// Прототипи функцій
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void ScanForSlave();



void setup()
{
 WiFi.mode(WIFI_STA);
  esp_now_init();
  esp_now_register_send_cb(OnDataSent);
  ScanForSlave(); // WiFi.macAddress()
  esp_now_add_peer(&slave);

	// Init Serial Monitor
	Serial.begin(115200);

	if (!mpu.begin())
	{
		Serial.println("Failed to find MPU6050 chip");
		while (1)
		{
			delay(10);
		}
	}

	mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
	mpu.setGyroRange(MPU6050_RANGE_2000_DEG);
	mpu.setFilterBandwidth(MPU6050_BAND_260_HZ);
	delay(100);

	// Розрахуємо відхилення за допомогою знаходження середнього значення (500 ітерацій)
	for (size_t i = 0; i < 500; i++)
	{
		// Читаємо дані з давача
		mpu.getEvent(&a, &g, &temp);
		deltaX = deltaX + g.gyro.x; //
		deltaY = deltaY + g.gyro.y; // сумуємо для кожної з осей значення від давачів
		deltaZ = deltaZ + g.gyro.z; //
	}
	
	deltaX = deltaX / 500; //
	deltaY = deltaY / 500; // Ділимо кожне значення на кількість ітерацій
	deltaZ = deltaZ / 500; // і знаходимо середнє
}

void loop()
{	
	if((millis() - lastSendTime) >= 10) {
	  // Отримуємо дані від давача
		mpu.getEvent(&a, &g, &temp);

		// Створення кватерніона з кутових швидкостей (віднімаємо біаси)
		Quaternion qW(0, g.gyro.x - deltaX, g.gyro.y - deltaY, g.gyro.z - deltaZ);

		// Розраховуємо диференційне рівнняння орієнтації
		//       1
		// q' = --- * q * w
		//       2 
		// q - кватерніон орієнтації
		// w - кватерніон кутових швидкостей
		qDeviation = orientation * qW * 0.5;


		// Інтегруємо диференційне рівняння та отримуємо нову орієнтацію
		orientation = orientation + qDeviation * 0.01;
		//                 ^             ^        ^
		//             попередня     диференц.  проміжок
		//            орієнтація     рівняння    часу


		// Нормалізуємо кватерніон (довжина вектора кватерніона має бути рівна 1)
		// q = sqrt(w^2 + x^2 + y^2 + z^2)
		orientation.normalize();
		

		// Перетворення кватерніона в кути Ейлера
		eulerAngles = orientation.toEuler();

		
		Serial.print(orientation.w()); //
		Serial.print("\t"); //
		Serial.print(orientation.x()); // Виводимо значення кватерніона в послідовний порт
		Serial.print("\t"); //
		Serial.print(orientation.y()); //
		Serial.print("\t"); //
		Serial.print(orientation.z()); //

		Serial.print("\t\t"); //
		Serial.print(eulerAngles.x()); // Виводимо значення кутів Ейлера в послідовний порт
		Serial.print("\t"); //
		Serial.print(eulerAngles.y()); //
		Serial.print("\t"); //
		Serial.print(eulerAngles.z()); //
		Serial.println();     //
		
        data[0] = eulerAngles.x() * 180 / M_PI;
        data[1] = eulerAngles.z() * 180 / M_PI ; 
        if (data[0] > 180 ){data[0] = 180;}
        if (data[0] < 0 ){data[0] = 0;}
        if (data[1] > 180 ){data[1] = 180;}
        if (data[1] < 0 ){data[1] = 0;}
        esp_now_send(slave.peer_addr, (uint8_t *)data, sizeof(data));


		lastSendTime = millis();
	}
}
void ScanForSlave() {
  int8_t scanResults = WiFi.scanNetworks();

  for (int i = 0; i < scanResults; ++i) {
    String SSID = WiFi.SSID(i);
    String BSSIDstr = WiFi.BSSIDstr(i);

    if (SSID.indexOf("Meduza") == 0) {

      int mac[6];
      if ( 6 == sscanf(BSSIDstr.c_str(), "%x:%x:%x:%x:%x:%x",  &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5] ) ) {
        for (int ii = 0; ii < 6; ++ii ) {
          slave.peer_addr[ii] = (uint8_t) mac[ii];
        }
      }

      slave.channel = CHANNEL; // pick a channel
      slave.encrypt = 0; // no encryption
      break;
    }
  }
}

/** callback when data is sent from Master to Slave **/
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("I sent my data -> ");
  
  
}
