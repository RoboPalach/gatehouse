#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <EEPROM.h>

#define SS_PIN 10
#define RST_PIN 9
#define RELAY 7
#define BUTTON_PIN 5

LiquidCrystal_I2C lcd(0x27,20,4);  // set the lcd address to 0x27 for a 16 chars and 2 line display
MFRC522 rfid(SS_PIN, RST_PIN);

typedef uint8_t user_t[4];

user_t master;
user_t *bois;
int n_users = 0;

enum CardType {
    Unauthorized,
    Boi,
    Master
};

void loadUser(char index) {
    for (int i = 0; i < 4; i++) {
        bois[index][i] = EEPROM.read(5 + (4 * index) + i);
    }
}

void loadUsers() {
    n_users = EEPROM.read(0);

    bois = (user_t*) malloc(n_users * sizeof(user_t));

    for (int i = 0; i < 4; i++) {
        master[i] = EEPROM.read(i + 1);
    }

    for (int i = 0; i < n_users; i++) {
        loadUser(i);
    }
}

// Check the type of the card
CardType checkCard() {
    if (rfid.uid.uidByte[0] == master[0] && 
        rfid.uid.uidByte[1] == master[1] &&
        rfid.uid.uidByte[2] == master[2] &&
        rfid.uid.uidByte[3] == master[3])
        return Master;

    for (int i = 0; i < n_users; i++)
    {
        if (rfid.uid.uidByte[0] == bois[i][0] &&
            rfid.uid.uidByte[1] == bois[i][1] &&
            rfid.uid.uidByte[2] == bois[i][2] &&
            rfid.uid.uidByte[3] == bois[i][3])
            return Boi;
    }
    
    return Unauthorized;
}

// Senpai uwu
void masterMode() {
    Serial.println("I wasn't expecting you so early senpai *blush*"); // I want to fucking die
    lcd.clear();
    lcd.print("EPIC MASTER MODE");
    delay(500);

    // load new cards and do some other boring stuff
    while (true) {
        // Is new card present?
        if (!rfid.PICC_IsNewCardPresent())
            continue;

        // Is there a card?
        if (!rfid.PICC_ReadCardSerial())
            continue;

        // Get info about the card
        CardType cardType = checkCard();

        // Card is known, delete it from the EEPROM
        if (cardType == Boi) {
            for (int i = 0; i < n_users; i++)
            {
                if (rfid.uid.uidByte[0] == bois[i][0] &&
                    rfid.uid.uidByte[1] == bois[i][1] &&
                    rfid.uid.uidByte[2] == bois[i][2] &&
                    rfid.uid.uidByte[3] == bois[i][3]) {

                    bois[i][0] = bois[n_users - 1][0];
                    bois[i][1] = bois[n_users - 1][1];
                    bois[i][2] = bois[n_users - 1][2];
                    bois[i][3] = bois[n_users - 1][3];

                    EEPROM.write(5 + 4 * i + 0, bois[n_users - 1][0]);
                    EEPROM.write(5 + 4 * i + 1, bois[n_users - 1][1]);
                    EEPROM.write(5 + 4 * i + 2, bois[n_users - 1][2]);
                    EEPROM.write(5 + 4 * i + 3, bois[n_users - 1][3]);

                    break;
                }
            }
            
            n_users -= 1;
            EEPROM.write(0, n_users);

            lcd.clear();
            Serial.println("I'll get rid of him... *loads gun with religious intent*");
            lcd.print("  BEGONE THOT!");
            delay(500);

        // Card is not known, save it into the EEPROM!
        } else if (cardType == Unauthorized) {
            bois = (user_t*) realloc(bois, (n_users + 1) * sizeof(user_t));

            bois[n_users][0] = rfid.uid.uidByte[0];
            bois[n_users][1] = rfid.uid.uidByte[1];
            bois[n_users][2] = rfid.uid.uidByte[2];
            bois[n_users][3] = rfid.uid.uidByte[3];

            EEPROM.write(5 + n_users * 4 + 0, bois[n_users][0]);
            EEPROM.write(5 + n_users * 4 + 1, bois[n_users][1]);
            EEPROM.write(5 + n_users * 4 + 2, bois[n_users][2]);
            EEPROM.write(5 + n_users * 4 + 3, bois[n_users][3]);

            n_users++;

            EEPROM.write(0, n_users);
            
            Serial.println("new boi added xd");
            lcd.clear();
            lcd.print("NEW BOI ADDED XD");
            delay(500);

        // The card is master, leave master mode
        } else if (cardType == Master) {
            Serial.println("so long master uwu...");
            lcd.clear();
            lcd.print(" BYE SENPAI UWU");
            delay(1000);
            return;
        }

        lcd.clear();
        lcd.print("EPIC MASTER MODE");
    }
}

// Open Sesame!
void openDoor() {
    digitalWrite(RELAY, LOW);
    delay(3000);
    digitalWrite(RELAY, HIGH);
}


void setup(){
    Serial.begin(115200);
    
    SPI.begin();
    rfid.PCD_Init();
    rfid.PCD_SetAntennaGain(rfid.RxGain_max);

    lcd.init();
    lcd.backlight();

    lcd.print("Hello world...");
    lcd.setCursor(0, 1);
    Serial.println("oh this is running");
}

void loop(){
    Serial.println("loopin");
    if (digitalRead(BUTTON_PIN) == LOW) {
        openDoor();
        return;
    }

    // Is new card present?
    if (!rfid.PICC_IsNewCardPresent()){ 
        lcd.setCursor(0,1);
        lcd.println("New card");
        return;
    }

    // Is there a card?
    if (!rfid.PICC_ReadCardSerial()){ 
        lcd.setCursor(0,1);
        lcd.println("New card");
        return;
    }

    // Get info about the card
    CardType cardType = checkCard();

    // this is fucking awful
    Serial.print("new card: ");
    Serial.print(rfid.uid.uidByte[0]);
    Serial.print(" ");
    Serial.print(rfid.uid.uidByte[1]);
    Serial.print(" ");
    Serial.print(rfid.uid.uidByte[2]);
    Serial.print(" ");
    Serial.print(rfid.uid.uidByte[3]);
    Serial.print(" ");
    Serial.print(" - ");
    Serial.println(cardType);


    // If master, enter master mode
    if (cardType == Master) {
        masterMode();

    // If known, open door
    } else if (cardType == Boi) {
        Serial.println("i know the taste of this one uwu");
        lcd.clear();
        lcd.print(" WELCOME HUMAN!");
        openDoor();

    // Not known, begone thot!
    } else {
        Serial.println("Impossible. Perhaps the archives are incomplete?");
        lcd.clear();
        lcd.print("     OPENED");
        lcd.setCursor(0, 1);
        lcd.print(" jk go away lol");
        lcd.home();
        delay(3000);
    }

    lcd.clear();
    lcd.print("Welcome");
}
