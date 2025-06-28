# 📝 Changelog

Tous les changements notables de ce projet seront documentés dans ce fichier.

Le format est basé sur [Keep a Changelog](https://keepachangelog.com/fr/1.0.0/),
et ce projet adhère au [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.1.0] - 2025-01-28

### ✨ Ajouté
- Documentation technique complète (TECHNICAL.md)
- Fichier .gitignore pour projets Arduino/ESP32
- Configuration PlatformIO pour installation simplifiée
- Support de multiples environnements ESP32
- Instructions détaillées d'optimisation
- Guide de développement avancé

### 📚 Documentation
- Analyse des performances (RAM, Flash, CPU)
- Diagramme d'architecture du code
- Guide pour ajouter de nouveaux capteurs
- Benchmarks de consommation électrique
- Roadmap des fonctionnalités futures

## [1.0.1] - 2025-01-28

### 🔧 Corrigé
- Utilisation dynamique de la variable STATION_METEO
- Mapping automatique des codes de station vers les IDs XML

### 📝 Modifié
- Ajout de commentaires pour les codes de station disponibles
- Documentation mise à jour avec les codes de station

## [1.0.0] - 2025-01-28

### ✨ Version Initiale
- Système d'arrosage automatique complet pour ESP32
- Support de 3 capteurs d'humidité capacitifs
- Intégration API météo Environnement Canada (gratuite)
- Gestion des restrictions horaires (10h-17h)
- Mode calibration intégré
- Fonctionnement autonome sans WiFi si nécessaire
- Documentation complète en français

### 🌟 Fonctionnalités
- Mesure d'humidité moyenne sur 3 capteurs
- Vérification météo toutes les 30 minutes
- Arrosage de 30 secondes si conditions remplies
- Protection contre la pluie (actuelle et prévue)
- Configuration simple (3 paramètres)
- Commentaires ligne par ligne

### 🔒 Sécurité
- Mode fail-safe (assume pluie si erreur)
- Timeouts pour éviter les blocages
- Validation des valeurs des capteurs
- Gestion robuste des erreurs réseau