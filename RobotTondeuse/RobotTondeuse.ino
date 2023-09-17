#pragma region Imports

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#pragma endregion

#pragma region Variables

const int OBSTACLE_BACK_TIME = 10 * 1000;
const int TURN_TIME = 5 * 1000;

long lastTimeForward = 0;
long lastTimeObstacle = 0;
long lastTimeTurn = 0;

int motorStep = 0; // 0 = stop, 1 = forward, 2 = backward, 3 = right, 4 = left

// Moteur droit
const int MOTOR_RIGHT_ENABLE_PIN = 3;
const int MOTOR_RIGHT_PIN1 = 4;
const int MOTOR_RIGHT_PIN2 = 2;

// Relais
const int RELAY_PIN = 23;

// Moteur gauche
const int MOTOR_LEFT_ENABLE_PIN = 5;
const int MOTOR_LEFT_PIN1 = 7;
const int MOTOR_LEFT_PIN2 = 8;

// Buzzer
const int BUZZER_PIN = 12;

// Vitesse du moteur
const int MOTOR_MAX_SPEED = 255; // Vitesse maximale du moteur
const int MOTOR_SPEED_STEP = 15; // Vitesse de progression des moteurs (Diviseurs: 1, 3, 5, 15, 17, 51, 85, 255)
int motorRightSpeed = 0; // Vitesse actuelle du moteur droit
int motorLeftSpeed = 0; // Vitesse actuelle du moteur gauche

const bool DEV_SILENT_MODE = false; // Ce mode permet de tester/développer sans les tonnalités, les bruits de moteurs etc.

// Pare choc avant gauche
const int BUTTON_PIN = 10;

LiquidCrystal_I2C lcd(0x27, 16, 2);

#pragma endregion

void setup() {
  Serial.begin(9600);

  screen_print(0, 0, "Initialisation...");

  // Moteur droit
  pinMode(MOTOR_RIGHT_ENABLE_PIN, OUTPUT);
  pinMode(MOTOR_RIGHT_PIN1, OUTPUT);
  pinMode(MOTOR_RIGHT_PIN2, OUTPUT);
  
  // Moteur gauche
  pinMode(MOTOR_LEFT_ENABLE_PIN, OUTPUT);
  pinMode(MOTOR_LEFT_PIN1, OUTPUT);
  pinMode(MOTOR_LEFT_PIN2, OUTPUT);

  // Relais
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);

  // Button
  pinMode(BUTTON_PIN, INPUT);

  // Beeper
  pinMode(BUZZER_PIN, OUTPUT);

  lcd.init(); // Initialize the lcd
  lcd.backlight(); // Turn on the LCD screen backlight.

  // Le robot a initialisé toutes ses variables.
  screen_print(0, 0, "Pret a partir !");

  // On avertis via le beeper que la machine est prête à travailler.
  bip(500, 100);
  delay(100);
  bip(1000, 100);
  delay(100);
  bip(1500, 100);
  
  delay(1000);
  forward_and_cut();
}

void loop() {
  obstacle_thread();
  forward_thread();
  backward_thread();
  left_thread();
  right_thread();
}

void obstacle_thread(){
  const long NOW = millis();
  // Si obstacle, on recule.
  if (digitalRead(BUTTON_PIN) == HIGH) {
    if(lastTimeObstacle == 0){
      lastTimeObstacle = NOW;
      screen_print(0, 0, "Obstacle !");
      motor_stop(true);
      blade_off();
      backward();
    }
  }
  
  // Si le robot a fini de reculer.
  if(lastTimeObstacle != 0 && NOW - lastTimeObstacle > OBSTACLE_BACK_TIME){
    lastTimeObstacle = 0;
    motor_stop(true);
    int orientation = random(2); // 0 = Gauche, 1 = Droite
    lastTimeTurn = NOW;
    if(orientation == 0){
      left();
    }
    else{
      right();
    }
  }

  // Si le robot a fini de tourner.
  if(lastTimeTurn != 0 && NOW - lastTimeTurn > TURN_TIME){
    lastTimeTurn = 0;
    motor_stop(true);
    forward_and_cut();
  }
}

void right(){
  motorStep = 3;
  screen_print(0, 0, "Moteur : Droite");
  motorRightMoove(MOTOR_MAX_SPEED * -1);
  motorLeftMoove(MOTOR_MAX_SPEED);
}

// Cette fonction permet de faire tourner à gauche le robot.
void right_thread(){
  if(motorStep == 3){
    const long NOW = millis();

    if(motorLeftSpeed < MOTOR_MAX_SPEED && NOW - lastTimeTurn >= 250){
      lastTimeTurn = NOW;
      motorLeftSpeed = motorLeftSpeed + MOTOR_SPEED_STEP;

      analogWrite(MOTOR_LEFT_ENABLE_PIN, motorLeftSpeed);

      screen_print(0, 0, "Moteur : Droite : " + String(motorRightSpeed));
    }
  }
}

void left(){
  motorStep = 4;
  screen_print(0, 0, "Moteur : Gauche");
  motorRightMoove(MOTOR_MAX_SPEED);
  motorLeftMoove(MOTOR_MAX_SPEED * -1);
}

// Cette fonction permet de faire tourner à gauche le robot.
void left_thread(){
  if(motorStep == 4){
    const long NOW = millis();

    if(motorRightSpeed < MOTOR_MAX_SPEED && NOW - lastTimeTurn >= 250){
      lastTimeTurn = NOW;
      motorRightSpeed = motorRightSpeed + MOTOR_SPEED_STEP;

      analogWrite(MOTOR_RIGHT_ENABLE_PIN, motorRightSpeed);

      screen_print(0, 0, "Moteur : Gauche : " + String(motorRightSpeed));
    }
  }
}













