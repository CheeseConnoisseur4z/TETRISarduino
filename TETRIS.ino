#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3D
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
 

int tickCounter = 0;
int moveCooldown = 0;
byte moveSpeed = 5;
int points = 0;

byte x, y, r, s;
byte fallSpeed = 35;

byte grid[] = {255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
byte shapeGrid[] = {0, 0, 0};
byte saveShapeGrid[] = {0, 0, 0};
byte mask[] = {128, 64, 32, 16, 8, 4, 2, 1};

byte shape[][4] = {
  {0b01000010, 0b00011000, 0b01000010, 0b00011000}, // I
  {0b01011000, 0b01001010, 0b00011010, 0b01010010}, // T
  {0b00010110, 0b00010110, 0b00010110 ,0b00010110}, // O
  {0b01100010, 0b00011001, 0b01000110, 0b10011000}, // L
  {0b11000010, 0b00111000, 0b01000011, 0b00011100}, // rL
  {0b01110000, 0b01001001, 0b01110000, 0b01001001}, // S
};

void setup() {
  Serial.begin(9600);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
  }

  s = random(6);
  r = 0;
  x = 3;
  y = 15;
  byte beforeMove;
  makeShape();
  moveShape(0, x);

  while (true) {
    display.clearDisplay();

    if (moveCooldown >= moveSpeed) {
      copyArray(shapeGrid, saveShapeGrid);

      int xValue = analogRead(A0);
      int yValue = analogRead(A1);

      byte saveX = x;
      byte xl = (xValue > 980);
      byte xr = (xValue < 50);

      if (yValue < 50) {
        r = (r + 1 == 4) ? 0 : r + 1;
        makeShape();
        moveShape((x == 0), x * (x != 0));
      } else if (xl != 0 || xr != 0) {
        moveShape(xl, xr);
        x -= xl;
        x += xr;
      } else if (yValue > 980) {
        fallSpeed = 1;
      }

      if (checkCollision() || countBits() != ((s == 0) ? 3 : 4)) {
        x = saveX;
        copyArray(saveShapeGrid, shapeGrid);
      }

      moveCooldown = 0;
    }
  
    if (tickCounter % fallSpeed == 0) {
      y--;
      if (checkCollision()) {
        y++;

        grid[y + 1] = grid[y + 1] | shapeGrid[0];
        grid[y] = grid[y] | shapeGrid[1];
        grid[y - 1] = grid[y - 1] | shapeGrid[2];

        if (y == 15) Serial.println("GAME OVER");

        y = 15;
        x = 3;
        s = random(6);
        r = 0;
        fallSpeed = 35;

        deleteRows();
      
        makeShape();
        moveShape(0, x);
      }
    }

    drawEverything();

    display.display();
    tickCounter++;
    moveCooldown++;
  }
}

bool checkCollision() {
  if ((shapeGrid[0] & grid[y + 1]) != 0) return true;
  if ((shapeGrid[1] & grid[y]) != 0) return true;
  if ((shapeGrid[2] & grid[y - 1]) != 0) return true;
  return false;
}

void makeShape() {
  shapeGrid[0] = shape[s][r] & 0b11100000;
  shapeGrid[1] = ((shape[s][r] & 0b00010000 | 0b00001000) << 3) | ((shape[s][r] & 0b00001000) << 2);
  shapeGrid[2] = (shape[s][r] & 0b00000111) << 5;
}

void moveShape(byte l, byte r) {
  shapeGrid[0] = shapeGrid[0] << l >> r;
  shapeGrid[1] = shapeGrid[1] << l >> r;
  shapeGrid[2] = shapeGrid[2] << l >> r;
}

void copyArray(byte copyFrom[], byte copyTo[]) {
  for (int i = 0; i < 3; i++) {
    copyTo[i] = copyFrom[i];
  }
}

byte countBits() {
  byte c = 0;
  for (byte i = 0; i < 3; i++) {
    for (byte i2 = 0; i2 < 8; i2++) {
      if ((mask[i2] & shapeGrid[i]) != 0) c++;
    }
  }
  return c;
}

void deleteRows() {
  byte c = 0;
  for (byte i = 1; i < 18; i++) {
    if (grid[i] == 255) {
      c++;
      for (byte i2 = i; i2 < 18; i2++) {
        grid[i2] = grid[i2 + 1];
      }
      i--;
    }
  }
  points += c * c;
}

void drawEverything() {
  for (byte i = 1; i < 17; i++) {
    for (byte i2 = 0; i2 < 8; i2++) {
      if ((mask[i2] & grid[i]) != 0) drawSquare(i2 * 8, (i - 1) * 8);
    }
  }

  for (byte i = 0; i < 3; i++) {
    for (byte i2 = 0; i2 < 8; i2++) {
      if ((mask[i2] & shapeGrid[i]) != 0) drawSquare(i2 * 8, (y - 1) * 8 + (2 - i) * 8);
    }
  }
}

void drawSquare(byte x, byte y) {
  display.drawLine(y, x, y + 7, x, SSD1306_WHITE);
  display.drawLine(y, x, y, x + 7, SSD1306_WHITE);
  display.drawLine(y, x + 7, y + 7, x + 7, SSD1306_WHITE);
  display.drawLine(y + 7, x, y + 7, x + 7, SSD1306_WHITE);
}