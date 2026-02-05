#include <Servo.h> 

// -------- Puente H -------- 
const int IN1 = 4, IN2 = 2, ENA = 3;  
const int IN3 = 7, IN4 = 6, ENB = 5;  

// -------- Ultrasonico -------- 
const int trigPin = 13; 
const int echoPin = 12; 

// -------- Servo -------- 
const int servoPin = 8; 
Servo servo; 

// -------- CALIBRACIÓN FINA -------- 
// Hemos subido de 0.82 a 0.91 para encontrar el equilibrio.
float compDerecho = 0.91;   
float compIzquierdo = 1.0; 
// ----------------------------------

// -------- Estados -------- 
enum Estado { AVANZAR, PARAR, RETROCEDER, MIRAR_IZQ, MIRAR_DER, DECIDIR, GIRAR }; 
Estado estado = AVANZAR; 
unsigned long tEstado = 0; 

long dFrente = 0, dIzq = 0, dDer = 0; 
char giro = 'D'; 

// -------- Motores con Compensación -------- 
void mover(int a1, int a2, int b1, int b2, int v) {  
  digitalWrite(IN1, a1); digitalWrite(IN2, a2);  
  digitalWrite(IN3, b1); digitalWrite(IN4, b2);  
  
  // Cast a int para asegurar que el PWM sea un número entero
  analogWrite(ENA, (int)(v * compDerecho)); 
  analogWrite(ENB, (int)(v * compIzquierdo)); 
} 

void adelante(int v)  { mover(LOW, HIGH, LOW, HIGH, v); } 
void atras(int v)     { mover(HIGH, LOW, HIGH, LOW, v); } 
void izquierda(int v) { mover(HIGH, LOW, LOW, HIGH, v); }
void derecha(int v)   { mover(LOW, HIGH, HIGH, LOW, v); } 

void parar() { 
 analogWrite(ENA, 0); analogWrite(ENB, 0); 
 digitalWrite(IN1, LOW); digitalWrite(IN2, LOW); 
 digitalWrite(IN3, LOW); digitalWrite(IN4, LOW); 
} 

long medirDistancia() { 
 digitalWrite(trigPin, LOW); 
 delayMicroseconds(2); 
 digitalWrite(trigPin, HIGH); 
 delayMicroseconds(10); 
 digitalWrite(trigPin, LOW); 
 long d = pulseIn(echoPin, HIGH, 20000); 
 long cm = d * 0.034 / 2; 
 if (cm == 0) cm = 100; 
 return cm; 
} 

void setup() { 
 pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT); pinMode(ENA, OUTPUT);  
 pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT); pinMode(ENB, OUTPUT);  
 pinMode(trigPin, OUTPUT); pinMode(echoPin, INPUT); 
 servo.attach(servoPin); 
 servo.write(90); 
} 

void loop() { 
 switch (estado) { 
  case AVANZAR: 
   dFrente = medirDistancia(); 
   adelante(150); 
   if (dFrente < 25) { estado = PARAR; tEstado = millis(); } 
   break; 

  case PARAR: 
   parar(); 
   if (millis() - tEstado > 200) { estado = RETROCEDER; tEstado = millis(); } 
   break; 

  case RETROCEDER: 
   atras(150); 
   if (millis() - tEstado > 400) {  
    parar(); 
    servo.write(150); 
    estado = MIRAR_IZQ; tEstado = millis(); 
   } 
   break; 

  case MIRAR_IZQ: 
   if (millis() - tEstado > 500) {  
    dIzq = medirDistancia(); servo.write(30); 
    estado = MIRAR_DER; tEstado = millis(); 
   } 
   break; 

  case MIRAR_DER: 
   if (millis() - tEstado > 500) {  
    dDer = medirDistancia(); servo.write(90); 
    estado = DECIDIR; 
   } 
   break; 

  case DECIDIR: 
   giro = (dIzq > dDer) ? 'I' : 'D';
   estado = GIRAR; tEstado = millis(); 
   break; 

  case GIRAR: 
   if (giro == 'I') izquierda(160); else derecha(160); 
   if (millis() - tEstado > 500) { parar(); estado = AVANZAR; } 
   break; 
 } 
}