#include <SoftwareSerial.h>

#include "music.h"

#define TIMEOUT 15000 // ms

#define LED_R 8
#define LED_G 9
#define LED_B 10

#define UNLOCKED_CHECK_PERIOD 4000 // ms
#define LOCKED_CHECK_PERIOD 4000 // ms
#define ALARM_UPDATE_PERIOD 3000 // ms

typedef enum {
  UNLOCKED,
  LOCKED,
  ALARM,
} state_t;

volatile state_t state;
volatile unsigned long nextCheck;
volatile bool firstAlarm;

// A software only way to reset the board
void (*resetBoard) (void) = 0;

// `bsd` implementation of the strlen method
// We have this to avoid importing the whole string.h library
size_t strlen(const char *str) {
  const char *s;

  for (s = str; *s; ++s);

  return (s - str);
}

// A method that formats logs in such a way that they will never be interpreted by the ESP
// and automatically ignores the ERROR it returns
void log(const char* message, int length) {
  delay(50);

  if (length == 0) {
    Serial.print("log:#\r\n");
  } else {
    bool newLine = true;
    for (int i = 0; i < length; i++) {
      if (newLine) {
        Serial.print("log:");
        newLine = false;
      }

      char c = message[i];
      Serial.write(c);

      if (c == '\r' || c == '\n') {
        if (c == '\r') i++;
        Serial.print("#\r\n");
        newLine = true;
      }
    }

    if (!newLine) {
      Serial.print("#\r\n");
    }
  }

  delay(500);
  emptySerial();
}

void log(const char* message) {
  log(message, strlen(message));
}

void log(String message) {
  log(message.c_str(), message.length());
}

// Read everything from the Serial
// Useful when searching for specific characters there
// (such as when interpreting JSON)
void emptySerial() {
  while(Serial.available() > 0)
    Serial.read();
}

const int Serial_timeout = 100; // ms

// Serial::read with timeout
int Serial_read_to() {
  long deadline = millis() + Serial_timeout;

  while (millis() < deadline) {
    if (Serial.available()) {
      return Serial.read();
    }
  }

  return -1;
}

// Serial::peek with timeout
int Serial_peek_to() {
  long deadline = millis() + Serial_timeout;

  while (millis() < deadline) {
    if (Serial.available()) {
      return Serial.peek();
    }
  }

  return -1;
}

// Find the given keyword within the given time
// Returns whether the kw was found
bool echoFind(String keyword, long timeout){
  byte current_char = 0;
  byte keyword_length = keyword.length();
  
  long deadline = millis() + timeout;

  while(millis() < deadline){
    if (Serial.available()){
      char ch = Serial.read();

      if (ch == keyword[current_char]) {
        if (++current_char == keyword_length) {
          return true;
        }
      }
    }
  }

  return false; // Timed out
}

void configureNetwork(bool full) {
  while (1) {
    if (full) {
      log("Initializing WiFi");
      delay(1000);

      Serial.println();
      emptySerial();
      
      Serial.print("AT+RST\r\n");
      if (!echoFind("OK", 1000)) continue;
      
      delay(3000);

      Serial.print("AT\r\n");
      if (!echoFind("OK", 1000)) continue;

      Serial.print("AT+CWMODE=1\r\n");
      if (!echoFind("OK", 1000)) continue;

      delay(100);
    }
    Serial.print("AT+CWJAP=\"marcelo-acer\",\"abcdefgh\"\r\n");
    if (!echoFind("OK", 15000)) continue;

    if (full) {
      Serial.print("AT+CIFSR\r\n");
      while(Serial.available() == 0) {};

      delay(10);

      Serial.print("AT+CIPMUX=0\r\n");
      if (!echoFind("OK", 1000)) continue;
    }

    break;
  }
}

void setup() {
  Serial.begin(115200);

  while (!Serial) {
    delay(10); // wait for serial port to connect
  }

  configureNetwork(true);

  // If we're here, connection must be OK
  pinMode(BUZZER, OUTPUT);

  // Play network connected sound
  play1UpSound();

  state = UNLOCKED;

  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  // Attach interrupt for motion detection
  pinMode(2, INPUT);
  attachInterrupt(digitalPinToInterrupt(2), startAlarm, RISING);
}

// Given a key, looks for said key surrounded by " and followed by :
bool Serial_findJSONKey(const char *key) {
  bool found = false;
  bool next = false;

  int length = strlen(key);
  
  while (!found && (next || Serial.find('"'))) {
    next = false;
    found = true;
    for (int i = 0; i < length; i++) {
      if (key[i] != Serial_read_to()) {
        found = false;
        break;
      }
    }

    if (found) {
      if (Serial_read_to() == '"') {
        next = true;
        if (Serial_peek_to() == ':')
          Serial.read();
          break;
      }

      found = false;
    }
  }

  return found;
}

// Connects the ESP to our remote server
bool startTCPConnection() {
  Serial.print("AT+CIPSTATUS\r\n");
  if (!echoFind("STATUS:", TIMEOUT)) {
    log("Error when reading status");
    return false;
  }

  delay(10);
  char status = Serial.read();

  char conn_st[20] = "Connection status ";
  conn_st[18] = status;

  log(conn_st);
  
  switch (status) {
    default:
      log("Resetting WiFi connection");
      configureNetwork(false);
    case '2':
      log("Starting TCP Connection");

      Serial.print("AT+CIPSTART=\"TCP\",\"34.163.150.160\",80\r\n");
      return echoFind("OK", TIMEOUT);
    case '3':
      return true;
  }
}

volatile unsigned int fails = 0;

