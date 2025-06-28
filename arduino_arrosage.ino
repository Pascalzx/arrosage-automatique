// arduino_arrosage.ino
// Système d'arrosage automatique pour Arduino Uno/Mega
// Version simplifiée sans WiFi - utilise des heures fixes
// Compatible avec Arduino Uno, Nano, Mega, etc.

#include <EEPROM.h>

// ========== CONFIGURATION ==========
// Seuils et paramètres
const int SEUIL_HUMIDITE = 30;                    // Seuil d'humidité en % (arrosage si < 30%)
const int DUREE_ARROSAGE_MS = 30000;              // Durée d'arrosage en millisecondes (30s)
const unsigned long INTERVALLE_MESURE_MS = 1800000; // Intervalle entre mesures (30 min)

// Broches matérielles (adaptées pour Arduino Uno)
const int BROCHE_CAPTEUR_1 = A0;                  // Broche analogique pour capteur 1
const int BROCHE_CAPTEUR_2 = A1;                  // Broche analogique pour capteur 2
const int BROCHE_CAPTEUR_3 = A2;                  // Broche analogique pour capteur 3
const int BROCHE_RELAIS = 7;                      // Broche digitale pour relais

// Calibration des capteurs (valeurs ADC 10 bits pour Arduino)
const int ADC_SEC = 750;                          // Valeur ADC capteur sec (ajuster selon vos capteurs)
const int ADC_HUMIDE = 300;                       // Valeur ADC capteur humide

// Heures d'arrosage autorisées (évite 10h-17h)
const int HEURE_ARROSAGE_MATIN = 6;              // Arrosage possible à partir de 6h
const int HEURE_ARROSAGE_MATIN_FIN = 10;         // Jusqu'à 10h
const int HEURE_ARROSAGE_SOIR = 17;              // Et à partir de 17h
const int HEURE_ARROSAGE_SOIR_FIN = 22;          // Jusqu'à 22h

// Mode debug
#define DEBUG_MODE                                 // Commenter pour désactiver
// #define DEBUG_CALIB                             // Décommenter pour calibration

// Variables globales
unsigned long derniereMesure = 0;
bool arrosageEnCours = false;
unsigned long debutArrosage = 0;
unsigned long heureActuelle = 0;                  // Simulation d'horloge simple

// ========== FONCTIONS ==========

// Simulation simple de l'heure (incrémente toutes les minutes)
void updateTime() {
    static unsigned long dernierUpdate = 0;
    unsigned long maintenant = millis();
    
    if (maintenant - dernierUpdate >= 60000) {    // Toutes les minutes
        heureActuelle++;
        dernierUpdate = maintenant;
        
        // Sauvegarde l'heure en EEPROM toutes les heures
        if (heureActuelle % 60 == 0) {
            EEPROM.put(0, heureActuelle);
        }
    }
}

// Obtenir l'heure courante (0-23)
int getHeure() {
    return (heureActuelle / 60) % 24;
}

// Vérifier si c'est une heure autorisée pour l'arrosage
bool isHeureAutorisee() {
    int heure = getHeure();
    return (heure >= HEURE_ARROSAGE_MATIN && heure < HEURE_ARROSAGE_MATIN_FIN) ||
           (heure >= HEURE_ARROSAGE_SOIR && heure < HEURE_ARROSAGE_SOIR_FIN);
}

// Lire l'humidité moyenne des capteurs
int readSoil() {
    int raw1 = analogRead(BROCHE_CAPTEUR_1);
    int raw2 = analogRead(BROCHE_CAPTEUR_2);
    int raw3 = analogRead(BROCHE_CAPTEUR_3);
    
    // Conversion en pourcentage
    int humidite1 = map(raw1, ADC_SEC, ADC_HUMIDE, 0, 100);
    int humidite2 = map(raw2, ADC_SEC, ADC_HUMIDE, 0, 100);
    int humidite3 = map(raw3, ADC_SEC, ADC_HUMIDE, 0, 100);
    
    // Limiter entre 0 et 100
    humidite1 = constrain(humidite1, 0, 100);
    humidite2 = constrain(humidite2, 0, 100);
    humidite3 = constrain(humidite3, 0, 100);
    
    int moyenne = (humidite1 + humidite2 + humidite3) / 3;
    
    #ifdef DEBUG_MODE
    Serial.print(F("Humidité - C1:"));
    Serial.print(humidite1);
    Serial.print(F("%("));
    Serial.print(raw1);
    Serial.print(F(") C2:"));
    Serial.print(humidite2);
    Serial.print(F("%("));
    Serial.print(raw2);
    Serial.print(F(") C3:"));
    Serial.print(humidite3);
    Serial.print(F("%("));
    Serial.print(raw3);
    Serial.print(F(") Moy:"));
    Serial.print(moyenne);
    Serial.println(F("%"));
    #endif
    
    return moyenne;
}

