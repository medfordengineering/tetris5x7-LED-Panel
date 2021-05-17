#include <Wire.h>
#include <SparkFunSX1509.h> // Include SX1509 library

//Buttons for controlling the pieces
#define B_R   4
#define B_L   2
#define B_T   3

#define FULL_ON 80
#define FULL_OFF 0
#define ROW_OFFSET  2 //Offsets where print_map starts from with respect to current_map

SX1509 s[3];

const byte SX1509_A[3] = {0x3E, 0x3F, 0x70};
const int rows = 7;
const int columns = 5;

int8_t angle = 0;            //tracks four orienations of piece
uint8_t piece_id = 1;         //tracks which piece is being addressed
uint8_t shift_left = 0;       //tracks horizonatal location of piece, starts with a 1 offset
uint8_t row_count = 0;        //keeps track of which row is being addressed

//Map of panel numbered starting at upper right
int pannel[ rows ][ columns ] = {
  { 4, 3, 2, 1, 0 },
  {  9, 8, 7, 6, 5 },
  { 14, 13, 12, 11, 10},
  { 19, 18, 17, 16, 15},
  { 24, 23, 22, 21, 20},
  { 29, 28, 27, 26, 25},
  { 34, 33, 32, 31, 30}

};

//This map defines a boarder around the 5x7 tetris panel
uint16_t current_map [12] = {
  0x0000,
  0x0000,

  0x0020,//top of 5x7
  0x0020,
  0x0020,
  0x0020,
  0x0020,
  0x0020,
  0x0020,//bottom of 5x7

  0xFFFF,//lower barrier

  0x0000,
  0x0000,
};

const char piece_I[12] = {
  B00000000,
  B00000111,
  B00000000,

  B00000010,
  B00000010,
  B00000010,

  B00000000,
  B00000111,
  B00000000,

  B00000010,
  B00000010,
  B00000010,
};

const char piece_T[12] = {
  B00000001,
  B00000011,
  B00000001,

  B00000010,
  B00000111,
  B00000000,

  B00000010,
  B00000011,
  B00000010,

  B00000000,
  B00000111,
  B00000010,
};

const char piece_L[12] = {
  B00000001,
  B00000001,
  B00000011,

  B00000100,
  B00000111,
  B00000000,

  B00000011,
  B00000010,
  B00000010,

  B00000000,
  B00000111,
  B00000001,
};

const char piece_Z[12] = {
  B00000100,
  B00000110,
  B00000010,

  B00000011,
  B00000110,
  B00000000,

  B00000010,
  B00000011,
  B00000001,

  B00000000,
  B00000011,
  B00000110,
};


const char *pieces[4] = {
  piece_T,
  piece_I,
  piece_Z,
  piece_L
};

const char *piece;

//Controls on/off/brightness of any LED panel in matrix
void tpanel(uint8_t x, uint8_t y, uint8_t level) {
  uint8_t xy = pannel[x][y];
  s[xy / 16].analogWrite(xy % 16, level);  //For Brightness Mode
}

//Tests potential overlap of piece.
bool collision() {
  int collision = false;
  for (int row = 0; row < 3; row++) {
    if (current_map[row + row_count] & (piece[row + (angle * 3)] << shift_left)) {
      collision = true;
    }
  }
  return collision;
}

//Adds current piece to current map
void add_piece() {
  for (int row = 0; row < 3; row++) {
    current_map[row + row_count] |= (piece[row + (angle * 3)] << shift_left);
  }
}

//Removes current piece from map
void remove_piece() {
  for (int row = 0; row < 3; row++) {
    current_map[row + row_count] &= ~(piece[row + (angle * 3)] << shift_left);
  }
}

//Prints 5x7 (starting upper right with offset) of current_map to the 5x7 display
void print_map() {
  for (int row = 0; row < 7; row++) {
    for (int col = 0; col < 5; col++) {
      if (current_map[row + ROW_OFFSET] & 1 << col ) {
        tpanel(row, col, FULL_ON);
      } else {
        tpanel(row, col, FULL_OFF);
      }
    }
  }
}

void setup() {
  uint8_t i, x, y;

  pinMode(B_R, INPUT_PULLUP);
  pinMode(B_L, INPUT_PULLUP);
  pinMode(B_T, INPUT_PULLUP);

  Serial.begin(9600);

  //Initialize all three multiplexors
  for ( i = 0; i < 3; i++) {
    s[i].begin(SX1509_A[i]);
    s[i].clock(INTERNAL_CLOCK_2MHZ, 4);
  }

  //Set all pins on multiplexor to analog/pwm
  for ( i = 0; i < 3; i++) {
    for (uint8_t x = 0; x < 16; x++) {
      s[i].pinMode(x, ANALOG_OUTPUT); //For Brightness Mode
    }
  }
  randomSeed(analogRead(0));
  //Initialize the first piece for the game
  piece = pieces[random(4)];
}

void new_piece() {
  angle = random(4);
  piece_id = random(4);         //tracks which piece is being addressed
  shift_left = 0;       //tracks horizonatal location of piece, starts with a 1 offset
  row_count = 0;        //keeps track of which row is being addressed
}

void loop() {

  //1. Remove piece: You need to remove the present piece from the map before testing rotation, shift or increment. Otherwise
  //the piece will return a collision with itself.
  remove_piece();

  //2. Rotate piece: Rotate piece and check for collision. If a collision occurs, return piece to its previous rotation.
  if (++angle >= 4) angle = 0;
  //delay(200);
  if (collision() == true) {
    if (--angle < 0) angle = 3;
  }

  //3. Shift piece: Shift piece and check for collision. If a collision occurs, return piece to its previous shift value.
  shift_left++;
  //delay(200);
  if (collision() == true) {
    shift_left--;
  }

  //4. Increment counter: Increment row counter and check for collision. 
  row_count++;

  if (collision() == true) {
    row_count--;
    add_piece();
    new_piece();
  } else {
    add_piece();
  }
  print_map();

  delay(1000);


}
