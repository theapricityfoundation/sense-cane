// SENSE CANE -- FINAL CODE
// GitHub  : github.com/[yourhandle]/sense-cane
//
// Three ultrasonic sensors detect obstacles at different heights.
// A vibration motor communicates what was found through buzz patterns.
//
// Pattern reference:
//   High only       →  SINGLE PULSE
//   Med only        →  DOUBLE PULSE
//   Low only        →  TRIPLE PULSE
//   Med + High      →  SINGLE + DOUBLE PULSE
//   Low + High      →  SINGLE + TRIPLE PULSE
//   Low + Med       →  DOUBLE + TRIPLE PULSE
//   All three       →  SINGLE + DOUBLE + TRIPLE PULSE


// ---- Pins -------------------------------------------------------

const int trigLow  = 10,  echoLow  = 11;
const int trigMed  = 4,   echoMed  = 5;
const int trigHigh = 6,   echoHigh = 7;
const int motorPin = 9;


// ---- Thresholds (cm) --------------------------------------------
// Tune these to your environment and walking speed.

const int thresholdLow  = 70;
const int thresholdMed  = 100;
const int thresholdHigh = 50;

// Obstacle clears only when distance exceeds threshold + hysteresis.
// Hysteresis is nothing but a buffer zone we add to account for flickering.
// Prevents flickering when an object sits right at the edge.
const int hysteresis = 15;


// ---- Vibration timing (ms) --------------------------------------

const int buzzDuration = 200;
const int buzzGap      = 200;   // gap between buzzes in a group
const int groupGap     = 350;   // gap between groups in a combo


// ---- Crosstalk prevention ---------------------------------------
// Sensors fire one at a time, 35ms apart, so their pulses don't
// interfere with each other's receivers. This is a stagger implementation.

const int sensorStagger = 35;


// -----------------------------------------------------------------

int getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);
  if (duration == 0) return 999;
  return duration * 0.034 / 2;
}

void buzz(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(motorPin, HIGH);
    delay(buzzDuration);
    digitalWrite(motorPin, LOW);
    if (i < times - 1) delay(buzzGap);
  }
}

void playPattern(int groups[], int count) {
  for (int i = 0; i < count; i++) {
    buzz(groups[i]);
    if (i < count - 1) delay(groupGap);
  }
}

// Arms when distance drops below threshold.
// Disarms only when distance exceeds threshold + hysteresis.
bool applyHysteresis(bool active, int distance, int threshold) {
  if (!active && distance < threshold)              return true;
  if ( active && distance > threshold + hysteresis) return false;
  return active;
}


// -----------------------------------------------------------------

bool activeLow  = false;
bool activeMed  = false;
bool activeHigh = false;

void setup() {
  Serial.begin(9600);

  pinMode(trigLow,  OUTPUT); pinMode(echoLow,  INPUT);
  pinMode(trigMed,  OUTPUT); pinMode(echoMed,  INPUT);
  pinMode(trigHigh, OUTPUT); pinMode(echoHigh, INPUT);
  pinMode(motorPin, OUTPUT);
  digitalWrite(motorPin, LOW);

  // Startup confirmation buzz
  digitalWrite(motorPin, HIGH);
  delay(600);
  digitalWrite(motorPin, LOW);

  Serial.println("Sense Cane ready.");
}

void loop() {
  // Staggered reads to prevent crosstalk
  int distLow  = getDistance(trigLow,  echoLow);  delay(sensorStagger);
  int distMed  = getDistance(trigMed,  echoMed);  delay(sensorStagger);
  int distHigh = getDistance(trigHigh, echoHigh);

  Serial.print("Low: ");   Serial.print(distLow);
  Serial.print("  Med: "); Serial.print(distMed);
  Serial.print("  High: "); Serial.print(distHigh);
  Serial.println();

  // Update active states with hysteresis
  activeLow  = applyHysteresis(activeLow,  distLow,  thresholdLow);
  activeMed  = applyHysteresis(activeMed,  distMed,  thresholdMed);
  activeHigh = applyHysteresis(activeHigh, distHigh, thresholdHigh);

  // Play pattern every loop as long as an obstacle is detected.
  // Pattern duration naturally spaces out repetitions.
  if (!activeLow && !activeMed && !activeHigh) {
    // all clear — stay silent
    return;
  }

  if      ( activeLow && !activeMed && !activeHigh) { int p[] = {3};       playPattern(p, 1); }
  else if (!activeLow &&  activeMed && !activeHigh) { int p[] = {2};       playPattern(p, 1); }
  else if (!activeLow && !activeMed &&  activeHigh) { int p[] = {1};       playPattern(p, 1); }
  else if ( activeLow &&  activeMed && !activeHigh) { int p[] = {2, 3};    playPattern(p, 2); }
  else if ( activeLow && !activeMed &&  activeHigh) { int p[] = {1, 3};    playPattern(p, 2); }
  else if (!activeLow &&  activeMed &&  activeHigh) { int p[] = {1, 2};    playPattern(p, 2); }
  else if ( activeLow &&  activeMed &&  activeHigh) { int p[] = {1, 2, 3}; playPattern(p, 3); }

  delay(50);
}


// ---- Design decisions -------------------------------------------
//
// Three approaches were considered for the alarming system:
//
// 1. CONTINUOUS ALARM — buzz the entire time an obstacle is present.
//    Rejected: indoors, walls are always in range. Motor never stops,
//    user goes numb to it within seconds. Useless.
//
// 2. COOLDOWN TIMER — buzz once, wait N seconds, buzz again.
//    Rejected: a fixed timer is always wrong. Too short = spam.
//    Too long = user turns toward a new obstacle and hears nothing.
//    Also, a noisy high sensor could block low sensor alerts entirely.
//
// 3. STATE CHANGE ONLY — buzz only when the obstacle situation changes.
//    Rejected: if you turn from one wall to another, the low sensor
//    stays armed the whole time. State never changes. Silence. Safety Hazard.
//
// CHOSEN: PATTERN-PACED CONTINUOUS ALARM
//    Pattern replays every loop cycle as long as an obstacle is present.
//    The pattern duration itself spaces out repetitions naturally —
//    no timer needed. Turning toward a new obstacle always triggers
//    a response because we never wait for a state change.
//    Hysteresis handles flickering. Stagger handles crosstalk.
//    Simple, honest, safe.


// ---------------------------------------------------------------------

// Vision is NOT solely about having sight. 
// DEVELOPED BY MOHAMMAD TALHA SHAHZAD - A PROJECT BY THE APRICITY FOUNDATION

