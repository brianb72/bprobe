
/* Global Keyboard key codes -
      The 8th bit will be turned on to show if the code was a 
   0 + scancode key, such as F1. These will be refered to as 
   'null keys'. Otherwise the code is the returned scancode. 
*/

#define KEY_NULL  0x0

/* F1 null keys */
#define KEY_F1    (0x3B | 0x80)
#define KEY_F2    (0x3C | 0x80)

/* Basic keys */
#define ENTER     0x0D
#define ESCAPE    0x1B
#define BACKSPACE 0x08
#define TAB       0x09

/* Keypad with no numlock, and arrow keys. Preceded by 0 */
#define KEY_LEFT   (0x4B | 0x80)
#define KEY_RIGHT  (0x4D | 0x80)
#define KEY_DOWN   (0x50 | 0x80)
#define KEY_UP     (0x48 | 0x80)
#define KEY_CENTER (0x4C | 0x80)      /* keypad 5 */
#define KEY_PGUP   (0x49 | 0x80)
#define KEY_PGDN   (0x51 | 0x80)
#define KEY_END    (0x4F | 0x80)
#define KEY_HOME   (0x47 | 0x80)

/* Keypad with numlock on */
#define KEY_1  0x31
#define KEY_2  0x32
#define KEY_3  0x33
#define KEY_4  0x34
#define KEY_5  0x35
#define KEY_6  0x36
#define KEY_7  0x37
#define KEY_8  0x38
#define KEY_9  0x39


