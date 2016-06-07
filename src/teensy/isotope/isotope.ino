#include <usb_dev.h>

#define READ_TIMEOUT 500

#define OP_CUSTOM 0x00
#define OP_KEYBOARD 0x1
#define OP_MOUSE 0x2
#define OP_JOYSTICK 0x3

char rx_buffer[32] = {0};
char id = 0x0;

// Handler functions
void init_id();
void on_custom();
void on_keyboard();
void on_mouse();
void on_joystick();

// Protocol utility functions
char read_blocking(char* target, int max_attempts);
char proto_opcode(char header);
char proto_args(char header);
short proto_short(char start);
int proto_int(char start);

void setup() {
  // Initialize the UART
  Serial2.begin(115200,SERIAL_8E1);

  // Turn on the LED
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);

  // Read Config Pins
  pinMode(18, INPUT);
  pinMode(19, INPUT);
  pinMode(20, INPUT);

  init_id();

  usb_init();
  while(!usb_configured);

  if(!Serial2.available())
    Serial2.println("USBC "+id);

  // Turn the LED off
  digitalWrite(13, LOW);
}

void loop() {
  char i, opcode, args, target;

  if (!usb_configuration) {
     Serial2.println("USBD");
     delay(50);
    _reboot_Teensyduino_();
  }

  if (!Serial2.available()) return;

  digitalWrite(13, HIGH);

  // Reset the buffer
  for(i = 0; i < 32; i++) rx_buffer[i] = 0;

  // Read the header
  if(!read_blocking(rx_buffer, READ_TIMEOUT) || rx_buffer[0] == 0xf8) {
  	digitalWrite(13, LOW);
    return;
  }

  // Process the header values and store their outputs
  target = proto_target(rx_buffer[0]);
  args = proto_args(rx_buffer[0]);

  // Read the arguments
  if(!read_n_blocking(rx_buffer + 1, args, READ_TIMEOUT)) {
    digitalWrite(13, LOW);
    return;
  }

  opcode = rx_buffer[1];

  if(target == id) {
    // Call the function responsible for the relevant op-code
    switch(opcode) {
      case OP_CUSTOM:
        on_custom();
        break;
        case OP_KEYBOARD:
        on_keyboard();
        break;
        case OP_MOUSE:
        on_mouse();
        break;
        default:
        break;
      }
    }

  digitalWrite(13, LOW);
}

void init_id() {
  if ( digitalRead(18) == HIGH)
    id |= 0x1;
  if ( digitalRead(19) == HIGH)
    id |= 0x2;
  if ( digitalRead(20) == High)
    id |= 0x4;
}

// Handler functions
void on_custom() {
}

void on_keyboard() {
  Keyboard.set_modifier(rx_buffer[2]);
  Keyboard.set_key1(rx_buffer[3]);
  Keyboard.set_key2(rx_buffer[4]);
  Keyboard.set_key3(rx_buffer[5]);
  Keyboard.set_key4(rx_buffer[6]);
  Keyboard.set_key5(rx_buffer[7]);
  Keyboard.set_key6(rx_buffer[8]);
  Keyboard.send_now();
}

void on_mouse() {
  Mouse.set_buttons(!!(rx_buffer[2] & 0x1), !!(rx_buffer[2] & 0x2), !!(rx_buffer[2] & 0x4));
  Mouse.move(rx_buffer[3], rx_buffer[4], rx_buffer[5]);
}

// Utility functions
char read_blocking(char* target, int max_attempts) {
  while(max_attempts--) {
    if(!Serial2.available()) continue;
    *target = Serial2.read();
    return 1;
  }
  return 0;
}

char read_n_blocking(char* target, char length, int max_attempts) {
  byte tmp;
  char count = 0;
  while(max_attempts-- && count < length) {
    count += Serial2.readBytes(target + count, length - count);
  }
  return count == length;
}

char proto_target(char header) {
  return (header & 0xE0) >> 5;
}

char proto_args(char header) {
  return header & 0x1F;
}

short proto_short(char start) {
  // Returns a 2-byte short from the given starting position
  return *((short*)(rx_buffer + start));
}

int proto_int(char start) {
  // Returns a 4-byte integer from the given starting position
  return *((int*)(rx_buffer + start));
}
