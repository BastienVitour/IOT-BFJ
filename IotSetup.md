# ESP32 File Monitor

Un système de surveillance de fichiers en temps réel utilisant un ESP32 et un écran LCD RGB pour afficher les notifications d'activité sur les fichiers d'un **Raspberry Pi** servant de serveur de stockage.

## Fonctionnalités

- **Connexion WiFi** automatique
- **Connexion TCP persistante** vers un Raspberry Pi
- **Surveillance en temps réel** des fichiers sur le Raspberry Pi
- **Affichage déporté** des activités de fichiers
- **Écran LCD RGB** avec rétroéclairage adaptatif
- **Support de multiples actions** : création, suppression, modification, ouverture, fermeture
- **Interface utilisateur** intuitive avec mode d'attente


## 🛠️ Matériel utilisé

| Composant | Quantité | Description |
|-----------|----------|-------------|
| **Raspberry Pi** | 1 | Serveur de stockage (avec script de monitoring) |
| **ESP32** | 1 | Carte de développement ESP32 (client d'affichage) |
| **Écran LCD RGB** | 1 | Écran 16x2 avec rétroéclairage RGB (Grove compatible) |
| **Câbles Dupont** | 4 | Pour les connexions ESP32 ↔ LCD |
| **Réseau WiFi** | 1 | Réseau local commun Raspberry Pi ↔ ESP32 |

## 🏗️ Architecture du système

```
┌─────────────────┐    WiFi/TCP     ┌─────────────────┐
│  Raspberry Pi   │◄──────────────► │     ESP32       │
│   (Serveur)     │    Port 8000    │   (Client)      │
│                 │                 │                 │
│ • Stockage      │                 │ • Affichage LCD │
│ • Monitoring    │                 │ • Notifications │
│ • Script Python │                 │ • Interface     │
└─────────────────┘                 └─────────────────┘
        │                                    │
        ▼                                    ▼
┌─────────────────┐                 ┌─────────────────┐
│   Fichiers      │                 │   LCD RGB 16x2  │
│   surveillés    │                 │                 │
└─────────────────┘                 └─────────────────┘
```


## 🔌 Schéma de branchement

### Connexions ESP32 ↔ LCD RGB

| ESP32 Pin | LCD Pin | Câble | Description |
|-----------|---------|-------|-------------|
| 5V | VCC | Rouge | Alimentation |
| GND | GND | Noir | Masse |
| SDA (D21) | SDA | Blanc | Communication I2C |
| SCL (D22) | SCL | Jaune | Horloge I2C |

### Schéma visuel

```
     ESP32                    LCD RGB 16x2
    ┌─────┐                  ┌─────────────┐
    │ 5V  ├──────────────────┤ VCC (Rouge) │
    │ GND ├──────────────────┤ GND (Noir)  │
    │ D21 ├──────────────────┤ SDA (Blanc) │
    │ D22 ├──────────────────┤ SCL (Jaune) │
    └─────┘                  └─────────────┘
```

## ⚙️ Configuration

### 1. Configuration du Raspberry Pi (Serveur)

Le Raspberry Pi doit exécuter un **script de monitoring** qui :
- Surveille les fichiers d'un dossier spécifique
- Détecte les actions (création, suppression, modification)
- Envoie les notifications via TCP sur le port 8000
- Formate les messages selon le protocole défini

**Format de sortie requis :**
```
ACTION|UTILISATEUR|ADRESSE_IP|NOM_FICHIER\n
```

### 2. Configuration ESP32 (Client)

Modifiez ces variables dans le code :

```cpp
const char* ssid = "VotreReseauWiFi";        // Nom de votre réseau WiFi
const char* password = "VotreMotDePasse";     // Mot de passe WiFi
const char* host = "192.168.1.100";          // IP du Raspberry Pi
const uint16_t port = 8000;                  // Port d'écoute du Raspberry Pi
```


### 3. Réseau local

- **Raspberry Pi** et **ESP32** doivent être sur le **même réseau WiFi**
- Notez l'**adresse IP du Raspberry Pi** : `hostname -I` ou `ifconfig`
- Configurez le **port 8000** sur le Raspberry Pi (ou modifiez dans le code)

### Actions supportées

| Code | Action | Affichage |
|------|--------|-----------|
| `C` ou `CREATE_FILE` | Création | "X a cree" |
| `D` ou `UNLINKAT` | Suppression | "X a supprime" |

### Exemple

```
C|bastien|192.168.1.50|document.txt
```

Affichera :
```
bastien a cree
document.txt
```


## 🚀 Installation

### 1. Arduino IDE

1. Installez l'Arduino IDE
2. Ajoutez le support ESP32 :
   - Fichier → Préférences
   - URLs supplémentaires : `https://dl.espressif.com/dl/package_esp32_index.json`
   - Outils → Type de carte → Gestionnaire de cartes
   - Recherchez "esp32 by Espressif Systems" et installez

### 2. Bibliothèques

Installez la bibliothèque rgb_lcd :
- Croquis → Inclure une bibliothèque → Gérer les bibliothèques
- Recherchez "Grove - LCD RGB Backlight by Seeed Studio" et installez

### 3. Upload du code

1. Connectez l'ESP32 via USB
2. Sélectionnez le bon port et la carte ESP32
3. Téléversez le code

## 🖥️ Interface utilisateur

### États de l'écran

| État | Couleur | Description |
|------|---------|-------------|
| **Connexion WiFi** | Bleu | Tentative de connexion |
| **WiFi connecté** | Vert | Connexion WiFi réussie |
| **Connexion serveur** | Jaune | Connexion au serveur |
| **En attente** | Faible (10,10,10) | Mode veille |
| **Nouveau message** | Blanc | Affichage notification (5s) |
| **Erreur** | Rouge | Problème de connexion |

### Cycle d'affichage

1. **Démarrage** : Connexion WiFi → Connexion serveur
2. **Mode veille** : "En attente..." avec éclairage faible
3. **Notification** : Message pendant 5 secondes en blanc
4. **Retour veille** : Retour au mode d'attente

## 🐛 Débogage

### Problèmes courants

| Problème | Solution |
|----------|----------|
| **"WiFi echec"** | Vérifiez SSID/mot de passe WiFi |
| **"Serveur echec"** | Vérifiez que le Raspberry Pi est allumé et script démarré |
| **Pas de messages** | Vérifiez que le script Raspberry Pi envoie bien les données |
| **"Connexion perdue"** | Vérifiez la stabilité du réseau WiFi |
| **Affichage corrompu** | Vérifiez les connexions I2C ESP32 ↔ LCD |
| **Mauvaise IP** | Utilisez `hostname -I` sur le Raspberry Pi |

### Tests de connectivité

**Depuis un PC sur le même réseau :**
```bash
# Tester la connexion au Raspberry Pi
ping 192.168.1.100

# Tester le port 8000
telnet 192.168.1.100 8000
```

**Sur le Raspberry Pi :**
```bash
# Vérifier que le script écoute bien
sudo netstat -tulpn | grep :8000

## 📋 Spécifications techniques

- Alimentation : 5V via USB ou alimentation externe
- Consommation : ~80mA (WiFi actif)
- Portée WiFi : Selon antenne ESP32 (jusqu'à 50m)
- Protocole : TCP socket
- Affichage : 16 caractères × 2 lignes
- Temps d'affichage : 5 secondes par notification

## 🔧 Personnalisation

### Modifier les couleurs

```cpp
lcd.setRGB(255, 0, 0);    // Rouge
lcd.setRGB(0, 255, 0);    // Vert  
lcd.setRGB(0, 0, 255);    // Bleu
lcd.setRGB(255, 255, 255); // Blanc
```

### Changer la durée d'affichage

```cpp
delay(5000); // Changer 5000 pour modifier la durée (ms)
```

### Ajouter de nouvelles actions

```cpp
else if (action == "VOTRE_ACTION") {
    line1 = user + " votre texte";
}
```


## 👥 Contributeurs

- **Bastien** - Développement/Configuration Raspberry PI
- **Fabien** - Développement/Configuration écran LCD
- **Julien** - Développement/Configuration écran LCD
