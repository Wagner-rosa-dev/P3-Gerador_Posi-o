//Pino do potenciometro
const int pinoVelocidade = 35;
const int pinoDirecao = 32;
//Velocidade que o trator pode atingir
const float VELOCIDADE_MAXIMA = 20.0f;
//Variavel para guardar o ultimo valor enviado, para evitar envios repetidos
float ultimoValorVel = -1.0f;
float ultimoValorDir = -1;
//limite de mudança para enviar um novo valor. Evita ruido do potenciometro
const float LIMITE_MUDANCA_VEL = 0.1f;
const float LIMITE_MUDANCA_DIR = 1;


void setup() {
  Serial.begin(115200);
}

void loop() {
  //Le o valor do pino analogico
  int ValorLidoVel = analogRead(pinoVelocidade);
  //Mapeia o valor lido par anossa faixa de velocidade com precisão
  //Usamos o map com inteiros e depois divimos para ter o float
  long valorMapeadoInt = map(ValorLidoVel, 0, 4095, 0, (long)(VELOCIDADE_MAXIMA * 100));
  float velocidadeAtual = (float)valorMapeadoInt / 100.0;

  //Leitura Direção (Volante)
  int valorLidoDir = analogRead(pinoDirecao);
  int direcaoAtual = map(valorLidoDir, 0, 4095, 0, 100);
 
  //Otimização: Só envia o dado se a veocidade mudou mais que o limite
  if (abs(velocidadeAtual - ultimoValorVel) > LIMITE_MUDANCA_VEL || abs(direcaoAtual - ultimoValorDir) > LIMITE_MUDANCA_DIR) {
    //Cria a string no formato CSV: "velocidade,direção"
    String data = String(velocidadeAtual) + "," + String(direcaoAtual);

    //envia os dados
    Serial.println(data);

    //Atualiza os ultimos valores enviados
    ultimoValorVel = velocidadeAtual;
    ultimoValorDir = direcaoAtual;
  }
  delay(20);
}
