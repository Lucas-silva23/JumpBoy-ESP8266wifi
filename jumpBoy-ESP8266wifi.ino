#include <FastLED.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define LED_TYPE    WS2812B
#define NUM_LEDS    256

#define DATA_PIN D4
#define SWITCH_PIN D3

#define CHAO_Y2 10

#define BOY_POSITION_X 14
#define BOY_POSITION_X2 14

#define CHAO_Y 2       // Altura do chão
#define BOY_HEIGHT 3  // Altura do boy

CRGB leds[NUM_LEDS];

// Variáveis do jogo
int boyY = CHAO_Y;      // Posição inferior do boy1
int boyY2 = CHAO_Y2;     // Posição inferior do boy2

int boyYabaixo = CHAO_Y; 
int boyYabaixo2 = CHAO_Y2; 

bool isJumping1 = false; // Indica se o boy1 está no ar
bool isJumping2 = false; // Indica se o boy1 está no ar

bool isDown1 = false;
bool isDown2 = false;

int jumpCounter1 = 0;    // Contador para o salto 1
int jumpCounter2 = 0;    // Contador para o salto 2


int obstacleX = 15;     // Posição inicial do obstáculo
int obstacleX2 = 15;     // Posição inicial do obstáculo

bool gameOver1 = false;
bool gameOver2 = false;

int POSITION_OBSTACLE = 0; // Posição do obstáculo (0 = chão, 1 = topo)
int POSITION_OBSTACLE2 = 0; // Posição do obstáculo (0 = chão, 1 = topo)

// Configurações da rede Wi-Fi
const char* ssid = "motog-lucas";
const char* password = "12345678";

ESP8266WebServer server(80); // Cria o servidor web na porta 80

// Função para renderizar a página HTML
void handleRoot() {
  String player = server.arg("player"); // Obtém o parâmetro da URL
  String html = "<!DOCTYPE html><html>";
  html += "<head><title>Escolha seu Player</title></head>";
  html += "<body style='font-family: Arial; text-align: center;'>";
  html += "<h1>Escolha seu Jogador</h1>";

  if (player == "") {
    html += "<p><button onclick='choosePlayer(1)' style='padding: 10px 20px; font-size: 16px;'>Player 1</button></p>";
    html += "<p><button onclick='choosePlayer(2)' style='padding: 10px 20px; font-size: 16px;'>Player 2</button></p>";
  } else if (player == "1") {
    html += "<h2>Player 1</h2>";
    html += "<p><button onclick='jumpBoy1()' style='padding: 10px 20px; font-size: 16px;'>Pular Boy 1</button></p>";
    html += "<p><button onclick='DownBoy1()' style='padding: 10px 20px; font-size: 16px;'>Abaixar Boy 1</button></p>";
  } else if (player == "2") {
    html += "<h2>Player 2</h2>";
    html += "<p><button onclick='jumpBoy2()' style='padding: 10px 20px; font-size: 16px;'>Pular Boy 2</button></p>";
    html += "<p><button onclick='DownBoy2()' style='padding: 10px 20px; font-size: 16px;'>Abaixar Boy 2</button></p>";
  }

  html += "<script>";
  html += "function choosePlayer(player) { window.location.href = '/?player=' + player; }";
  html += "function jumpBoy1() { fetch('/jumpBoy1'); }";
  html += "function jumpBoy2() { fetch('/jumpBoy2'); }";
  html += "function DownBoy1() { fetch('/DownBoy1'); }";
  html += "function DownBoy2() { fetch('/DownBoy2'); }";
  html += "</script>";

  html += "</body></html>";

  server.send(200, "text/html", html);
}

// Função para obter o estado do interruptor
void getSwitchState() {
  int switchState = digitalRead(SWITCH_PIN); // Lê o estado do switch
  if (switchState == LOW) {
    server.send(200, "text/plain", "ON");
  } else {
    server.send(200, "text/plain", "OFF");
  }
}

