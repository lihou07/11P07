
#include <Servo.h>

// Arduino pin assignment
#define PIN_LED 9
#define PIN_TRIG 12
#define PIN_ECHO 13
#define PIN_SERVO 10

// Configurable parameters
#define SND_VEL 346.0
#define INTERVAL 100
#define PULSE_DURATION 10

#define _DIST_MIN 180.0
#define _DIST_MAX 360.0

#define _DUTY_MIN 1000
#define _DUTY_MAX 2000
#define _DUTY_NEU ((_DUTY_MIN + _DUTY_MAX) / 2)

#define ALPHA 0.3

#define TIMEOUT ((INTERVAL / 2) * 1000UL)
#define SCALE (0.001 * 0.5 * SND_VEL)

Servo myservo;

float dist_raw = 270.0;
float dist_ema = 270.0;

void setup() {
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  myservo.attach(PIN_SERVO, _DUTY_MIN, _DUTY_MAX);
  myservo.writeMicroseconds(_DUTY_NEU);

  digitalWrite(PIN_LED, LOW);

  Serial.begin(57600);
}

void loop() {
  unsigned long last_time = millis();

  // Ultrasonic sensor trigger
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(PIN_TRIG, LOW);

  // Measure distance (mm)
  unsigned long duration = pulseIn(PIN_ECHO, HIGH, TIMEOUT);

  if (duration > 0) {
    dist_raw = duration * SCALE;

    // Range filter and LED control
    if (dist_raw >= _DIST_MIN && dist_raw <= _DIST_MAX) {
      digitalWrite(PIN_LED, HIGH);
    } else {
      digitalWrite(PIN_LED, LOW);
    }

    // Limit distance to 18~36cm
    float dist_filtered = constrain(dist_raw, _DIST_MIN, _DIST_MAX);

    // EMA filter
    dist_ema = ALPHA * dist_filtered
             + (1.0 - ALPHA) * dist_ema;

    // Convert distance to servo angle
    float angle = (dist_ema - _DIST_MIN) * 180.0
                / (_DIST_MAX - _DIST_MIN);

    angle = constrain(angle, 0.0, 180.0);

    // Convert angle to PWM pulse width
    int duty = _DUTY_MIN
             + (int)(angle * (_DUTY_MAX - _DUTY_MIN) / 180.0);

    myservo.writeMicroseconds(duty);
  } else {
    // Keep previous position when measurement fails
    digitalWrite(PIN_LED, LOW);
  }

  // Serial plotter output
  Serial.print("Min:");
  Serial.print(_DIST_MIN);

  Serial.print(",dist:");
  Serial.print(dist_raw);

  Serial.print(",ema:");
  Serial.print(dist_ema);

  Serial.print(",Servo:");
  Serial.print(myservo.read());

  Serial.print(",Max:");
  Serial.print(_DIST_MAX);

  Serial.println("");

  // Maintain sampling interval
  while (millis() - last_time < INTERVAL) {
  }
}
