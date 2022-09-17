#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <Wire.h>
#include <RTClib.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h> //Librería con Json

// Pines I2C
// SDA - D2
// SCL - D1

// Pines BUZZER - D8
// Cambiar los parametros de conexion mqtt y los del WIFI



// Prueba

//#include <string.h>

RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

// Notas
char notes[] = " zxzmzyzeecdefedcbcclbcdcblzllbcfeecdefedcddcbc#gfpdpdq#gfpfe";
// float beats[] = {0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5};
float beats[] = {0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 1.0, 1.0, 0.5, 0.5, 0.5, 0.5, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.5, 0.5, 0.5, 0.5, 1.0, 1.0, 1.0, 1.0, 1.0, 0.5, 0.5, 1.0, 1.0, 1.0, 0.5, 1.5, 0.5, 0.5, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.5,
                 0.5, 0.5, 1, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 1, 0.5, 0.5, 0.5, 2};
int tempo = 400;
int speakerPin = D8;

String convertIntToString(int number) {
  char text[5]; 
    sprintf(text, "%d", number);   

  String numberString = text;
  return numberString;
}

void playTone(int tone, int duration)
{
  for (long i = 0; i < duration * 1000L; i += tone * 2)
  {
    digitalWrite(speakerPin, HIGH);
    delayMicroseconds(tone);
    digitalWrite(speakerPin, LOW);
    delayMicroseconds(tone);
  }
}

// EJECUTAR NOTA
void playNote(char note, int duration)
{
  char names[] = {'z', 'x', 'y', 'm', 'c', 'd', 'e', 'f', 'g', 'a', 'b', 'l', '#', 'p', 'q'};
  float tones[] = {2550, 2864, 3400, 3038, 1915, 1700, 1519, 1432, 1275, 1136, 1975, 2272, 1190, 1609, 1061.75};
  for (int i = 0; i < sizeof(names); i++)
  {
    if (names[i] == note)
    {
      playTone(tones[i], duration);
    }
  }
}

// PIN DE BUZZER
//  Donde se guardarán los datos
// JsonObject memory[3]; 
JsonObject current;
JsonObject previous;

//----Datos del Wifi------
String _SSID = "Empresur S.A.S";
String _PASSWORD = "Esas9004945E";

//----Datos del Brocker-----
const char *mqtt_server = "node02.myqtthub.com";
const char *id = "ESP8266";
const char *user = "miguelcode";
const char *password = "miguelcode";

// Objeto Cliente WiFi
WiFiClient espClient;
// Objeto cliente mqtt
PubSubClient client(espClient);
// Datos del mensaje
String _topic;
String _payload;

// Configuración basica
// String predetermined = "\
//   {\
//     me:0,\
//     ma: {\
//         t: 2,\
//         a: 'lunes,martes,16/12/2002',\
//         s: 3,\
//         0: {\
//           h: 14,\
//           m: 49,\
//           d: 'Inicio de Jornada'\
//         },\
//         1: {\
//           h: 11,\
//           m: 10,\
//           d: 'Para la casa'\
//         }\  
//     },\
//     ta: none,\
//     no: none,\
//   }\
// ";

String predetermined = "\
  {\
    t: 2,\
    0: {\
      h: 14,\
      m: 49,\
      d: 'Inicio de Jornada'\
    },\
    1: {\
      h: 11,\
      m: 10,\
      d: 'Para la casa'\
    }\  
  }\
";

DynamicJsonDocument doc(1024); // Importante no se para que sirve
DynamicJsonDocument doc2(1024);
DynamicJsonDocument doc3(1024);
void wifiConnect()
{
  delay(10);
  /*if(!WiFi.config(ip_local,gateway, mask)){
    Serial.println("Error de configuración");
  }*/
  WiFi.begin(_SSID, _PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    lcd.print(".");
    delay(500);
    lcd.print(".");
    delay(500);
    lcd.print(".");
    delay(500);
  }
  Serial.println("WiFi connected");
  Serial.printf("IP: %s \n GATEWAY: %s \n MASK: %s", WiFi.localIP().toString().c_str(), WiFi.gatewayIP().toString().c_str(), WiFi.subnetMask().toString().c_str());
}

void reconnect()
{
  while (!client.connected())
  {
    if (client.connect(id, user, password))
    {
      Serial.println("Conectado");
      client.subscribe("horario");
    }
    else
    {
      Serial.print("Falla, EStado: ");
      Serial.println(client.state());
      Serial.println("Intentando en 5 segundos");
      delay(5000);
    }
  }
}

