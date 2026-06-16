/*
 * CÓDIGO COMPLETO DE EFECTOS LED PARA TIRAS WS2812
 * Compatible con: ESP32, ESP32-C3, ESP8266 D1, ESP32 GROOM-32
 * Características:
 * - Múltiples efectos LED
 * - Selección de pin de datos
 * - Un botón para control completo
 * - Brillo ajustable
 * - Funciona con WS2812 y similares
 * 
 * Autor: Sistema
 * Fecha: 2026
 */

#include <Adafruit_NeoPixel.h>
#include <math.h>

// ========== CONFIGURACIÓN DE PINES Y PARÁMETROS ==========

// Descomenta la placa que estés usando
#define BOARD_ESP32           // ESP32 estándar
// #define BOARD_ESP32_C3     // ESP32-C3
// #define BOARD_ESP8266_D1   // ESP8266 D1
// #define BOARD_ESP32_GROOM  // ESP32 GROOM-32

// SELECCIONA EL PIN DE DATOS (GPIO) - PIN 2 PARA TODOS
#ifdef BOARD_ESP32
  #define LED_PIN 2           // GPIO2 para ESP32
  #define BUTTON_PIN 0        // GPIO0 (botón BOOT) - Cambia si necesitas otro
#endif

#ifdef BOARD_ESP32_C3
  #define LED_PIN 2           // GPIO2 para ESP32-C3
  #define BUTTON_PIN 9        // GPIO9 para botón
#endif

#ifdef BOARD_ESP8266_D1
  #define LED_PIN 2           // GPIO2 para ESP8266 D1
  #define BUTTON_PIN D8       // GPIO15 (D8) para botón
#endif

#ifdef BOARD_ESP32_GROOM
  #define LED_PIN 2           // GPIO2 para ESP32 GROOM-32
  #define BUTTON_PIN 35       // GPIO35 para botón
#endif

// PARÁMETROS DE LA TIRA LED
#define NUM_LEDS 60           // Número de LEDs en la tira (ajusta según tu tira)
#define LED_TYPE NEO_GRB      // Tipo: NEO_GRB, NEO_RGB, NEO_GRBW, NEO_RGBW
#define BRIGHTNESS 255        // Brillo máximo (0-255) - 255 es muy alto
#define UPDATE_INTERVAL 50    // Intervalo de actualización en ms

// ========== VARIABLES GLOBALES ==========

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, LED_TYPE + NEO_KHZ800);

// Estados
bool ledPower = false;
int currentEffect = 0;
int currentBrightness = BRIGHTNESS;
unsigned long lastButtonPress = 0;
unsigned long lastUpdate = 0;
const unsigned long DEBOUNCE_DELAY = 200;
int effectCount = 17; // Número total de efectos disponibles

// Variables para efectos
uint32_t colorWheel = 0;
int effectSpeed = 50;
int rainbowIndex = 0;
int sparkleCount = 0;

// Variables para manejo del botón
int pressCount = 0;
unsigned long firstPressTime = 0;
bool buttonWasReleased = true;

// ========== CONFIGURACIÓN INICIAL ==========

void setup() {
  Serial.begin(115200);
  delay(500);
  
  Serial.println("\n\n========== SISTEMA LED INICIADO ==========");
  Serial.println("Inicializando tira LED WS2812...");
  
  // Inicializar tira LED
  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  strip.show();
  
  // Configurar pin del botón
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  Serial.println("✓ Tira LED inicializada");
  Serial.print("✓ Número de LEDs: ");
  Serial.println(NUM_LEDS);
  Serial.print("✓ Pin de datos: ");
  Serial.println(LED_PIN);
  Serial.print("✓ Pin del botón: ");
  Serial.println(BUTTON_PIN);
  Serial.println("✓ Brillo inicial: 255 (MÁXIMO)");
  Serial.println("\nPRESIONA EL BOTÓN PARA:");
  Serial.println("1. Encender/Apagar");
  Serial.println("2. Cambiar efecto (mantén presionado)");
  Serial.println("3. Subir brillo (doble clic rápido)");
  Serial.println("==========================================\n");
  
  // Mostrar color de inicio (verde suave)
  colorFill(strip.Color(0, 50, 0), 100);
}

// ========== BUCLE PRINCIPAL ==========

