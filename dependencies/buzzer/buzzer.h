#ifndef rtiobuzzer
#define rtiobuzzer

void buzzer_init(void);
void buzzer_cleanup(void);

// //turns the buzzer on. 
// void buzzer_on(void);
// //turns the buzzer off.
// void buzzer_off(void);

//turns on/off the buzzer 'ring'. Only call buzzer_ring_on once before ring_off.
void buzzer_ring_on(void);
void buzzer_ring_off(void);

//comment redacted.
void buzzer_secret(void);


#endif