// main.cpp
// Système d'arrosage automatique pour jardin - Sherbrooke, QC
// ESP32 + 3 capteurs d'humidité capacitifs + relais/électrovanne

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

// ========== CONFIGURATION - TODO: REMPLIR CES VALEURS ==========
const char* WIFI_SSID = "TODO_VOTRE_SSID";                    // TODO: Remplacer par votre SSID WiFi
const char* WIFI_PASSWORD = "TODO_VOTRE_PASSWORD";            // TODO: Remplacer par votre mot de passe WiFi
const char* STATION_METEO = "qc-155";                         // Code de station météo (par défaut: Sherbrooke)
                                                               // Codes disponibles:
                                                               // qc-155 = Sherbrooke
                                                               // qc-147 = Montréal
                                                               // qc-133 = Québec
                                                               // qc-126 = Gatineau
                                                               // qc-130 = Trois-Rivières

// Seuils et paramètres
const int SEUIL_HUMIDITE = 30;                                // Seuil d'humidité en % (arrosage si < 30%)
const int SEUIL_PROBABILITE_PLUIE = 50;                       // Seuil de probabilité de pluie en %
const int DUREE_ARROSAGE_MS = 30000;                          // Durée d'arrosage en millisecondes (30s)
const unsigned long INTERVALLE_MESURE_MS = 1800000;           // Intervalle entre mesures (30 min = 1800000 ms)

// Broches matérielles
const int BROCHE_CAPTEUR_1 = 34;                              // Broche ADC pour capteur d'humidité 1
const int BROCHE_CAPTEUR_2 = 35;                              // Broche ADC pour capteur d'humidité 2
const int BROCHE_CAPTEUR_3 = 32;                              // Broche ADC pour capteur d'humidité 3
const int BROCHE_RELAIS = 25;                                 // Broche pour contrôler le relais

// Calibration des capteurs (valeurs ADC)
const int ADC_SEC = 3000;                                      // Valeur ADC quand le capteur est sec (dans l'air)
const int ADC_HUMIDE = 1200;                                  // Valeur ADC quand le capteur est dans l'eau

// Configuration du fuseau horaire
const char* TZ_INFO = "EST5EDT,M3.2.0,M11.1.0";              // Fuseau horaire Est (Québec)

// Mode debug pour calibration (décommenter pour activer)
// #define DEBUG_CALIB

// Variables globales
unsigned long derniereMesure = 0;                              // Timestamp de la dernière mesure
bool arrosageEnCours = false;                                  // État de l'arrosage
unsigned long debutArrosage = 0;                               // Timestamp du début d'arrosage

// ========== FONCTIONS UTILITAIRES ==========

// Fonction pour vérifier si on est dans les heures interdites (10h-17h)
bool isForbiddenHour() {
    struct tm timeinfo;                                        // Structure pour stocker l'heure locale
    
    if (!getLocalTime(&timeinfo)) {                          // Récupère l'heure locale
        Serial.println("Erreur: impossible d'obtenir l'heure");
        return true;                                          // Par sécurité, on interdit l'arrosage si pas d'heure
    }
    
    int heure = timeinfo.tm_hour;                            // Extrait l'heure actuelle
    
    // Retourne vrai si entre 10h et 16h59 (17h non inclus)
    return (heure >= 10 && heure < 17);
}

