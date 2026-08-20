# PZEM004T_Enhanced

[![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)](PZEM004T_Enhanced.h)
[![Licence](https://img.shields.io/badge/licence-MIT-green.svg)](LICENSE)

Bibliothèque Arduino pour le compteur d'énergie **PZEM-004T Enhanced v1.0.0** (interface Modbus-RTU / RS485, 9600 bauds).

Elle ajoute aux mesures natives du module le calcul des puissances **apparente (S)** et **réactive (Q)**, ainsi que `readAddress()`, des constructeurs par référence et le support **ESP32** (broches RX/TX configurables).

Elle est dérivée de la bibliothèque [PZEM-004T v3.0](https://github.com/mandulaj/PZEM-004T-v30) de Jakub Mandula (licence MIT), synchronisée avec son master actuel.

![PZEM-004T v3.0](PZEM004T_Enhanced.jpg)

![PZEM-004T v3.0 100A (transformateur externe)](PZEM-004T-100A.png)

---

## Fonctionnalités

- Mesure de la **tension** (V)
- Mesure du **courant** (A)
- Mesure de la **puissance active P** (W) — *mesurée directement par le module*
- Calcul de la **puissance apparente S** (VA) — S = U·I
- Calcul de la **puissance réactive Q** (VAR) — Q = S·sin(φ) avec sinφ = √(1−PF²)
- Mesure du **facteur de puissance** (PF / cos φ)
- Mesure de la **fréquence** (Hz)
- Mesure de l'**énergie active** (kWh)
- Lecture de l'adresse réelle du module : `readAddress()`
- Gestion des adresses Modbus (`setAddress()`, `getAddress()`)
- Seuil d'alarme de puissance (`setPowerAlarm()`, `getPowerAlarm()`)
- Réinitialisation du compteur d'énergie (`resetEnergy()`)
- Compatible **HardwareSerial** et **SoftwareSerial** (AVR, ESP8266, ESP32)

## Compatibilité

| MCU                 | Hardware Serial | Software Serial |
|---------------------|:---------------:|:---------------:|
| ATmega328 (Uno/Nano) | ✔️ (Serial, sans console) | ✔️ |
| ATmega2560 (Mega)   | ✔️ (Serial1/2/3) | ✔️ |
| ESP8266             | ✔️ | ✔️ |
| ESP32               | ✔️ (pins configurables) | — |

## Branchement

| PZEM-004T | Arduino / ESP |
|-----------|---------------|
| VCC       | 5V            |
| GND       | GND           |
| RX        | TX (pin série)|
| TX        | RX (pin série)|
| CT (transformateur) | Fil à mesurer |

![Branchement PZEM-004T](PZEM004T_Hardware.webp)

> ⚠️ Le module fonctionne en **5V** et la communication série est en **3.3V / 5V** selon le modèle. Vérifiez la compatibilité de votre microcontrôleur.
> ⚠️ Le module doit être **branché sur le secteur (230 V AC)** pour communiquer : la broche 5V n'alimente que les optocoupleurs, pas le circuit de mesure.
> 💡 Si vous obtenez des `NaN` avec uniquement la LED TX qui clignote, **inversez les fils RX/TX**.

## Installation

### PlatformIO (recommandé)

Ajoutez la bibliothèque à votre `platformio.ini` (forme git complète) :

```ini
lib_deps =
    https://github.com/Fo170/PZEM004T_Enhanced.git
```

### Arduino IDE (Gestionnaire de bibliothèques / ZIP)

1. **ZIP** : téléchargez le dépôt (Code → Download ZIP) puis *Croquis → Inclure une bibliothèque → Ajouter la bibliothèque .ZIP*.
2. Ou copiez le dossier dans `Documents/Arduino/libraries/`.

### Fichier unique

1. Téléchargez le fichier [`PZEM004T_Enhanced.h`](PZEM004T_Enhanced.h)
2. Placez-le dans le dossier de votre projet Arduino
3. Incluez-le dans votre croquis :

```cpp
#include "PZEM004T_Enhanced.h"
```

## Exemples

Chaque exemple est un projet PlatformIO autonome (`.ino` compatible Arduino IDE + `platformio.ini`).

```bash
# Arduino Uno / ESP8266 (SoftwareSerial)
pio run -d examples/PZEM_SoftwareSerial

# Arduino Mega / ESP32 (HardwareSerial)
pio run -d examples/PZEM_HardwareSerial

# ESP32 (constructeur par broches)
pio run -d examples/PZEM_ESP32
```

- [`examples/PZEM_HardwareSerial/`](examples/PZEM_HardwareSerial/) — Arduino Mega (Serial1) et ESP32 (`&Serial1`)
- [`examples/PZEM_SoftwareSerial/`](examples/PZEM_SoftwareSerial/) — Arduino Uno / ESP8266 (SoftwareSerial)
- [`examples/PZEM_ESP32/`](examples/PZEM_ESP32/) — ESP32, constructeur par broches `(Serial1, rx, tx)`

## Utilisation

```cpp
#include "PZEM004T_Enhanced.h"

// HardwareSerial (Arduino Mega) :
PZEM004T_Enhanced pzem(Serial1);

// ESP32 (broches obligatoires) :
// PZEM004T_Enhanced pzem(Serial1, 16, 17);

// SoftwareSerial (Arduino Uno) :
// #include <SoftwareSerial.h>
// SoftwareSerial pzemSerial(10, 11);
// PZEM004T_Enhanced pzem(pzemSerial);

void setup() {
    Serial.begin(115200);
}

void loop() {
    float V  = pzem.voltage();        // V
    float I  = pzem.current();        // A
    float P  = pzem.power();          // W  (active)
    float S  = pzem.apparentPower();  // VA (apparente)
    float Q  = pzem.reactivePower();  // VAR (réactive)
    float PF = pzem.pf();             // facteur de puissance
    float F  = pzem.frequency();      // Hz
    float E  = pzem.energy();         // kWh

    if (isnan(V)) {
        Serial.println("Erreur de lecture (vérifiez RX/TX et l'alimentation)");
    } else {
        Serial.printf("V=%.1f  I=%.2f  P=%.1f W  S=%.1f VA  Q=%.1f VAR  PF=%.2f  F=%.1f Hz  E=%.3f kWh\n",
                      V, I, P, S, Q, PF, F, E);
    }
    delay(2000);
}
```

## API

| Méthode | Description |
|---------|-------------|
| `float voltage()` | Tension en V |
| `float current()` | Courant en A |
| `float power()` | Puissance active P en W |
| `float energy()` | Énergie active en kWh |
| `float frequency()` | Fréquence en Hz |
| `float pf()` | Facteur de puissance (cos φ) |
| `float apparentPower()` | Puissance apparente S = U·I en VA |
| `float reactivePower()` | Puissance réactive Q = S·sin(φ) en VAR |
| `float powerFactor()` | Alias de `pf()` |
| `bool setAddress(uint8_t addr)` | Change l'adresse Modbus (0x01..0xF7) |
| `uint8_t getAddress()` | Adresse Modbus interne |
| `uint8_t readAddress(bool update=false)` | Adresse réelle lue dans le registre du module (0 si échec) |
| `bool setPowerAlarm(uint16_t watts)` | Seuil d'alarme (max 25000 W) |
| `bool getPowerAlarm()` | Alarme déclenchée ? |
| `bool resetEnergy()` | Réinitialise le compteur d'énergie |
| `void search()` | Scanne les adresses du bus (débogage, ~25 s) |

### Constructeurs

- `PZEM004T_Enhanced()` — vide, pour tableaux / affectation ultérieure
- `PZEM004T_Enhanced(HardwareSerial& port [, addr])` — non ESP32
- `PZEM004T_Enhanced(HardwareSerial& port, rx, tx [, addr])` — **ESP32 uniquement**
- `PZEM004T_Enhanced(HardwareSerial* port [, addr])` — rétrocompatible (toutes cibles)
- `PZEM004T_Enhanced(uint8_t rx, uint8_t tx [, addr])` — SoftwareSerial par broches (AVR/ESP8266)
- `PZEM004T_Enhanced(SoftwareSerial& port [, addr])` — appelle `begin(9600)`
- `PZEM004T_Enhanced(Stream& port [, addr])` — sans appel à `begin()`

Les mesures sont mises en cache : une trame Modbus au plus toutes les 200 ms. En cas d'échec CRC/lecture, les getters renvoient `NAN`.

## Licence

Distribué sous **licence MIT**. Voir le fichier [LICENSE](LICENSE).

Basé sur la bibliothèque [PZEM-004T v3.0](https://github.com/mandulaj/PZEM-004T-v30) de Jakub Mandula (MIT). Auteur : Olivier FOURNET ([@Fo170](https://github.com/Fo170)).
