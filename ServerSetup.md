# Mise en place d’un serveur NAS avec Samba sur Raspberry Pi 3 (64 bits)

## 🎯 Objectif

Mettre en place un serveur NAS léger avec surveillance en temps réel des fichiers créés et supprimés via Samba, sur un Raspberry Pi 3 équipé de Raspberry Pi OS 64 bits.

---

## 🧱 Étapes de configuration

### 1. Réinstallation de l’OS

- Installation de **Raspberry Pi OS 64 bits**.
- Configuration réseau et système de base.

### 2. Installation et configuration de Samba

- Installation du service **Samba**.
- Création et configuration du dossier partagé (`shared`) dans la configuration de Samba.
- Attribution des droits nécessaires pour l’accès réseau.

### 3. Partitionnement et montage du dossier partagé

- Aucun stockage externe n’étant utilisé, une **partition interne de 7 Mo** a été créée et montée pour accueillir le dossier partagé.

### 4. Activation du module `full_audit` de Samba

- Configuration du module **full_audit** dans Samba.
- Suivi des événements :
  - `create_file` pour la création de fichiers.
  - `unlinkat` pour la suppression de fichiers.

### 5. Traitement des logs avec un script Python

- Création d’un **script Python** pour lire les logs Samba via `journalctl`.
- Extraction et traitement des événements d’ajout et de suppression de fichiers.

### 6. Communication en temps réel avec un projet externe

- Mise en place d’un **serveur socket** en Python.
- Le script envoie en **temps réel** les notifications Samba à l'ESP32.