// Fonction pour lire l'humidité moyenne des 3 capteurs
int readSoil() {
    // Lecture des valeurs ADC brutes des 3 capteurs
    int raw1 = analogRead(BROCHE_CAPTEUR_1);                 // Lit le capteur 1
    int raw2 = analogRead(BROCHE_CAPTEUR_2);                 // Lit le capteur 2
    int raw3 = analogRead(BROCHE_CAPTEUR_3);                 // Lit le capteur 3
    
    // Conversion des valeurs ADC en pourcentage d'humidité
    // map(valeur, min_entrée, max_entrée, min_sortie, max_sortie)
    // ADC_SEC (3000) = 0% humidité, ADC_HUMIDE (1200) = 100% humidité
    int humidite1 = map(raw1, ADC_SEC, ADC_HUMIDE, 0, 100);  // Convertit capteur 1 en %
    int humidite2 = map(raw2, ADC_SEC, ADC_HUMIDE, 0, 100);  // Convertit capteur 2 en %
    int humidite3 = map(raw3, ADC_SEC, ADC_HUMIDE, 0, 100);  // Convertit capteur 3 en %
    
    // Limite les valeurs entre 0 et 100%
    humidite1 = constrain(humidite1, 0, 100);                // Borne capteur 1 entre 0-100
    humidite2 = constrain(humidite2, 0, 100);                // Borne capteur 2 entre 0-100
    humidite3 = constrain(humidite3, 0, 100);                // Borne capteur 3 entre 0-100
    
    // Calcul de la moyenne
    int moyenneHumidite = (humidite1 + humidite2 + humidite3) / 3;
    
    // Affichage des valeurs pour debug
    Serial.print("Humidité Sol - Capteur 1: ");
    Serial.print(humidite1);
    Serial.print("% (ADC: ");
    Serial.print(raw1);
    Serial.print("), Capteur 2: ");
    Serial.print(humidite2);
    Serial.print("% (ADC: ");
    Serial.print(raw2);
    Serial.print("), Capteur 3: ");
    Serial.print(humidite3);
    Serial.print("% (ADC: ");
    Serial.print(raw3);
    Serial.print("), Moyenne: ");
    Serial.print(moyenneHumidite);
    Serial.println("%");
    
    return moyenneHumidite;                                   // Retourne la moyenne en %
}

