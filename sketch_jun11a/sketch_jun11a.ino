const int pinoPotenciometro = 35;
const int pino2 = 32;



void setup() {
  Serial.begin(115200);


  Serial.println("Programa iniciado no ESP32. Girando o Potenciometro...");
  
  }

void loop() {
  
  int valorLido = analogRead(pinoPotenciometro);
  int valor2 = analogRead(pino2);
  
  int valorMapeado = map(valorLido, 0, 4095, 0, 100);
  int valorM2 = map(valor2, 0, 4095, 0, 10);

  Serial.print("Velocidade: ");
  Serial.print(valorM2);
  Serial.print(" K/h");
  Serial.print(" | ");  
  Serial.print("Latitude: ");
  Serial.println(valorMapeado);
  

  delay(100);

}
