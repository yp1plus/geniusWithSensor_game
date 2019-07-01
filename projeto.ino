//Baseado no código disponível em: http://labdegaragem.com/profiles/blogs/arduino-genius-jogo-da-mem-ria

//Desenvolvido por: Gabriel Rossati, Nathan Rodrigues e Yure Pablo

// Definições de pinos

//Define as 3 cores para o LED RGB no projeto
#define R 1
#define G 2
#define B 3

//Define cada um dos leds principais do jogo
#define LED_RED 4
#define LED_YELLOW 5
#define LED_GREEN 6
#define LED_BLUE 7

//Define cada um dos botões correspondentes a cada um dos leds
#define BT_RED 8
#define BT_YELLOW 9
#define BT_GREEN 10
#define BT_BLUE 11

//Define a entrada para o sensor e o buzzer
#define PINO_SENSOR 12
#define PINO_AUDIO 13

//Define as notas reproduzidas pelo Buzzer para cada uma das cores
#define NOTE_RED  880
#define NOTE_YELLOW  1760
#define NOTE_GREEN  3520
#define NOTE_BLUE 7040

/* 
 * Cada LED possui um pino e um botão e uma nota associados, armazenados numa struct.
 * A struct é inicializada com as quatro cores do jogo.
 */
const struct _led{
  const int pino;
  const int botao;
  const int tom;
} LED[4] = { {LED_RED, BT_RED, NOTE_RED}, {LED_YELLOW, BT_YELLOW, NOTE_YELLOW}, {LED_GREEN, BT_GREEN, NOTE_GREEN}, 
             {LED_BLUE, BT_BLUE, NOTE_BLUE} };

//Struct para organizar o LED RGB
struct _RGB{
  const int pino[3];
  boolean ledAceso;
} RGB = { R, G, B, false};

// Variáveis que controlam o tempo para o LED RGB ser aceso
unsigned long tempo_inicial;
int tempo;
 
// O vetor sequencia armazena as informações de cada rodada do jogo, sendo o máximo de sequências igual a 100
int sequencia[100] = {};

// Guarda a rodada (das cem disponiveis) em que o usuário está
int rodada_atual = 0;

// Guarda em qual posicao da sequencia na rodada o usuário está
int passo_atual_na_sequencia = 0;

// Verifica se o jogador perdeu ou não o jogo
boolean perdeu_o_jogo = false;

// Define quando o usuário aperta um botão
int botao_pressionado = 0;

void setup() {
  //Inicialização dos inputs e outputs

  for (int i = 0; i < 3; i++)
  {
    pinMode(RGB.pino[i], OUTPUT);
  }

  for (int i = 0; i < 4; i++) 
  {

    pinMode(LED[i].pino, OUTPUT);
    pinMode(LED[i].botao, INPUT);

  }

  pinMode(PINO_AUDIO, OUTPUT);

  Serial.begin(9600);

  /* 
   * Inicializando o random através de uma leitura da porta analógica.
   * Esta leitura gera um valor variável entre 0 e 1023.
   */

  randomSeed(analogRead(0));
}
 
void loop() {

  // Se o jogador perdeu o jogo reinicializamos todas as variáveis.

  if (perdeu_o_jogo) 
  {

    int sequencia[100] = {};

    rodada_atual = 0;
    passo_atual_na_sequencia = 0;

    apagarRGB();

    perdeu_o_jogo = false;

  }
 
  // Toca um som de início para anunciar que o jogo está começando quando é a primeira rodada.
  if (rodada_atual == 0) 
  {

    iniciarJogo();

    delay(500);

  }

  // Chama a função que inicializa a próxima rodada.

  proximaRodada();

  // Reproduz a sequência atual.

  reproduzirSequencia();

  // Aguarda os botões serem pressionados pelo jogador.

  aguardarJogador();

  // Aguarda 1 segundo entre cada jogada.

  delay(1000);
}

// Faz uma animação de entrada
void iniciarJogo() {
  tone(PINO_AUDIO, LED[0].tom);

  combinarLeds(500);

  noTone(PINO_AUDIO);
}

/*
  * Sorteia uma nova cor para ser mostrada e adiciona-a à sequência
  * Reseta o LED RGB
  * Sorteia um novo tempo de espera até o RGB ser aceso novamente
  */
void proximaRodada() {
    apagarRGB();
    int numero_sorteado = random(0, 4);
    tempo_inicial = millis();
    tempo = random(3, 12);
    sequencia[rodada_atual++] = numero_sorteado;
}