// Fonction pour vérifier s'il va pleuvoir dans les 3 prochaines heures
bool isRainingSoon() {
    HTTPClient http;                                          // Crée un client HTTP
    
    // Construction de l'URL pour l'API d'Environnement Canada (RSS)
    // Mapping des codes de station vers les identifiants XML
    String stationId = "s0000598";  // Par défaut Sherbrooke
    if (String(STATION_METEO) == "qc-155") stationId = "s0000598";  // Sherbrooke
    else if (String(STATION_METEO) == "qc-147") stationId = "s0000635";  // Montréal
    else if (String(STATION_METEO) == "qc-133") stationId = "s0000620";  // Québec
    else if (String(STATION_METEO) == "qc-126") stationId = "s0000623";  // Gatineau
    else if (String(STATION_METEO) == "qc-130") stationId = "s0000616";  // Trois-Rivières
    
    String url = "https://dd.weather.gc.ca/citypage_weather/xml/QC/" + stationId + "_f.xml";
    
    http.begin(url);                                         // Initialise la connexion HTTP
    int httpCode = http.GET();                              // Effectue la requête GET
    
    if (httpCode != 200) {                                  // Vérifie si la requête a réussi
        Serial.print("Erreur API météo, code: ");
        Serial.println(httpCode);
        http.end();                                          // Ferme la connexion
        return true;                                         // Par sécurité, on suppose qu'il va pleuvoir
    }
    
    // Récupère la réponse XML
    String payload = http.getString();                       // Récupère la réponse
    http.end();                                             // Ferme la connexion HTTP
    
    // Recherche des conditions actuelles
    int conditionStart = payload.indexOf("<currentConditions>");
    int conditionEnd = payload.indexOf("</currentConditions>");
    
    if (conditionStart != -1 && conditionEnd != -1) {
        String currentConditions = payload.substring(conditionStart, conditionEnd);
        
        // Vérifie s'il pleut actuellement
        if (currentConditions.indexOf("pluie") != -1 || 
            currentConditions.indexOf("Pluie") != -1 ||
            currentConditions.indexOf("averse") != -1 ||
            currentConditions.indexOf("Averse") != -1) {
            Serial.println("Il pleut actuellement");
            return true;                                    // Retourne vrai car il pleut
        }
    }
    
    // Recherche des prévisions horaires
    int forecastStart = payload.indexOf("<hourlyForecastGroup>");
    int forecastEnd = payload.indexOf("</hourlyForecastGroup>");
    
    if (forecastStart != -1 && forecastEnd != -1) {
        String forecast = payload.substring(forecastStart, forecastEnd);
        
        // Compte les prévisions horaires analysées
        int heuresAnalysees = 0;
        int index = 0;
        
        // Parcourt les 3 premières prévisions horaires
        while (heuresAnalysees < 3) {
            int hourStart = forecast.indexOf("<hourlyForecast", index);
            if (hourStart == -1) break;
            
            int hourEnd = forecast.indexOf("</hourlyForecast>", hourStart);
            if (hourEnd == -1) break;
            
            String hourForecast = forecast.substring(hourStart, hourEnd);
            
            // Extrait la probabilité de précipitations
            int popStart = hourForecast.indexOf("<lop category=");
            if (popStart != -1) {
                int popValueStart = hourForecast.indexOf(">", popStart) + 1;
                int popValueEnd = hourForecast.indexOf("</lop>", popValueStart);
                
                if (popValueStart != -1 && popValueEnd != -1) {
                    String popStr = hourForecast.substring(popValueStart, popValueEnd);
                    int pop = popStr.toInt();
                    
                    Serial.print("Probabilité pluie H+");
                    Serial.print(heuresAnalysees + 1);
                    Serial.print(": ");
                    Serial.print(pop);
                    Serial.println("%");
                    
                    if (pop >= SEUIL_PROBABILITE_PLUIE) {
                        Serial.println("Forte probabilité de pluie prévue");
                        return true;                        // Retourne vrai car pluie probable
                    }
                }
            }
            
            // Vérifie aussi les conditions prévues
            int condStart = hourForecast.indexOf("<condition>");
            int condEnd = hourForecast.indexOf("</condition>");
            
            if (condStart != -1 && condEnd != -1) {
                String condition = hourForecast.substring(condStart + 11, condEnd);
                if (condition.indexOf("pluie") != -1 || 
                    condition.indexOf("Pluie") != -1 ||
                    condition.indexOf("averse") != -1 ||
                    condition.indexOf("Averse") != -1) {
                    Serial.println("Pluie prévue dans les prochaines heures");
                    return true;                            // Retourne vrai car pluie prévue
                }
            }
            
            index = hourEnd + 1;
            heuresAnalysees++;
        }
    }
    
    Serial.println("Pas de pluie prévue");
    return false;                                           // Retourne faux si pas de pluie
}

// Fonction optionnelle pour la calibration des capteurs
void setupCalibrationSerial() {
    #ifdef DEBUG_CALIB
    Serial.println("=== MODE CALIBRATION ===");
    Serial.println("Affichage des valeurs ADC brutes toutes les 2 secondes");
    Serial.println("1. Placez les capteurs dans l'air sec -> notez les valeurs (environ 3000)");
    Serial.println("2. Placez les capteurs dans l'eau -> notez les valeurs (environ 1200)");
    Serial.println("3. Mettez à jour ADC_SEC et ADC_HUMIDE dans le code");
    Serial.println("========================");
    
    while (true) {                                          // Boucle infinie pour calibration
        int raw1 = analogRead(BROCHE_CAPTEUR_1);          // Lit capteur 1
        int raw2 = analogRead(BROCHE_CAPTEUR_2);          // Lit capteur 2
        int raw3 = analogRead(BROCHE_CAPTEUR_3);          // Lit capteur 3
        
        Serial.print("RAW - Capteur 1: ");
        Serial.print(raw1);
        Serial.print(", Capteur 2: ");
        Serial.print(raw2);
        Serial.print(", Capteur 3: ");
        Serial.println(raw3);
        
        delay(2000);                                       // Attend 2 secondes
    }
    #endif
}

