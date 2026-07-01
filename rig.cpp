// ============================================================
// XMR RIG HARVESTER - BINARY SELECTION ENGINE
// BARE BONES - NO GOOFS, NO DEMO DATA
// ============================================================

#include <Arduino.h>

// === HARDWARE PINS (CHANGE THESE TO YOUR ACTUAL PINS) ===
#define PIN_GPU0  A0
#define PIN_GPU1  A1
#define PIN_GPU2  A2
#define PIN_GPU3  A3
#define PIN_PSU0  A4
#define PIN_PSU1  A5
#define PIN_PSU2  A6
#define PIN_PSU3  A7
#define PIN_TEMP0 A8
#define PIN_TEMP1 A9
#define PIN_TEMP2 A10
#define PIN_TEMP3 A11

#define NUM_CHANNELS 12
#define THRESHOLD 2048  // 50% of 4096 (12-bit ADC)

// Global arrays
int adcValues[NUM_CHANNELS];
bool isWinner[NUM_CHANNELS];

// Pin mapping
int pins[NUM_CHANNELS] = {
    PIN_GPU0, PIN_GPU1, PIN_GPU2, PIN_GPU3,
    PIN_PSU0, PIN_PSU1, PIN_PSU2, PIN_PSU3,
    PIN_TEMP0, PIN_TEMP1, PIN_TEMP2, PIN_TEMP3
};

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("=====================================");
    Serial.println("  XMR RIG HARVESTER - BINARY ENGINE  ");
    Serial.println("=====================================");
    Serial.println();
    
    // For ESP32: analogReadResolution(12);  // Uncomment if using ESP32
}

void loop() {
    // ============================================
    // STEP 1: READ ALL ANALOG SENSORS
    // ============================================
    for(int i = 0; i < NUM_CHANNELS; i++) {
        adcValues[i] = analogRead(pins[i]);  // ACTUAL HARDWARE READ
        delayMicroseconds(100);  // Small delay between reads for stability
    }
    
    // ============================================
    // STEP 2: THE BINARY DECISION (X > Y ? TRUE : FALSE)
    // ============================================
    int winnerCount = 0;
    
    for(int i = 0; i < NUM_CHANNELS; i++) {
        // HIGHER is better for GPUs and PSU (channels 0-7)
        // LOWER is better for TEMP (channels 8-11) - inverted logic
        if(i < 8) {
            // GPU/PSU: Higher value = Better performance
            isWinner[i] = (adcValues[i] > THRESHOLD);
        } else {
            // Temperature: Lower value = Better (not overheating)
            isWinner[i] = (adcValues[i] < THRESHOLD);
        }
        
        if(isWinner[i]) {
            winnerCount++;
        }
    }
    
    // ============================================
    // STEP 3: FIND THE DOMINANT CHANNEL (BEST PERFORMER)
    // ============================================
    int dominantIndex = 0;
    int maxValue = adcValues[0];
    
    for(int i = 1; i < NUM_CHANNELS; i++) {
        if(adcValues[i] > maxValue) {
            maxValue = adcValues[i];
            dominantIndex = i;
        }
    }
    
    // ============================================
    // STEP 4: CALCULATE SELECTION POWER
    // ============================================
    float selectionRatio = (float)winnerCount / NUM_CHANNELS * 100.0f;
    bool rigIsHealthy = (winnerCount > (NUM_CHANNELS / 2));
    
    // ============================================
    // STEP 5: PRINT RESULTS (EVERY 2 SECONDS)
    // ============================================
    printReport(winnerCount, selectionRatio, rigIsHealthy, 
                dominantIndex, maxValue);
    
    delay(2000);  // Update every 2 seconds
}

// ============================================================
// REPORT FUNCTION
// ============================================================
void printReport(int winners, float ratio, bool healthy, 
                 int dominant, int maxVal) {
    
    Serial.println("=== RIG STATUS ===");
    
    // === BINARY MAP VISUALIZATION ===
    Serial.print("Map: ");
    for(int i = 0; i < NUM_CHANNELS; i++) {
        Serial.print(isWinner[i] ? "1" : "0");
        if((i+1) % 4 == 0) Serial.print(" ");
    }
    Serial.println();
    
    // === RAW ADC VALUES ===
    Serial.print("ADC: ");
    for(int i = 0; i < NUM_CHANNELS; i++) {
        Serial.print(adcValues[i]);
        if(adcValues[i] < 1000) Serial.print(" ");
        if(adcValues[i] < 100) Serial.print(" ");
        Serial.print(" ");
        if((i+1) % 4 == 0) Serial.print("| ");
    }
    Serial.println();
    
    // === STATISTICS ===
    Serial.print("Winners: ");
    Serial.print(winners);
    Serial.print("/");
    Serial.print(NUM_CHANNELS);
    Serial.print(" (");
    Serial.print(ratio, 1);
    Serial.println("%)");
    
    Serial.print("Dominant: Ch");
    Serial.print(dominant);
    Serial.print(" (");
    Serial.print(maxVal);
    Serial.println(")");
    
    Serial.print("Health: ");
    Serial.println(healthy ? "GOOD" : "WARNING!");
    
    // === COMPETITIVE DECISION OUTPUT ===
    if(healthy) {
        Serial.println(">>> HARVEST MODE: FULL POWER <<<");
    } else {
        Serial.println(">>> WARNING: CHECK SENSORS <<<");
        Serial.print(">>> ");
        for(int i = 0; i < NUM_CHANNELS; i++) {
            if(!isWinner[i]) {
                Serial.print("Ch");
                Serial.print(i);
                Serial.print(" ");
            }
        }
        Serial.println("underperforming");
    }
    
    Serial.println("==========================\n");
}

// ============================================================
// OPTIONAL: TEST MODE - Use if you don't have sensors connected
// ============================================================
void testMode() {
    // Generate random test data (remove when sensors are connected)
    for(int i = 0; i < NUM_CHANNELS; i++) {
        adcValues[i] = random(0, 4096);
    }
}
