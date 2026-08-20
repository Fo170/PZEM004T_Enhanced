/*
 * Exemple : PZEM004T_Enhanced sur port série matériel (HardwareSerial)
 *
 * Cartes visées : Arduino Mega (Serial1/Serial2/Serial3), ESP32 (Serial1...)
 * Sur ESP32, il faut préciser les broches RX/TX du port (voir l'exemple PZEM_ESP32).
 */

#include "PZEM004T_Enhanced.h"

// Arduino Mega : Serial1 est disponible, sinon passer le port matériel par référence
#if defined(ESP32)
// Sur ESP32, utiliser plutôt l'exemple PZEM_ESP32 (broches requises)
PZEM004T_Enhanced pzem(&Serial1);
#else
PZEM004T_Enhanced pzem(Serial1);
#endif

void setup() {
    Serial.begin(115200);
    delay(2000);

    Serial.println("PZEM004T_Enhanced - HardwareSerial");

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
    float Q = pzem.reactivePower();    // Puissance réactive en VAR
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
        Serial.print("Q réact.  : "); Serial.print(Q); Serial.println(" VAR");
        Serial.print("PF        : "); Serial.println(PF);
        Serial.print("Fréquence : "); Serial.print(F); Serial.println(" Hz");
        Serial.print("Énergie   : "); Serial.print(E); Serial.println(" kWh");
    }

    Serial.println();
    delay(2000);
}
