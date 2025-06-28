# 🚀 Guide d'Installation Rapide

## 📋 Prérequis

- ESP32 DevKit (n'importe quel modèle)
- 3 capteurs d'humidité capacitifs
- 1 module relais
- 1 électrovanne 12V
- Alimentation 12V

## 🔧 Installation - 3 Méthodes

### Méthode 1 : PlatformIO (Recommandée) ⭐

1. **Installer VS Code** + extension PlatformIO
2. **Cloner le projet** :
   ```bash
   git clone https://github.com/Pascalzx/arrosage-automatique.git
   cd arrosage-automatique
   ```
3. **Ouvrir dans PlatformIO**
4. **Modifier les paramètres** dans `main.cpp` :
   ```cpp
   const char* WIFI_SSID = "VotreWiFi";
   const char* WIFI_PASSWORD = "VotreMotDePasse";
   ```
5. **Compiler et téléverser** : 
   - Cliquer sur ✓ (Build)
   - Cliquer sur → (Upload)

### Méthode 2 : Arduino IDE

1. **Installer Arduino IDE** (v2.0+)
2. **Ajouter l'ESP32** :
   - Fichier → Préférences
   - URLs additionnelles : `https://dl.espressif.com/dl/package_esp32_index.json`
   - Outils → Type de carte → Gestionnaire → Installer "esp32"
3. **Installer ArduinoJson** :
   - Outils → Gérer les bibliothèques
   - Rechercher "ArduinoJson" by Benoit Blanchon
   - Installer la dernière version
4. **Configurer** :
   - Carte : "ESP32 Dev Module"
   - Port : Sélectionner votre port COM/USB
5. **Copier le code** de `main.cpp`
6. **Modifier les paramètres WiFi**
7. **Téléverser** (→)

### Méthode 3 : En Ligne de Commande

```bash
# Cloner le projet
git clone https://github.com/Pascalzx/arrosage-automatique.git
cd arrosage-automatique

# Installer PlatformIO CLI
pip install platformio

# Configurer WiFi dans main.cpp
nano main.cpp  # ou votre éditeur préféré

# Compiler et téléverser
pio run -t upload

# Moniteur série
pio device monitor
```

## ⚡ Configuration Express

1. **WiFi** (obligatoire) :
   ```cpp
   const char* WIFI_SSID = "VotreWiFi";
   const char* WIFI_PASSWORD = "MotDePasse";
   ```

2. **Station météo** (optionnel) :
   ```cpp
   const char* STATION_METEO = "qc-155";  // Sherbrooke par défaut
   // qc-147 = Montréal
   // qc-133 = Québec
   ```

## 🔌 Câblage Rapide

```
ESP32          Composant
------         ----------
GPIO34    →    Capteur 1 (AOUT)
GPIO35    →    Capteur 2 (AOUT)
GPIO32    →    Capteur 3 (AOUT)
GPIO25    →    Relais (IN)
3.3V      →    Capteurs (VCC)
5V        →    Relais (VCC)
GND       →    Tout (GND)

Relais    →    Électrovanne
NO        →    Fil électrovanne
COM       →    12V+
```

## ✅ Test Rapide

1. Ouvrir le moniteur série (115200 baud)
2. Vérifier :
   - ✓ WiFi connecté
   - ✓ Heure synchronisée
   - ✓ Lectures des capteurs
3. Tester l'arrosage forcé :
   - Débrancher temporairement un capteur
   - L'humidité devrait chuter
   - L'arrosage se déclenche si < 30%

## 🔧 Calibration (Important!)

1. Décommenter dans le code :
   ```cpp
   #define DEBUG_CALIB
   ```
2. Téléverser et ouvrir le moniteur série
3. Noter les valeurs :
   - Capteurs dans l'air : ~3000
   - Capteurs dans l'eau : ~1200
4. Mettre à jour si nécessaire :
   ```cpp
   const int ADC_SEC = 3000;
   const int ADC_HUMIDE = 1200;
   ```
5. Recommenter `DEBUG_CALIB` et téléverser

## 🆘 Aide Rapide

| Problème | Solution |
|----------|----------|
| WiFi échoue | Vérifier SSID/mot de passe |
| Pas de lecture capteurs | Vérifier câblage 3.3V |
| Arrosage ne démarre pas | Vérifier l'heure (pas 10h-17h) |
| Erreur compilation | Installer ArduinoJson |

## 📱 Prochaines Étapes

- Voir `TECHNICAL.md` pour optimisations
- Voir `README.md` pour utilisation complète
- Rejoindre les discussions sur GitHub

**Temps total d'installation : ~15 minutes** ⏱️