// ========== SETUP ==========
void setup() {
    Serial.begin(115200);                                  // Initialise la communication série
    delay(1000);                                          // Petit délai pour stabilisation
    
    Serial.println("=== Système d'arrosage automatique ===");
    Serial.println("Démarrage...");
    
    // Configuration des broches
    pinMode(BROCHE_RELAIS, OUTPUT);                       // Configure la broche relais en sortie
    digitalWrite(BROCHE_RELAIS, LOW);                     // S'assure que le relais est éteint
    
    // Configuration des broches ADC (pas strictement nécessaire mais bonne pratique)
    pinMode(BROCHE_CAPTEUR_1, INPUT);                     // Configure capteur 1 en entrée
    pinMode(BROCHE_CAPTEUR_2, INPUT);                     // Configure capteur 2 en entrée
    pinMode(BROCHE_CAPTEUR_3, INPUT);                     // Configure capteur 3 en entrée
    
    // Mode calibration si activé
    setupCalibrationSerial();                             // Appel fonction calibration (ne fait rien si DEBUG_CALIB non défini)
    
    // Connexion WiFi
    Serial.print("Connexion WiFi à ");
    Serial.print(WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);                // Démarre la connexion WiFi
    
    int tentatives = 0;                                   // Compteur de tentatives
    while (WiFi.status() != WL_CONNECTED && tentatives < 30) {  // Essaie pendant 30 secondes max
        delay(1000);                                      // Attend 1 seconde
        Serial.print(".");
        tentatives++;                                     // Incrémente le compteur
    }
    
    if (WiFi.status() != WL_CONNECTED) {                 // Si connexion échouée
        Serial.println("\nErreur: Impossible de se connecter au WiFi");
        Serial.println("Le système fonctionnera sans météo");
    } else {                                              // Si connexion réussie
        Serial.println("\nWiFi connecté!");
        Serial.print("Adresse IP: ");
        Serial.println(WiFi.localIP());
        
        // Configuration de l'heure
        configTime(0, 0, "pool.ntp.org");                // Configure NTP
        setenv("TZ", TZ_INFO, 1);                        // Configure le fuseau horaire
        tzset();                                         // Applique le fuseau horaire
        
        Serial.println("Synchronisation de l'heure...");
        delay(2000);                                     // Attend la synchronisation
        
        struct tm timeinfo;
        if (getLocalTime(&timeinfo)) {                  // Vérifie si l'heure est disponible
            Serial.print("Heure locale: ");
            Serial.println(&timeinfo, "%Y-%m-%d %H:%M:%S");
        }
    }
    
    Serial.println("Système prêt!");
    Serial.println("================================");
}

// ========== LOOP ==========
void loop() {
    unsigned long maintenant = millis();                  // Récupère le temps actuel en ms
    
    // Gestion de l'arrêt de l'arrosage après la durée définie
    if (arrosageEnCours && (maintenant - debutArrosage >= DUREE_ARROSAGE_MS)) {
        digitalWrite(BROCHE_RELAIS, LOW);                 // Éteint le relais
        arrosageEnCours = false;                          // Met à jour l'état
        Serial.println("Arrosage terminé");
        derniereMesure = maintenant;                      // Reset le timer pour attendre 30 min
    }
    
    // Vérification toutes les 30 minutes (ou au démarrage)
    if (!arrosageEnCours && (maintenant - derniereMesure >= INTERVALLE_MESURE_MS || derniereMesure == 0)) {
        Serial.println("\n--- Nouvelle vérification ---");
        
        // Lecture de l'humidité du sol
        int humidite = readSoil();                        // Lit l'humidité moyenne
        
        // Vérification des conditions d'arrosage
        bool heureInterdite = isForbiddenHour();          // Vérifie l'heure
        bool pluiePrevue = false;
        
        // Vérifie la météo seulement si WiFi connecté
        if (WiFi.status() == WL_CONNECTED) {              // Si connecté au WiFi
            pluiePrevue = isRainingSoon();                // Vérifie la météo
        } else {
            Serial.println("WiFi non connecté - pas de vérification météo");
        }
        
        // Affichage du statut
        Serial.print("Conditions - Humidité: ");
        Serial.print(humidite);
        Serial.print("%, Heure interdite: ");
        Serial.print(heureInterdite ? "OUI" : "NON");
        Serial.print(", Pluie prévue: ");
        Serial.println(pluiePrevue ? "OUI" : "NON");
        
        // Décision d'arrosage
        if (humidite < SEUIL_HUMIDITE && !heureInterdite && !pluiePrevue) {
            Serial.println("ARROSAGE DÉCLENCHÉ!");
            digitalWrite(BROCHE_RELAIS, HIGH);             // Active le relais
            arrosageEnCours = true;                        // Met à jour l'état
            debutArrosage = maintenant;                    // Enregistre le début
        } else {
            Serial.println("Arrosage non nécessaire");
            if (humidite >= SEUIL_HUMIDITE) {
                Serial.println("  -> Sol suffisamment humide");
            }
            if (heureInterdite) {
                Serial.println("  -> Heure interdite (10h-17h)");
            }
            if (pluiePrevue) {
                Serial.println("  -> Pluie prévue/en cours");
            }
        }
        
        derniereMesure = maintenant;                       // Met à jour le timestamp
    }
    
    delay(1000);                                          // Petit délai pour éviter de surcharger le CPU
}

