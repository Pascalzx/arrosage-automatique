# Guide d'Installation Facile - Système d'Arrosage Arduino

## 🚀 Installation en 5 minutes

### Étape 1: Matériel nécessaire
- **Arduino** (Uno, Nano ou Mega)
- **3 capteurs d'humidité** capacitifs
- **1 module relais** 5V
- **1 électrovanne** 12V
- **Alimentation 12V** (1-2A minimum)
- **Fils de connexion**

### Étape 2: Câblage simple

#### Capteurs d'humidité (×3):
```
Capteur 1: VCC→5V, GND→GND, AOUT→A0
Capteur 2: VCC→5V, GND→GND, AOUT→A1  
Capteur 3: VCC→5V, GND→GND, AOUT→A2
```

#### Module relais:
```
VCC → 5V Arduino
GND → GND Arduino
IN → Pin 7
COM → 12V+ (alimentation)
NO → Électrovanne
```

### Étape 3: Installation du code

1. **Télécharger Arduino IDE**: https://www.arduino.cc/en/software
2. **Ouvrir** le fichier `arduino_arrosage.ino`
3. **Connecter** votre Arduino via USB
4. **Sélectionner** votre carte: `Tools → Board → Arduino Uno`
5. **Sélectionner** le port: `Tools → Port → COMx`
6. **Cliquer** sur Upload (flèche →)

✅ **C'est tout!** Le système démarre automatiquement.

## 📊 Calibration rapide (optionnel)

Si vos capteurs donnent des valeurs étranges:

1. Ouvrir `arduino_arrosage.ino`
2. Décommenter la ligne: `// #define DEBUG_CALIB`
3. Téléverser et ouvrir le moniteur série (9600 baud)
4. Noter les valeurs:
   - Capteurs dans l'air = ~750
   - Capteurs dans l'eau = ~300
5. Mettre à jour les valeurs dans le code
6. Re-commenter `DEBUG_CALIB` et téléverser

## ⚙️ Configuration

### Paramètres modifiables dans le code:
```cpp
SEUIL_HUMIDITE = 30;        // Arrose si < 30%
DUREE_ARROSAGE_MS = 30000;  // 30 secondes
INTERVALLE_MESURE_MS = 1800000; // 30 minutes
```

### Heures d'arrosage:
- **Matin**: 6h à 10h
- **Soir**: 17h à 22h

## 🔧 Dépannage

| Problème | Solution |
|----------|----------|
| Capteurs toujours à 0% ou 100% | Faire la calibration |
| Relais ne clique pas | Vérifier alimentation 5V |
| Électrovanne ne s'ouvre pas | Vérifier alimentation 12V |
| Arrosage constant | Vérifier le câblage du relais |

## 📌 Schéma de câblage

```
Arduino Uno
    5V ─────┬─── VCC (Capteur 1)
            ├─── VCC (Capteur 2)
            ├─── VCC (Capteur 3)
            └─── VCC (Relais)
            
   GND ─────┬─── GND (Capteur 1)
            ├─── GND (Capteur 2)
            ├─── GND (Capteur 3)
            ├─── GND (Relais)
            └─── GND (Alim 12V)
            
    A0 ───────── AOUT (Capteur 1)
    A1 ───────── AOUT (Capteur 2)
    A2 ───────── AOUT (Capteur 3)
    
    D7 ───────── IN (Relais)

Alimentation 12V
   12V+ ──────── COM (Relais)
   
Relais
    NO ────────── Électrovanne (+)
    
Électrovanne
    (-) ───────── GND (Alim 12V)
```

## 💡 Conseils

1. **Placement des capteurs**: Espacer de 30-50cm dans le jardin
2. **Protection**: Mettre l'Arduino dans un boîtier étanche
3. **Câbles**: Utiliser des câbles résistants aux UV pour l'extérieur
4. **Test**: Faire un test manuel en baissant `SEUIL_HUMIDITE` à 80%

## 🎯 Fonctionnement

Le système:
- ✅ Mesure l'humidité toutes les 30 minutes
- ✅ Arrose si le sol est sec (<30%)
- ✅ Respecte les heures autorisées
- ✅ Arrose pendant 30 secondes
- ✅ Garde l'heure même après redémarrage

---

**Besoin d'aide?** Le code est commenté en français pour faciliter les modifications!