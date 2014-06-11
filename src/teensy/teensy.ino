#define OP_CUSTOM 0x00
#define OP_KEYBOARD 0x1
#define OP_MOUSE 0x2
#define OP_JOYSTICK 0x3

char rx_buffer[16] = {0};

// Handler functions
void on_custom();
void on_keyboard();
void on_mouse();
void on_joystick();

// Protocol utility functions
char proto_opcode(char header);
char proto_args(char header);
short proto_short(char start);
int proto_int(char start);

void setup() {
  // Initialize the UART
  Serial1.begin(57600);
  
  // Enable manual mode for Joystick timing
  Joystick.useManualSend(true);
}

void loop() {  
  char i, opcode, args;
  
  // Reset the buffer
  for(i = 0; i < 16; i++) rx_buffer[i] = 0;
  
  // Read the header
  rx_buffer[0] = Serial1.read();
  
  // Process the header values and store their outputs
  opcode = proto_opcode(rx_buffer[0]);
  args = proto_args(rx_buffer[0]);
  
  // Read the arguments
  for(i = 1; i <= args; i++) rx_buffer[i] = Serial1.read();
  
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
    case OP_JOYSTICK:
      on_joystick();
      break;
  }
}

// Handler functions
void on_custom() {
  
}

void on_keyboard() {
  Keyboard.set_modifier(rx_buffer[1]);
  Keyboard.set_key1(rx_buffer[2]);
  Keyboard.set_key2(rx_buffer[3]);
  Keyboard.set_key3(rx_buffer[4]);
  Keyboard.set_key4(rx_buffer[5]);
  Keyboard.set_key5(rx_buffer[6]);
  Keyboard.set_key6(rx_buffer[7]);
  Keyboard.send_now();
}

void on_mouse() {
  Mouse.set_buttons(!!(rx_buffer[1] & 0x1), !!(rx_buffer[1] & 0x2), !!(rx_buffer[1] & 0x4));
  Mouse.move(rx_buffer[2], rx_buffer[3], rx_buffer[4]);
}

void on_joystick() {
  char i;
  int pack;
  
  
  // Load the buttons
  pack = proto_int(1);
  for(i = 0; i < 32; i++) Joystick.button(i, !!(pack & (0x00000001 << i)));
  
  // X, Y, Z
  pack = proto_int(5);
  Joystick.Z(pack & 0x3FF);
  pack = pack >> 10;
  Joystick.Y(pack & 0x3FF);
  pack = pack >> 10;
  Joystick.X(pack & 0x3ff);
  
  // rZ, sL, sR
  pack = proto_int(9);
  Joystick.Zrotate(pack & 0x3FF);
  pack = pack >> 10;
  Joystick.sliderLeft(pack & 0x3FF);
  pack = pack >> 10;
  Joystick.sliderRight(pack & 0x3ff);
  
  // Hat Switch
  i = rx_buffer[13];
  if(i == 0xFF) Joystick.hat(-1);
  else Joystick.hat(45 * i);
  
  Joystick.send_now();
}

// Utility functions
char proto_opcode(char header) {
  return (header & 0xE) >> 5;
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
