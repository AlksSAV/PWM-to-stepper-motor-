/*
Controlling a Stepper Motor with a Remote Control
I demonstrate how to use Arduino and remote control to control the speed of a stepper motor. 
*/
/*
Controlling a Stepper Motor with a Remote Control
Improved version with better signal processing, safety features and smoother control.
*/

// Define Pins
#define RCPin 2
#define DIR 4
#define STEP 5
#define MS3 6
#define MS2 7
#define MS1 8
#define LED_PIN 13  // Индикатор состояния

// Set up time variables for Stepper motor
unsigned long previousMotorTime = 0;
unsigned long stepStartTime = 0;
long MotorInterval = 1000; // Default slow speed
bool stepState = false;
const int minStepPulse = 5; // Минимальная длительность импульса в микросекундах

// Set up time variables for RC Read
volatile unsigned long pulseStart = 0;
volatile int pulseWidth = 1500; // Default to neutral position
volatile bool newSignal = false;
const int signalTimeout = 100; // Максимальное время между сигналами в мс
unsigned long lastSignalTime = 0;

// Speed control parameters
const int minSpeed = 1;     // Минимальный интервал (максимальная скорость)
const int maxSpeed = 2000;  // Максимальный интервал (минимальная скорость)
const int deadZoneMin = 1450; // Начало мертвой зоны
const int deadZoneMax = 1550; // Конец мертвой зоны
const int minPulse = 1000;   // Минимальная ширина импульса
const int maxPulse = 2000;   // Максимальная ширина импульса

void setup() {
  Serial.begin(115200);  // Увеличена скорость для лучшей отладки

  // RC Read Setup
  pinMode(RCPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RCPin), risingPulse, RISING); 

  // Set pins 4-8 to Outputs
  for (int i = 4; i <= 8; i++) {
    pinMode(i, OUTPUT);
  }
  
  pinMode(LED_PIN, OUTPUT);

  // Set MS1 to full step mode
  digitalWrite(MS1, LOW);
  digitalWrite(MS2, LOW);
  digitalWrite(MS3, LOW);
  
  // Initialize motor direction
  digitalWrite(DIR, LOW);
  digitalWrite(STEP, LOW);
  
  Serial.println("Stepper motor RC control initialized");
}

void loop() {
  // Check for signal timeout
  if (millis() - lastSignalTime > signalTimeout) {
    // Signal lost - stop motor
    MotorInterval = maxSpeed; // Slowest speed
    digitalWrite(LED_PIN, LOW); // Turn off LED to indicate signal loss
  } else if (newSignal) {
    // Process new RC signal
    newSignal = false;
    
    // Validate signal range
    if (pulseWidth >= minPulse && pulseWidth <= maxPulse) {
      // Map pulse width to motor speed
      if (pulseWidth >= deadZoneMin && pulseWidth <= deadZoneMax) {
        // Motor doesn't move if the joystick is near the midpoint
        MotorInterval = maxSpeed; // Stop motor
      } 
      else if (pulseWidth < deadZoneMin) {
        digitalWrite(DIR, LOW); // Reverse direction
        // Map the RC signal to the motor speed in reverse
        MotorInterval = map(pulseWidth, minPulse, deadZoneMin, minSpeed, maxSpeed);
      } 
      else if (pulseWidth > deadZoneMax) {
        digitalWrite(DIR, HIGH); // Forward direction
        // Map the RC signal to the motor speed
        MotorInterval = map(pulseWidth, deadZoneMax, maxPulse, maxSpeed, minSpeed);
      }
      
      digitalWrite(LED_PIN, HIGH); // Signal LED on
    }
    
    // For debugging
    Serial.print("Pulse: ");
    Serial.print(pulseWidth);
    Serial.print(" | Interval: ");
    Serial.println(MotorInterval);
  }
  
  // Generate step pulses
  unsigned long currentTime = micros();
  
  // Handle step pulse timing
  if (stepState) {
    // Currently in HIGH state of step pulse
    if (currentTime - stepStartTime >= minStepPulse) {
      digitalWrite(STEP, LOW); // End step pulse
      stepState = false;
    }
  } else {
    // Currently in LOW state, check if it's time for a new step
    if (currentTime - previousMotorTime >= MotorInterval) {
      digitalWrite(STEP, HIGH); // Start step pulse
      stepStartTime = currentTime;
      stepState = true;
      previousMotorTime = currentTime;
    }
  }
}

// Interrupt handler for rising edge
void risingPulse() {
  pulseStart = micros();
  attachInterrupt(digitalPinToInterrupt(RCPin), fallingPulse, FALLING);
}

// Interrupt handler for falling edge
void fallingPulse() {
  pulseWidth = micros() - pulseStart;
  lastSignalTime = millis();
  newSignal = true;
  attachInterrupt(digitalPinToInterrupt(RCPin), risingPulse, RISING);
}
