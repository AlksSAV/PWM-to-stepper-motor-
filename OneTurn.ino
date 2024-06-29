//Define Pins
#define RCPin 2
#define DIR 4
#define STEP 5
#define MS3 6
#define MS2 7
#define MS1 8

int steps = 0;

//Set up time variables for Stepper motor
unsigned long previousMotorTime = millis();
long MotorInterval;
volatile long StartTime = 0;
volatile long CurrentTime = 0;

//Set up time variables for RC Read
volatile long Pulses = 0;
int PulseWidth = 0;

//define the number of steps for one revolution
#define STEPS_PER_REVOLUTION 200

//define the maximum and minimum pulse width for the joystick
#define MIN_PULSE_WIDTH 1000
#define MAX_PULSE_WIDTH 1984

//define the step interval for one full step
#define STEP_INTERVAL 50

void setup() {
  Serial.begin(9600);

  //RC Read Setup
  pinMode(RCPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RCPin), PulseTimer, CHANGE);

  // Set pins 4-8 to Outputs
  for (int i = 4; i <= 8; i++){
    pinMode(i, OUTPUT);
  }

  //Set MS1 to full step mode
  digitalWrite(MS1, LOW);
  digitalWrite(MS2, LOW);
  digitalWrite(MS3, LOW);

  //set the initial direction and step count
  digitalWrite(DIR, HIGH);
  int steps = 0;
}

void loop() {
  digitalWrite(STEP, LOW);

  //Only save HIGH pulse lengths
  if (Pulses < 2000){
    PulseWidth = Pulses;
  }

  //Stepper motor direction and speed
  if (PulseWidth >= MIN_PULSE_WIDTH && PulseWidth <= MAX_PULSE_WIDTH){
    steps = map(PulseWidth, MIN_PULSE_WIDTH, MAX_PULSE_WIDTH, 0, STEPS_PER_REVOLUTION);
    if (PulseWidth < 1500){
      digitalWrite(DIR, LOW);
      MotorInterval = map(PulseWidth, MIN_PULSE_WIDTH, 1500, STEP_INTERVAL, 1);
    }
    else{
      digitalWrite(DIR, HIGH);
      MotorInterval = map(PulseWidth, 1500, MAX_PULSE_WIDTH, 1, STEP_INTERVAL);
    }
  }
  else{
    //reset the step count if the joystick is near the midpoint
    steps = 0;
  }

  //check if the MotorInterval time has elapsed and step the motor
  unsigned long currentMotorTime = millis();
  if (currentMotorTime - previousMotorTime > MotorInterval){
    digitalWrite(STEP, HIGH);
    previousMotorTime = currentMotorTime;
    steps++;
    if (steps >= STEPS_PER_REVOLUTION){
      steps = 0;
    }
  }
}

//Function to measure the length of the pulses from the remote control
void PulseTimer(){
  CurrentTime = micros();
  if (CurrentTime > StartTime){
    Pulses = CurrentTime - StartTime;
    StartTime = CurrentTime;
  }
}