// Cette fonction permet faire tourner le moteur de droite.
void motorRightMoove(int speed){
  digitalWrite(MOTOR_RIGHT_PIN1, speed > 0 ? HIGH : LOW);
  digitalWrite(MOTOR_RIGHT_PIN2, speed > 0 ? LOW : HIGH);
}

// Cette fonction permet faire tourner le moteur de gauche.
void motorLeftMoove(int speed){
  digitalWrite(MOTOR_LEFT_PIN1, speed > 0 ? HIGH : LOW);
  digitalWrite(MOTOR_LEFT_PIN2, speed > 0 ? LOW : HIGH);
}

// Cette fonction allume progressivement la lame de tonte.
void blade_on(){
  Serial.println("[BLADE] -> On");

  // On avertit l'utilisateur d'une triple tonalité que la lame va démarrer.
  bip(500, 500);
  delay(1000);
  bip(500, 500);
  delay(1000);
  bip(1500, 500);

  // Déclenchement du relai moteur de tonte preogressif
  if(!DEV_SILENT_MODE){
    digitalWrite(RELAY_PIN, LOW);
    delay (100);
    digitalWrite(RELAY_PIN, HIGH);
    delay (500);
    
    digitalWrite(RELAY_PIN, LOW);
    delay (100);
    digitalWrite(RELAY_PIN, HIGH);
    delay (400);
    
    digitalWrite(RELAY_PIN, LOW);
    delay (100);
    digitalWrite(RELAY_PIN, HIGH);
    delay (300);
    
    digitalWrite(RELAY_PIN, LOW);
    delay (100);
    digitalWrite(RELAY_PIN, HIGH);
    delay (200);
    
    digitalWrite(RELAY_PIN, LOW);
    delay (100);
    digitalWrite(RELAY_PIN, HIGH);
    delay (100);

    digitalWrite(RELAY_PIN, LOW);
  }

  delay(1000);
}

// Cette fonction ordonne au robot d'avancer en coupant l'herbe.
void forward_and_cut(){
  blade_on();
  motorStep = 1;
}

void backward(){
    motorRightSpeed = 0;
    motorLeftSpeed = motorRightSpeed;
    motorRightMoove(MOTOR_MAX_SPEED * -1);
    motorLeftMoove(MOTOR_MAX_SPEED * -1);
    motorStep = 2;
}

// Cette fonction permet de faire avancer le robot.
void backward_thread(){
  if(motorStep == 2){
    long now = millis();

    if(now - lastTimeForward >= 250){
      if(motorRightSpeed < MOTOR_MAX_SPEED){
        lastTimeForward = now;
        motorRightSpeed = motorRightSpeed + MOTOR_SPEED_STEP;
        motorLeftSpeed = motorRightSpeed;

        analogWrite(MOTOR_RIGHT_ENABLE_PIN, motorRightSpeed);
        analogWrite(MOTOR_LEFT_ENABLE_PIN, motorLeftSpeed);

        screen_print(0, 0, "Reculer : " + String(motorRightSpeed));
      }
    }
  }
}

// Cette fonction permet de faire avancer le robot.
void forward_thread(){
  if(lastTimeObstacle == 0 && motorStep == 1){
    motorRightMoove(MOTOR_MAX_SPEED);
    motorLeftMoove(MOTOR_MAX_SPEED);

    long now = millis();

    if(motorRightSpeed < MOTOR_MAX_SPEED && lastTimeObstacle == 0 && now - lastTimeForward >= 250){
      lastTimeForward = now;
      motorRightSpeed = motorRightSpeed + MOTOR_SPEED_STEP;
      motorLeftSpeed = motorRightSpeed;

      analogWrite(MOTOR_RIGHT_ENABLE_PIN, motorRightSpeed);
      analogWrite(MOTOR_LEFT_ENABLE_PIN, motorLeftSpeed);

      screen_print(0, 0, "Avancer : " + String(motorRightSpeed));
    }
  }
}

// Cette fonction bloque les deux moteurs de traction.
void motor_stop(bool printOnScreen){
  // On bloque le moteur droit en précisant "HIGH" sur les deux pins.
  digitalWrite(MOTOR_RIGHT_PIN1, HIGH); 
  digitalWrite(MOTOR_RIGHT_PIN2, HIGH);
  
  // On bloque le moteur gauche en précisant "HIGH" sur les deux pins.
  digitalWrite(MOTOR_LEFT_PIN1, HIGH); 
  digitalWrite(MOTOR_LEFT_PIN2, HIGH);

  motorRightSpeed = 0;
  motorLeftSpeed = 0;

  if(printOnScreen){
    screen_print(0, 0, "Moteur : Stop");
  }
}

// Cette fonction éteint la lame de tonte.
void blade_off(){
  Serial.println("[BLADE] -> Off");
  digitalWrite(RELAY_PIN, HIGH);
}

// Cette fonction permet d'afficher le texte souhaité sur l'écran.
void screen_print(int x, int y, String text){
  lcd.clear(); // On efface ce qui est inscrit à l'écran.
  lcd.setCursor(x, y); // On défini les coordonnées où écrire.
  lcd.print(text); // On écrit.

  // Si un ordinateur est branché, on affiche le message dans la console ("Serial Monitor").
  Serial.println("[SCREEN] -> " + text);
}

// Cette fonction permet de déclencher le beeper.
void bip(int frequency, int duration){
  if(!DEV_SILENT_MODE){
    tone(BUZZER_PIN, frequency, duration);
  }
  else{
    Serial.println("[BIP] (" + String(frequency) + ", " + String(duration) + ")");
  }
}