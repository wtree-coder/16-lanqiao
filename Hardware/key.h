#ifndef __KEY_H
#define __KEY_H

#include "main.h"

extern uint8_t key_long[4];
extern uint8_t key_short[4];

uint8_t  get_state(uint8_t id);
void key_tick(void);

#endif
