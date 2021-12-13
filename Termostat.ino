#include <LiquidCrystal.h>
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

#define AVG_REGENERATION_RATIO 0.75 
#define MIN_RELAY_SWITCH_DELAY 60000
#define REFRESH_DELAY 25
#define MAX_AVG_INTERVAL 90

float avgTempAccum;
int   avgTempAccumSize;
unsigned long avgTempAccumLastReset;

bool isRelayOn;
unsigned long lastRelaySwitchTime;

void setup() {
  lcd.begin(16, 2);
  Serial.begin(9600);

  avgTempAccum = 0.0;
  avgTempAccumSize = 0;
  avgTempAccumLastReset = millis();
  
  isRelayOn = false;
  lastRelaySwitchTime = 0;
}

void dbg(String s, int val){
  Serial.print(s);
  Serial.print(":");
  Serial.println(val);
}
void dbg(String s, float val){
  Serial.print(s);
  Serial.print(":");
  Serial.println(val);
}
void dbg(String s, int v0, int v1){
  Serial.print(s);
  Serial.print(":");
  Serial.print(v0);
  Serial.print(",");
  Serial.println(v1);
}
void dbg(String s, int v0, float v1){
  Serial.print(s);
  Serial.print(":");
  Serial.print(v0);
  Serial.print(",");
  Serial.println(v1);
}

char line1[16];
char line2[16];
void loop() {
  Serial.println("----");

  int settingHysteresis = round(99.0*(analogRead(A5)/1024.0));
  int settingTempAccumInterval = round(MAX_AVG_INTERVAL *(analogRead(A6)/1024.0));
  float temperatureSensorReading = analogRead(A3) * (5.0/10.24);
  int settingGoalTemp = round(analogRead(A4) / 6.80);

  dbg("settingHysteresis", analogRead(A5), settingHysteresis);
  dbg("settingTempAccumInterval", analogRead(A6), settingTempAccumInterval);
  dbg("temperatureSensorReading", analogRead(A3), temperatureSensorReading);
  dbg("settingGoalTemp", analogRead(A4), settingGoalTemp);

  
  unsigned long now = millis();
  avgTempAccum += temperatureSensorReading;
  avgTempAccumSize++;
  
  float currentAverageTemp = avgTempAccum / avgTempAccumSize;

  dbg("AccumSize, AvgTemp", avgTempAccumSize, currentAverageTemp);

  if( now - avgTempAccumLastReset > (REFRESH_DELAY*settingTempAccumInterval)) {
    
    int regeneratedAvgTempAccumSize = (int)(avgTempAccumSize * AVG_REGENERATION_RATIO);
    float regeneratedAvgTempAccum = currentAverageTemp * regeneratedAvgTempAccumSize;

    avgTempAccumSize = regeneratedAvgTempAccumSize;
    avgTempAccum = regeneratedAvgTempAccum;
    avgTempAccumLastReset = now;
  }

  dbg("relay check", settingGoalTemp + (settingHysteresis/20), settingGoalTemp - (settingHysteresis/20));
  
  if( currentAverageTemp >= (settingGoalTemp + (settingHysteresis/20))) {
      setRelay(false);
  } else if( currentAverageTemp < (settingGoalTemp - (settingHysteresis/20))){
      setRelay(true);
  }

  sprintf (line1, "%s:%03d.%1dC>%03dC", isRelayOn ? "On " : "Off" , (int)currentAverageTemp,  abs((int)(currentAverageTemp*10)%10), settingGoalTemp);
  sprintf (line2, "Hst:%1d.%1dcAvg:%2ds", settingHysteresis/10,  settingHysteresis%10,  settingTempAccumInterval);

  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
  
  delay(REFRESH_DELAY);
}

void setRelay(bool on){
  if( isRelayOn == on ) return;
  
  unsigned long now = millis();
  dbg("relay time check", (int)(now - lastRelaySwitchTime));
  if( now - lastRelaySwitchTime > MIN_RELAY_SWITCH_DELAY ) {
    dbg("relay switch", on ? 1 : 0 );
    analogWrite(A2, on ? 1023 : 0);
    lastRelaySwitchTime = now;
    isRelayOn = on;
  }
}
