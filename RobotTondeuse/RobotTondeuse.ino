#pragma region Imports

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#pragma endregion

#pragma region Variables

const int OBSTACLE_BACK_TIME = 4 * 1000;
int turnTime = 0;

long lastTimeForward = 0;
long lastTimeObstacle = 0;
long lastTimeTurn = 0;

int motorStep = 0; // 0 = stop, 1 = forward, 2 = backward, 3 = right, 4 = left

// Moteur droit
const int MOTOR_RIGHT_PIN1 = 4;
const int MOTOR_RIGHT_PIN2 = 3;

// Relais
const int MOTOR_BLADE_PIN1 = 23;
const int MOTOR_BLADE_PIN2 = 25;

// Moteur gauche
const int MOTOR_LEFT_PIN1 = 6;
const int MOTOR_LEFT_PIN2 = 7;

// Buzzer
const int BUZZER_PIN = 12;

// Vitesse du moteur
const int MOTOR_MAX_SPEED = 252; // Vitesse maximale du moteur
const int MOTOR_SPEED_STEP = 14; // Vitesse de progression des moteurs (Diviseurs: 1, 2, 3, 4, 6, 7, 9, 12, 14, 18, 21, 28, 36, 42, 63, 84, 126, 252)
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
  pinMode(MOTOR_RIGHT_PIN1, OUTPUT);
  pinMode(MOTOR_RIGHT_PIN2, OUTPUT);
  
  // Moteur gauche
  pinMode(MOTOR_LEFT_PIN1, OUTPUT);
  pinMode(MOTOR_LEFT_PIN2, OUTPUT);

  // Relais
  pinMode(MOTOR_BLADE_PIN1, OUTPUT);
  pinMode(MOTOR_BLADE_PIN2, OUTPUT);
	digitalWrite(MOTOR_BLADE_PIN1, LOW);
	digitalWrite(MOTOR_BLADE_PIN2, LOW);

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
    turnTime = random(1000, 3000);
    Serial.println("[DEBUG] -> " + String(turnTime));
    if(lastTimeTurn == 0){
      lastTimeTurn = NOW;
      if(orientation == 0){
        left();
      }
      else{
        right();
      }
    }
  }

  // Si le robot a fini de tourner.
  if(lastTimeTurn != 0){
    if(NOW - lastTimeTurn > turnTime){
      lastTimeTurn = 0;
      motor_stop(true);
      forward_and_cut();
    }
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
      //lastTimeTurn = NOW;
      motorLeftSpeed = motorLeftSpeed + MOTOR_SPEED_STEP;

      motorLeftMoove(motorLeftSpeed);

      screen_print(0, 0, "Moteur : Droite : " + String(motorLeftSpeed));
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
      //lastTimeTurn = NOW;
      motorRightSpeed = motorRightSpeed + MOTOR_SPEED_STEP;

      motorRightMoove(motorLeftSpeed);

      screen_print(0, 0, "Moteur : Gauche : " + String(motorRightSpeed));
    }
  }
}













// Cette fonction permet faire tourner le moteur de droite.
void motorRightMoove(int speed){
  analogWrite(MOTOR_RIGHT_PIN1, speed > 0 ? speed : 0);
  analogWrite(MOTOR_RIGHT_PIN2, speed < 0 ? abs(speed) : 0);
}

// Cette fonction permet faire tourner le moteur de gauche.
void motorLeftMoove(int speed){
  analogWrite(MOTOR_LEFT_PIN1, speed > 0 ? speed : 0);
  analogWrite(MOTOR_LEFT_PIN2, speed < 0 ? abs(speed) : 0);
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
  delay(1000);

  // Déclenchement du relai moteur de tonte preogressif
  if(!DEV_SILENT_MODE){
	  analogWrite(MOTOR_BLADE_PIN1, MOTOR_MAX_SPEED);
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

        motorRightMoove(motorRightSpeed * -1);
        motorLeftMoove(motorLeftSpeed * -1);

        //screen_print(0, 0, "Reculer : " + String(motorRightSpeed));
      }
    }
  }
}

// Cette fonction permet de faire avancer le robot.
void forward_thread(){
  if(lastTimeObstacle == 0 && motorStep == 1){
    long now = millis();

    if(motorRightSpeed < MOTOR_MAX_SPEED && lastTimeObstacle == 0 && now - lastTimeForward >= 250){
      lastTimeForward = now;
      motorRightSpeed = motorRightSpeed + MOTOR_SPEED_STEP;
      motorLeftSpeed = motorRightSpeed;

      motorRightMoove(motorRightSpeed);
      motorLeftMoove(motorLeftSpeed);
      //analogWrite(MOTOR_RIGHT_ENABLE_PIN, motorRightSpeed);
      //analogWrite(MOTOR_LEFT_ENABLE_PIN, motorLeftSpeed);

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
	digitalWrite(MOTOR_BLADE_PIN1, LOW);
	digitalWrite(MOTOR_BLADE_PIN2, LOW);
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