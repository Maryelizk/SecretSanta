#include <SPI.h>
#include <MFRC522.h>

#define DEBUG_PRINTS 0  // Set to 1 to enable Serial prints, 0 to disable

#if DEBUG_PRINTS
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#endif

// RFID pins definition
#define RST_PIN         10    // RST -> D10
#define SS_PIN          9     // SDA -> D9
// Standard SPI pins:
// SCK  -> D13 (hardware SPI)
// MOSI -> D11 (hardware SPI)
// MISO -> D12 (hardware SPI)

// Create MFRC522 instance
MFRC522 mfrc522(SS_PIN, RST_PIN);

#define MAX_PEOPLE      8          
#define ACTIVE_GIFTERS  7          

// LED pins in the physical order they appear
const int LED_PINS[MAX_PEOPLE] = {8, 7, 6, 3, 4, 5, A2, A1};  // Updated to match physical layout
// Maps person index to LED array index for correct physical positioning
const int LED_MAP[MAX_PEOPLE] = {0, 1, 2, 3, 4, 5, 6, 7};  // This array maps person indices to LED positions

// Structure to represent a person
struct Person {
    char name[10];         
    byte rfidUID[4];       
    int assigned[2];       
    bool uidStored;
    int partner_id; // ID of this person's partner (-1 if single)
    bool isKiddo;
};

Person people[MAX_PEOPLE];
int numPeople = 0;
bool displayActive = false;

// Helper functions updated for new LED sequence
void clearLEDs() {
    for (int i = 0; i < MAX_PEOPLE; i++) {
        digitalWrite(LED_PINS[i], LOW);
    }
    displayActive = false;
}

void sequentialLEDs(bool reverse = false) {
    if (!reverse) {
        for (int i = 0; i < MAX_PEOPLE; i++) {
            digitalWrite(LED_PINS[i], HIGH);
            delay(100);
            digitalWrite(LED_PINS[i], LOW);
        }
    } else {
        for (int i = MAX_PEOPLE - 1; i >= 0; i--) {
            digitalWrite(LED_PINS[i], HIGH);
            delay(100);
            digitalWrite(LED_PINS[i], LOW);
        }
    }
}

void doubleBlink(int ledIndex) {
    for (int i = 0; i < 2; i++) {
        digitalWrite(LED_PINS[ledIndex], HIGH);
        delay(200);
        digitalWrite(LED_PINS[ledIndex], LOW);
        delay(200);
    }
}
void celebrateRegistration(int personIndex) {
    // Get the LED position for this person
    int ledPosition = LED_MAP[personIndex];
    
    // Initial sequence up and down
    sequentialLEDs(false);  // Forward
    sequentialLEDs(true);   // Reverse
    
    // Now do "spinning" effect that slows down and stops at the person's LED
    int currentPos = 0;
    int cycles = 2;  // Number of full cycles before slowing down
    int delayTime = 50;  // Start fast
    
    // Do some full-speed cycles
    for (int cycle = 0; cycle < cycles; cycle++) {
        for (int i = 0; i < MAX_PEOPLE; i++) {
            digitalWrite(LED_PINS[i], HIGH);
            delay(delayTime);
            digitalWrite(LED_PINS[i], LOW);
        }
    }
    
    // Now gradually slow down until we reach the target LED
    while (true) {
        digitalWrite(LED_PINS[currentPos], HIGH);
        delay(delayTime);
        digitalWrite(LED_PINS[currentPos], LOW);
        
        if (currentPos == ledPosition && delayTime >= 200) {
            break;
        }
        
        currentPos = (currentPos + 1) % MAX_PEOPLE;
        
        if (currentPos == 0) {
            delayTime += 50;
        }
    }
    
    // Final double blink on the registered person's LED
    doubleBlink(ledPosition);
}

// Initialize people's names and partnerships
void initializePeople() {
    // First couple
    strncpy(people[0].name, "Kim", 9);
    people[0].uidStored = false;
    people[0].partner_id = -1; // Now partnered with Bob
    people[0].isKiddo = false;
    
    strncpy(people[1].name, "Henry", 9); // Henry is now in the second position
    people[1].uidStored = false;
    people[1].partner_id = -1;
    people[1].isKiddo = true;
    
    // Second couple
    strncpy(people[2].name, "Zach", 9);
    people[2].uidStored = false;
    people[2].partner_id = 3;
    people[2].isKiddo = false;
    
    strncpy(people[3].name, "Maryeliz", 9);
    people[3].uidStored = false;
    people[3].partner_id = 2;
    people[3].isKiddo = false;
    
    // Third couple
    strncpy(people[4].name, "Andreas", 9);
    people[4].uidStored = false;
    people[4].partner_id = 5;
    people[4].isKiddo = false;
    
    strncpy(people[5].name, "Julianne", 9);
    people[5].uidStored = false;
    people[5].partner_id = 4;
    people[5].isKiddo = false;
    
    // Fourth couple
    strncpy(people[6].name, "Elise", 9);
    people[6].uidStored = false;
    people[6].partner_id = 7;
    people[6].isKiddo = false;
    
    strncpy(people[7].name, "Alex", 9);
    people[7].uidStored = false;
    people[7].partner_id = 6; 
    people[7].isKiddo = false;

    numPeople = 8;
}


int countGiftsReceiving(int personIndex) {
    int count = 0;
    for (int i = 0; i < numPeople; i++) { // Loop over all participants
        if (people[i].isKiddo) continue; // Skip Henry
        for (int j = 0; j < 2; j++) {
            if (people[i].assigned[j] == personIndex) {
                count++;
            }
        }
    }
    return count;
}