// Mode calibration
void calibrationMode() {
    #ifdef DEBUG_CALIB
    Serial.println(F("=== MODE CALIBRATION ==="));
    Serial.println(F("Valeurs ADC brutes (0-1023 pour Arduino):"));
    Serial.println(F("1. Capteurs dans l'air sec -> noter les valeurs"));
    Serial.println(F("2. Capteurs dans l'eau -> noter les valeurs"));
    Serial.println(F("3. Mettre à jour ADC_SEC et ADC_HUMIDE"));
    
    while (true) {
        int raw1 = analogRead(BROCHE_CAPTEUR_1);
        int raw2 = analogRead(BROCHE_CAPTEUR_2);
        int raw3 = analogRead(BROCHE_CAPTEUR_3);
        
        Serial.print(F("RAW - C1:"));
        Serial.print(raw1);
        Serial.print(F(" C2:"));
        Serial.print(raw2);
        Serial.print(F(" C3:"));
        Serial.println(raw3);
        
        delay(2000);
    }
    #endif
}

// ========== SETUP ==========
void setup() {
    Serial.begin(9600);
    
    Serial.println(F("=== Système Arrosage Arduino ==="));
    Serial.println(F("Version simplifiée"));
    Serial.println(F("Démarrage..."));
    
    // Configuration des broches
    pinMode(BROCHE_RELAIS, OUTPUT);
    digitalWrite(BROCHE_RELAIS, LOW);
    
    // Récupérer l'heure sauvegardée
    EEPROM.get(0, heureActuelle);
    if (heureActuelle == 0xFFFFFFFF) {  // EEPROM vide
        heureActuelle = 8 * 60;          // Démarre à 8h par défaut
    }
    
    Serial.print(F("Heure actuelle: "));
    Serial.print(getHeure());
    Serial.println(F("h"));
    
    // Mode calibration si activé
    calibrationMode();
    
    Serial.println(F("Système prêt!"));
    Serial.println(F("===================="));
}

// ========== LOOP ==========
void loop() {
    unsigned long maintenant = millis();
    
    // Mise à jour de l'heure
    updateTime();
    
    // Arrêt de l'arrosage après la durée définie
    if (arrosageEnCours && (maintenant - debutArrosage >= DUREE_ARROSAGE_MS)) {
        digitalWrite(BROCHE_RELAIS, LOW);
        arrosageEnCours = false;
        Serial.println(F("Arrosage terminé"));
        derniereMesure = maintenant;
    }
    
    // Vérification périodique
    if (!arrosageEnCours && (maintenant - derniereMesure >= INTERVALLE_MESURE_MS || derniereMesure == 0)) {
        Serial.println(F("\n--- Vérification ---"));
        Serial.print(F("Heure: "));
        Serial.print(getHeure());
        Serial.println(F("h"));
        
        int humidite = readSoil();
        bool heureOK = isHeureAutorisee();
        
        Serial.print(F("Conditions - Humidité:"));
        Serial.print(humidite);
        Serial.print(F("% Heure OK:"));
        Serial.println(heureOK ? F("OUI") : F("NON"));
        
        // Décision d'arrosage
        if (humidite < SEUIL_HUMIDITE && heureOK) {
            Serial.println(F("ARROSAGE ACTIVÉ!"));
            digitalWrite(BROCHE_RELAIS, HIGH);
            arrosageEnCours = true;
            debutArrosage = maintenant;
        } else {
            Serial.println(F("Pas d'arrosage"));
            if (humidite >= SEUIL_HUMIDITE) {
                Serial.println(F("  -> Sol humide"));
            }
            if (!heureOK) {
                Serial.println(F("  -> Heure interdite"));
            }
        }
        
        derniereMesure = maintenant;
    }
    
    delay(1000);
}

/*
================================================================================
GUIDE D'INSTALLATION RAPIDE - ARDUINO
================================================================================

MATÉRIEL REQUIS:
- Arduino Uno/Nano/Mega
- 3x Capteurs d'humidité capacitifs
- 1x Module relais 5V
- 1x Électrovanne 12V
- Alimentation 12V pour l'électrovanne
- Fils de connexion

CÂBLAGE:
1. Capteurs d'humidité:
   - VCC → 5V Arduino
   - GND → GND Arduino
   - AOUT Capteur 1 → A0
   - AOUT Capteur 2 → A1
   - AOUT Capteur 3 → A2

2. Module relais:
   - VCC → 5V Arduino
   - GND → GND Arduino
   - IN → Pin 7
   - COM → 12V+
   - NO → Électrovanne

3. Alimentation:
   - 12V+ → COM du relais
   - GND → GND Arduino (masse commune)

INSTALLATION:
1. Télécharger Arduino IDE
2. Ouvrir ce fichier dans Arduino IDE
3. Sélectionner votre carte (Tools → Board)
4. Sélectionner le port COM (Tools → Port)
5. Cliquer sur Upload

CALIBRATION:
1. Décommenter "#define DEBUG_CALIB" ligne 24
2. Téléverser et ouvrir le moniteur série (9600 baud)
3. Noter les valeurs avec capteurs secs
4. Noter les valeurs avec capteurs dans l'eau
5. Mettre à jour ADC_SEC et ADC_HUMIDE
6. Re-commenter DEBUG_CALIB et téléverser

UTILISATION:
- Le système vérifie l'humidité toutes les 30 min
- Arrose si sol sec ET heure autorisée (6h-10h ou 17h-22h)
- Durée d'arrosage: 30 secondes
- L'heure est maintenue même après redémarrage

================================================================================
*/