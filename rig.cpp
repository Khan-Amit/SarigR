// ============================================================
// XMR RIG HARVESTER - Binary Logic Selection Engine
// ============================================================

#include <Arduino.h>

// === CONFIGURATION ===
#define NUM_CHANNELS 12
#define ADC_MAX 4095.0f
#define V_REF 3.3f

// Your 12 ADC channels (GPUs, PSU rails, temp sensors)
int adcValues[NUM_CHANNELS] = {0};
uint8_t binaryThresholds[NUM_CHANNELS] = {0};

// Competition threshold - What's considered "winning"
int globalThreshold = 2048; // 50% of 4096 ADC range

// Performance tracking
float selectionHistory[100] = {0};
int historyIndex = 0;

void setup() {
    Serial.begin(115200);
    Serial.println("=== XMR RIG HARVESTER INITIALIZED ===");
    Serial.println("Binary Selection Engine ACTIVE");
    Serial.println("=====================================\n");
    
    // Optional: Set ADC resolution for better precision
    analogReadResolution(12); // 0-4095 (if using ESP32)
}

void loop() {
    // === STEP 1: READ ALL SENSORS ===
    readAllSensors();
    
    // === STEP 2: BINARY SELECTION ANALYSIS ===
    SelectionResult result = analyzeSelection();
    
    // === STEP 3: COMPETITIVE SORTING ===
    sortByPerformance();
    
    // === STEP 4: EXECUTE DECISIONS ===
    executeMiningStrategy(result);
    
    // === STEP 5: LOG AND REPORT ===
    printReport(result);
    
    delay(1000); // 1Hz update rate
}

// ============================================================
// CORE FUNCTIONS
// ============================================================

void readAllSensors() {
    for(int i = 0; i < NUM_CHANNELS; i++) {
        // Simulated ADC reads - replace with actual analogRead()
        // For ESP32: adcValues[i] = analogRead(i);
        
        // DEMO: Generate realistic mining rig data (hash rates, temps, voltages)
        adcValues[i] = generateSimulatedData(i);
    }
}

int generateSimulatedData(int channel) {
    // Simulates real rig telemetry:
    // Channels 0-3: GPU Hash rates (higher = better)
    // Channels 4-7: PSU Voltage rails (should be stable)
    // Channels 8-11: Temperature sensors (lower = better, invert logic)
    
    int baseValue = 2000; // Center point
    
    if(channel < 4) {
        // Hash rates: 1500-3500 (higher is winning)
        return baseValue + random(-500, 1500);
    } else if(channel < 8) {
        // Voltages: 1800-2200 (stable ~2000)
        return baseValue + random(-200, 200);
    } else {
        // Temperatures: 500-3000 (LOWER is winning - inverted later)
        return baseValue + random(-500, 1000);
    }
}

// ============================================================
// THE BINARY DECISION ENGINE
// ============================================================

struct SelectionResult {
    int winners;
    int losers;
    int dominantIndex;
    int dominantValue;
    int minValue;
    int spread;
    float selectionRatio;
    bool isRigHealthy;
};

SelectionResult analyzeSelection() {
    SelectionResult result;
    result.winners = 0;
    result.losers = 0;
    result.dominantIndex = -1;
    result.dominantValue = 0;
    result.minValue = 4095;
    result.spread = 0;
    
    // === THE BINARY LOGIC LOOP ===
    for(int i = 0; i < NUM_CHANNELS; i++) {
        // For temperature sensors (channels 8-11), invert logic
        bool isWinner;
        if(i >= 8) {
            // Temperature: LOWER is better
            isWinner = (adcValues[i] < globalThreshold);
        } else {
            // Performance: HIGHER is better
            isWinner = (adcValues[i] > globalThreshold);
        }
        
        // Binary assignment (THE DECISION)
        if(isWinner) {
            binaryThresholds[i] = 255;  // TRUE - WINNER
            result.winners++;
        } else {
            binaryThresholds[i] = 0;    // FALSE - LOSER
            result.losers++;
        }
        
        // Track extremes for competitive analysis
        if(adcValues[i] > result.dominantValue) {
            result.dominantValue = adcValues[i];
            result.dominantIndex = i;
        }
        if(adcValues[i] < result.minValue) {
            result.minValue = adcValues[i];
        }
    }
    
    // Calculate competitive metrics
    result.selectionRatio = (float)result.winners / NUM_CHANNELS * 100.0f;
    result.spread = result.dominantValue - result.minValue;
    result.isRigHealthy = (result.winners > (NUM_CHANNELS / 2));
    
    return result;
}

// ============================================================
// COMPETITIVE SORTING (Selection Sort using Binary Compare)
// ============================================================

void sortByPerformance() {
    // Create a copy with indices for sorting
    int sortedIndices[NUM_CHANNELS];
    int sortedValues[NUM_CHANNELS];
    
    for(int i = 0; i < NUM_CHANNELS; i++) {
        sortedIndices[i] = i;
        sortedValues[i] = adcValues[i];
    }
    
    // Selection sort - THE PERFECT COMPARATOR
    for(int i = 0; i < NUM_CHANNELS - 1; i++) {
        int maxIdx = i;
        for(int j = i + 1; j < NUM_CHANNELS; j++) {
            // BINARY COMPARISON: Is j BETTER than i?
            if(sortedValues[j] > sortedValues[maxIdx]) {
                maxIdx = j;
            }
        }
        if(maxIdx != i) {
            // Swap values
            int tempVal = sortedValues[i];
            sortedValues[i] = sortedValues[maxIdx];
            sortedValues[maxIdx] = tempVal;
            
            // Swap indices
            int tempIdx = sortedIndices[i];
            sortedIndices[i] = sortedIndices[maxIdx];
            sortedIndices[maxIdx] = tempIdx;
        }
    }
    
    // Store top performer for quick access
    // (Already tracked in analyzeSelection)
}

