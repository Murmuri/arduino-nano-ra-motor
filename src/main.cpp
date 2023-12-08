#include <Arduino.h>
#include <Stepper.h>
#include <RtcDS1302.h>
#include <ThreeWire.h>

// Serial
const int SERIAL_SPEED =        9600;
// RA drive
const int RA_DIRECTION_PIN =    2;
const int RA_SPEED_PIN =        3;
const long RA_GEAR_TRAIN =      2;
const int RA_DEFAULT_RPM =      3;
const long RA_MOTOR_STEPS =     800;
const long RA_MOUNT_STEPS =     130;
const long RA_MICRO_STEPS =     16;
const int motorInterfaceType =  1;
const int INTERATION_STEPS =    10000;
// Watch
const int  WATCH_CLK_PIN =      4;
const int  WATCH_DAT_PIN =      5;
const int  WATCH_RST_PIN =      6;
const int HOURS_IN_SEC =        60 * 60;

long stepsPerFullTurn = RA_MOUNT_STEPS * RA_MOTOR_STEPS * RA_GEAR_TRAIN * RA_MICRO_STEPS;
double secondsForFullTurn = 24.00 * HOURS_IN_SEC;
double secondsPerStep = stepsPerFullTurn / secondsForFullTurn;
long positionTime = 0;

ThreeWire myWire(WATCH_DAT_PIN, WATCH_CLK_PIN, WATCH_RST_PIN);
RtcDS1302<ThreeWire> Rtc(myWire);
Stepper raStepper(RA_MOTOR_STEPS, RA_DIRECTION_PIN, RA_SPEED_PIN);

void updatePosition(long);
long getLateTime();
void move(long);
long getStepsToMove(long);

void setup()
{ 
  Serial.begin(SERIAL_SPEED);
  Serial.println("Initialize ...");
  // Watch initialize
  Serial.print("Time module...");
  Rtc.Begin();
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  Rtc.SetDateTime(compiled);
  RtcDateTime startTime = Rtc.GetDateTime();
  positionTime = startTime.Unix32Time();
  Serial.println("Ok");
  // RA drive initialize
  Serial.print("Stepper module...");
  raStepper.setSpeed(RA_DEFAULT_RPM);
  Serial.println("Ok");
  Serial.println("Initialized!");
}

void loop()
{
  long lateTime = getLateTime();
  Serial.print("Late time: ");
  Serial.println(lateTime);

  if (lateTime > 0) {
    updatePosition(1);
  }
}

long getLateTime()
{
  RtcDateTime now = Rtc.GetDateTime();
  long latePerSeconds = now.Unix32Time() - positionTime;
  positionTime = now.Unix32Time();

  return latePerSeconds;
}

void updatePosition(long lateTime)
{
  if (lateTime > 0)
  {
    long steps = getStepsToMove(lateTime);
    Serial.println("Moving... ");
    Serial.print("Steps count: ");
    Serial.println(steps);
    move(steps);
    Serial.println("move over");
  }
}

long getStepsToMove(long sec)
{
  return sec * secondsPerStep;
}

void move(long steps)
{
  int index = abs(steps / HOURS_IN_SEC);
  int remainderSteps = steps % HOURS_IN_SEC;

  while (index > 0)
  {
    raStepper.step(HOURS_IN_SEC);
    index--;
  }

  raStepper.step(remainderSteps);
}