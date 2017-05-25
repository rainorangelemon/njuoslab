#ifndef KEYBOARD
#define KEYBOARD

void press_key(int);
void release_key(int);
bool query_key(int);
int last_key_code(void);
void keyboard_event(int );
void set_keyboard_intr_handler(void(*ptr)(int));
void clear_letter_pressed();

#endif
