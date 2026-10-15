#include <U8g2lib.h>
#include <Wire.h>

// ================== ピン設定 ==================
#define PIN_DOWN    16
#define PIN_LEFT    17
#define PIN_RIGHT   18
#define PIN_ROTATE  19
#define PIN_LED      2     // オンボードLED
#define PIN_BUZZER  23     // 圧電サウンダ
#define PIN_VOLTAGE 26

// ================== U8g2 ==================
U8G2_SSD1309_128X64_NONAME0_F_HW_I2C u8g2(U8G2_R1, U8X8_PIN_NONE, /*SCL*/22, /*SDA*/21);

// ================== ゲーム定数 ==================
const int BOARD_W = 10;
const int BOARD_H = 18;
const int BLOCK_SIZE = 4;
const int GAP = 1;
const int CELL = BLOCK_SIZE + GAP;

const int BOARD_X = 7;
const int BOARD_Y = 24;

// テトリミノ定義
const uint8_t tetrominoes[7][4][4][4] = {
  {{{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}},{{0,0,1,0},{0,0,1,0},{0,0,1,0},{0,0,1,0}},{{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}},{{0,0,1,0},{0,0,1,0},{0,0,1,0},{0,0,1,0}}},
  {{{0,0,0,0},{0,1,1,0},{0,1,1,0},{0,0,0,0}},{{0,0,0,0},{0,1,1,0},{0,1,1,0},{0,0,0,0}},{{0,0,0,0},{0,1,1,0},{0,1,1,0},{0,0,0,0}},{{0,0,0,0},{0,1,1,0},{0,1,1,0},{0,0,0,0}}},
  {{{0,0,0,0},{1,1,1,0},{0,1,0,0},{0,0,0,0}},{{0,1,0,0},{1,1,0,0},{0,1,0,0},{0,0,0,0}},{{0,1,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},{{0,1,0,0},{0,1,1,0},{0,1,0,0},{0,0,0,0}}},
  {{{0,0,0,0},{1,1,1,0},{0,0,1,0},{0,0,0,0}},{{0,1,0,0},{0,1,0,0},{1,1,0,0},{0,0,0,0}},{{1,0,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},{{0,1,1,0},{0,1,0,0},{0,1,0,0},{0,0,0,0}}},
  {{{0,0,0,0},{1,1,1,0},{1,0,0,0},{0,0,0,0}},{{1,1,0,0},{0,1,0,0},{0,1,0,0},{0,0,0,0}},{{0,0,1,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},{{0,1,0,0},{0,1,0,0},{0,1,1,0},{0,0,0,0}}},
  {{{0,1,1,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}},{{1,0,0,0},{1,1,0,0},{0,1,0,0},{0,0,0,0}},{{0,1,1,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}},{{1,0,0,0},{1,1,0,0},{0,1,0,0},{0,0,0,0}}},
  {{{1,1,0,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},{{0,1,0,0},{1,1,0,0},{1,0,0,0},{0,0,0,0}},{{1,1,0,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},{{0,1,0,0},{1,1,0,0},{1,0,0,0},{0,0,0,0}}}
};

const int32_t next_offset_y[7] = {0, 1, 1, 1, 1, 0, 0};

int board[BOARD_H][BOARD_W] = {0};

int currentPiece, nextPiece, rotation;
int posX, posY;
int score = 0, level = 0;
unsigned long dropTimer = 0;
unsigned long dropInterval = 500;
bool gameOver = false, started = false;
bool isClearingLines = false;

bool modeB = false;                          // false=A, true=B
const unsigned long B_MODE_INTERVAL = 170;   // Bモード固定速度

// ================== 音の定義 ==================
const float tone_base_do = 261.626 * 2;
const float tone_base_re = 293.665 * 2;
const float tone_base_mi = 329.628 * 2;
const float tone_base_fa = 349.228 * 2;
const float tone_base_so = 391.995 * 2;
const float tone_base_ra = 440.000 * 2;
const float tone_base_si = 493.883 * 2;
const float tone_sharp = 1.06;

const int tone_si0 = (int)(tone_base_si * 0.5);
const int tone_do1 = (int)(tone_base_do * 1.0);
const int tone_re1 = (int)(tone_base_re * 1.0);
const int tone_mi1 = (int)(tone_base_mi * 1.0);
const int tone_fa1 = (int)(tone_base_fa * 1.0);
const int tone_so1 = (int)(tone_base_so * 1.0);
const int tone_ra1 = (int)(tone_base_ra * 1.0);
const int tone_si1 = (int)(tone_base_si * 1.0);
const int tone_do2 = (int)(tone_base_do * 2.0);
const int tone_re2 = (int)(tone_base_re * 2.0);
const int tone_mi2 = (int)(tone_base_mi * 2.0);
const int tone_fa2 = (int)(tone_base_fa * 2.0);
const int tone_so2 = (int)(tone_base_so * 2.0);
const int tone_ra2 = (int)(tone_base_ra * 2.0);
const int tone_si2 = (int)(tone_base_si * 2.0);
const int tone_do3 = (int)(tone_base_do * 3.0);
const int tone_re3 = (int)(tone_base_re * 3.0);
const int tone_mi3 = (int)(tone_base_mi * 3.0);
const int tone_fa3 = (int)(tone_base_fa * 3.0);
const int tone_so3 = (int)(tone_base_so * 3.0);
const int tone_ra3 = (int)(tone_base_ra * 3.0);
const int tone_si3 = (int)(tone_base_si * 3.0);
const int tone_do4 = (int)(tone_base_do * 4.0);

const int len0 = 20;
const int len1 = 200;
const int len2 = 400;
const int len3 = 600;
const int len4 = 800;

struct Note {
  int freq;
  int duration;
};

const float spd_tetorisu = 1.05;
const Note tetorisu[] = {
  {tone_mi2, (int)(len2/spd_tetorisu)},
  {tone_si1, (int)(len1/spd_tetorisu)},
  {tone_do2, (int)(len1/spd_tetorisu)},
  {tone_re2, (int)(len2/spd_tetorisu)},
  {tone_do2, (int)(len1/spd_tetorisu)},
  {tone_si1, (int)(len1/spd_tetorisu)},
  {tone_ra1, (int)(len2/spd_tetorisu)},
  {tone_ra1, (int)(len1/spd_tetorisu)},
  {tone_do2, (int)(len1/spd_tetorisu)},
  {tone_mi2, (int)(len2/spd_tetorisu)},
  {tone_re2, (int)(len1/spd_tetorisu)},
  {tone_do2, (int)(len1/spd_tetorisu)},
  {tone_si1, (int)(len2/spd_tetorisu)},
  {tone_si1, (int)(len1/spd_tetorisu)},
  {tone_do2, (int)(len1/spd_tetorisu)},
  {tone_re2, (int)(len2/spd_tetorisu)},
  {tone_mi2, (int)(len2/spd_tetorisu)},
  {tone_do2, (int)(len2/spd_tetorisu)},
  {tone_ra1, (int)(len2/spd_tetorisu)},
  {tone_ra1, (int)(len3/spd_tetorisu)},
  {0,        (int)(len1/spd_tetorisu)},
  {tone_re2, (int)(len2/spd_tetorisu)},
  {tone_fa2, (int)(len1/spd_tetorisu)},
  {tone_ra2, (int)(len2/spd_tetorisu)},
  {tone_so2, (int)(len1/spd_tetorisu)},
  {tone_fa2, (int)(len1/spd_tetorisu)},
  {tone_mi2, (int)(len3/spd_tetorisu)},
  {tone_do2, (int)(len1/spd_tetorisu)},
  {tone_mi2, (int)(len2/spd_tetorisu)},
  {tone_re2, (int)(len1/spd_tetorisu)},
  {tone_do2, (int)(len1/spd_tetorisu)},
  {tone_si1, (int)(len2/spd_tetorisu)},
  {tone_si1, (int)(len1/spd_tetorisu)},
  {tone_do2, (int)(len1/spd_tetorisu)},
  {tone_re2, (int)(len2/spd_tetorisu)},
  {tone_mi2, (int)(len2/spd_tetorisu)},
  {tone_do2, (int)(len2/spd_tetorisu)},
  {tone_ra1, (int)(len2/spd_tetorisu)},
  {tone_ra1, (int)(len3/spd_tetorisu)},
  {0,        (int)(len1/spd_tetorisu)},
};
const int tetorisu_len = sizeof(tetorisu) / sizeof(Note);

const Note clear_lines[] = {
  {tone_do1, len1/4},
  {tone_do1*tone_sharp, len1/4},
  {tone_re1, len1/4},
  {tone_re1*tone_sharp, len1/4},
  {tone_mi1, len1/4},
  {tone_fa1, len1/4},
  {tone_fa1*tone_sharp, len1/4},
  {0, len1/2},
  {tone_do1, len1/2},
};
const int clear_lines_len = sizeof(clear_lines) / sizeof(Note);

const Note clear_lines4[] = {
  {tone_mi3, len1/3},
  {tone_do3, len1/3},
  {tone_mi3, len1/3},
  {tone_do3, len1/2},
  {0, len1/3},
  {tone_do1, len2},
};
const int clear_lines_len4 = sizeof(clear_lines4) / sizeof(Note);

const Note gameover_melody[] = {
  {tone_do1, len1/4},
  {tone_do1*tone_sharp, len1/4},
  {tone_do1, len1/4},
  {tone_si0, len2},
  {0, len1/2},
  {tone_mi2, len1/2},
  {tone_re2, len1/2},
  {tone_mi2, len1/2},
  {tone_re2, len1/2},
  {tone_mi2, len1/2},
  {tone_fa2, len1/2},
  {tone_mi2, len1/2},
  {tone_re2, len1/2},
  {tone_mi2, len2/2}
};
const int gameover_melody_len = sizeof(gameover_melody) / sizeof(Note);

// ================== オーディオ管理 ==================
int bgmTotalDuration = 0;
unsigned long bgmStartTime = 0;

volatile const Note* interruptMelody = NULL;
volatile int interruptMelodyLength = 0;
volatile unsigned long interruptStartTime = 0;
volatile unsigned long interruptDuration = 0;

void playSE(const Note* melody, int length) {
  int totalDur = 0;
  for (int i = 0; i < length; i++) {
    totalDur += melody[i].duration + len0;
  }
  interruptMelody = melody;
  interruptMelodyLength = length;
  interruptDuration = totalDur;
  interruptStartTime = millis();
}

void play_melody_thread(void *pvParameters) {
  int lastFreq = -1;
  while (1) {
    unsigned long now = millis();
    int currentFreq = 0;

    if (interruptMelody != NULL) {
      if (now - interruptStartTime < interruptDuration) {
        unsigned long elapsed = now - interruptStartTime;
        unsigned long accum = 0;
        for (int i = 0; i < interruptMelodyLength; i++) {
          int noteTotal = interruptMelody[i].duration + len0;
          if (elapsed < accum + noteTotal) {
            if (elapsed < accum + interruptMelody[i].duration) {
              currentFreq = interruptMelody[i].freq;
            } else {
              currentFreq = 0;
            }
            break;
          }
          accum += noteTotal;
        }
      } else {
        interruptMelody = NULL;
      }
    }

    if (interruptMelody == NULL) {
      if (started && !gameOver && bgmTotalDuration > 0) {
        unsigned long elapsed = (now - bgmStartTime) % bgmTotalDuration;
        unsigned long accum = 0;
        for (int i = 0; i < tetorisu_len; i++) {
          int noteTotal = tetorisu[i].duration + len0;
          if (elapsed < accum + noteTotal) {
            if (elapsed < accum + tetorisu[i].duration) {
              currentFreq = tetorisu[i].freq;
            } else {
              currentFreq = 0;
            }
            break;
          }
          accum += noteTotal;
        }
      } else {
        currentFreq = 0;
      }
    }

    if (currentFreq != lastFreq) {
      if (currentFreq > 0) {
        tone(PIN_BUZZER, currentFreq);
      } else {
        noTone(PIN_BUZZER);
      }
      lastFreq = currentFreq;
    }

    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

// 前方宣言
void showLogo();
void startGame();
void spawnNewPiece();
bool collision();
bool moveDown();
void lockPiece();
void clearLines();
void handleInput();
void drawAll();
void drawBoard();
void drawCurrentPiece();
void drawNext();
void drawUI();
void drawGameOver();
void placeGarbage();

unsigned long graceUntil = 0;   // この時刻まで自動落下しない

void setup() {
  Serial.begin(115200);
  Serial.println("hello. esp32_oled_tetris.");

  pinMode(PIN_DOWN, INPUT_PULLUP);
  pinMode(PIN_LEFT, INPUT_PULLUP);
  pinMode(PIN_RIGHT, INPUT_PULLUP);
  pinMode(PIN_ROTATE, INPUT_PULLUP);

  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  pinMode(PIN_BUZZER, OUTPUT);
  bgmTotalDuration = 0;
  for (int i = 0; i < tetorisu_len; i++) {
    bgmTotalDuration += tetorisu[i].duration + len0;
  }

  xTaskCreate(play_melody_thread, "play_melody_thread", 2048, NULL, 1, NULL);

  u8g2.begin();
  u8g2.setBusClock(400000);
  u8g2.setFlipMode(0);
  u8g2.setContrast(60);
  u8g2.setFont(u8g2_font_5x7_tf);

  delay(1000);
  getBattVoltage();

  showLogo();
}

float getBattVoltage() {
  int readv = analogRead(PIN_VOLTAGE);
  const float VOLT = 6.6;
  const float ANALOG_MAX = 4096.0;
  float voltage = ((float)readv * VOLT) / ANALOG_MAX;
  return voltage;
}

void loop() {
  if (!started) {
    static unsigned long lastModeChange = 0;
    if (millis() - lastModeChange > 200) {
      if (digitalRead(PIN_LEFT) == LOW || digitalRead(PIN_RIGHT) == LOW) {
        modeB = !modeB;
        showLogo();
        lastModeChange = millis();
      }
    }
    if (digitalRead(PIN_ROTATE) == LOW) {
      delay(200);
      startGame();
    }
    return;
  }

  if (gameOver) {
    drawGameOver();
    return;
  }

  handleInput();

  // 自動落下（猶予中はスキップ）
  if (millis() >= graceUntil) {
    bool down = (digitalRead(PIN_DOWN) == LOW);
    unsigned long currentInterval = down ? 40 : dropInterval;

    if (millis() - dropTimer > currentInterval) {
      dropTimer = millis();
      if (!moveDown()) lockPiece();
    }
  }
  drawAll();
  u8g2.sendBuffer();
  delay(20);
}

// ================== コア関数 ==================
void showLogo() {
  u8g2.clearBuffer();

  u8g2.drawStr(17, 40, "TETRIS");
  u8g2.drawStr(14, 55, "Press A");
  u8g2.drawStr(12, 70, "to start");

  u8g2.setCursor(12, 90);
  u8g2.print("MODE:");
  u8g2.print(modeB ? "B" : "A");

  u8g2.setCursor(8, 120);
  u8g2.print("batt ");
  u8g2.print(getBattVoltage(), 2);
  u8g2.print("V");

  u8g2.sendBuffer();
}

void placeGarbage() {
  for (int y = BOARD_H - 10; y < BOARD_H; y++) {
    int count = random(1, 10);  // 1〜9
    bool used[BOARD_W] = {false};
    for (int i = 0; i < count; i++) {
      int x;
      do {
        x = random(BOARD_W);
      } while (used[x]);
      used[x] = true;
      board[y][x] = 1;
    }
  }
}

void startGame() {
  randomSeed(millis());
  memset(board, 0, sizeof(board));
  score = 0;
  level = 0;

  if (modeB) {
    dropInterval = B_MODE_INTERVAL;
    placeGarbage();
    graceUntil = millis() + 1000;   // 1000msの初見猶予
  } else {
    dropInterval = 500;
    graceUntil = 0;
  }

  nextPiece = random(7);
  spawnNewPiece();
  started = true;
  gameOver = false;
  dropTimer = millis();
  bgmStartTime = millis();
}

void spawnNewPiece() {
  currentPiece = nextPiece;
  nextPiece = random(7);
  rotation = 0;
  posX = BOARD_W / 2 - 2;
  posY = 0;
  if (collision()) {
    gameOver = true;
    playSE(gameover_melody, gameover_melody_len);
  }
}

bool collision() {
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      if (tetrominoes[currentPiece][rotation][y][x]) {
        int bx = posX + x;
        int by = posY + y;
        if (bx < 0 || bx >= BOARD_W || by >= BOARD_H) return true;
        if (by >= 0 && board[by][bx]) return true;
      }
    }
  }
  return false;
}

bool moveDown() {
  posY++;
  if (collision()) { posY--; return false; }
  return true;
}

void lockPiece() {
  static const Note se_lock[] = {{tone_do1, 30}};
  playSE(se_lock, 1);
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      if (tetrominoes[currentPiece][rotation][y][x]) {
        int by = posY + y;
        if (by >= 0) board[by][posX + x] = currentPiece + 1;
      }
    }
  }
  clearLines();
  spawnNewPiece();
}

void clearLines() {
  bool lineFull[BOARD_H] = {false};
  int lines = 0;

  for (int y = 0; y < BOARD_H; y++) {
    bool full = true;
    for (int x = 0; x < BOARD_W; x++) {
      if (!board[y][x]) { full = false; break; }
    }
    if (full) {
      lineFull[y] = true;
      lines++;
    }
  }

  if (lines == 0) return;

  if (lines == 4) {
    playSE(clear_lines4, clear_lines_len4);
  } else {
    playSE(clear_lines, clear_lines_len);
  }

  isClearingLines = true;

  int tempBoard[BOARD_H][BOARD_W];
  memcpy(tempBoard, board, sizeof(board));

  for (int i = 0; i < 3; i++) {
    for (int y = 0; y < BOARD_H; y++) {
      if (lineFull[y]) {
        for (int x = 0; x < BOARD_W; x++) board[y][x] = 0;
      }
    }
    digitalWrite(PIN_LED, LOW);
    drawAll();
    u8g2.sendBuffer();
    delay(100);

    memcpy(board, tempBoard, sizeof(board));
    digitalWrite(PIN_LED, HIGH);
    drawAll();
    u8g2.sendBuffer();
    delay(100);
  }

  digitalWrite(PIN_LED, LOW);
  isClearingLines = false;

  for (int y = BOARD_H - 1; y >= 0; y--) {
    bool full = true;
    for (int x = 0; x < BOARD_W; x++) {
      if (!board[y][x]) { full = false; break; }
    }
    if (full) {
      for (int yy = y; yy > 0; yy--) {
        for (int x = 0; x < BOARD_W; x++) board[yy][x] = board[yy - 1][x];
      }
      memset(board[0], 0, BOARD_W * sizeof(int));
      y++;
    }
  }

  if (lines) {
    score += lines;
    if (!modeB && score / 10 > level) {
      level = score / 10;
      dropInterval = max(100, 500 - level * 30);
    }
  }
}

void handleInput() {
  static unsigned long last = 0;
  if (millis() - last < 130) return;

  if (digitalRead(PIN_LEFT) == LOW)  { posX--; if (collision()) posX++; last = millis(); }
  if (digitalRead(PIN_RIGHT) == LOW) { posX++; if (collision()) posX--; last = millis(); }
  if (digitalRead(PIN_ROTATE) == LOW) {
    tryRotate();
    last = millis();
  }
}

void tryRotate() {
  int oldRot = rotation;
  rotation = (rotation + 1) % 4;
  if (!collision()) return;          // そのまま成功

  // 右に1マスキック
  posX++;
  if (!collision()) return;

  // 左に1マスキック（元位置から）
  posX -= 2;
  if (!collision()) return;

  // どちらもダメなら元に戻す
  posX++;
  rotation = oldRot;
}

// ================== 描画 ==================
void drawAll() {
  u8g2.clearBuffer();
  drawBoard();
  drawCurrentPiece();
  drawNext();
  drawUI();
}

void drawBoard() {
  u8g2.drawFrame(BOARD_X - 2, BOARD_Y - 1, BOARD_W * CELL + 3, BOARD_H * CELL + 2);

  for (int y = 0; y < BOARD_H; y++) {
    for (int x = 0; x < BOARD_W; x++) {
      if (board[y][x]) {
        int px = BOARD_X + x * CELL;
        int py = BOARD_Y + y * CELL;
        u8g2.drawBox(px, py, BLOCK_SIZE, BLOCK_SIZE);
      }
    }
  }
}

void drawCurrentPiece() {
  if (isClearingLines) return;

  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      if (tetrominoes[currentPiece][rotation][y][x]) {
        int px = BOARD_X + (posX + x) * CELL;
        int py = BOARD_Y + (posY + y) * CELL;
        u8g2.drawBox(px, py, BLOCK_SIZE, BLOCK_SIZE);
      }
    }
  }
}

void drawNext() {
  const int pos_y = 6;
  const int pos_x = 38;

  const int NEXT_BLOCK = 4;
  const int NEXT_GAP   = 1;
  const int NEXT_CELL  = NEXT_BLOCK + NEXT_GAP;

  int offset_y = next_offset_y[nextPiece];
  for (int y = 0; y < 2; y++) {
    for (int x = 0; x < 4; x++) {
      if (tetrominoes[nextPiece][0][y + offset_y][x]) {
        u8g2.drawBox(pos_x + x * NEXT_CELL,
                     pos_y + y * NEXT_CELL,
                     NEXT_BLOCK, NEXT_BLOCK);
      }
    }
  }
}

void drawUI() {
  u8g2.setCursor(6, 14);
  u8g2.print("L:");
  u8g2.print(score);
}

void drawGameOver() {
  u8g2.clearBuffer();

  u8g2.drawStr(22, 50, "GAME");
  u8g2.drawStr(22, 65, "OVER");

  u8g2.setCursor(17, 90);
  u8g2.print("LINES:");

  char buf[8];
  snprintf(buf, sizeof(buf), "%d", score);
  int scoreWidth = strlen(buf) * 5;
  u8g2.setCursor((64 - scoreWidth) / 2, 105);
  u8g2.print(score);

  u8g2.sendBuffer();

  if (digitalRead(PIN_ROTATE) == LOW) {
    delay(200);
    started = false;
    showLogo();
  }
}