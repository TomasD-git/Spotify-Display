#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

// pin assigment
#define TFT_CS    5
#define TFT_DC    8
#define TFT_RST   7
#define TFT_SCLK  4
#define TFT_MOSI  6

#define BTN_P1_UP     10
#define BTN_P1_DOWN   0   
#define BTN_P2_UP     3 
#define BTN_P2_DOWN   2   
#define BTN_SERVE     9   
#define BTN_RESET     1  

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

#define SCREEN_W 160
#define SCREEN_H 128

#define PADDLE_W 4
#define PADDLE_H 26
#define PADDLE_SPEED 2
#define P1_X 6
#define P2_X (SCREEN_W - 6 - PADDLE_W)

float p1Y = (SCREEN_H - PADDLE_H) / 2.0f;
float p2Y = (SCREEN_H - PADDLE_H) / 2.0f;
float p1YOld = p1Y, p2YOld = p2Y;

#define BALL_SIZE 4
float ballX, ballY, ballXOld, ballYOld;
float ballVX = 0, ballVY = 0;
bool ballInPlay = false;

int score1 = 0, score2 = 0;
int score1Old = -1, score2Old = -1;

#define FIELD_TOP 14
#define FIELD_BOTTOM (SCREEN_H - 1)

unsigned long lastFrame = 0;

bool paused = false;
int resetBtnPrevState = HIGH;
unsigned long resetPressStart = 0;
bool resetHoldTriggered = false;
const unsigned long RESET_HOLD_MS = 2000;

void resetBall(int direction) {
  ballX = SCREEN_W / 2.0f;
  ballY = (FIELD_TOP + FIELD_BOTTOM) / 2.0f;
  ballXOld = ballX;
  ballYOld = ballY;
  ballInPlay = false;
  ballVX = 1.1f * direction;
  ballVY = ((millis() % 2 == 0) ? 1.0f : -1.0f) * 0.7f;
}

void drawCourt() {
  tft.fillScreen(ST77XX_BLACK);
  for (int y = FIELD_TOP; y < FIELD_BOTTOM; y += 8) {
    tft.drawFastVLine(SCREEN_W / 2, y, 4, ST77XX_WHITE);
  }
  tft.drawFastHLine(0, FIELD_TOP - 2, SCREEN_W, ST77XX_RED);
  score1Old = -1; score2Old = -1;
  drawScore();
  p1YOld = -100; p2YOld = -100;
  ballXOld = -100;
}

void drawScore() {
  if (score1 != score1Old || score2 != score2Old) {
    tft.fillRect(0, 0, SCREEN_W, FIELD_TOP - 2, ST77XX_BLACK);
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_GREEN);
    tft.setCursor(SCREEN_W / 2 - 30, 3);
    tft.print("P1:");
    tft.print(score1);
    tft.setCursor(SCREEN_W / 2 + 10, 3);
    tft.print("P2:");
    tft.print(score2);
    score1Old = score1;
    score2Old = score2;
  }
}

void drawPauseOverlay() {
  int boxW = 74, boxH = 26;
  int boxX = (SCREEN_W - boxW) / 2;
  int boxY = (SCREEN_H - boxH) / 2;
  tft.fillRect(boxX, boxY, boxW, boxH, ST77XX_BLACK);
  tft.drawRect(boxX, boxY, boxW, boxH, ST77XX_WHITE);
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(boxX + 12, boxY + 10);
  tft.print("PAUSED");
}

void fullReset() {
  score1 = 0;
  score2 = 0;
  p1Y = (SCREEN_H - PADDLE_H) / 2.0f;
  p2Y = (SCREEN_H - PADDLE_H) / 2.0f;
  resetBall((millis() % 2 == 0) ? -1 : 1);
  paused = false;
  drawCourt();
}

void handlePauseResetButton() {
  int reading = digitalRead(BTN_RESET);

  if (reading == LOW && resetBtnPrevState == HIGH) {
    resetPressStart = millis();
    resetHoldTriggered = false;
  }

  if (reading == LOW && !resetHoldTriggered) {
    if (millis() - resetPressStart >= RESET_HOLD_MS) {
      fullReset();
      resetHoldTriggered = true;
    }
  }

  if (reading == HIGH && resetBtnPrevState == LOW) {
    if (!resetHoldTriggered) {
      paused = !paused;
      if (paused) {
        drawPauseOverlay();
      } else {
        drawCourt();
      }
    }
  }

  resetBtnPrevState = reading;
}

