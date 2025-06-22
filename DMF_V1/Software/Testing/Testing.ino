    #include <Arduino.h>

    // HV507 pin definitions (update these as needed)
    #define HV507_DATA   14  // Example pin for DATA
    #define HV507_CLK    18  // Example pin for CLOCK
    #define HV507_LATCH  17  // Example pin for LATCH
    #define HV507_BLANK  15  // Optional: BLANK/ENABLE
    #define HV507_POL  16  // Optional: POLARITY (if needed)
    // Buttons pin definitions (update these as needed)
    #define ENTER_BUTTON_PIN  8
    #define UP_BUTTON_PIN     9
    #define LEFT_BUTTON_PIN  10 
    #define RIGHT_BUTTON_PIN 11
    #define DOWN_BUTTON_PIN  12
    #define BOMB_BUTTON_PIN  11
    // Number of electrodes (channels) in HV507
    #define HV507_CHANNELS 64
    #define BUZZER_PIN 47

    // State of all electrodes (1 = ON, 0 = OFF)
    uint64_t electrodeState = 0;

    void HV507_init() {
        pinMode(HV507_DATA, OUTPUT);
        pinMode(HV507_CLK, OUTPUT);
        pinMode(HV507_LATCH, OUTPUT);
        pinMode(HV507_BLANK, OUTPUT);
        digitalWrite(HV507_DATA, LOW);
        digitalWrite(HV507_CLK, LOW);
        digitalWrite(HV507_LATCH, LOW);
        digitalWrite(HV507_BLANK, LOW); // Enable outputs
    }
    void Buttons_init() {
        pinMode(ENTER_BUTTON_PIN, INPUT_PULLUP);
        pinMode(UP_BUTTON_PIN, INPUT_PULLUP);
        pinMode(LEFT_BUTTON_PIN, INPUT_PULLUP);
        pinMode(RIGHT_BUTTON_PIN, INPUT_PULLUP);
        pinMode(DOWN_BUTTON_PIN, INPUT_PULLUP);
    }
    void turnOnAllOutputs() {
    electrodeState = 0xFFFFFFFFFFFFFFFFULL; // All 64 bits set to 1
    HV507_update();
    }
    void setup() {
        HV507_init();
        Serial.begin(115200);
        Buttons_init();
        pinMode(BUZZER_PIN, OUTPUT);
        digitalWrite(BUZZER_PIN, LOW);
        Serial.println("Ready. Press Enter_button to move droplet.");
        turnOnAllOutputs();
    }

    // Send the current electrodeState to HV507
    void HV507_update() {
        digitalWrite(HV507_LATCH, LOW);
        for (int i = HV507_CHANNELS - 1; i >= 0; i--) {
            digitalWrite(HV507_CLK, LOW);
            digitalWrite(HV507_DATA, (electrodeState >> i) & 0x1);
            digitalWrite(HV507_CLK, HIGH);
        }
        digitalWrite(HV507_LATCH, HIGH);
        delayMicroseconds(1);
        digitalWrite(HV507_LATCH, LOW);
    }

    // Set a specific electrode ON or OFF
    void setElectrode(int idx, bool on) {
        if (idx < 0 || idx >= HV507_CHANNELS) return;
        if (on) electrodeState |= ((uint64_t)1 << idx);
        else   electrodeState &= ~((uint64_t)1 << idx);
        HV507_update();
    }

    const int dropletPath[] = {58,57,56,55,54,53,48,46,45,44,43,41,40,32,14,27,34,35,39};
    const int dropletPathLen = sizeof(dropletPath)/sizeof(dropletPath[0]);

    void buzz() {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(50); // Buzzer on for 50 ms
        digitalWrite(BUZZER_PIN, LOW);
    }

    void moveDropletPath(bool forward) {
        electrodeState = 0;
        HV507_update();
        delay(100);
        if (forward) {
            for (int i = 0; i < dropletPathLen; ++i) {
                Serial.printf("Step %2d/%2d: Move droplet to electrode %d\n", i+1, dropletPathLen, dropletPath[i]);
                setElectrode(dropletPath[i], true);
                buzz();
                if (i > 0) {
                    delay(200);
                    setElectrode(dropletPath[i-1], false);
                    Serial.printf("Electrode %d OFF\n", dropletPath[i-1]);
                }
                delay(200);
                Serial.print("Electrode state: 0b");
                for (int b = HV507_CHANNELS-1; b >= 0; --b) {
                    Serial.print((electrodeState >> b) & 0x1);
                }
                Serial.println();
            }
            Serial.printf("Droplet stopped at electrode %d (remains ON)\n", dropletPath[dropletPathLen-1]);
            Serial.println("Droplet movement complete.");
        } else {
            for (int i = dropletPathLen - 1; i >= 0; --i) {
                Serial.printf("Step %2d/%2d: Move droplet to electrode %d (backward)\n", dropletPathLen-i, dropletPathLen, dropletPath[i]);
                setElectrode(dropletPath[i], true);
                buzz();
                if (i < dropletPathLen - 1) {
                    delay(200);
                    setElectrode(dropletPath[i+1], false);
                    Serial.printf("Electrode %d OFF\n", dropletPath[i+1]);
                }
                delay(200);
                Serial.print("Electrode state: 0b");
                for (int b = HV507_CHANNELS-1; b >= 0; --b) {
                    Serial.print((electrodeState >> b) & 0x1);
                }
                Serial.println();
            }
            Serial.printf("Droplet stopped at electrode %d (remains ON)\n", dropletPath[0]);
            Serial.println("Droplet movement complete (backward).");
        }
    }

    void bombSound() {
        // Simulate a bomb sound: slow beeps, then a long beep, total ~10 seconds
        int beepCount = 12;
        int baseDelay = 350; // Start with 350ms per beep
        for (int i = 0; i < beepCount; ++i) {
            digitalWrite(BUZZER_PIN, HIGH);
            delay(baseDelay - i*20); // Beeps get slightly faster
            digitalWrite(BUZZER_PIN, LOW);
            delay(baseDelay - i*20);
        }
        delay(200);
        digitalWrite(BUZZER_PIN, HIGH);
        delay(2000); // Long final beep
        digitalWrite(BUZZER_PIN, LOW);
    }

    void loop() {
        static bool lastEnterButtonState = HIGH;
        static bool lastBombButtonState = HIGH;
        static bool forward = true;
        bool enterButtonState = digitalRead(ENTER_BUTTON_PIN);
        bool bombButtonState = digitalRead(BOMB_BUTTON_PIN);
        if (lastEnterButtonState == HIGH && enterButtonState == LOW) {
            if (forward) {
                Serial.println("Button pressed. Moving droplet forward...");
                moveDropletPath(true);
                forward = false;
            } else {
                Serial.println("Button pressed. Moving droplet backward...");
                moveDropletPath(false);
                forward = true;
            }
        }
        if (lastBombButtonState == HIGH && bombButtonState == LOW) {
            Serial.println("Bomb button pressed! Simulating bomb sound...");
            bombSound();
        }
        lastEnterButtonState = enterButtonState;
        lastBombButtonState = bombButtonState;
    }
