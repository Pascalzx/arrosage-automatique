# 🌱 Système d'Arrosage Automatique Intelligent pour ESP32

Un système d'arrosage automatique pour jardin utilisant un ESP32, des capteurs d'humidité capacitifs et l'API météo d'Environnement Canada (gratuite, sans clé API). Conçu spécifiquement pour les conditions météorologiques de Sherbrooke, Québec.

## 📋 Fonctionnalités

- **Mesure d'humidité du sol** avec 3 capteurs capacitifs pour une lecture fiable
- **Intégration météo** via Environnement Canada pour éviter l'arrosage en cas de pluie
- **Respect des restrictions horaires** (pas d'arrosage entre 10h et 17h)
- **Mode calibration** intégré pour ajuster les capteurs
- **Fonctionnement autonome** avec vérification toutes les 30 minutes
- **Gestion intelligente** de l'arrosage basée sur plusieurs critères

## 🔧 Matériel Requis

- **1x ESP32** (module WiFi intégré)
- **3x Capteurs d'humidité capacitifs KeeYees** (sortie analogique 0-3V)
- **1x Module relais 1 canal** (compatible 3.3V ou 5V)
- **1x Électrovanne 12V**
- **1x Alimentation 12V** pour l'électrovanne
- Câbles de connexion
- Boîtier étanche (recommandé pour installation extérieure)

## 📐 Schéma de Câblage

### Capteurs d'humidité
| Capteur | VCC | GND | AOUT |
|---------|-----|-----|------|
| Capteur 1 | 3.3V | GND | GPIO34 |
| Capteur 2 | 3.3V | GND | GPIO35 |
| Capteur 3 | 3.3V | GND | GPIO32 |

### Module Relais
| Broche Relais | Connexion |
|---------------|-----------|
| VCC | 5V (ou 3.3V selon module) |
| GND | GND |
| IN | GPIO25 |
| NO | Électrovanne |
| COM | Alimentation 12V+ |

### Électrovanne 12V
- Fil 1 → Alimentation 12V+
- Fil 2 → NO du relais
- GND alimentation → GND commun avec ESP32

## 🚀 Installation

### 1. Préparation de l'environnement

Installez [PlatformIO](https://platformio.org/) ou [Arduino IDE](https://www.arduino.cc/en/software) avec le support ESP32.

### 2. Installation des bibliothèques

Via le gestionnaire de bibliothèques, installez :
- **ArduinoJson** by Benoit Blanchon
- Les bibliothèques ESP32 standard sont incluses

### 3. Configuration du code

Ouvrez `main.cpp` et modifiez les paramètres dans la section configuration :

```cpp
const char* WIFI_SSID = "VotreSSID";              // Votre réseau WiFi
const char* WIFI_PASSWORD = "VotreMotDePasse";    // Mot de passe WiFi
const char* STATION_METEO = "qc-155";             // Code de station météo
```

### 4. Configurer la station météo

1. Le code par défaut utilise la station de Sherbrooke (qc-155)
2. Pour une autre ville, trouvez votre code de station sur [Environnement Canada](https://weather.gc.ca/)
3. Codes populaires au Québec :
   - Sherbrooke : qc-155
   - Montréal : qc-147
   - Québec : qc-133
   - Gatineau : qc-126
   - Trois-Rivières : qc-130

### 5. Téléversement

Connectez votre ESP32 et téléversez le code.

## 🔬 Calibration des Capteurs

La calibration est essentielle pour des mesures précises :

1. **Décommentez** la ligne `#define DEBUG_CALIB` dans le code
2. **Téléversez** et ouvrez le moniteur série (115200 baud)
3. **Placez les capteurs dans l'air sec**
   - Notez les valeurs ADC (environ 3000)
4. **Plongez les capteurs dans l'eau**
   - Notez les valeurs ADC (environ 1200)
5. **Mettez à jour** les constantes dans le code :
   ```cpp
   const int ADC_SEC = 3000;      // Votre valeur à sec
   const int ADC_HUMIDE = 1200;   // Votre valeur dans l'eau
   ```
6. **Recommentez** `#define DEBUG_CALIB` et téléversez

## 📊 Règles de Fonctionnement

Le système arrose **uniquement** si toutes ces conditions sont remplies :

- ✅ Humidité moyenne du sol < 30%
- ✅ Heure actuelle hors de 10h-17h
- ✅ Pas de pluie actuelle
- ✅ Probabilité de pluie < 50% dans les 3 prochaines heures

**Cycle d'arrosage :**
- Durée : 30 secondes
- Vérification : toutes les 30 minutes
- Nouvelle mesure après chaque arrosage

## 🐛 Dépannage

| Problème | Solution |
|----------|----------|
| Capteurs toujours à 0% ou 100% | Recalibrez les capteurs (voir section Calibration) |
| Pas de connexion WiFi | Vérifiez SSID/mot de passe et la portée du signal |
| Erreur API météo | Vérifiez la connexion internet et le code de station |
| Électrovanne ne s'ouvre pas | Vérifiez l'alimentation 12V et les connexions du relais |
| Pas d'affichage série | Vérifiez la vitesse du port série (115200 baud) |

## 📝 Personnalisation

Vous pouvez ajuster ces paramètres selon vos besoins :

```cpp
const int SEUIL_HUMIDITE = 30;                    // Seuil d'arrosage (%)
const int SEUIL_PROBABILITE_PLUIE = 50;           // Seuil pluie (%)
const int DUREE_ARROSAGE_MS = 30000;              // Durée arrosage (ms)
const unsigned long INTERVALLE_MESURE_MS = 1800000; // Intervalle (ms)
```

## 🌍 Adaptation Géographique

Pour utiliser dans une autre région :

1. Modifiez le code de station météo :
   ```cpp
   const char* STATION_METEO = "qc-155";  // Votre code de station
   ```
   Trouvez votre code sur [Environnement Canada](https://weather.gc.ca/)

2. Ajustez le fuseau horaire si nécessaire :
   ```cpp
   const char* TZ_INFO = "EST5EDT,M3.2.0,M11.1.0";
   ```
   [Liste des fuseaux horaires](https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv)

## 📸 Photos du Projet

*Ajoutez ici des photos de votre installation*

## 🤝 Contribution

Les contributions sont les bienvenues ! N'hésitez pas à :
- Signaler des bugs
- Proposer des améliorations
- Partager vos adaptations

## 📄 Licence

Ce projet est sous licence MIT - voir le fichier [LICENSE](LICENSE) pour plus de détails.

## 🙏 Remerciements

- Communauté ESP32
- Environnement Canada pour l'API météo gratuite et sans restriction
- Bibliothèque ArduinoJson de Benoit Blanchon

---

**Note :** Ce système est conçu pour un usage personnel. Assurez-vous de respecter les réglementations locales concernant l'utilisation de l'eau et l'arrosage automatique.