/*
================================================================================
README - SYSTÈME D'ARROSAGE AUTOMATIQUE ESP32
================================================================================

CÂBLAGE DES COMPOSANTS:
-----------------------
1. Capteurs d'humidité capacitifs KeeYees:
   - Capteur 1: VCC -> 3.3V, GND -> GND, AOUT -> GPIO34
   - Capteur 2: VCC -> 3.3V, GND -> GND, AOUT -> GPIO35
   - Capteur 3: VCC -> 3.3V, GND -> GND, AOUT -> GPIO32

2. Module relais:
   - VCC -> 5V (ou 3.3V selon votre module)
   - GND -> GND
   - IN -> GPIO25
   - NO (Normally Open) -> Électrovanne
   - COM -> Alimentation 12V+

3. Électrovanne 12V:
   - Fil 1 -> Alimentation 12V+
   - Fil 2 -> NO du relais
   - GND alimentation -> GND commun avec ESP32

CALIBRATION DES CAPTEURS:
-------------------------
1. Décommentez "#define DEBUG_CALIB" (ligne ~42)
2. Téléversez le code et ouvrez le moniteur série (115200 baud)
3. Placez tous les capteurs dans l'air sec
   -> Notez les valeurs ADC affichées (environ 3000)
4. Plongez tous les capteurs dans un verre d'eau
   -> Notez les valeurs ADC affichées (environ 1200)
5. Mettez à jour les constantes ADC_SEC et ADC_HUMIDE avec vos valeurs
6. Recommentez "#define DEBUG_CALIB" et téléversez à nouveau

CONFIGURATION REQUISE:
---------------------
1. Remplissez les TODO dans la section configuration:
   - WIFI_SSID: Nom de votre réseau WiFi
   - WIFI_PASSWORD: Mot de passe WiFi
   - STATION_METEO: Code de station météo (par défaut: qc-155 pour Sherbrooke)

2. Bibliothèques nécessaires (via Library Manager):
   - ArduinoJson (by Benoit Blanchon)
   - Bibliothèques ESP32 standard (WiFi, HTTPClient)

3. Codes de stations météo du Québec:
   - Sherbrooke: qc-155 (s0000598)
   - Montréal: qc-147 (s0000635)
   - Québec: qc-133 (s0000620)
   - Trouvez votre code sur: https://weather.gc.ca/

DÉPANNAGE:
----------
- Si les capteurs donnent 0% ou 100% tout le temps: recalibrez (voir ci-dessus)
- Si pas de connexion WiFi: vérifiez SSID/password et la portée du signal
- Si erreur API météo: vérifiez la connexion internet et le code de station
- Si l'électrovanne ne s'ouvre pas: vérifiez l'alimentation 12V et le câblage

================================================================================
*/