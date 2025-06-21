#include <Arduino.h>
#include <WiFi.h>

constexpr byte redPin = 27;
constexpr byte bluePin = 26;
constexpr byte greenPin = 25;

int redVal;
int greenVal;
int blueVal;

const char *ssid = "SSID";
const char *password = "PASSWORD";

WiFiServer server(80);

void setColour(int r, int g, int b);
void updateRgb();
void handleServer();

bool running = true;
bool rgb = true;
float hue = 0;
unsigned long prevTime = 0;
constexpr long timeout = 2000;
unsigned long lastFadeUpdate = 0;
int rgbSpeed = 11;

void setup() {
  Serial.begin(9600);
  Serial.println("Hello, world!");

  Serial.print("Connecting to: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected.");
  Serial.println("IP Address: ");
  Serial.println(WiFi.localIP());
  server.begin();

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
}

void loop() {
  handleServer();

  if (running) {
    if (rgb) {
      updateRgb();
    } else {
      setColour(redVal, greenVal, blueVal);
    }
  } else {
    hue = 0;
    setColour(0, 0, 0);
  }

  delay(1);
}

void parseCommands(String header) {
  if (header.indexOf("GET /on") >= 0) {
    Serial.println("LEDs on");
    running = true;
  } else if (header.indexOf("GET /off") >= 0) {
    Serial.println("LEDs off");
    running = false;
  } else if (header.indexOf("GET /rgb/on") >= 0) {
    Serial.println("RGB enabled");
    rgb = true;
  } else if (header.indexOf("GET /rgb/off") >= 0) {
    Serial.println("RGB disabled");
    rgb = false;
    hue = 0;
  } else if (header.indexOf("GET /speed") >= 0) {
    int s = header.indexOf("s=");
    if (s >= 0) {
      rgbSpeed = header.substring(s + 2).toInt();
      Serial.printf("Speed set: %d", rgbSpeed);
    }
  } else if (header.indexOf("GET /set") >= 0) {
    int r = header.indexOf("r=");
    int g = header.indexOf("g=");
    int b = header.indexOf("b=");
    if (r >= 0 && g >= 0 && b >= 0) {
      redVal = header.substring(r + 2, header.indexOf('&', r)).toInt();
      greenVal = header.substring(g + 2, header.indexOf('&', g)).toInt();
      blueVal = header.substring(b + 2).toInt();
      running = true;
      rgb = false;
      Serial.printf("Manual RGB set: R=%d G=%d B=%d\n", redVal, greenVal, blueVal);
    }
  }
}

void buildPage(WiFiClient &client, String header) {
  client.println("<!DOCTYPE html><html lang=\"en\"><head>");
  client.println("<meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">");
  client.println("<title>LED Controller</title><style>");
  client.println(".on{padding:2%;border:2px solid lightgreen;background-color:white;transition-duration:0.25s;}");
  client.println(".on:hover{background-color:lightgreen;}");
  client.println(".off{padding:2%;border:2px solid lightcoral;background-color:white;transition-duration:0.25s;}");
  client.println(".off:hover{background-color:lightcoral;}");
  client.println("*{font-size:large;font-family:sans-serif;}h1{font-size:larger;}</style></head><body>");
  client.println("<h1>Toggle LED</h1>");
  if (running) {
    client.println("<button class=\"off\" onclick=\"location.href='/off'\">Turn off LED</button>");
  } else {
    client.println("<button class=\"on\" onclick=\"location.href='/on'\">Turn on LED</button>");
  }

  client.println("<h1>Toggle RGB</h1>");
  client.printf("<input type=\"checkbox\" id=\"rgb\" onchange=\"location.href='/rgb/%s'\" %s>\n",
                rgb ? "off" : "on",
                rgb ? "checked" : "");

  if (rgb) {
    client.println("<h1>Edit speed</h1>");
    client.println("<form action=\"/speed\" method=\"get\">");
    client.printf("<input type=\"range\" name=\"s\" min=\"3\" max=\"30\" value=\"%d\">\n", rgbSpeed);
    client.println("<button type=\"submit\">Save</button></form>");
  } else {
    client.println("<h1>Edit colour</h1>");
    client.println("<form action=\"/set\" method=\"get\">");
    client.printf("<label for=\"red\">R</label><input type=\"range\" name=\"r\" min=\"0\" max=\"255\" value=\"%d\"><br>\n", redVal);
    client.printf("<label for=\"green\">G</label><input type=\"range\" name=\"g\" min=\"0\" max=\"255\" value=\"%d\"><br>\n", greenVal);
    client.printf("<label for=\"blue\">B</label><input type=\"range\" name=\"b\" min=\"0\" max=\"255\" value=\"%d\"><br>\n", blueVal);
    client.println("<button type=\"submit\">Save</button></form>");
  }

  client.println("</body></html>");
}

void handleServer() {
  WiFiClient client = server.available();
  if (client) {
    String currentLine = "";
    Serial.println("New client detected");
    prevTime = millis();
    String header;

    while (client.connected() && millis() - prevTime < timeout) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        header += c;

        if (c == '\n') {
          if (currentLine == "") {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");
            client.println();

            parseCommands(header);
            buildPage(client, header);

            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }

    header = "";
    client.stop();
    Serial.println("Client disconnected\n");
  }
}


void HSVtoRGB(float h, float s, float v, int &r, int &g, int &b) {
  float c = v * s;
  float x = c * (1 - fabs(fmod(h / 60.0, 2) - 1));
  float m = v - c;

  float r_, g_, b_;
  if (h < 60) {
    r_ = c; g_ = x; b_ = 0;
  } else if (h < 120) {
    r_ = x; g_ = c; b_ = 0;
  } else if (h < 180) {
    r_ = 0; g_ = c; b_ = x;
  } else if (h < 240) {
    r_ = 0; g_ = x; b_ = c;
  } else if (h < 300) {
    r_ = x; g_ = 0; b_ = c;
  } else {
    r_ = c; g_ = 0; b_ = x;
  }

  r = static_cast<int>((r_ + m) * 255);
  g = static_cast<int>((g_ + m) * 255);
  b = static_cast<int>((b_ + m) * 255);
}

void setColour(int r, int g, int b) {
  analogWrite(redPin, 255 - r);
  analogWrite(greenPin, 255 - g);
  analogWrite(bluePin, 255 - b);
}

void updateRgb() {
  const unsigned long now = millis();
  if (now - lastFadeUpdate < rgbSpeed) return;
  lastFadeUpdate = now;

  HSVtoRGB(hue, 1.0, 1.0, redVal, greenVal, blueVal);
  setColour(redVal, greenVal, blueVal);

  hue++;
  if (hue >= 360) hue = 0;
}
