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

// -------- CALIBRACIÓN DE MOTORES -------- 
// Ajuste final para que camine recto (ajusta si es necesario)
float factorDerecho = 0.85;  
float factorIzquierdo = 1.0; 

// -------- Estados -------- 
enum Estado { AVANZAR, PARAR, RETROCEDER, MIRAR_IZQ, MIRAR_DER, DECIDIR, GIRAR }; 
Estado estado = AVANZAR; 
unsigned long tEstado = 0; 

long dFrente = 0, dIzq = 0, dDer = 0; 
char giro = 'D'; 

// -------- Funciones de Movimiento -------- 
void mover(int a1, int a2, int b1, int b2, int v) {  
  digitalWrite(IN1, a1); digitalWrite(IN2, a2);  
  digitalWrite(IN3, b1); digitalWrite(IN4, b2);  
  analogWrite(ENA, (int)(v * factorDerecho)); 
  analogWrite(ENB, (int)(v * factorIzquierdo)); 
} 

void adelante(int v)  { mover(LOW, HIGH, LOW, HIGH, v); } 
void atras(int v)     { mover(HIGH, LOW, HIGH, LOW, v); } 
void izquierda(int v) { mover(HIGH, LOW, LOW, HIGH, v); }
void derecha(int v)   { mover(LOW, HIGH, HIGH, LOW, v); } 
void parar()          { mover(LOW, LOW, LOW, LOW, 0); }

long medirDistancia() { 
  digitalWrite(trigPin, LOW); 
  delayMicroseconds(2); 
  digitalWrite(trigPin, HIGH); 
  delayMicroseconds(10); 
  digitalWrite(trigPin, LOW); 
  long d = pulseIn(echoPin, HIGH, 25000); 
  long cm = d * 0.034 / 2; 
  if (cm <= 0 || cm > 400) return 250; 
  return cm; 
} 

void setup() { 
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT); pinMode(ENA, OUTPUT);  
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT); pinMode(ENB, OUTPUT);  
  pinMode(trigPin, OUTPUT); pinMode(echoPin, INPUT); 
  servo.attach(servoPin); 
  servo.write(90); 
  delay(1000); 
} 

void loop() { 
  switch (estado) { 
    case AVANZAR: 
      dFrente = medirDistancia(); 
      adelante(150); 
      if (dFrente < 30) { // Detectar un poco antes (30cm) para tener margen
        estado = PARAR; 
        tEstado = millis(); 
      } 
      break; 

    case PARAR: 
      parar(); 
      if (millis() - tEstado > 400) { 
        estado = RETROCEDER; 
        tEstado = millis(); 
      } 
      break; 

    case RETROCEDER: 
      atras(160); 
      if (millis() - tEstado > 700) { // Retroceder suficiente para poder girar
        parar(); 
        servo.write(165); // Escaneo Izquierda
        estado = MIRAR_IZQ; 
        tEstado = millis(); 
      } 
      break; 

    case MIRAR_IZQ: 
      if (millis() - tEstado > 800) { 
        dIzq = medirDistancia(); 
        servo.write(15); // Escaneo Derecha
        estado = MIRAR_DER; 
        tEstado = millis(); 
      } 
      break; 

    case MIRAR_DER: 
      if (millis() - tEstado > 800) { 
        dDer = medirDistancia(); 
        servo.write(90); // Regresar al centro
        estado = DECIDIR; 
        tEstado = millis();
      } 
      break; 

    case DECIDIR: 
      if (millis() - tEstado > 500) {
        // DECISIÓN LÓGICA: Ir por donde haya más espacio
        if (dIzq > dDer) { 
          giro = 'I'; 
        } else { 
          giro = 'D'; 
        }
        estado = GIRAR; 
        tEstado = millis(); 
      } 
      break; 

    case GIRAR: 
      // Girar con potencia para asegurar que el robot rote
      if (giro == 'I') izquierda(180); 
      else derecha(180); 
      
      // Aumentamos el tiempo de giro a 850ms para una evasión COMPLETA
      if (millis() - tEstado > 850) { 
        parar(); 
        delay(200); // Pequeña pausa de estabilidad
        estado = AVANZAR; 
      } 
      break; 
  } 
}
