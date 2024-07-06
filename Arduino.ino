#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Endereço do LCD (0x27 para o modelo 16x2 com módulo I2C)
LiquidCrystal_I2C lcd(0x27, 16, 2);

const int buttonPin1 = 3; // Pino do botão 1
const int buttonPin2 = 9; // Pino do botão 2

int buttonState1 = 0;
int buttonState2 = 0;
int lastButtonState1 = 0;
int lastButtonState2 = 0;

int currentScreen = 0; // 0: Tela Inicial, 1: Tela PC Selecionado, 2: Tela Ino Selecionado
bool pcSelected = true;

const char letters[] = {'A', 'E', 'I', 'O', 'U'};
int currentLetterIndex = 0;
char message[6] = ""; // Array para armazenar a mensagem de 5 caracteres
int messageIndex = 0;

void setup() {
  // Inicializa o LCD
  lcd.init();
  lcd.backlight();
  displayInitialScreen();

  // Configura os pinos dos botões como entradas
  pinMode(buttonPin1, INPUT);
  pinMode(buttonPin2, INPUT);

  // Inicializa a comunicação serial
  Serial.begin(9600);
}

void loop() {
  // Lê o estado dos botões
  buttonState1 = digitalRead(buttonPin1);
  buttonState2 = digitalRead(buttonPin2);

  // Verifica se o botão 1 foi pressionado para alternar a seleção
  if (buttonState1 == HIGH && lastButtonState1 == LOW) {
    if (currentScreen == 0) {
      pcSelected = !pcSelected; // Alterna entre PC e Ino
      displayInitialScreen();
      Serial.println(pcSelected ? "PC SELECTED" : "INO SELECTED");
    } else if (currentScreen == 2) {
      // Alterna a letra atual na tela Ino
      currentLetterIndex = (currentLetterIndex + 1) % 5; // Alterna entre as letras
      displayInoScreen();
    } else if (currentScreen == 1) {
      // Retorna à tela inicial a partir da tela "PC Selected"
      currentScreen = 0;
      displayInitialScreen();
      Serial.println("MENU SELECTED");
    }
  }

  // Verifica se o botão 2 foi pressionado para confirmar a seleção
  if (buttonState2 == HIGH && lastButtonState2 == LOW) {
    if (currentScreen == 0) {
      if (pcSelected) {
        currentScreen = 1;
        displayPCScreen();
        Serial.println("PC SELECIONADO, ESCREVA UM TEXTO.");
      } else {
        currentScreen = 2;
        displayInoScreen();
        Serial.println("INO SELECIONADO");
      }
    } else if (currentScreen == 2) {
      // Adiciona a letra selecionada à mensagem
      message[messageIndex] = letters[currentLetterIndex];
      messageIndex++;
     
      if (messageIndex < 5) {
        // Se ainda não atingiu o tamanho máximo, continua a seleção
        displayInoScreen();
      } else {
        // Se atingiu o tamanho máximo, envia a mensagem e volta à tela inicial
        message[5] = '\0'; // Termina a string
        Serial.println(message);
        displaySentMessage();
        delay(2000); // Espera 2 segundos antes de voltar à tela inicial
        currentScreen = 0;
        messageIndex = 0; // Reseta o índice da mensagem
        displayInitialScreen();
        Serial.println("MENU SELECTED");
      }
    }
  }

  // Verifica se estamos na tela "PC Selected" e recebe dados da serial
  if (currentScreen == 1) {
    if (Serial.available() > 0) {
      String receivedText = Serial.readStringUntil('\n');
      displayPCText(receivedText);
      Serial.println("TEXT RECEIVED");
      delay(3000); // Aguarda 3 segundos antes de voltar ao menu
      currentScreen = 0;
      displayInitialScreen();
      Serial.println("MENU SELECTED");
    }
  }

  // Verifica se há comandos recebidos via serial
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    if (command.startsWith("PC SELECIONADO")) {
      currentScreen = 1;
      displayPCScreen();
    } else if (command.startsWith("INO SELECIONADO")) {
      currentScreen = 2;
      displayInoScreen();
    }
  }

  // Atualiza os últimos estados dos botões
  lastButtonState1 = buttonState1;
  lastButtonState2 = buttonState2;

  // Pequeno delay para debouncing
  delay(50);
}

void displayInitialScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PC ");
  if (pcSelected) {
    lcd.print("(X) Ino ( )");
  } else {
    lcd.print("( ) Ino (X)");
  }
}

void displayPCScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PC Selected");
  lcd.setCursor(0, 1);
  lcd.print("Awaiting input...");
}

void displayInoScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Ino Selected");
  lcd.setCursor(0, 1);
  lcd.print("Letter: ");
  lcd.print(letters[currentLetterIndex]);
  lcd.setCursor(10, 1); // Move cursor para onde a próxima letra será colocada
  lcd.print(message); // Exibe as letras já selecionadas
}

void displayPCText(String text) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(text);
  lcd.setCursor(0, 1);
  lcd.print("Press Btn1 to Back");
}

void displaySentMessage() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sent: ");
  lcd.print(message);
}