void setup() {
  Serial.begin(115200);

  pinMode(BTN_P1_UP, INPUT_PULLUP);
  pinMode(BTN_P1_DOWN, INPUT_PULLUP);
  pinMode(BTN_P2_UP, INPUT_PULLUP);
  pinMode(BTN_P2_DOWN, INPUT_PULLUP);
  pinMode(BTN_SERVE, INPUT_PULLUP);
  pinMode(BTN_RESET, INPUT_PULLUP);

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);

  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.setCursor(50, 55);
  tft.println("PONG");
  tft.setCursor(20, 70);
  tft.println("Press SERVE to start");
  delay(1200);

  resetBall((millis() % 2 == 0) ? -1 : 1);
  drawCourt();

  lastFrame = millis();
}

void handleInput() {
  if (digitalRead(BTN_P1_UP) == LOW)   p1Y -= PADDLE_SPEED;
  if (digitalRead(BTN_P1_DOWN) == LOW) p1Y += PADDLE_SPEED;
  if (digitalRead(BTN_P2_UP) == LOW)   p2Y -= PADDLE_SPEED;
  if (digitalRead(BTN_P2_DOWN) == LOW) p2Y += PADDLE_SPEED;

  if (p1Y < FIELD_TOP) p1Y = FIELD_TOP;
  if (p1Y > FIELD_BOTTOM - PADDLE_H) p1Y = FIELD_BOTTOM - PADDLE_H;
  if (p2Y < FIELD_TOP) p2Y = FIELD_TOP;
  if (p2Y > FIELD_BOTTOM - PADDLE_H) p2Y = FIELD_BOTTOM - PADDLE_H;

  if (!ballInPlay && digitalRead(BTN_SERVE) == LOW) {
    ballInPlay = true;
  }
}

void updateBall() {
  if (!ballInPlay) return;

  ballX += ballVX;
  ballY += ballVY;

  if (ballY <= FIELD_TOP) { ballY = FIELD_TOP; ballVY = -ballVY; }
  if (ballY >= FIELD_BOTTOM - BALL_SIZE) { ballY = FIELD_BOTTOM - BALL_SIZE; ballVY = -ballVY; }

  if (ballX <= P1_X + PADDLE_W && ballX >= P1_X &&
      ballY + BALL_SIZE >= p1Y && ballY <= p1Y + PADDLE_H && ballVX < 0) {
    ballX = P1_X + PADDLE_W;
    ballVX = -ballVX * 1.03f;
    float hitPos = ((ballY - p1Y) / PADDLE_H) - 0.5f;
    ballVY += hitPos * 1.6f;
  }
  if (ballX + BALL_SIZE >= P2_X && ballX <= P2_X + PADDLE_W &&
      ballY + BALL_SIZE >= p2Y && ballY <= p2Y + PADDLE_H && ballVX > 0) {
    ballX = P2_X - BALL_SIZE;
    ballVX = -ballVX * 1.03f;
    float hitPos = ((ballY - p2Y) / PADDLE_H) - 0.5f;
    ballVY += hitPos * 1.6f;
  }

  if (ballVY > 3) ballVY = 3;
  if (ballVY < -3) ballVY = -3;

  if (ballX < 0) {
    score2++;
    resetBall(1);
    drawCourt();
  } else if (ballX > SCREEN_W) {
    score1++;
    resetBall(-1);
    drawCourt();
  }
}

void render() {
  if ((int)p1YOld != (int)p1Y) {
    tft.fillRect(P1_X, FIELD_TOP, PADDLE_W, FIELD_BOTTOM - FIELD_TOP, ST77XX_BLACK);
    tft.fillRect(P1_X, (int)p1Y, PADDLE_W, PADDLE_H, ST77XX_CYAN);
    p1YOld = p1Y;
  }
  if ((int)p2YOld != (int)p2Y) {
    tft.fillRect(P2_X, FIELD_TOP, PADDLE_W, FIELD_BOTTOM - FIELD_TOP, ST77XX_BLACK);
    tft.fillRect(P2_X, (int)p2Y, PADDLE_W, PADDLE_H, ST77XX_YELLOW);
    p2YOld = p2Y;
  }

  if ((int)ballXOld != (int)ballX || (int)ballYOld != (int)ballY) {
    tft.fillRect((int)ballXOld, (int)ballYOld, BALL_SIZE, BALL_SIZE, ST77XX_BLACK);
    tft.fillRect((int)ballX, (int)ballY, BALL_SIZE, BALL_SIZE, ST77XX_WHITE);
    ballXOld = ballX;
    ballYOld = ballY;
  }

  drawScore();
}

void loop() {
  unsigned long now = millis();
  if (now - lastFrame < 16) return;
  lastFrame = now;

  handlePauseResetButton();
  if (paused) return;

  handleInput();
  updateBall();
  render();
}