void spawn_boy() {
  // Desenhar o boy (3 LEDs de altura)
  if(!isDown1){
    for (int i = 0; i < BOY_HEIGHT; i++) {
      controlLED(BOY_POSITION_X, boyY + i, CRGB::Blue);
    }
  } else {
    for(int i = 0; i < BOY_HEIGHT - 1; i++){
      controlLED(BOY_POSITION_X, boyY + i, CRGB::Blue);
    }
    controlLED(BOY_POSITION_X -1, 3, CRGB::Blue);
  }
  
  if(!isDown2){
    for (int j = 0; j < BOY_HEIGHT; j++) {
      controlLED(BOY_POSITION_X, boyY2 + j, CRGB::DeepPink);
    }
  } else {
    for(int j = 0; j < BOY_HEIGHT - 1; j++){
      controlLED(BOY_POSITION_X, boyY2 + j, CRGB::DeepPink);
    }
    controlLED(BOY_POSITION_X -1, 11, CRGB::DeepPink);
  }
}

void updateBoy1() {
  if (isJumping1) {
    if (jumpCounter1 < 3) {
      boyY++; // Subindo
    } else if (jumpCounter1 < 6) {
      boyY--; // Descendo
    } else {
      isJumping1 = false; // Terminar o salto
    }
    jumpCounter1++;
  }
}

void updateBoy2() {
  if (isJumping2) {
    if (jumpCounter2 < 3) {
      boyY2++; // Subindo
    } else if (jumpCounter2 < 6) {
      boyY2--; // Descendo
    } else {
      isJumping2 = false; // Terminar o salto
    }
    jumpCounter2++;
  }
}

void resetGame() {
  boyY = CHAO_Y;
  boyY2 = CHAO_Y2;
  obstacleX = 15;
  obstacleX2 = 15;
  isJumping1 = false;
  isJumping2 = false;
  isDown1 = false;
  isDown2 = false;
  gameOver1 = false;
  gameOver2 = false;
}

void displayGameOver() {
  // Mostrar mensagem de "Game Over"
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < NUM_LEDS; j++) {
      leds[j] = (i % 2 == 0) ? CRGB::DeepPink : CRGB::Black;
    }
    FastLED.show();
    delay(500);
  }

  // Reiniciar o jogo
  resetGame();
}

void displayGameOver2() {
  // Mostrar mensagem de "Game Over"
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < NUM_LEDS; j++) {
      leds[j] = (i % 2 == 0) ? CRGB::Blue : CRGB::Black;
    }
    FastLED.show();
    delay(500);
  }

  // Reiniciar o jogo
  resetGame();
}

void DownBoy1(){
  if(!isDown1)
    isDown1 = true;
    boyYabaixo--;

  server.send(200, "text/plain", "Boy 1 is Down");
}

void DownBoy2(){
  if(!isDown2)
    isDown2 = true;
    boyYabaixo2--;

  server.send(200, "text/plain", "Boy 2 is Down");
}

void jumpBoy1() {
  if (!isJumping1) {
    isJumping1 = true;
    isDown1 = false;
    jumpCounter1 = 0;
  }
  server.send(200, "text/plain", "Boy 1 Jumped");
}

void jumpBoy2() {
  if (!isJumping2) {
    isJumping2 = true;
    isDown2 = false;
    jumpCounter2 = 0;
  }
  server.send(200, "text/plain", "Boy 2 Jumped");
}

void setup() {
  // Configurar LEDs
  FastLED.addLeds<LED_TYPE, DATA_PIN, GRB>(leds, NUM_LEDS);
  Serial.begin(115200);

  // Configura o NodeMCU como ponto de acesso
  WiFi.softAP(ssid, password);
  Serial.println("Ponto de acesso criado!");
  Serial.print("IP do AP: ");
  Serial.println(WiFi.softAPIP());

  IPAddress apIP = WiFi.softAPIP();
  Serial.print("IP do AP: ");
  Serial.println(apIP);


  // Configuração do servidor
  server.on("/", handleRoot);          // Página principal
  server.on("/getSwitchState", getSwitchState); // Endpoint para estado do switch
  server.on("/jumpBoy1", jumpBoy1);
  server.on("/jumpBoy2", jumpBoy2);
  server.on("/DownBoy1", DownBoy1);
  server.on("/DownBoy2", DownBoy2);
  
  
  server.begin();
  Serial.println("Servidor iniciado.");

  // Configuração dos pinos
  pinMode(DATA_PIN, OUTPUT);
  pinMode(SWITCH_PIN, INPUT_PULLUP); // Ativa resistor pull-up interno

  // Inicializar o jogo
  chao();
  spawn_boy();
}


