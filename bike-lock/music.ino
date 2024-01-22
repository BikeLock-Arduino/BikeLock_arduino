#include "music.h"

//// Manual songs ////

void play1UpSound() {
  tone(BUZZER, 659);
  delay(117);
  tone(BUZZER, 784);
  delay(117);
  tone(BUZZER, 1318);
  delay(117);
  tone(BUZZER, 1046);
  delay(117);
  tone(BUZZER, 1175);
  delay(117);
  tone(BUZZER, 1568);
  delay(234);
  noTone(BUZZER);
}

//// Auto Songs ////

unsigned long start;
unsigned long currentMicros;
unsigned long next = 0;
int index = 0;
bool finished = false;
int sz_pmb = sizeof(melody_pmb) / sizeof(long);;

void resetSong() {
  next = micros();
  index = 0;
  finished = false;
}

bool playPokemonBattle() {
  currentMicros = micros();

  if (currentMicros < next) return;
  
  unsigned long d = pgm_read_dword_near(delays_pmb + index); // us

  next += d;

  if (index >= sz_pmb) return;

  int note = pgm_read_dword_near(melody_pmb + index);

  if (note != 0)
    tone(BUZZER, note);
  else
    noTone(BUZZER);
  
  index++;

  bool isFinished = index >= sz_pmb;
  if (isFinished) noTone(BUZZER);
  return isFinished;
}