String findDayOfTheWeek(int day)
{
  switch (day)
  {
  case 1:
    return "Lunes";
    break;
  case 2:
    return "Martes";
    break;
  case 3:
    return "Miercoles";
    break;
  case 4:
    return "Jueves";
    break;
  case 5:
    return "Viernes";
    break;
  case 6:
    return "Sabado";
    break;
  case 7:
    return "Domingo";
    break;
  }
  return "";
}

// Pasar los datos del Json
JsonObject convertJSON(String data, DynamicJsonDocument doco)
{
  // DynamicJsonDocument doc(1024); // Importante no se para que sirve
  deserializeJson(doc, data);
  return doc.as<JsonObject>();
}

// -------------Obsoleto------------
boolean comprobarDia(char *days, String day)
{
  while (days != NULL)
  {
    String Days = days;
    if (Days == day)
    {
      return true;
    }
    days = strtok(NULL, ",");
  }
  return false;
}
// -------------------------------

void tocar(JsonObject toque)
{
  String descripcion = toque[F("d")];
  int hora = toque[F("h")];
  int minuto = toque[F("m")];
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(descripcion);
  lcd.setCursor(0, 1);
  lcd.print(hora);
  lcd.print(":");
  lcd.print(minuto);
  for (int i = 0; i < sizeof(notes); i++)
  {
    if (notes[i] == ' ')
    {
      delay(beats[i] * tempo);
    }
    else
    {
      playNote(notes[i], beats[i] * tempo);
    }
    delay(20);
  }
  delay(10);
}

void comprobarToques()
{
  int toques = current[F("t")];
  DateTime now = rtc.now();
  int Hora = now.hour();
  int minuto = now.minute();

  for (int i = 0; i < toques; i++)
  {
    char text[2];
    sprintf(text, "%d", i);
    String position = text;
    JsonObject toque = current[(position)];
    if (Hora == toque[F("h")] && minuto == toque[F("m")])
    {
      // lcd.clear();
      // lcd.print("Entró weeee");
      // delay(10000);
      tocar(toque);
    }
  }
}

// void prueba(){
//   // String data = current[F("ma")];
//   // lcd.clear();
//   // lcd.print(data);
//   // delay(5000);
//   JsonObject manana = current[F("ma")];
//   lcd.clear();
//   String manan = (current[F("ma")]);
//   lcd.print(manan);
//   delay(3000);
//   comprobarToques(manana);
// }

// ---------------------Obsoleto---------------------------------------
// void findCurrent()
// {
//   DateTime now = rtc.now();
//   String day = findDayOfTheWeek(now.dayOfTheWeek());
//   String dataDay = "";
//   char DataDay2[0];
//   char *days;
//   for (int i = 0; i < 3; i++)
//   {
//     JsonObject temp = memory[i];
//     if (temp != NULL)
//     {
//       if (temp[F("ma")])
//       {
//         JsonObject jornada = temp[F("ma")];
//         String JornadaString = temp[F("ma")];
//         String dataDay = jornada[F("a")];
//         DataDay2[dataDay.length()];
//         strcpy(DataDay2, dataDay.c_str());
//         days = strtok(DataDay2, ",");
//         if (comprobarDia(days, day))
//         {
//           lcd.clear();
//           lcd.print(jornada);
//           delay(1000);
//           current[F("ma")] = convertJSON(JornadaString);
//           lcd.clear();
//           lcd.print("Chao");
//           delay(1000);
//           // prueba();
//         }
//       }
//       if (temp[F("ta")])
//       {
//         JsonObject jornada = temp[F("ta")];
//         String JornadaString = temp[F("ta")];
//         String dataDay = jornada[F("a")];
//         DataDay2[dataDay.length()];
//         strcpy(DataDay2, dataDay.c_str());
//         days = strtok(DataDay2, ",");
//         if (comprobarDia(days, day))
//         {
//           lcd.clear();
//           lcd.print(jornada);
//           delay(1000);
//           current[F("ta")] = convertJSON(JornadaString);
//           lcd.clear();
//           lcd.print("Chao");
//           delay(1000);
//           // prueba();
//         }
//       }
//       if (temp[F("no")])
//       {
//         JsonObject jornada = temp[F("no")];
//         String JornadaString = temp[F("no")];
//         String dataDay = jornada[F("a")];
//         DataDay2[dataDay.length()];
//         strcpy(DataDay2, dataDay.c_str());
//         days = strtok(DataDay2, ",");
//         if (comprobarDia(days, day))
//         {
//           lcd.clear();
//           lcd.print(jornada);
//           delay(1000);
//           current[F("no")] = convertJSON(JornadaString);
//           lcd.clear();
//           lcd.print("Chao");
//           delay(1000);
//           // prueba();
//         }
//       }
//     }
//   }
// }
// -------------------------------------------------------------------