// Reproduz a sequência a ser memorizada
void reproduzirSequencia() {
  
  for (int i = 0; i < rodada_atual; i++) 
  {
    int indice_cor = sequencia[i];

    mostrarLed(LED[indice_cor].tom, LED[indice_cor].pino, 500);

    delay(100); 
  }

}

// Aguarda a jogada e verifica se a cor escolhida é igual a da posicao atual na sequencia
void aguardarJogador() {
  for (int i = 0; i < rodada_atual; i++) 
  {
    aguardarJogada();

    //Esse primeiro teste verifica se a funcao do sensor já não chamou gameOver()
    if (perdeu_o_jogo) 
    {
      break;
    }
    
    if (sequencia[passo_atual_na_sequencia] != botao_pressionado) 
    {
      gameOver();
    }

    if (perdeu_o_jogo) 
    {
      break;
    }

    passo_atual_na_sequencia++;
  }

  passo_atual_na_sequencia = 0;
}

/* 
 * Aguarda o jogador pressionar um botão e guarda qual foi pressionado
 * Nesse momento, entre uma posicao e outra da sequencia, o jogo verifica se deve acender o LED RGB
 */
void aguardarJogada() {
  boolean jogada_efetuada = false;
  
  while (!jogada_efetuada) 
  {
    verificarRGB();

    //Se o jogador perdeu no sensor, sai do loop
    if (perdeu_o_jogo)
    {
      jogada_efetuada = true;
      break;
    }

    for (int i = 0; i <= 3; i++) 
    {
      if (digitalRead(LED[i].botao) == HIGH) 
      {

        botao_pressionado = i;

        mostrarLed(LED[i].tom, LED[i].pino, 300);

        jogada_efetuada = true;

      }
    }

    delay(10);
  }
}

/* 
 * Verifica se o tempo decorrido desde o inicio da rodada é igual ao tempo aleatório gerado.
 * Se sim, o LED deve ser aceso.
 */
void verificarRGB(){
  if (millis() - tempo_inicial >= tempo*1000)
  {
      setColor(255, 135, 75);
      RGB.ledAceso = true;
      delay(1200);
      controlarSensor();
      tempo_inicial = millis();
  }
}

/*
 * Quando o estado de LED RGB é alterado, essa função é chamada.
 * Verifica se o jogador colocou ou não a mão (ou o dedo) em frente ao sensor.
 * Se durante três segundos a condição for falsa, o jogador perde o jogo.
 */ 
void controlarSensor(){
   unsigned long t = millis();
   
   while((millis() - t <= 3000) && RGB.ledAceso)
   {
      if (digitalRead(PINO_SENSOR) != LOW)
      { 
          RGB.ledAceso = false;
          gameOver();
      }
   }

   apagarRGB();      
}

// Apaga o led RGB
void apagarRGB(){
  setColor(255, 255, 255); 
  RGB.ledAceso = false;
}

// Escolhe a cor para ser inserida no LED RGB
void setColor(int pixel_vermelho, int pixel_verde, int pixel_azul){
  analogWrite(RGB.pino[0], pixel_vermelho);
  analogWrite(RGB.pino[1], pixel_verde);
  analogWrite(RGB.pino[2], pixel_azul);
}

//Mostra um led especifico com seu respectivo tom por um determinado tempo
void mostrarLed(const int tom, const int pinoLed, int t){
    tone(PINO_AUDIO, tom);
    digitalWrite(pinoLed, HIGH);

    delay(t);

    digitalWrite(pinoLed, LOW);
    noTone(PINO_AUDIO);
}

// Recebe um estado e seta os leds para esse estado
void setLeds(int estado){
  for (int i = 0; i < 4; i++)
  {
    digitalWrite(LED[i].pino, estado);
  }
}

// Ajuda na execução das animações de LED'S
void combinarLeds(int t){
      setLeds(HIGH);

      delay(t);

      setLeds(LOW);

      delay(t);
}

/* 
 * Faz uma pequena animação com os LED'S para indicar fim de jogo. 
 * Atualiza a variável perdeu_o_jogo.
 */
void gameOver(){
  for (int i = 0; i <= 3; i++) 
  {
      mostrarLed(LED[i].tom, LED[i].pino, 200);
  }
  
  tone(PINO_AUDIO, LED[3].tom);

  for (int i = 0; i <= 3; i++) 
  {
      combinarLeds(100);
  }

   noTone(PINO_AUDIO);

   perdeu_o_jogo = true;
}
