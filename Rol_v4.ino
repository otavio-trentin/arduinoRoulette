#include "SoftwareSerial.h"

#define PushButton 12

SoftwareSerial bluetooth(10, 11); //TX, RX (Bluetooth)

const int ledPins[] = {2, 4, 5, 9, 8, 7, 6, 3}; // Define the 8 LED pins
const int numLeds = sizeof(ledPins) / sizeof(ledPins[0]);
const int flashAmount = 10;
const int rounds = 7; // Número de voltas antes de parar

int currentLed = 0;
int nextLed;

struct Product {
  const char* name;
  int quantity;
};

Product products[8] = {
  {"Produto 0", 300},
  {"Produto 1", 300},
  {"Produto 2", 300},
  {"Produto 3", 100},
  {"Produto 4", 300},
  {"Produto 5", 300},
  {"Produto 6", 3},
  {"Produto 7", 300}
};
int numProducts = sizeof(products) / sizeof(products[0]);

const unsigned long longPressTime = 10000; // Tempo para entrar no modo edição (10 segundos)
unsigned long buttonPressTime = 0; // Tempo de início da pressão do botão

bool inEditMode = false; // Flag para o modo edição

void setup() {

  bluetooth.begin(9600); // Initialize bluetooth communication for debugging
  

  for (int i = 0; i < numLeds; i++) pinMode(ledPins[i], OUTPUT);
  pinMode(PushButton, INPUT_PULLUP);
  digitalWrite(ledPins[currentLed], HIGH); 
}

void giveProduct(int ledIndex) {
  while (products[ledIndex].quantity <= 0) {
    ledIndex = (ledIndex + 1) % numLeds; // Go to the next product if the current one is out of stock
  }
  products[ledIndex].quantity--; // Decrease the quantity of the selected product
  bluetooth.print("Produto sorteado: ");
  bluetooth.println(products[ledIndex].name);
  bluetooth.println("---- Estoque ----");
  for (int i = 0; i < numLeds; i++){
    bluetooth.print(products[i].name);
    bluetooth.print(": ");
    bluetooth.println(products[i].quantity);
  }

  bluetooth.println("-#-#-#-#-#-#-#-#-#-#-#-#-#-#-");
}

void selectNextLedByProbability() {
  float sumQuantities = 0;
  for (int i = 0; i < numLeds; i++) {
    if (products[i].quantity > 0) { // Only include LEDs with available products
      sumQuantities += products[i].quantity;
    }
  }
  while (sumQuantities == 0){
    bluetooth.println("run out of stock");
    bluetooth.println("Restart the game");
    delay(1000);
  }

  float r = random(1000) / 1000.0 * sumQuantities; // Generate a random number between 0 and the sum of quantities
  float cumulativeQuantity = 0;
  for (int i = 0; i < numLeds; i++) {
    if (products[i].quantity > 0) { // Only include LEDs with available products
      cumulativeQuantity += products[i].quantity;
      if (r < cumulativeQuantity) {
        nextLed = i;
        return;
      }
    }
  }
}

void gameMode(){
  digitalWrite(ledPins[currentLed], LOW);
  for (int round = currentLed; round < rounds * numLeds + nextLed; round++) {
    
    currentLed++;
    if (currentLed >= numLeds) currentLed = 0;
    // Acende o LED atual
    digitalWrite(ledPins[currentLed], HIGH);

    // Tempo de espera entre LEDs
    delay(100 + round * 5); // Aumenta o atraso para simular desaceleração

    // Apaga o LED atual
    digitalWrite(ledPins[currentLed], LOW);
  }
  // Flash o LED final selecionado para celebrar
  for (int flash = 0; flash < flashAmount; flash++) {
    digitalWrite(ledPins[currentLed], LOW);
    delay(100);
    digitalWrite(ledPins[currentLed], HIGH);
    delay(100);
  }
  giveProduct(currentLed);
}

void editMode(){
  const unsigned long timeOutLimit = 30000; // Timeout in milliseconds
  unsigned long startTime;

  bluetooth.println("-#-#-#-#-#-#- Modo de Edicao Iniciado -#-#-#-#-#-#-");
  bluetooth.println("-#-#-#-#-#- Solte o botao pressionado -#-#-#-#-#-#-");
  while (!digitalRead(PushButton));
  digitalWrite(ledPins[currentLed], LOW);

  for (int index = 0; index < numProducts; index++) {
    digitalWrite(ledPins[index], HIGH);
    bluetooth.print(products[index].name);
    bluetooth.print(" possui em estoque ");
    bluetooth.print(products[index].quantity);
    bluetooth.println(", insira um novo valor para alterar.");
    
    startTime = millis();

    while (bluetooth.available() == 0) {
      if (millis() - startTime > timeOutLimit) {
        bluetooth.println("Timeout, indo para a edicao do proximo produto.");
        digitalWrite(ledPins[index], LOW);
        break; // Exit the loop if timeout occurs
      }
    }

    if (bluetooth.available() > 0) {
      String quantityInput = bluetooth.readStringUntil('\n');
      quantityInput.trim();
      int newQuantity = quantityInput.toInt();
      
      if ((quantityInput == "0") || (newQuantity > 0)){
        // Atualiza apenas se valor é valido
        products[index].quantity = newQuantity;
        bluetooth.print("Atualizando ");
        bluetooth.print(products[index].name);
        bluetooth.print(" para a nova quantidade: ");
        bluetooth.println(products[index].quantity);
      }
    }
    digitalWrite(ledPins[index], LOW);
  }
  
  //confirmacao visual que a atualizao foi finalizada
  for (int round = 0; round < rounds * numLeds; round++) {
    currentLed++;
    if (currentLed >= numLeds) currentLed = 0;
    // Acende o LED atual
    digitalWrite(ledPins[currentLed], HIGH);
    // Tempo de espera entre LEDs
    delay(50); // Aumenta o atraso para simular desaceleração
    // Apaga o LED atual
    digitalWrite(ledPins[currentLed], LOW);
  }
  digitalWrite(ledPins[currentLed], HIGH);

  for (int i = 0; i < numLeds; i++){
    bluetooth.print(products[i].name);
    bluetooth.print(": ");
    bluetooth.println(products[i].quantity);
  }

  bluetooth.println("-#-#-#-#-#-#- Modo de Edicao Finalizado -#-#-#-#-#-#-");
}

void waitForButton(){
  while (digitalRead(PushButton));
  delay(100); //debouncing
  while (digitalRead(PushButton));
  buttonPressTime = millis(); // Record the time the button was pressed
  
  while (digitalRead(PushButton) == LOW) {
    // Check if the button is held down long enough for a long press
    if (millis() - buttonPressTime >= longPressTime) {
      inEditMode = true;
      break;
    }
  }
  while (!digitalRead(PushButton));    
}

void loop() {
  selectNextLedByProbability(); // if run out of stock it will not let me play anymore

  waitForButton(); // here defines if in editMode
  
  if (!inEditMode) {
    // Short press detected, so enter game mode
    gameMode();
  }
  else{
    editMode();
    inEditMode = false;
  }
  
}




