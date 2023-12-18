#include <Tone.h>
#include "music.h"

#define BUZZER_1 11
#define BUZZER_2 12
#define BUTTON 2
#define LED A0

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

volatile bool playing = false;

void setup() {
  tone1.begin(BUZZER_1);
  tone2.begin(BUZZER_2);
  
  pinMode(BUTTON, INPUT);
  attachInterrupt(digitalPinToInterrupt(BUTTON), buttonPressHandler, FALLING);

  pinMode(LED, OUTPUT);
}

void loop() {
  if (!playing) return;  // Exit loop if not playing
  
  currentMicros = micros();

  if (currentMicros >= next1) {
    playNotes1();
  }

  if (currentMicros >= next2) {
    playNotes2();
  }
}

void playNotes1() {
  int d1 = pgm_read_word_near(delays1 + index1);

  next1 = currentMicros + 1000l * d1;

  if (index1 < sizeof(melody1)/sizeof(int)) {
    int note1 = pgm_read_word_near(melody1 + index1);
    if (note1 != 0) {
      tone1.play(note1, pgm_read_word_near(delays1 + index1));
    }
    index1++;
  }

  // Check if melodies finished
  if (index1 >= sizeof(melody1)/sizeof(int)) {
    if (finished2)
      resetPlayback(true);
    else
      finished1 = true;
  }
}

void playNotes2() {
  int d2 = pgm_read_word_near(delays2 + index2);

  next2 = currentMicros + 1000l * d2;

  if (index2 < sizeof(melody2)/sizeof(int)) {
    int note2 = pgm_read_word_near(melody2 + index2);
    if (note2 != 0) {
      tone2.play(note2, pgm_read_word_near(delays2 + index2));
    }
    index2++;
  }

  // Check if melodies finished
  if (index1 >= sizeof(melody1)/sizeof(int)) {
    if (finished1)
      resetPlayback(true);
    else
      finished2 = true;
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
  
}

void buttonPressHandler() {
  int time = millis();
  if (time - lastPress < 30) return; // 30ms debounce
  lastPress = time;
  playing = !playing;
  resetPlayback(false);
  digitalWrite(LED, playing ? HIGH : LOW);
}