void loop() {
  handleButtonPress();
  
  if (millis() - lastUpdate >= UPDATE_INTERVAL) {
    lastUpdate = millis();
    
    if (ledPower) {
      executeEffect(currentEffect);
    } else {
      strip.clear();
      strip.show();
    }
  }
}

// ========== MANEJO DEL BOTÓN ==========

void handleButtonPress() {
  if (digitalRead(BUTTON_PIN) == LOW) { // Botón presionado (pullup activo)
    buttonWasReleased = false;
    
    if (millis() - lastButtonPress >= DEBOUNCE_DELAY) {
      lastButtonPress = millis();
      
      if (pressCount == 0) {
        firstPressTime = millis();
      }
      
      pressCount++;
      
      // Verificar si fue un doble clic
      if (pressCount == 2 && (millis() - firstPressTime) < 500) {
        Serial.println("→ Subiendo brillo");
        increaseBrightness();
        pressCount = 0;
        buttonWasReleased = true;
      }
    }
  } else { // Botón liberado
    if (!buttonWasReleased) {
      // Si el botón se suelta después de presión larga
      if (pressCount == 1 && (millis() - firstPressTime) > 800) {
        Serial.println("→ Cambiando efecto");
        changeEffect();
      }
      // Si el botón se suelta rápidamente (clic simple)
      else if (pressCount == 1 && (millis() - firstPressTime) <= 500) {
        Serial.println("→ Alternando encendido/apagado");
        togglePower();
      }
      
      pressCount = 0;
      buttonWasReleased = true;
    }
  }
}

// ========== FUNCIONES DE CONTROL ==========

void togglePower() {
  ledPower = !ledPower;
  if (ledPower) {
    Serial.println("✓ LEDs ENCENDIDOS");
    colorFill(strip.Color(50, 50, 50), 100);
  } else {
    Serial.println("✗ LEDs APAGADOS");
    strip.clear();
    strip.show();
  }
}

void changeEffect() {
  currentEffect = (currentEffect + 1) % effectCount;
  Serial.print("✓ Efecto cambiado a: ");
  Serial.println(currentEffect);
  printEffectName(currentEffect);
  rainbowIndex = 0;
}

void increaseBrightness() {
  currentBrightness = min(255, currentBrightness + 51); // Aumenta en pasos de ~20%
  strip.setBrightness(currentBrightness);
  Serial.print("✓ Brillo: ");
  Serial.print(currentBrightness);
  Serial.println("/255");
  
  if (currentBrightness >= 255) {
    currentBrightness = 0; // Cicla de vuelta a 0
  }
}

void printEffectName(int effect) {
  switch(effect) {
    case 0: Serial.println("   → ARCOÍRIS CORRIDO"); break;
    case 1: Serial.println("   → COLORES SÓLIDOS GIRANDO"); break;
    case 2: Serial.println("   → DESTELLO ALEATORIO"); break;
    case 3: Serial.println("   → ONDA DE COLOR"); break;
    case 4: Serial.println("   → PULSO SUAVE"); break;
    case 5: Serial.println("   → CARRERA DE ARCOÍRIS"); break;
    case 6: Serial.println("   → LLUVIA DE COLORES"); break;
    case 7: Serial.println("   → DESTELLO BLANCO"); break;
    case 8: Serial.println("   → FUEGO"); break;
    case 9: Serial.println("   → EFECTO THEATER"); break;
    case 10: Serial.println("   → SCANNER"); break;
    case 11: Serial.println("   → ARCO DISCO"); break;
    case 12: Serial.println("   → FUEGO AVANZADO"); break;
    case 13: Serial.println("   → ARCOÍRIS FLUIDO"); break;
    case 14: Serial.println("   → RGB ALTERNADO"); break;
    case 15: Serial.println("   → FUEGO CON CHISPAS"); break;
    case 16: Serial.println("   → EFECTO PLASMA"); break;
    default: Serial.println("   → DESCONOCIDO"); break;
  }
}

// ========== EJECUCIÓN DE EFECTOS ==========