bool isValidAssignment(int giver, int receiver) {
    if (giver == receiver) return false; // A person can't give to themselves
    if (people[giver].isKiddo || people[receiver].isKiddo) return false; // Skip Henry
    if (people[giver].partner_id == receiver) return false; // Avoid assigning to partner
    return true;
}


void shuffleArray(int arr[], int size) {
    for (int i = size - 1; i > 0; i--) {
        int j = random(i + 1);
        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

bool generateAssignments() {
    const int MAX_ATTEMPTS = 100;
    int attempts = 0;

    while (attempts < MAX_ATTEMPTS) {
        DEBUG_PRINT("Attempt: ");
        DEBUG_PRINTLN(attempts);

        // Reset assignments
        for (int i = 0; i < numPeople; i++) {
            people[i].assigned[0] = -1;
            people[i].assigned[1] = -1;
        }

        bool success = true;

        // Assign gifts for active gifters (excluding Henry)
        for (int i = 0; i < numPeople && success; i++) {
            if (people[i].isKiddo) continue;

            DEBUG_PRINT("Assigning for: ");
            DEBUG_PRINTLN(people[i].name);

            int availableRecipients[numPeople - 1];
            int recipientCount = 0;

            // Populate `availableRecipients` array
            for (int j = 0; j < numPeople; j++) {
                if (j != i && !people[j].isKiddo) {
                    availableRecipients[recipientCount++] = j;
                }
            }

            shuffleArray(availableRecipients, recipientCount);

            int assignedCount = 0;
            for (int j = 0; j < recipientCount && assignedCount < 2; j++) {
                int recipient = availableRecipients[j];
                if (isValidAssignment(i, recipient) && countGiftsReceiving(recipient) < 2) {
                    people[i].assigned[assignedCount++] = recipient;
                    DEBUG_PRINT("Assigned to: ");
                    DEBUG_PRINTLN(people[recipient].name);
                }
            }

            if (assignedCount < 2) {
                success = false;
                DEBUG_PRINTLN("Failed to assign both gifts!");
            }
        }

        if (success) {
            DEBUG_PRINTLN("Assignments generated successfully!");
            return true;
        }
        attempts++;
    }

    DEBUG_PRINTLN("Failed to generate assignments after maximum attempts.");
    return false;
}


void showAssignments(int personIndex) {
    clearLEDs();
    if (people[personIndex].isKiddo) {
        digitalWrite(LED_PINS[LED_MAP[personIndex]], HIGH); // Only light up his LED
    } else {
        // Map the assigned people indices to their LED positions
        digitalWrite(LED_PINS[LED_MAP[people[personIndex].assigned[0]]], HIGH);
        digitalWrite(LED_PINS[LED_MAP[people[personIndex].assigned[1]]], HIGH);
    }
    displayActive = true;
}


bool compareUIDs(byte* uid1, byte* uid2) {
    for (int i = 0; i < 4; i++) {
        if (uid1[i] != uid2[i]) return false;
    }
    return true;
}

int findPersonByUID(byte* uid) {
    for (int i = 0; i < numPeople; i++) {
        if (people[i].uidStored) {
            if (compareUIDs(people[i].rfidUID, uid)) return i;
        }
    }
    return -1;
}

bool registerNewCard(byte* uid) {
    for (int i = 0; i < numPeople; i++) {
        if (!people[i].uidStored) {
            for (int j = 0; j < 4; j++) {
                people[i].rfidUID[j] = uid[j];
            }
            people[i].uidStored = true;
            Serial.print(F("Registered card for "));
            Serial.println(people[i].name);
            celebrateRegistration(i);
            return true;
        }
    }
    return false;
}

void setup() {
    // Initialize LEDs
    for (int i = 0; i < MAX_PEOPLE; i++) {
        pinMode(LED_PINS[i], OUTPUT);
        digitalWrite(LED_PINS[i], LOW);
    }

    // Sequential LED loop until RFID is initialized
    bool rfidInitialized = false;
    while (!rfidInitialized) {
        for (int i = 0; i < MAX_PEOPLE; i++) {
            digitalWrite(LED_PINS[i], HIGH);
            delay(100);
            digitalWrite(LED_PINS[i], LOW);
        }

        // Try to initialize RFID reader
        SPI.begin();
        mfrc522.PCD_Init();
        delay(4);
        byte v = mfrc522.PCD_ReadRegister(MFRC522::VersionReg);

        if (v != 0x00 && v != 0xFF) {
            rfidInitialized = true;
        }
    }

    // Flash all LEDs twice to indicate RFID is ready
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < MAX_PEOPLE; j++) {
            digitalWrite(LED_PINS[j], HIGH);
        }
        delay(300);
        for (int j = 0; j < MAX_PEOPLE; j++) {
            digitalWrite(LED_PINS[j], LOW);
        }
        delay(300);
    }

    // Initialize random seed
    randomSeed(analogRead(A0));

    // Initialize people and assignments
    initializePeople();
    while (!generateAssignments()) {
        DEBUG_PRINTLN(F("Retrying assignment generation..."));
        delay(100);
    }

    // Flash all LEDs twice to indicate assignments are ready
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < MAX_PEOPLE; j++) {
            digitalWrite(LED_PINS[j], HIGH);
        }
        delay(300);
        for (int j = 0; j < MAX_PEOPLE; j++) {
            digitalWrite(LED_PINS[j], LOW);
        }
        delay(300);
    }
}

void loop() {
    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
        return;
    }

    int personIndex = findPersonByUID(mfrc522.uid.uidByte);

    if (personIndex == -1) {
        registerNewCard(mfrc522.uid.uidByte);
    } else {
        if (displayActive) {
            clearLEDs();
        } else {
            DEBUG_PRINT(F("Showing assignments for "));
            DEBUG_PRINTLN(people[personIndex].name);
            showAssignments(personIndex);
        }
    }

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
}