void controlLED(int y, int x, CRGB color) {
  int ledIndex;

  if (y % 2 == 0) { // even row
    ledIndex = x + y * 16;
  } else { // odd row
    ledIndex = (15 - x) + y * 16;
  }

  leds[ledIndex] = color;
}

void chao() {
  // Inicializar cores do chão
  CRGB green = CRGB::Chocolate;
  CRGB brown = CRGB::Green;

  for (int i = 0; i < 16; i++) {
    for (int j = 0; j < 1; j++) {
        controlLED(i, j, green);
      }
    }
  

  for (int i = 0; i < 16; i++) {
    for (int j = 0; j < 4; j++) {
        if (j == 1) {
        controlLED(i, j, brown);
      }
    }
  }

  for (int i = 0; i < 16; i++) {
    for (int j = 8; j < 10; j++) {
        controlLED(i, j, green);
      }
    }

  for (int i = 0; i < 16; i++) {
    for (int j = 8; j < 12; j++) {
        if (j == 9) {
        controlLED(i, j, brown);
      }
    }
  }
}

void updateObstacle() {
  // Mover o obstáculo para a esquerda
  obstacleX++;
  if (obstacleX > 15) {
    POSITION_OBSTACLE = random(0, 3); // Aleatório: 0 para chão, 1 para topo, 2 para meio
    obstacleX = 0; // Reiniciar na borda direita
  }

  obstacleX2++;
  if (obstacleX2 > 15) {
    POSITION_OBSTACLE2 = random(0, 3); // Aleatório: 0 para chão, 1 para topo, 2 para meio
    obstacleX2 = 0; // Reiniciar na borda direita
  }
}

void checkCollision() {
  // Verificar se o boy colide com o obstáculo
  for (int i = 0; i < BOY_HEIGHT; i++) {
    if (obstacleX == BOY_POSITION_X) {
      if ((POSITION_OBSTACLE == 0 && boyY - i == CHAO_Y) || 
          (POSITION_OBSTACLE == 1 && boyY + i == CHAO_Y + 3) || // Obstáculo no topo
          (POSITION_OBSTACLE == 2 && boyY + i == CHAO_Y + 2 && !isDown1)) {
        gameOver1 = true;
        break;
      }
    }
  }

  for (int j = 0; j < BOY_HEIGHT; j++) {
    if (obstacleX2 == BOY_POSITION_X2) {
      if ((POSITION_OBSTACLE2 == 0 && boyY2 - j == CHAO_Y2) || 
          (POSITION_OBSTACLE2 == 1 && boyY2 + j == CHAO_Y2 + 3)|| // Obstáculo no topo
          (POSITION_OBSTACLE2 == 2 && boyY2 + j == CHAO_Y2 + 2 && !isDown2)){
        gameOver2 = true;
        break;
      }
    }
  }
}

void renderGame() {
  // Limpar a matriz
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }

  // Redesenhar o chão
  chao();

  // Desenhar o boy
  spawn_boy();

  // Renderizar o obstáculo
  renderObstacle();

  FastLED.show();
}

void renderObstacle() {
  // Desenhar o obstáculo no chão ou no topo
  if (POSITION_OBSTACLE == 0) {
     controlLED(obstacleX, CHAO_Y, CRGB::Red); // Obstáculo no chão
  } else if(POSITION_OBSTACLE == 1) {
      controlLED(obstacleX, CHAO_Y+3, CRGB::Red);
    } else {
      controlLED(obstacleX, CHAO_Y+2, CRGB::Red);
    }

  if (POSITION_OBSTACLE2 == 0) {
     controlLED(obstacleX2, CHAO_Y2, CRGB::Red); // Obstáculo no chão
  } else if(POSITION_OBSTACLE2 == 1){
     controlLED(obstacleX2, CHAO_Y2+3, CRGB::Red);
    } else {
      controlLED(obstacleX2, CHAO_Y2+2, CRGB::Red);
    }
}

void loop() {
  server.handleClient();

  if (gameOver1) {
    displayGameOver();
    return;
  }

  if (gameOver2) {
    displayGameOver2();
    return;
  }

  // Atualizar posição do boy1
  updateBoy1();

  // Atualizar posição do boy2
  updateBoy2();

  // Atualizar posição do obstáculo
  updateObstacle();

  // Verificar colisões
  checkCollision();

  // Atualizar LEDs
  renderGame();

  // Controle de velocidade do jogo
  delay(100);
}
