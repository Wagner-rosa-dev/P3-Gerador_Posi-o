//Pino do potenciometro
const int pinoPotenciometro = 35;
//Velocidade que o trator pode atingir
const float VELOCIDADE_MAXIMA = 10.0f;
//Variavel para guardar o ultimo valor enviado, para evitar envios repetidos
float ultimoValorEnviado = -1.0f;
//limite de mudança para enviar um novo valor. Evita ruido do potenciometro
const float LIMITE_DE_MUDANCA = 0.1f;

void setup() {
  Serial.begin(115200);
}

void loop() {
  //Le o valor do pino analogico
  int ValorLido = analogRead(pinoPotenciometro);
  //Mapeia o valor lido par anossa faixa de velocidade com precisão
  //Usamos o map com inteiros e depois divimos para ter o float
  long valorMapeadoInt = map(ValorLido, 0, 4095, 0, (long)(VELOCIDADE_MAXIMA * 100));
  float velocidadeAtual = (float)valorMapeadoInt / 100.0;

  //Otimização: Só envia o dado se a veocidade mudou mais que o limite
  if (abs(velocidadeAtual - ultimoValorEnviado) > LIMITE_DE_MUDANCA) {
    //Envia o valor da velocidade formatado e pula a linha
    Serial.println(velocidadeAtual);
    //Atualiza o ultimo valor enviado
    ultimoValorEnviado = velocidadeAtual;
  }
  delay(20);
}
