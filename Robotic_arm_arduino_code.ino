// Serial-controlled 5-DOF Robotic Arm + Gripper
// Wrist Pitch removed. Preset positions added.

#include <Servo.h>

// Servo pins
const int PIN_J1 = 3;   // Base
const int PIN_J2 = 5;   // Shoulder
const int PIN_J3 = 6;   // Elbow
const int PIN_J5 = 10;  // Wrist Roll
const int PIN_J6 = 11;  // Wrist Rotate
const int PIN_G  = 12;  // Gripper

// Angle limits
const int J1_MIN=0,  J1_MAX=180;
const int J2_MIN=0,  J2_MAX=180;
const int J3_MIN=0,  J3_MAX=180;
const int J5_MIN=0,  J5_MAX=180;
const int J6_MIN=0,  J6_MAX=180;
const int G_MIN =0,  G_MAX =90;

// Smooth movement speed
const int MOVE_STEP_DELAY_MS = 8;

// Delay between each servo motion
const int SERVO_DELAY_MS = 150;

// Servo objects
Servo s1,s2,s3,s5,s6,sg;

// ---------------------- PRESET POSITIONS ----------------------
// {J1, J2, J3, J5, J6, G}

int PRESET_HOME[6] =  {90, 60, 120, 80, 0, 45};

int PRESET_P1[6]   = {90, 60, 90, 40, 0, 45};
int PRESET_P2[6]   = {180, 60, 90, 40, 0, 45};
int PRESET_P3[6]   = {180, 60, 105, 60, 0, 60};

int PRESET_PP1[6] = {180, 60, 105, 60, 0, 23};

int PRESET_P4[6]   = {180, 60, 90, 80, 0, 25};
int PRESET_P5[6]   = {45, 60, 110, 80, 0, 25};
int PRESET_P6[6]   = {45, 60, 120, 80, 0, 50};

int PRESET_PLACE[6] = {45, 60, 100, 40, 0, 50};

int PRESET_P7[6]   = {45, 60, 100, 80, 0, 50};


// ------------ Clamp helper -------------
int clampInt(int v,int lo,int hi){
  if(v<lo) return lo;
  if(v>hi) return hi;
  return v;
}

// -------- Smooth servo motion ----------
void smoothWrite(Servo &s, int target){
  int curr = s.read();
  if(curr == target) return;

  if(curr < target){
    for(int a = curr; a <= target; a++){
      s.write(a);
      delay(MOVE_STEP_DELAY_MS);
    }
  } else {
    for(int a = curr; a >= target; a--){
      s.write(a);
      delay(MOVE_STEP_DELAY_MS);
    }
  }
}

// ------------ Move all servos ----------
void moveToAngles(int j1,int j2,int j3,int j5,int j6,int g){

  smoothWrite(s1, clampInt(j1,J1_MIN,J1_MAX));
  delay(SERVO_DELAY_MS);

  smoothWrite(s2, clampInt(j2,J2_MIN,J2_MAX));
  delay(SERVO_DELAY_MS);

  smoothWrite(s3, clampInt(j3,J3_MIN,J3_MAX));
  delay(SERVO_DELAY_MS);

  smoothWrite(s5, clampInt(j5,J5_MIN,J5_MAX));
  delay(SERVO_DELAY_MS);

  smoothWrite(s6, clampInt(j6,J6_MIN,J6_MAX));
  delay(SERVO_DELAY_MS);

  smoothWrite(sg, clampInt(g ,G_MIN ,G_MAX ));
}

// ------------ Move preset --------------
void movePreset(int preset[6]){
  moveToAngles(preset[0],preset[1],preset[2],
               preset[3],preset[4],preset[5]);
}

// ------------- Serial command handler --------------
void handleLine(String raw){
  raw.trim();
  if(raw.length()==0) return;

  String up = raw;  
  up.toUpperCase();

  // ----------- PRESET COMMANDS ----------
  if(up=="HOME"){ movePreset(PRESET_HOME); return; }
  if(up=="P1"){ movePreset(PRESET_P1); return; }
  if(up=="P2"){ movePreset(PRESET_P2); return; }
  if(up=="P3"){ movePreset(PRESET_P3); return; }
  if(up=="PP1"){ movePreset(PRESET_PP1); return; }
  if(up=="P4"){ movePreset(PRESET_P4); return; }
  if(up=="P5"){ movePreset(PRESET_P5); return; }
  if(up=="P6"){ movePreset(PRESET_P6); return; }
  if(up=="PLACE"){ movePreset(PRESET_PLACE); return; }
  if(up=="P7"){ movePreset(PRESET_P7); return; }

  // ---------- HELP ----------
  if(up=="HELP"){
    Serial.println(F("Commands:"));
    Serial.println(F("  J1 J2 J3 J5 J6 G"));
    Serial.println(F("  Named: J1:90 J3:120 G:20"));
    Serial.println(F("Preset commands:"));
    Serial.println(F("  HOME, P1, P2, P3, PICK, P4, P5, P6, PLACE, P7"));
    return;
  }

  // Replace separators with spaces
  for(unsigned int i=0;i<raw.length();i++){
    char c = raw[i];
    if(c==':'||c==','||c=='=') raw[i]=' ';
  }

  // Token parsing
  char buf[150];
  raw.toCharArray(buf,sizeof(buf));
  char *tok = strtok(buf," \t");

  int nums[6];
  int n_nums = 0;

  bool sp[6]={0};
  int val[6]={0};

  while(tok){
    if(tok[0] != '\0'){

      if(isalpha((unsigned char)tok[0])){
        String T = String(tok);
        T.toUpperCase();

        char key = toupper(tok[0]);
        char *p = tok+1;

        while(*p && !((*p>='0'&&*p<='9')||*p=='-'||*p=='.')) p++;

        int v = (int)round(atof(p));

        int idx=-1;

        if(key=='J' && isdigit(tok[1])){
          int j = tok[1]-'0';
          if(j==1) idx=0;
          else if(j==2) idx=1;
          else if(j==3) idx=2;
          else if(j==5) idx=3;
          else if(j==6) idx=4;
        }
        else if(T.startsWith("G")) idx=5;

        if(idx>=0 && idx<6){
          sp[idx] = true;
          val[idx] = v;
        }
      }
      else {
        if(n_nums<6){
          nums[n_nums++] = (int)round(atof(tok));
        }
      }
    }
    tok = strtok(NULL," \t");
  }

  int currVals[6] = {
    s1.read(), s2.read(), s3.read(),
    s5.read(), s6.read(), sg.read()
  };

  int pos=0;
  for(int i=0;i<6;i++){
    if(sp[i]) currVals[i]=val[i];
    else if(pos<n_nums) currVals[i]=nums[pos++];
  }

  moveToAngles(currVals[0],currVals[1],currVals[2],
               currVals[3],currVals[4],currVals[5]);

  Serial.println(F("Done."));
}


// ------------------- MAIN LOOP --------------------
void setupServos(){
  s1.attach(PIN_J1);
  s2.attach(PIN_J2);
  s3.attach(PIN_J3);
  s5.attach(PIN_J5);
  s6.attach(PIN_J6);
  sg.attach(PIN_G);
}

void setup(){
  Serial.begin(115200);
  setupServos();
  delay(500);

  Serial.println(F("5-DOF Arm Ready. Type 'help'."));
}

void loop(){
  static String line = "";
  while(Serial.available()){
    char c = (char)Serial.read();
    if(c=='\r') continue;

    if(c=='\n'){
      handleLine(line);
      line="";
    } else {
      line += c;
      if(line.length()>200){
        line = line.substring(line.length()-200);
      }
    }
  }
}
