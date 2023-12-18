#include <Tone.h>
#include "songs/garden.h"

#define BUZZER_1 11
#define BUZZER_2 12
#define BUTTON 2
#define LED 13

Tone tone1;
Tone tone2;

int lastPress = 0;
unsigned long currentMicros;
unsigned long next1 = 0;
unsigned long next2 = 0;
int index1 = 0;
int index2 = 0;
bool finished1 = false;
bool finished2 = false;

int sz1, sz2;

volatile bool playing = false;

void setup() {
  Serial.begin(9600);
  tone1.begin(BUZZER_1);
  tone2.begin(BUZZER_2);
  
  pinMode(BUTTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON), buttonPressHandler, RISING);

  pinMode(LED, OUTPUT);

  sz1 = sizeof(melody1)/sizeof(long);
  sz2 = sizeof(melody2)/sizeof(long);

  Serial.println("Hi");
  Serial.println(pgm_read_dword_near(delays1 + 0));
  Serial.println("Bye");
}

void loop() {
  if (!playing) return;  // Exit loop if not playing
  
  currentMicros = micros();

  if (currentMicros >= next1) {
    playNotes(&tone1, &next1, &index1, delays1, melody1, sz1);
  }

  if (currentMicros >= next2) {
    playNotes(&tone2, &next2, &index2, delays2, melody2, sz2);
  }
}

void playNotes(Tone *tn, unsigned long *next, int *index, const unsigned long *delays, const unsigned long *melody, int sz) {
  unsigned long d = pgm_read_dword_near(delays + *index); // us

  *next += d;

  if (*index < sz) {
    int note = pgm_read_dword_near(melody + *index);
    if (note != 0) {
      tn->play(note, d / 1000L);
    }
    (*index)++;

    if (index1 >= sz1 && index2 >= sz2)
      resetPlayback(true);
  }
}

void resetPlayback(bool stop) {
  if (stop)
    playing = false;
  digitalWrite(LED, LOW);
  index1 = 0;
  index2 = 0;
  finished1 = false;
  finished2 = false;
  tone1.stop();
  tone2.stop();
}

void buttonPressHandler() {
  int time = millis();
  if (time - lastPress < 30) return; // 30ms debounce
  lastPress = time;
  delay(2500);
  playing = !playing;
  resetPlayback(false);
  digitalWrite(LED, playing ? HIGH : LOW);
}