void publishMQTT(String message)
{
  client.publish("confirm", (char *)message.c_str());
}

int idx = 0;
int count = 0;
int tiempo = 0;
JsonObject data;
void callback(char* topic, byte* payload, unsigned int length)
{
  String conc_payload_;
  for (size_t i = 0; i < length; i++)
  { 
    conc_payload_.concat((char)payload[i]);
  }
  String _topic = topic;
  _payload = conc_payload_;


  deserializeJson(doc2, _payload);
  data = doc2.as<JsonObject>();
  // data = convertJSON(_payload, doc2);
  int iscount = data[F("t")];
  if(iscount) {
    if(iscount > 1) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Recibiendo datos");
      lcd.setCursor(0, 1);
      lcd.print("Espere...");

      deserializeJson(doc3, _payload);
      previous = doc3.as<JsonObject>();
      idx = 1;
      count = iscount;
      String env = convertIntToString(idx) + "/" + convertIntToString(count);
      publishMQTT(env);
    }else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("HORARIO ESTABLECIDO");
      current = data;
      String env = convertIntToString(idx) + "/" + convertIntToString(count);
      publishMQTT(env);
      delay(2000);
    }
  }else if(idx != 0 && idx<count) {
    char text[5]; 
    sprintf(text, "%d", idx);   

    String position = text;
    previous[(position)] =  data;

    idx++;
    tiempo = 0;
    String env = convertIntToString(idx) + "/" + convertIntToString(count);
    publishMQTT(env);
    if(idx == count) {
      current = previous;
      // publishMQTT(previous[F("t")]);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("HORARIO ESTABLECIDO");
      delay(5000);
      idx = 0;
    }
  }

  Serial.println(_topic);
  Serial.println(_payload);
}

// void callback(char* topic, byte* payload, unsigned long length) {
//   String conc_payload_;
//   for (int i = 0; i < length; i++)
//   {
//     conc_payload_+= (char)payload[i];
//   }
//   _topic = topic;
//   _payload = conc_payload_;

//   current = convertJSON(_payload);  //Dato estructurado

//   lcd.clear();
//   lcd.setCursor(0,0);
//   lcd.print("HORARIO ESTABLECIDO");

//   Serial.println(_topic);
//   Serial.println(_payload);
//   delay(2000);
// }


void setup()
{
  Serial.begin(115200);
  Serial.println("Hola:");

  lcd.begin(16, 2);
  // // Para el buzzer
  pinMode(D8, OUTPUT);
  // // Cargamos la configuración predeterminada
  current = convertJSON(predetermined, doc);
  lcd.setCursor(0, 0);
  lcd.print("-Iniciando-");
  wifiConnect();

  client.setServer(mqtt_server, 1883); // Establecer la conexión con el servidor MQTT
  client.setCallback(callback);
  if (!rtc.begin())
  {
    lcd.print("Couldn't find RTC");
    while (1)
      ;
  }

  if (rtc.lostPower())
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
  // Publicar el estado
  if(idx == 0) {
    delay(1);
    DateTime now = rtc.now();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Fecha: ");
    lcd.print(now.year());
    lcd.print("/");
    lcd.print(now.month());
    lcd.print("/");
    lcd.print(now.day());
    lcd.setCursor(0, 1);
    lcd.print("Hora: ");
    lcd.print(now.hour());
    lcd.print(":");
    lcd.print(now.minute());
    lcd.print(":");
    lcd.print(now.second());

    comprobarToques();
  }else {
    tiempo+=1000;
    if(tiempo == 5000) {
      String env = convertIntToString(idx) + "/" + convertIntToString(count);
      publishMQTT(env);
    }
    if(tiempo >= 10000) {
      tiempo = 0;
      idx = 0;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Tiempo de espera");
      lcd.setCursor(0, 1);
      lcd.print("Agotado");
      delay(5000);
    }
  }
  delay(1000);
}