// Sends a TCP request to the remote server
int api_sendTCP(const char *command) {
  if (!startTCPConnection()) {
    fails++;

    if (fails >= 4 && fails < 8) {
      configureNetwork(true);
    } else if (fails >= 8) {
      resetBoard();
    }

    return -1;
  }

  fails = 0;

  int length = strlen(command);

  // log("Sending command:");
  // log(command, length);

  Serial.print("AT+CIPSEND=");
  Serial.print(length);
  Serial.print("\r\n");

  if (!echoFind(">", 1000)) return -1;
  // delay(100);

  Serial.println(command);

  if (!echoFind("+IPD,", 1000)) return -1;
  
  int bytes = Serial.parseInt(SKIP_NONE);

  if (Serial.peek() == ':')
    Serial.read();

  return bytes;
}

void api_confirmLock() {
  log("Starting POST Request");

  emptySerial();
  delay(1000);

  const char *confirmLockRequest = "POST /api/locking/device/1 HTTP/1.1\r\n"
    "Host: 34.163.150.160\r\n"
    "Connection: keep-alive\r\n"
    "Accept: */*\r\n"
    "Content-Type: application/json\r\n"
    "Content-Length: 65\r\n"
    "\r\n"
    "{\"battery\":72,\"location\":\"43.60183040716412, 1.4555872369679554\"}\r\n";

  if (api_sendTCP(confirmLockRequest) == -1) return;

  if (!echoFind("OK", 1000)) return;

  log("Good response, API should have updated");

  state = LOCKED;
  nextCheck = millis() + LOCKED_CHECK_PERIOD;
}

void api_confirmFinished() {
  log("Starting POST Request");

  const char *confirmLockRequest = "POST /api/locking/device/1/unlock HTTP/1.1\r\n"
    "Host: 34.163.150.160\r\n"
    "Connection: keep-alive\r\n"
    "Accept: */*\r\n"
    "Content-Length: 0\r\n"
    "\r\n";

  if (api_sendTCP(confirmLockRequest) == -1) return;

  if (!echoFind("OK", 1000)) return;

  log("Good response, API should have updated");

  state = UNLOCKED;
}

void api_checkLock() {
  const char *checkLockRequest = "GET /api/locking/device/1 HTTP/1.1\r\n"
    "Host: 34.163.150.160\r\n"
    "Connection: keep-alive\r\n"
    "Accept: */*\r\n"
    "Content-Length: 0\r\n"
    "\r\n";

  if (api_sendTCP(checkLockRequest) == -1) return;

  if (!Serial.find('{')) {
    log("No locking requested");
    return;
  }

  if (!Serial_findJSONKey("isConfirmed")) {
    log("Couldn't find field \"isConfirmed\" in JSON response");
    return;
  }

  char ch = Serial_read_to();

  bool confirmed = ch == 't';

  if (!confirmed) {
    log("isConfirmed was false");
    emptySerial();
    delay(500);
    api_confirmLock();
    return;
  }

  if (state == UNLOCKED)
    state = LOCKED;

  if (!Serial_findJSONKey("isFinished")) {
    log("Couldn't find field \"isFinished\" in JSON response");
    return;
  }

  ch = Serial_read_to();

  bool finished = ch == 't';
  log(String("isFinished = ") + String(finished));

  emptySerial();

  if (finished) {
    delay(500);
    api_confirmFinished();
    return;
  }
}

void api_sendAlarm() {
  log("Sending alarm POST request");

  const char *confirmLockRequest = "POST /api/locking/device/1/alarm HTTP/1.1\r\n"
    "Host: 34.163.150.160\r\n"
    "Connection: keep-alive\r\n"
    "Accept: */*\r\n"
    "Content-Type: application/json\r\n"
    "Content-Length: 65\r\n"
    "\r\n"
    "{\"battery\":69,\"location\":\"43.60132623487517, 1.4518017866765192\"}\r\n";

  if (api_sendTCP(confirmLockRequest) == -1) return;

  if (!echoFind("OK", 1000)) return;

  if (!Serial.find('{')) {
    log("No locking requested");
    delay(10000);
    return;
  }

  if (!Serial_findJSONKey("isFinished")) {
    log("Couldn't find field \"isFinished\" in JSON response");
    delay(10000);
    return;
  }

  char ch = Serial_read_to();

  bool finished = ch == 't';
  log(String("isFinished = ") + String(finished));

  if (finished) {
    api_confirmFinished();
  }
}

void startAlarm() {
  if (state == LOCKED) {
    state = ALARM;
    digitalWrite(LED_R, LOW);
  }
}

void loop() {
  digitalWrite(LED_R, HIGH);
  digitalWrite(LED_G, HIGH);
  digitalWrite(LED_B, HIGH);

  if (state != ALARM)
    firstAlarm = true;

  switch (state) {
    case UNLOCKED:
      digitalWrite(LED_B, LOW);

      log("Unlocked");
      delay(UNLOCKED_CHECK_PERIOD);
      api_checkLock();
      break;
    case LOCKED:
      digitalWrite(LED_G, LOW);

      if (millis() > nextCheck) {
        nextCheck = millis() + LOCKED_CHECK_PERIOD;

        api_checkLock();
      }
      break;
    case ALARM:
      if (firstAlarm) {
        resetSong();
        api_sendAlarm();
        firstAlarm = false;
      }
      digitalWrite(LED_R, LOW);
      if (false)
        state = LOCKED;
      if (playPokemonBattle()) {
        api_sendAlarm();
        resetSong();
      }
      break;
    default:
      log("Unexpected state...");
      while(1){}
  }
}