// ============================================================
// STRATEGY EXECUTION
// ============================================================

void executeMiningStrategy(SelectionResult result) {
    if(result.isRigHealthy) {
        // RIG IS OPTIMAL - Harvest at full power
        setGlobalPower(100);
        
        // Boost the DOMINANT channel
        if(result.dominantIndex >= 0 && result.dominantIndex < 4) {
            // It's a GPU - give it more hashing priority
            setGPUPriority(result.dominantIndex, 120); // Overclock
        }
        
    } else {
        // RIG IS UNDERPERFORMING - Diagnostic mode
        setGlobalPower(80);
        
        // Find and fix bottlenecks
        for(int i = 0; i < NUM_CHANNELS; i++) {
            if(binaryThresholds[i] == 0) { // LOSER
                if(i < 4) {
                    // GPU underperforming - reduce load
                    setGPUPriority(i, 60);
                    Serial.print("WARNING: GPU ");
                    Serial.print(i);
                    Serial.println(" is underperforming");
                } else if(i >= 8) {
                    // Overheating sensor
                    Serial.print("WARNING: Overheat detected on sensor ");
                    Serial.println(i);
                    activateCooling(i);
                }
            }
        }
    }
}

// ============================================================
// HARDWARE CONTROL STUBS (Replace with actual hardware calls)
// ============================================================

void setGlobalPower(int percentage) {
    // Send PWM signal to PSU controller
    // Example: analogWrite(POWER_CONTROL_PIN, map(percentage, 0, 100, 0, 255));
    // For demo, just print:
    // Serial.print("Global Power: "); Serial.println(percentage);
}

void setGPUPriority(int gpuIndex, int priority) {
    // Send I2C/SPI commands to GPU power stages
    // Example: i2cWrite(gpuIndex, priority);
    // For demo, just store:
    // gpuPriority[gpuIndex] = priority;
}

void activateCooling(int sensorIndex) {
    // Trigger fans or alerts
    // digitalWrite(FAN_PIN[sensorIndex], HIGH);
}

// ============================================================
// REPORTING & MONITORING
// ============================================================

void printReport(SelectionResult result) {
    Serial.println("=== RIG STATUS REPORT ===");
    
    // Binary visualization
    Serial.print("Binary Map: ");
    for(int i = 0; i < NUM_CHANNELS; i++) {
        Serial.print(binaryThresholds[i] == 255 ? "█" : "░");
        if((i+1) % 4 == 0) Serial.print(" ");
    }
    Serial.println();
    
    // Raw values
    Serial.print("Raw ADC: ");
    for(int i = 0; i < NUM_CHANNELS; i++) {
        Serial.print(adcValues[i]);
        Serial.print(" ");
        if((i+1) % 4 == 0) Serial.print("| ");
    }
    Serial.println();
    
    // Metrics
    Serial.print("Winners: ");
    Serial.print(result.winners);
    Serial.print("/");
    Serial.print(NUM_CHANNELS);
    Serial.print(" (");
    Serial.print(result.selectionRatio, 1);
    Serial.println("%)");
    
    Serial.print("Dominant Channel: ");
    Serial.print(result.dominantIndex);
    Serial.print(" | Value: ");
    Serial.print(result.dominantValue);
    Serial.print(" | Spread: ");
    Serial.println(result.spread);
    
    Serial.print("Rig Health: ");
    Serial.println(result.isRigHealthy ? "OPTIMAL ✓" : "WARNING ⚠");
    
    // Competitive Advantage calculation
    float competitiveEdge = (float)result.dominantValue / (result.minValue + 1);
    Serial.print("Competitive Edge: ");
    Serial.println(competitiveEdge, 2);
    
    Serial.println("============================\n");
}

// ============================================================
// WORLD EYES EXTENSION - For Visual/Image Processing
// ============================================================

uint8_t* processWorldEyes(uint8_t image[][640], int rows, int cols, uint8_t threshold) {
    static uint8_t output[480][640];
    int brightPixels = 0;
    int totalPixels = rows * cols;
    
    for(int y = 0; y < rows; y++) {
        for(int x = 0; x < cols; x++) {
            // THE BINARY DECISION
            if(image[y][x] > threshold) {
                output[y][x] = 255;   // TRUE - Target detected
                brightPixels++;
            } else {
                output[y][x] = 0;     // FALSE - Background
            }
        }
    }
    
    float selectionPower = (float)brightPixels / totalPixels * 100.0f;
    Serial.print("World Eyes Selection Power: ");
    Serial.print(selectionPower, 1);
    Serial.println("%");
    
    return (uint8_t*)output;
}

// ============================================================
// OPTIONAL: Moving Average Filter for Stable Readings
// ============================================================

class MovingAverage {
private:
    int window[10];
    int index;
    int count;
    int sum;
    
public:
    MovingAverage() : index(0), count(0), sum(0) {}
    
    int filter(int newValue) {
        if(count < 10) {
            window[count++] = newValue;
            sum += newValue;
            return sum / count;
        } else {
            sum -= window[index];
            window[index] = newValue;
            sum += newValue;
            index = (index + 1) % 10;
            return sum / 10;
        }
    }
};

// Instantiate filters for each channel
MovingAverage filters[NUM_CHANNELS];

// ============================================================
// READY TO HARVEST!
// ============================================================
