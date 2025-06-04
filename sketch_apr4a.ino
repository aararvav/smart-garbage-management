//.......................................................................................................................................................................

#define BLYNK_TEMPLATE_ID "TMPL6g6zRy_4G"
#define BLYNK_TEMPLATE_NAME "Garbage Management Dustbin"
#define BLYNK_AUTH_TOKEN "7XAS_3mzj3VkT1vkL26mxnWCkWNASHpR"

#include <ESP32Servo.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

#define TRIG_PIN_1 13
#define ECHO_PIN_1 12

#define TRIG_PIN_2 27
#define ECHO_PIN_2 26

#define SERVO_PIN 14

char auth[] = "7XAS_3mzj3VkT1vkL26mxnWCkWNASHpR";
char ssid[] = "aarav";
char pass[] = "ekseaath";

Servo myServo;

int servoOpenAngle = 180;
int servoCloseAngle = 0;

bool isOpen = false;
unsigned long lastCloseTime = 0;

// Blynk event variables
unsigned long binFullStartTime = 0;
bool binHasBeenFull = false;
unsigned long lastNotificationTime = 0;

void setup() {
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);

  pinMode(TRIG_PIN_1, OUTPUT);
  pinMode(ECHO_PIN_1, INPUT);

  pinMode(TRIG_PIN_2, OUTPUT);
  pinMode(ECHO_PIN_2, INPUT);

  myServo.attach(SERVO_PIN);
  myServo.write(servoCloseAngle);
}

float readDistanceCM(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);

  if (duration == 0) {
    Serial.println("No Echo Received from sensor");
    return 999;
  }

  float distance = duration * 0.034 / 2;
  return distance;
}

void loop() {
  Blynk.run();

  float distance1 = readDistanceCM(TRIG_PIN_1, ECHO_PIN_1);
  float distance2 = readDistanceCM(TRIG_PIN_2, ECHO_PIN_2);

  Serial.print("Distance 1 (Lid Control): ");
  Serial.print(distance1);
  Serial.println(" cm");

  Serial.print("Distance 2 (Bin Level Reading): ");
  Serial.print(distance2);
  Serial.println(" cm");

  // Lid control logic
  if (distance1 < 15 && distance1 > 1) {
    myServo.write(servoOpenAngle);
    isOpen = true;
    lastCloseTime = millis();
  } else if (isOpen && millis() - lastCloseTime >= 5000) {
    myServo.write(servoCloseAngle);
    isOpen = false;
  }

  // Bin full detection with 8 second rule and 2-minute cooldown
  if (distance2 < 10) {
    if (binFullStartTime == 0) {
      binFullStartTime = millis();
    } else if (!binHasBeenFull && millis() - binFullStartTime >= 2000) {
      if (millis() - lastNotificationTime >= 60000 ) { //120000 for 2 minutes ; 60000 for 1 min ; 30000 for 30 seconds 
        Blynk.logEvent("bin_full"); // Must match event name on Blynk dashboard
        Serial.println("Sent logEvent!");
        lastNotificationTime = millis();
      } else {
        Serial.println("Notification skipped due to cooldown");
      }
      binHasBeenFull = true;
    }
  } else {
    binFullStartTime = 0;
    binHasBeenFull = false;
  }

  delay(500);
}