void executeEffect(int effect) {
  switch(effect) {
    case 0:
      rainbowCycle(5);
      break;
    case 1:
      rotatingColors();
      break;
    case 2:
      randomFlash();
      break;
    case 3:
      colorWave(strip.Color(255, 0, 0), 3);
      break;
    case 4:
      softPulse();
      break;
    case 5:
      rainbowRunning(10);
      break;
    case 6:
      colorRain();
      break;
    case 7:
      whiteFlash();
      break;
    case 8:
      fireEffect();
      break;
    case 9:
      theaterChase(strip.Color(255, 0, 0), 50);
      break;
    case 10:
      scanner(strip.Color(0, 255, 0));
      break;
    case 11:
      discoBall();
      break;
    case 12:
      fireAdvanced();
      break;
    case 13:
      rainbowFluid();
      break;
    case 14:
      rgbAlternado();
      break;
    case 15:
      fireWithSparks();
      break;
    case 16:
      plasmaEffect();
      break;
    default:
      colorFill(strip.Color(255, 255, 255), 50);
  }
}

// ========== EFECTOS LED ==========

// Efecto 0: Arcoíris en ciclo
void rainbowCycle(uint8_t wait) {
  for(int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, Wheel((i + rainbowIndex) & 255));
  }
  strip.show();
  rainbowIndex = (rainbowIndex + 1) & 255;
}

// Efecto 1: Colores sólidos girando
void rotatingColors() {
  uint32_t colors[] = {
    strip.Color(255, 0, 0),    // Rojo
    strip.Color(0, 255, 0),    // Verde
    strip.Color(0, 0, 255),    // Azul
    strip.Color(255, 255, 0),  // Amarillo
    strip.Color(255, 0, 255),  // Magenta
    strip.Color(0, 255, 255)   // Cian
  };
  
  static int colorIndex = 0;
  colorIndex = (colorIndex + 1) % 6;
  colorFill(colors[colorIndex], 0);
}

// Efecto 2: Destello aleatorio
void randomFlash() {
  for(int i = 0; i < NUM_LEDS; i++) {
    if (random(0, 100) < 30) {
      strip.setPixelColor(i, strip.Color(255, 255, 255));
    } else {
      strip.setPixelColor(i, strip.Color(0, 0, 0));
    }
  }
  strip.show();
}

// Efecto 3: Onda de color
void colorWave(uint32_t color, uint8_t speed) {
  static int waveIndex = 0;
  waveIndex = (waveIndex + 1) % (NUM_LEDS * 2);
  
  for(int i = 0; i < NUM_LEDS; i++) {
    int brightness = (int)(fabs(sin((float)(i + waveIndex) / 5.0)) * 255.0);
    uint8_t r = (color >> 16) & 255;
    uint8_t g = (color >> 8) & 255;
    uint8_t b = color & 255;
    strip.setPixelColor(i, strip.Color(r * brightness / 255, g * brightness / 255, b * brightness / 255));
  }
  strip.show();
}

// Efecto 4: Pulso suave
void softPulse() {
  static int pulseValue = 0;
  static int pulseDirection = 1;
  
  pulseValue += pulseDirection * 3;
  if (pulseValue >= 255) pulseDirection = -1;
  if (pulseValue <= 0) pulseDirection = 1;
  
  colorFill(strip.Color(pulseValue, pulseValue / 2, 0), 0);
}

// Efecto 5: Arcoíris corriendo
void rainbowRunning(uint8_t wait) {
  static int rainPos = 0;
  rainPos = (rainPos + 1) % NUM_LEDS;
  
  for(int i = 0; i < NUM_LEDS; i++) {
    int distance = abs(i - rainPos);
    if (distance > 10) distance = 20 - distance;
    uint32_t color = Wheel((distance * 256 / 10) & 255);
    strip.setPixelColor(i, color);
  }
  strip.show();
}

// Efecto 6: Lluvia de colores
void colorRain() {
  static int rainIndex = 0;
  rainIndex = (rainIndex + 1) % NUM_LEDS;
  
  for(int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, Wheel((i + rainIndex * 5) & 255));
  }
  strip.show();
}

// Efecto 7: Destello blanco
void whiteFlash() {
  static int flashCounter = 0;
  flashCounter++;
  
  if (flashCounter % 4 < 2) {
    colorFill(strip.Color(255, 255, 255), 0);
  } else {
    strip.clear();
    strip.show();
  }
}

