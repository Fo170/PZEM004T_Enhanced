/*
 * Exemple : PZEM004T_Enhanced sur ESP32 (HardwareSerial avec broches configurables)
 *
 * Sur ESP32, le constructeur par référence exige les broches RX/TX :
 * le port est initialisé avec begin(9600, SERIAL_8N1, rx, tx).
 *
 * Câblage (exemple avec Serial1 sur GPIO 16/17) :
 *   PZEM-004T TX -> GPIO16 (RX du port)  PZEM-004T RX -> GPIO17 (TX du port)
 *   PZEM-004T 5V/GND -> alimentation (le module a besoin du 230V AC branché !)
 */

#include "PZEM004T_Enhanced.h"

// Serial1 : broches RX = 16, TX = 17 (à adapter à votre câblage)
PZEM004T_Enhanced pzem(Serial1, 16, 17);

void setup() {
    Serial.begin(115200);
    delay(2000);

    Serial.println("PZEM004T_Enhanced - ESP32 (Serial1, RX=16, TX=17)");

    // Optionnel : réinitialiser le compteur d'énergie interne
    // if (pzem.resetEnergy()) Serial.println("Compteur d'énergie réinitialisé");

    // Optionnel : changer l'adresse Modbus du module (0x01..0xF7)
    // pzem.setAddress(0x01);

    // Affiche l'adresse Modbus réelle du module
    Serial.print("Adresse Modbus : ");
    Serial.println(pzem.readAddress(), HEX);
}

void loop() {
    float V = pzem.voltage();          // Tension en V
    float I = pzem.current();          // Courant en A
    float P = pzem.power();            // Puissance active en W
    float S = pzem.apparentPower();    // Puissance apparente en VA
    float N = pzem.nonActivePower();   // Puissance non-active en VAR
    float PF = pzem.pf();              // Facteur de puissance
    float F = pzem.frequency();        // Fréquence en Hz
    float E = pzem.energy();           // Énergie en kWh

    if (isnan(V)) {
        Serial.println("Erreur de lecture (Vérifiez les fils RX/TX et l'alimentation)");
    } else {
        Serial.println("--- Mesures ---");
        Serial.print("Tension   : "); Serial.print(V); Serial.println(" V");
        Serial.print("Courant   : "); Serial.print(I); Serial.println(" A");
        Serial.print("P active  : "); Serial.print(P); Serial.println(" W");
        Serial.print("S app.    : "); Serial.print(S); Serial.println(" VA");
        Serial.print("N non-act.: "); Serial.print(N); Serial.println(" VAR");
        Serial.print("PF        : "); Serial.println(PF);
        Serial.print("Fréquence : "); Serial.print(F); Serial.println(" Hz");
        Serial.print("Énergie   : "); Serial.print(E); Serial.println(" kWh");
    }

    Serial.println();
    delay(2000);
}
