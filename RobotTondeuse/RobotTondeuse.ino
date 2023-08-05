#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Moteur droit
int motorRightEnablePin = 3;
int motorRightPin1 = 4;
int motorRightPin2 = 2;

// Moteur gauche
int motorLeftEnablePin = 5;
int motorLeftPin1 = 7;
int motorLeftPin2 = 8;

// Bouton
//int buttonPin = 8;

// Buzzer
const int BUZZER_PIN = 6;

// Vitesse du moteur
int motorSpeed = 255;

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  screenPrint(0, 0, "Initialisation...");

  // Moteur droit
  pinMode(motorRightEnablePin, OUTPUT);
  pinMode(motorRightPin1, OUTPUT);
  pinMode(motorRightPin2, OUTPUT);
  
  // Moteur gauche
  pinMode(motorLeftEnablePin, OUTPUT);
  pinMode(motorLeftPin1, OUTPUT);
  pinMode(motorLeftPin2, OUTPUT);

  // Buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  
  Serial.begin(9600);

  lcd.init(); // initialize the lcd
  lcd.backlight(); // Turn on the LCD screen backlight

  screenPrint(0, 0, "Pret a partir !");

  tone(BUZZER_PIN, 500, 100);
  delay(100);
  tone(BUZZER_PIN, 1000, 100);
  delay(100);
  tone(BUZZER_PIN, 1500, 100);
}

void loop() {
  if (Serial.available()) {
    int state = Serial.parseInt();
    switch (state) {
      case 1: // Avancer
        forward(motorSpeed);
        break;
      case 2: // Reculer
        backward(motorSpeed);
        break;
      case 3: // Droite
        right(motorSpeed);
        break;
      case 4: // Gauche
        left(motorSpeed);
        break;
      case 5: // Stop
        digitalWrite(motorRightPin1, HIGH); 
        digitalWrite(motorRightPin2, HIGH);
        
        digitalWrite(motorLeftPin1, HIGH); 
        digitalWrite(motorLeftPin2, HIGH);
        screenPrint(0, 0, "Moteur : Stop");
        break;
    }
  }
  delay(100);
}

// Cette fonction permet d'afficher le texte souhaité sur l'écran
void screenPrint(int x, int y, String text){
  lcd.clear();
  lcd.setCursor(x, y);
  lcd.print(text);
}

// Cette fonction permet faire tourner le moteur de droite.
void motorRightMoove(int speed){
  digitalWrite(motorRightPin1, speed > 0 ? HIGH : LOW);
  digitalWrite(motorRightPin2, speed > 0 ? LOW : HIGH);
  analogWrite(motorRightEnablePin, abs(speed));
}

// Cette fonction permet faire tourner le moteur de gauche.
void motorLeftMoove(int speed){
  digitalWrite(motorLeftPin1, speed > 0 ? HIGH : LOW);
  digitalWrite(motorLeftPin2, speed > 0 ? LOW : HIGH);
  analogWrite(motorLeftEnablePin, abs(speed));
}

void forward(int speed){
  screenPrint(0, 0, "Moteur : Avancer");
  motorRightMoove(speed);
  motorLeftMoove(speed);
}

void backward(int speed){
  screenPrint(0, 0, "Moteur : Reculer");
  motorRightMoove(speed * -1);
  motorLeftMoove(speed * -1);
}

void right(int speed){
  screenPrint(0, 0, "Moteur : Droite");
  motorRightMoove(speed * -1);
  motorLeftMoove(speed);
}

void left(int speed){
  screenPrint(0, 0, "Moteur : Gauche");
  motorRightMoove(speed);
  motorLeftMoove(speed * -1);
}