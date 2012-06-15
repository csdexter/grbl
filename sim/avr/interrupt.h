#ifndef interrupt_h
#define interrupt_h
#include <inttypes.h>

extern uint8_t timsk1;
extern uint8_t timsk2;
extern uint8_t tcnt2;
extern uint8_t tccr2a;
extern uint8_t tccr2b;
extern uint8_t tccr1b;
extern uint8_t tccr1a;
extern uint8_t ocr1a;

#define TIMER1_COMPA_vect
#define ISR(a) void interrupt_ ## a ()

void sei();

#define TIMSK1 timsk1
#define TIMSK2 timsk2
#define OCR1A ocr1a
#define OCIE1A 0
#define TCNT2 tcnt2
#define TCCR1A tccr1a
#define TCCR1B tccr1b
#define TCCR2A tccr2a
#define TCCR2B tccr2b
#define CS21 0
#define CS10 0
#define WGM13 0
#define WGM12 0
#define WGM11 0
#define WGM10 0
#define COM1A0 0
#define COM1B0 0
#define TOIE2 0

#endif