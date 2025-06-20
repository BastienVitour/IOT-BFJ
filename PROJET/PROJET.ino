#include <WiFi.h>
#include "rgb_lcd.h"

const char* ssid = "";
const char* password = "";
const uint16_t port = 8000;
const char* host = "192.168.43.16";

rgb_lcd lcd;
WiFiClient client;
String displayedData = "";

// Fonction pour parser et formater le message
String formatMessage(String rawMessage) {
  rawMessage.trim();
  
  // Diviser le message par '|'
  int firstPipe = rawMessage.indexOf('|');
  int secondPipe = rawMessage.indexOf('|', firstPipe + 1);
  int thirdPipe = rawMessage.indexOf('|', secondPipe + 1);
  
  if (firstPipe == -1 || secondPipe == -1 || thirdPipe == -1) {
    return "Message invalide";
  }
  
  String action = rawMessage.substring(0, firstPipe);
  String user = rawMessage.substring(firstPipe + 1, secondPipe);
  String ip = rawMessage.substring(secondPipe + 1, thirdPipe);
  String filename = rawMessage.substring(thirdPipe + 1);
  
  // Créer le message formaté selon l'action
  String line1 = "";
  String line2 = filename;
  
  if (action == "C" || action == "CREATE_FILE") {
    line1 = user + " a cree";
  }
  else if (action == "D" || action == "UNLINKAT") {
    line1 = user + " a supprime";
  }
  else if (action == "M" || action == "MODIFY") {
    line1 = user + " a modifie";
  }
  else if (action == "OPEN") {
    line1 = user + " a ouvert";
  }
  else if (action == "CLOSE") {
    line1 = user + " a ferme";
  }
  else {
    line1 = user + ": " + action;
  }
  
  // Tronquer si nécessaire (16 caractères max par ligne)
  if (line1.length() > 16) {
    line1 = line1.substring(0, 16);
  }
  if (line2.length() > 16) {
    line2 = line2.substring(0, 16);
  }
  
  return line1 + "|" + line2;  // Utiliser | comme séparateur interne
}

void displayFormattedMessage(String formattedMessage) {
  int separator = formattedMessage.indexOf('|');
  
  String line1 = "";
  String line2 = "";
  
  if (separator != -1) {
    line1 = formattedMessage.substring(0, separator);
    line2 = formattedMessage.substring(separator + 1);
  } else {
    line1 = formattedMessage.substring(0, 16);
    line2 = formattedMessage.substring(16, 32);
  }
  
  // Clear et configurer l'affichage
  lcd.clear();
  delay(50);  // Petite pause après clear
  lcd.setRGB(255, 255, 255);
  
  // Nettoyer manuellement la ligne 1 puis afficher
  lcd.setCursor(0, 0);
  lcd.print("                "); // 16 espaces pour effacer complètement
  lcd.setCursor(0, 0);
  lcd.print(line1);
  
  // Nettoyer manuellement la ligne 2 puis afficher
  lcd.setCursor(0, 1);
  lcd.print("                "); // 16 espaces pour effacer complètement
  lcd.setCursor(0, 1);
  lcd.print(line2);
  
  Serial.println("LCD Line 1: " + line1);
  Serial.println("LCD Line 2: " + line2);
}

void setup() {
  Serial.begin(115200);
  lcd.begin(16, 2);
  lcd.setRGB(0, 0, 255);
  lcd.print("Connexion WiFi...");
  
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print("...");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    lcd.clear();
    lcd.setRGB(0, 255, 0);
    lcd.print("Connecte !");
    Serial.println("WiFi connected!");
    Serial.println("IP: " + WiFi.localIP().toString());
    delay(1000);
    
    // Connexion unique au serveur
    lcd.clear();
    lcd.setRGB(255, 255, 0);
    lcd.print("Connexion serveur");
    
    if (client.connect(host, port)) {
      lcd.clear();
      lcd.setRGB(0, 255, 0);
      lcd.print("Serveur connecte");
      Serial.println("Connected to server!");
    } else {
      lcd.clear();
      lcd.setRGB(255, 0, 0);
      lcd.print("Serveur echec");
      Serial.println("Server connection failed!");
    }
    delay(1000);
    
    // Affichage d'attente
    lcd.clear();
    delay(50);
    lcd.setRGB(10, 10, 10);
    
    // Nettoyer manuellement les deux lignes
    lcd.setCursor(0, 0);
    lcd.print("                "); // 16 espaces ligne 1
    lcd.setCursor(0, 1);
    lcd.print("                "); // 16 espaces ligne 2
    
    // Afficher le message d'attente
    lcd.setCursor(0, 0);
    lcd.print("En attente...");
    
  } else {
    lcd.clear();
    lcd.setRGB(255, 0, 0);
    lcd.print("WiFi echec");
  }
}

void loop() {
  // Vérifier si la connexion est toujours active
  if (!client.connected()) {
    lcd.clear();
    lcd.setRGB(255, 0, 0);
    lcd.print("Connexion perdue");
    Serial.println("Connection lost, reconnecting...");
    
    // Tentative de reconnexion
    if (client.connect(host, port)) {
      lcd.clear();
      lcd.setRGB(0, 255, 0);
      lcd.print("Reconnecte");
      Serial.println("Reconnected!");
      delay(1000);
      
      // Retour à l'affichage d'attente - bien nettoyer d'abord
      lcd.clear();
      delay(100); // Petite pause pour s'assurer que le clear est effectif
      lcd.setRGB(10, 10, 10);
      
      // Nettoyer manuellement les deux lignes
      lcd.setCursor(0, 0);
      lcd.print("                "); // 16 espaces ligne 1
      lcd.setCursor(0, 1);
      lcd.print("                "); // 16 espaces ligne 2
      
      // Afficher le message d'attente
      lcd.setCursor(0, 0);
      lcd.print("En attente...");
      
    } else {
      Serial.println("Reconnection failed");
      delay(5000);
      return;
    }
  }
  
  // Lire les données disponibles
  if (client.available()) {
    String data = client.readStringUntil('\n');
    data.trim();
    data.replace("\"", ""); // Enlever les guillemets
    
    Serial.println("Message brut recu: " + data);
    
    // Afficher seulement si c'est un nouveau message ET qu'il n'est pas vide
    if (data != displayedData && data.length() > 0) {
      displayedData = data;
      
      // Parser et formater le message
      String formattedMessage = formatMessage(data);
      Serial.println("Message formate: " + formattedMessage);
      
      // Afficher le message formaté
      displayFormattedMessage(formattedMessage);
      
      delay(5000); // Afficher pendant 5 secondes
      
      // Retour à l'affichage d'attente
      lcd.clear();
      delay(100); // Petite pause pour s'assurer que le clear est effectif
      lcd.setRGB(10, 10, 10);
      
      // Nettoyer manuellement les deux lignes
      lcd.setCursor(0, 0);
      lcd.print("                "); // 16 espaces ligne 1
      lcd.setCursor(0, 1);
      lcd.print("                "); // 16 espaces ligne 2
      
      // Afficher le message d'attente
      lcd.setCursor(0, 0);
      lcd.print("En attente...");
    }
  }
  
  delay(100); // Petite pause pour éviter de surcharger
}
