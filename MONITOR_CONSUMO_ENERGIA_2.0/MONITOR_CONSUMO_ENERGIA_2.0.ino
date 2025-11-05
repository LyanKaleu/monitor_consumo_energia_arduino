#include <LiquidCrystal.h>

// pinos lcd
const int rs = A0;
const int en = A1;
const int d4 = A2;
const int d5 = A3;
const int d6 = A4;
const int d7 = A5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

float somaWh = 0;
float custo = 0;
float wattAtual = 0;



// pinos botões e leds
const int btnPins[4] = {7, 6, 5, 4};
const int ledPins[4] = {13, 12, 11, 10};
const int btnLcd = 3;
const int buzzer = 2;

// nomes eletrodomésticos
const char* deviceName[4] = {"Caixa de som", "Ventilador", "TV", "Geladeira"};

// faixa de consumo (W)
const float minWatt[4] = {2.0, 20.0, 20.0, 80.0};
const float maxWatt[4] = {60.0, 75.0, 150.0, 300.0};

// total (Wh)
float totalWh[4] = {0, 0, 0, 0};

// estados dos dispositivos
bool deviceState[4] = {false, false, false, false};
bool lastButtonState[4] = {HIGH, HIGH, HIGH, HIGH};
bool btnLcdState = false;
bool lastBntLcdState = HIGH;

// tarifa de energia (Teresina residencia normal + bandeira tarifária vermelha patamar 1)
const float tarifa = 0.84785;

// intervalo de medição (1 minuto = 60000 ms)
//const unsigned long intervalo = 60000UL;
const unsigned long intervalo = 5000UL; // 5 segundos para testes
unsigned long ultimoTempo = 0;

const unsigned long intervaloLcd = 1000;
unsigned long ultimoLcd = 0;


void PrintLcd(float somaWh, float custo, float wattAtual) {
  if(btnLcdState) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Watt atual:");
    lcd.print((int)wattAtual);
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Wh total:");
    lcd.print((int)somaWh);
    lcd.setCursor(0, 1);
    lcd.print("R$:");
    lcd.print(custo, 2);
  }
}

void PrintSerialCsv(unsigned long agora, float somaWh, float custo) {
  Serial.print(agora / 1000);
  Serial.print(";");
  Serial.print(somaWh, 2);
  Serial.print(";");
  Serial.print(custo, 2);
  for (int i = 0; i < 4; i++) {
    Serial.print(";");
    Serial.print(totalWh[i], 2);
  }
  Serial.println();
}


void setup() {
  lcd.begin(16, 2);
  lcd.print("Iniciando...");
  delay(2000);
  lcd.clear();

  Serial.begin(9600);
  Serial.println("Tempo;TotalWh;CustoR$;CaixaSomWh;VentiladorWh;TVWh;GeladeiraWh");

  for (int i = 0; i < 4; i++) {
    pinMode(btnPins[i], INPUT_PULLUP);
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }
  pinMode(btnLcd, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);

  ultimoTempo = millis();
}

void loop() {
  // leitura dos botões
  for (int i = 0; i < 4; i++) {
    bool leitura = digitalRead(btnPins[i]);

    // Detecta transição de HIGH -> LOW (botão pressionado)
    if (lastButtonState[i] == HIGH && leitura == LOW) {
      deviceState[i] = !deviceState[i]; // alterna o estado
      digitalWrite(ledPins[i], deviceState[i] ? HIGH : LOW);
      delay(200);
    }

    lastButtonState[i] = leitura;
  }

  if(deviceState[0]) {
    digitalWrite(buzzer, HIGH);
  } else {
    digitalWrite(buzzer, LOW);
  }

  bool btnLcdLeitura = digitalRead(btnLcd);
  if(lastBntLcdState == HIGH && btnLcdLeitura == LOW) {
    btnLcdState = !btnLcdState;
    delay(200);
  }
  lastBntLcdState = btnLcdLeitura;

  if (millis() - ultimoLcd >= intervaloLcd) {
    ultimoLcd = millis();
    PrintLcd(somaWh, custo, wattAtual);
  }

  unsigned long agora = millis();

  if (agora - ultimoTempo >= intervalo) {
    ultimoTempo = agora;
    wattAtual = 0;

    // cálculo de consumo
    for (int i = 0; i < 4; i++) {
      if (deviceState[i]) {
        float watt = minWatt[i] + ((float)random(1000) / 1000.0) * (maxWatt[i] - minWatt[i]);
        wattAtual += watt;
        float wh = watt * (1.0 / 60.0);  // consumo em Wh por minuto
        totalWh[i] += wh;
        somaWh += wh;
      }
    }

    // cálculo de custo
    float kWh = somaWh / 1000.0;
    custo = kWh * tarifa;

    PrintSerialCsv(agora, somaWh, custo);

  }
}