// Efecto 8: Fuego
void fireEffect() {
  for(int i = 0; i < NUM_LEDS; i++) {
    int flicker = random(100, 255);
    int r = flicker;
    int g = flicker / 2;
    int b = 0;
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  strip.show();
}

// Efecto 9: Theater Chase
void theaterChase(uint32_t color, uint8_t wait) {
  static int chaseIndex = 0;
  chaseIndex = (chaseIndex + 1) % 3;
  
  for(int i = 0; i < NUM_LEDS; i++) {
    if (i % 3 == chaseIndex) {
      strip.setPixelColor(i, color);
    } else {
      strip.setPixelColor(i, 0);
    }
  }
  strip.show();
}

// Efecto 10: Scanner (efecto de barrido)
void scanner(uint32_t color) {
  static int scanPos = 0;
  static int scanDir = 1;
  
  strip.clear();
  strip.setPixelColor(scanPos, color);
  if (scanPos > 0) strip.setPixelColor(scanPos - 1, strip.Color(50, 50, 50));
  if (scanPos < NUM_LEDS - 1) strip.setPixelColor(scanPos + 1, strip.Color(50, 50, 50));
  
  scanPos += scanDir;
  if (scanPos >= NUM_LEDS - 1) scanDir = -1;
  if (scanPos <= 0) scanDir = 1;
  
  strip.show();
}

// Efecto 11: Disco ball (colores aleatorios rápido)
void discoBall() {
  for(int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, Wheel(random(0, 256)));
  }
  strip.show();
}

// ========== NUEVOS 5 EFECTOS ==========

// Efecto 12: Fuego avanzado (más realista)
void fireAdvanced() {
  static int fireArray[60];
  
  // Inicializar array si es la primera vez
  static bool initialized = false;
  if (!initialized) {
    for(int i = 0; i < NUM_LEDS; i++) {
      fireArray[i] = 0;
    }
    initialized = true;
  }
  
  // Actualizar valores de fuego
  for(int i = 0; i < NUM_LEDS; i++) {
    if (i == 0) {
      fireArray[i] = random(200, 255);
    } else {
      fireArray[i] = (fireArray[i-1] + fireArray[i] + random(0, 100)) / 3;
    }
  }
  
  // Mostrar colores
  for(int i = 0; i < NUM_LEDS; i++) {
    int val = constrain(fireArray[i], 0, 255);
    int r = val;
    int g = val / 2;
    int b = 0;
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  strip.show();
}

// Efecto 13: Arcoíris fluido suave
void rainbowFluid() {
  static float phase = 0;
  phase += 0.02;
  
  for(int i = 0; i < NUM_LEDS; i++) {
    float hue = (i + phase * 10) / NUM_LEDS;
    uint32_t color = Wheel((int)((hue * 256)) & 255);
    strip.setPixelColor(i, color);
  }
  strip.show();
}

// Efecto 14: RGB Alternado (Rojo-Verde-Azul)
void rgbAlternado() {
  static int rgbPhase = 0;
  rgbPhase = (rgbPhase + 1) % 3;
  
  uint32_t color;
  switch(rgbPhase) {
    case 0:
      color = strip.Color(255, 0, 0);    // Rojo
      break;
    case 1:
      color = strip.Color(0, 255, 0);    // Verde
      break;
    case 2:
      color = strip.Color(0, 0, 255);    // Azul
      break;
  }
  
  colorFill(color, 0);
}

// Efecto 15: Fuego con chispas blancas
void fireWithSparks() {
  for(int i = 0; i < NUM_LEDS; i++) {
    int intensity = random(150, 255);
    int r = intensity;
    int g = random(0, intensity / 3);
    int b = 0;
    
    // 15% posibilidad de chispa blanca
    if (random(0, 100) < 15) {
      r = 255;
      g = 255;
      b = random(100, 200);
    }
    
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  strip.show();
}

// Efecto 16: Plasma (ondas de colores caóticas)
void plasmaEffect() {
  static int plasmaPhase = 0;
  plasmaPhase = (plasmaPhase + 1) & 255;
  
  for(int i = 0; i < NUM_LEDS; i++) {
    int r = (int)(128 + 127 * sin((i + plasmaPhase) * 0.1));
    int g = (int)(128 + 127 * sin((i + plasmaPhase) * 0.05 + 2));
    int b = (int)(128 + 127 * sin((i + plasmaPhase) * 0.15 + 4));
    
    r = constrain(r, 0, 255);
    g = constrain(g, 0, 255);
    b = constrain(b, 0, 255);
    
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  strip.show();
}

// ========== FUNCIONES AUXILIARES ==========

// Llenar toda la tira con un color
void colorFill(uint32_t color, uint8_t wait) {
  for(int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, color);
  }
  strip.show();
  if (wait > 0) delay(wait);
}

// Rueda de colores (arcoíris)
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

// ========== FIN DEL CÓDIGO ==========
