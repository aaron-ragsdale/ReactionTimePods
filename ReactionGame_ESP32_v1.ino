// ============================================================
// REACTION TIME GAME v11 — ESP32-WROOM-32, 38-PIN BREAKOUT
// USB-C facing UP; physical breakout rows numbered 1–19 downwards.
// IMPORTANT: Row positions assume the COMMON 38-pin DevKit layout.
// Verify the GPIO/power labels on your exact board before wiring.
//
// BUTTON     PRESS GPIO / BREAKOUT         NEOPIXEL DIN / BREAKOUT
//    1       GPIO18 / LEFT ROW 11           GPIO13 / RIGHT ROW 5
//    2       GPIO19 / LEFT ROW 12           GPIO25 / RIGHT ROW 10
//    3       GPIO21 / LEFT ROW 14           GPIO27 / RIGHT ROW 8
//    4       GPIO22 / LEFT ROW 17           GPIO33 / RIGHT ROW 11
//
// Each button switch: GPIO -> switch -> GND (internal pull-up).
// Each strip: DIN on designated GPIO; 5V from a suitable EXTERNAL
// regulated 5V power supply; LED GND and ESP32 GND MUST connect.
// Strongly recommend a 74AHCT125 logic level shifter for LED data.
//
// Common 38-pin breakout layout (USB-C up, viewed from module side):
// ROW | LEFT             | RIGHT
//   1 | GPIO6 (flash)    | VIN / 5V
//   2 | GPIO7 (flash)    | GPIO11 (flash)
//   3 | GPIO8 (flash)    | GPIO10 (flash)
//   4 | GPIO15           | GPIO9  (flash)
//   5 | GPIO2            | GPIO13  <-- LED 1
//   6 | GPIO0            | GPIO12
//   7 | GPIO4            | GPIO14
//   8 | GPIO16           | GPIO27  <-- LED 3
//   9 | GPIO17           | GPIO26
//  10 | GPIO5            | GPIO25  <-- LED 2
//  11 | GPIO18 <-- BTN 1 | GPIO33  <-- LED 4
//  12 | GPIO19 <-- BTN 2 | GPIO32
//  13 | GND              | GPIO35
//  14 | GPIO21 <-- BTN 3 | GPIO34
//  15 | GPIO3 / RX0      | GPIO39
//  16 | GPIO1 / TX0      | GPIO36
//  17 | GPIO22 <-- BTN 4 | EN
//  18 | GPIO23           | 3V3
//  19 | GND              | GND
//
// Startup LED diagnostic: flashes each button red, green, blue
// once before joining WiFi. Does NOT change game timing or scoring.
// Game: one green, calmer red/blue transitions, >=500ms cooldown,
// brightness 170/255, AP dashboard and CSV session history.
// ============================================================

#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>

// GPIO numbers DO NOT equal physical breakout row numbers.
// BTN 1=L11, BTN 2=L12, BTN 3=L14, BTN 4=L17.
const uint8_t BTN_PINS[4] = { 18, 19, 21, 22 };
// LED 1=R5, LED 2=R10, LED 3=R8, LED 4=R11.
const uint8_t LED_PINS[4] = { 13, 25, 27, 33 };
#define NEO_COUNT      20  // LEDs per button; edit if different
#define NEO_BRIGHTNESS 230 // 0..255; brighter than v7 (80); use adequate 5V power
#define NEO_TYPE       (NEO_GRB + NEO_KHZ800)

// ============================================================
//  GAME CONFIG
// ============================================================
const char* ssid     = "ReactionGame";
const char* password = "12345678";

#define NUM_BUTTONS   4
#define MAX_CORRECT   10
#define MAX_HISTORY   50

// How long green stays lit before it's a miss (ms)
const unsigned long GREEN_MIN_MS = 1000;
const unsigned long GREEN_MAX_MS = 3000;

// Random delay BETWEEN rounds before green appears (ms)
// Buttons show red/blue during this time — no green.
// WAIT_MIN_MS >= 500 guarantees >= 0.5s even after a fast press.
const unsigned long COOLDOWN_MIN_MS = 500; // HARD minimum after hit / timeout
const unsigned long WAIT_MIN_MS = 800;     // actual delay ranges 800..4999 ms
const unsigned long WAIT_MAX_MS = 5000;    // must exceed WAIT_MIN_MS

// ============================================================
//  COLORS
// ============================================================
#define C_OFF   0
#define C_RED   1
#define C_GREEN 2
#define C_BLUE  3

uint32_t neoColorTable[4];

// Each non-green button flips red <-> blue independently.
// Slower visual changes do NOT alter the cooldown or green window.
const unsigned long DISTRACTOR_MIN_MS = 600;
const unsigned long DISTRACTOR_MAX_MS = 1200; // exclusive upper bound; 600-1199 ms
uint8_t distractorColor[NUM_BUTTONS];
unsigned long nextDistractorChangeMs[NUM_BUTTONS];

// ============================================================
//  HARDWARE
// ============================================================
Adafruit_NeoPixel ring0(NEO_COUNT, LED_PINS[0], NEO_TYPE);
Adafruit_NeoPixel ring1(NEO_COUNT, LED_PINS[1], NEO_TYPE);
Adafruit_NeoPixel ring2(NEO_COUNT, LED_PINS[2], NEO_TYPE);
Adafruit_NeoPixel ring3(NEO_COUNT, LED_PINS[3], NEO_TYPE);
Adafruit_NeoPixel* rings[NUM_BUTTONS] = { &ring0, &ring1, &ring2, &ring3 };
WebServer server(80);

// ============================================================
//  LED CONTROL
// ============================================================
void setButtonColor(uint8_t btn, uint8_t color) {
  if (btn >= NUM_BUTTONS) return;
  Adafruit_NeoPixel& r = *rings[btn];
  uint32_t c = neoColorTable[color];
  for (uint16_t i = 0; i < r.numPixels(); i++) r.setPixelColor(i, c);
  r.show();
}

void allOff() {
  for (uint8_t i = 0; i < NUM_BUTTONS; i++) setButtonColor(i, C_OFF);
}

// Initialize the independent red/blue distractors.
// Every subsequent tick changes the actual displayed color, not just a random pick.
void setAllDistractors() {
  unsigned long now = millis();
  for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
    distractorColor[i] = random(0, 2) == 0 ? C_RED : C_BLUE;
    setButtonColor(i, distractorColor[i]);
    nextDistractorChangeMs[i] = now + random(DISTRACTOR_MIN_MS, DISTRACTOR_MAX_MS);
  }
}


// Startup hardware check: each strip lights R/G/B in sequence.
// If these do not light, check 5V, shared GND, DIN orientation and
// the actual GPIO labels on the ESP32 board (not row numbers alone).
void testLEDsAtStartup() {
  for (uint8_t b = 0; b < NUM_BUTTONS; b++) {
    Serial.printf("Testing button %u LEDs on GPIO %u\n", b + 1, LED_PINS[b]);
    setButtonColor(b, C_RED);   delay(220);
    setButtonColor(b, C_GREEN); delay(220);
    setButtonColor(b, C_BLUE);  delay(220);
    setButtonColor(b, C_OFF);
  }
}

// ============================================================
//  DEBOUNCE
// ============================================================
bool          lastBtnReading[NUM_BUTTONS];
bool          stableBtnState[NUM_BUTTONS];
unsigned long lastDebounceMs[NUM_BUTTONS];
const unsigned long DEBOUNCE_MS = 30;

bool buttonPressed(uint8_t i) {
  bool reading = digitalRead(BTN_PINS[i]);
  if (reading != lastBtnReading[i]) lastDebounceMs[i] = millis();
  if ((millis() - lastDebounceMs[i]) > DEBOUNCE_MS) {
    if (reading != stableBtnState[i]) {
      stableBtnState[i] = reading;
      if (stableBtnState[i] == LOW) {
        lastBtnReading[i] = reading;
        return true;
      }
    }
  }
  lastBtnReading[i] = reading;
  return false;
}

// ============================================================
//  GAME STATE MACHINE
// ============================================================
enum GameState {
  IDLE,     // waiting for web Start
  WAITING,  // between rounds — random delay before green appears
  ACTIVE    // green is lit, waiting for button press
};
GameState gameState = IDLE;

int           activeButton    = -1;  // which button is green (-1 = none)
int           previousButton  = -1;  // last green button (prevent repeat)
unsigned long waitStartMs     = 0;   // when the WAITING state began
unsigned long waitDurationMs  = 0;   // random wait duration
unsigned long greenStartMs    = 0;   // when green lit up
unsigned long greenDurationMs = 0;   // how long green stays on

int roundNumber  = 0;
int correctCount = 0;
int missCount    = 0;

unsigned long lastReactionMs  = 0;
unsigned long bestReactionMs  = 0;
unsigned long totalReactionMs = 0;

String statusMessage = "Press Start Game to begin.";

// Called continuously while waiting and while a green target is active.
// The green target is excluded and remains solid green until hit/timeout.
void updateDistractors() {
  unsigned long now = millis();
  for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
    if (gameState == ACTIVE && (int)i == activeButton) continue;
    if ((int32_t)(now - nextDistractorChangeMs[i]) >= 0) {
      distractorColor[i] = (distractorColor[i] == C_RED) ? C_BLUE : C_RED;
      setButtonColor(i, distractorColor[i]);
      nextDistractorChangeMs[i] = now + random(DISTRACTOR_MIN_MS, DISTRACTOR_MAX_MS);
    }
  }
}



// ============================================================
//  HISTORY
// ============================================================
struct GameResult {
  uint16_t gameNumber;
  uint32_t avgMs, bestMs;
  uint16_t correct, misses;
  uint8_t  accuracy;
  char     timestamp[9];
};
GameResult history[MAX_HISTORY];
uint8_t    historyCount = 0;
uint16_t   gameNumber   = 0;

String uptimeStr() {
  unsigned long s = millis() / 1000;
  unsigned long h = s / 3600; s %= 3600;
  unsigned long m = s / 60;   s %= 60;
  char buf[9]; snprintf(buf, 9, "%02lu:%02lu:%02lu", h, m, s);
  return String(buf);
}

void saveResult() {
  if (historyCount >= MAX_HISTORY) {
    for (uint8_t i = 0; i < MAX_HISTORY - 1; i++) history[i] = history[i+1];
    historyCount = MAX_HISTORY - 1;
  }
  uint32_t avg = correctCount > 0 ? totalReactionMs / correctCount : 0;
  int      tot = correctCount + missCount;
  uint8_t  acc = tot > 0 ? (correctCount * 100) / tot : 0;
  GameResult r;
  r.gameNumber = ++gameNumber;
  r.avgMs = avg; r.bestMs = bestReactionMs;
  r.correct = correctCount; r.misses = missCount; r.accuracy = acc;
  String ts = uptimeStr();
  strncpy(r.timestamp, ts.c_str(), 8);
  r.timestamp[8] = '\0';
  history[historyCount++] = r;
}

// ============================================================
//  ROUND TRANSITIONS
// ============================================================

// Pick a random button that isn't the same as last time
uint8_t randomNonRepeat() {
  if (NUM_BUTTONS == 1) return 0;
  uint8_t pick;
  do { pick = random(0, NUM_BUTTONS); } while ((int)pick == previousButton);
  return pick;
}

// Enter the WAITING state — show distractors, start random delay
void beginWaiting() {
  gameState       = WAITING;
  // Remove the green immediately before assigning distractor colors.
  if (activeButton >= 0) setButtonColor(activeButton, C_OFF);
  activeButton    = -1;
  setAllDistractors();
  waitDurationMs  = max(COOLDOWN_MIN_MS, (unsigned long)random(WAIT_MIN_MS, WAIT_MAX_MS));
  waitStartMs     = millis();
  Serial.printf("  Waiting %lu ms before next green\n", waitDurationMs);
}

// Enter the ACTIVE state — light one button green
void beginActive() {
  activeButton    = randomNonRepeat();
  previousButton  = activeButton; // remember actual target, prevent immediate repeat
  gameState       = ACTIVE;

  // Keep each existing distractor cycling on its own schedule.
  // The selected target alone becomes green.
  setButtonColor(activeButton, C_GREEN);

  greenDurationMs = random(GREEN_MIN_MS, GREEN_MAX_MS);
  greenStartMs    = millis();
  roundNumber++;
  statusMessage   = "Press now!";
  Serial.printf("Round %d — button %d green for %lu ms\n",
                roundNumber, activeButton + 1, greenDurationMs);
}

// ============================================================
//  GAME FLOW
// ============================================================
void startGame() {
  gameState      = IDLE;   // reset first
  roundNumber    = correctCount = missCount = 0;
  lastReactionMs = bestReactionMs = totalReactionMs = 0;
  activeButton   = previousButton = -1;
  statusMessage  = "Get ready...";
  beginWaiting();          // first round starts after a wait
}

void endGame() {
  saveResult();
  gameState = IDLE;
  allOff();
  unsigned long avg = correctCount > 0 ? totalReactionMs / correctCount : 0;
  statusMessage = "Done! Avg: " + String(avg) + " ms";
  Serial.println("=== Game over ===");
}

void stopGame() {
  gameState = IDLE;
  allOff();
  statusMessage = "Game stopped.";
}

void resetGame() {
  gameState      = IDLE;
  roundNumber    = correctCount = missCount = 0;
  lastReactionMs = bestReactionMs = totalReactionMs = 0;
  activeButton   = previousButton = -1;
  statusMessage  = "Press Start Game to begin.";
  allOff();
}

void handleCorrectPress() {
  lastReactionMs    = millis() - greenStartMs;
  correctCount++;
  totalReactionMs  += lastReactionMs;
  if (bestReactionMs == 0 || lastReactionMs < bestReactionMs)
    bestReactionMs  = lastReactionMs;
  statusMessage     = "Correct! " + String(lastReactionMs) + " ms";
  Serial.printf("  Hit! %lu ms (%d/%d)\n", lastReactionMs, correctCount, MAX_CORRECT);
  if (correctCount >= MAX_CORRECT) endGame();
  else beginWaiting();   // wait before showing next green
}

void handleWrongPress(uint8_t btn) {
  missCount++;
  lastReactionMs = 0;
  statusMessage  = "Miss! Press the green button.";
  Serial.printf("  Wrong button %d (green is %d)\n", btn + 1, activeButton + 1);
  // Stay in ACTIVE — player can still press the correct button
}

void handleGreenExpired() {
  missCount++;
  lastReactionMs = 0;
  statusMessage  = "Too slow!";
  Serial.println("  Green expired — miss");
  beginWaiting();        // wait before showing next green
}

// ============================================================
//  MAIN LOOP
// ============================================================
void loop() {
  server.handleClient();

  // --- IDLE ---
  if (gameState == IDLE) {
    // Keep all four LED rings OFF between games.
    return;
  }

  // --- WAITING: random pause before green appears ---
  if (gameState == WAITING) {
    updateDistractors();
    // Allow presses during wait — counts as early/wrong press
    for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
      if (buttonPressed(i)) {
        missCount++;
        statusMessage = "Too early!";
        Serial.println("  Pressed during wait — miss");
        // Reset the wait timer so they can't spam through it
        waitStartMs = millis();
        return;
      }
    }
    if (millis() - waitStartMs >= waitDurationMs) {
      beginActive();   // wait over — light up green
    }
    return;
  }

  // --- ACTIVE: green is lit ---
  if (gameState == ACTIVE) {
    updateDistractors();
    for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
      if (buttonPressed(i)) {
        if ((int)i == activeButton) handleCorrectPress();
        else                        handleWrongPress(i);
        return;
      }
    }
    // Green timed out
    if (millis() - greenStartMs >= greenDurationMs) {
      handleGreenExpired();
    }
  }
}

// ============================================================
//  WEB HANDLERS
// ============================================================
void handleRoot()  { server.send(200, "text/html",   htmlPage()); }
void handleStart() { startGame(); server.send(200, "text/plain", "ok"); }
void handleStop()  { stopGame();  server.send(200, "text/plain", "ok"); }
void handleReset() { resetGame(); server.send(200, "text/plain", "ok"); }

void handleState() {
  unsigned long avg = correctCount > 0 ? totalReactionMs / correctCount : 0;
  int tot = correctCount + missCount;
  int acc = tot > 0 ? (correctCount * 100) / tot : 0;
  const char* gs = gameState == ACTIVE  ? "Running" :
                   gameState == WAITING ? "Running" : "Idle";
  String j = "{";
  j += "\"gameState\":\""         + String(gs)           + "\",";
  j += "\"status\":\""            + statusMessage        + "\",";
  j += "\"roundNumber\":"         + String(roundNumber)  + ",";
  j += "\"correctCount\":"        + String(correctCount) + ",";
  j += "\"missCount\":"           + String(missCount)    + ",";
  j += "\"lastReactionTime\":"    + String(lastReactionMs) + ",";
  j += "\"bestReactionTime\":"    + String(bestReactionMs) + ",";
  j += "\"averageReactionTime\":" + String(avg)          + ",";
  j += "\"accuracy\":"            + String(acc);
  j += "}";
  server.send(200, "application/json", j);
}

void handleHistory() {
  String j = "{\"results\":[";
  for (uint8_t i = 0; i < historyCount; i++) {
    if (i > 0) j += ",";
    j += "{\"game\":"    + String(history[i].gameNumber)
      + ",\"avg\":"      + String(history[i].avgMs)
      + ",\"best\":"     + String(history[i].bestMs)
      + ",\"correct\":"  + String(history[i].correct)
      + ",\"misses\":"   + String(history[i].misses)
      + ",\"accuracy\":" + String(history[i].accuracy)
      + ",\"time\":\""   + String(history[i].timestamp) + "\"}";
  }
  j += "]}";
  server.send(200, "application/json", j);
}

void handleDownload() {
  if (historyCount == 0) { server.send(204, "text/plain", "No data"); return; }
  server.sendHeader("Content-Disposition", "attachment; filename=\"reaction_history.csv\"");
  String csv = "game,avg_ms,best_ms,correct,misses,accuracy_pct,time\n";
  for (uint8_t i = 0; i < historyCount; i++) {
    csv += String(history[i].gameNumber) + ","
         + String(history[i].avgMs)      + ","
         + String(history[i].bestMs)     + ","
         + String(history[i].correct)    + ","
         + String(history[i].misses)     + ","
         + String(history[i].accuracy)   + ","
         + String(history[i].timestamp)  + "\n";
  }
  server.send(200, "text/csv", csv);
}

// ============================================================
//  HTML PAGE
// ============================================================
String htmlPage() {
  return R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Reaction Game</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
:root{--bg:#0d1016;--s:#191e29;--s2:#242b38;--b:#394455;--g:#45f6a0;--r:#ff737b;--t:#ffffff;--m:#d1d8e5;--amber:#ffd16d;--font:system-ui,-apple-system,'Segoe UI',Roboto,Arial,sans-serif}
body{background:var(--bg);color:var(--t);font-family:var(--font);font-size:18px;line-height:1.45;min-height:100vh;display:flex;flex-direction:column}
.hdr{padding:1rem 1.5rem;border-bottom:1px solid var(--b);display:flex;align-items:center;justify-content:space-between;gap:12px}
.h-left{display:flex;align-items:center;gap:12px}
.pip{width:13px;height:13px;border-radius:50%;background:#8390a6;transition:background .25s}
.pip.live{background:var(--g)}
.h-title{font-size:22px;font-weight:800;letter-spacing:.025em;color:var(--t)}
.h-right{font-size:16px;color:var(--m);font-weight:600}
.btn-zone{padding:1.25rem 1.5rem;border-bottom:1px solid var(--b);display:grid;grid-template-columns:2fr 1fr 1fr;gap:12px}
.btn{font-family:var(--font);font-weight:800;border-radius:12px;cursor:pointer;border:2px solid;display:flex;align-items:center;justify-content:center;gap:8px;min-height:66px;transition:opacity .15s}
.btn-start{background:var(--g);color:#07110c;border-color:var(--g);font-size:22px;padding:1rem}
.btn-start:hover{opacity:.9}.btn-start:disabled{opacity:.4;pointer-events:none}
.btn-stop{background:transparent;color:var(--amber);border-color:var(--amber);font-size:20px;padding:.75rem}
.btn-reset{background:transparent;color:var(--t);border-color:#9baac0;font-size:20px;padding:.75rem}
.main{flex:1;padding:1.5rem;display:flex;flex-direction:column;gap:1rem;width:100%;max-width:1000px;margin:0 auto}
@media(min-width:700px){.main{display:grid;grid-template-columns:1fr 1fr;align-content:start;gap:1rem}.prog-wrap,.result-pill,.gameover,.history-section{grid-column:1/3}.rx-card{grid-column:1/2}.stat-grid{grid-column:2/3}}
.prog-wrap,.rx-card,.stat,.history-wrap{background:var(--s);border:1px solid var(--b);border-radius:12px}
.prog-wrap{padding:1.25rem 1.5rem}
.prog-hdr{display:flex;justify-content:space-between;align-items:center;gap:12px;margin-bottom:.8rem}
.prog-label{font-size:17px;color:var(--m);font-weight:650}
.prog-count{font-size:22px;font-weight:800}
.prog-track{height:12px;background:var(--s2);border-radius:6px;display:flex;gap:4px}
.prog-seg{flex:1;background:#3b4656;border-radius:4px;transition:background .2s}
.prog-seg.done{background:var(--g)}
.result-pill{display:flex;align-items:center;justify-content:center;gap:12px;padding:1.1rem 1.25rem;border-radius:12px;border:2px solid var(--b);background:var(--s);font-size:clamp(20px,3.4vw,28px);font-weight:800;color:var(--t);min-height:78px;text-align:center}
.result-pill.hit{border-color:var(--g);background:#163b2c;color:var(--g)}
.result-pill.miss{border-color:var(--r);background:#40242b;color:var(--r)}
.result-dot{width:14px;height:14px;border-radius:50%;background:currentColor;flex-shrink:0}
.rx-card{padding:1.5rem;text-align:center;display:flex;flex-direction:column;justify-content:center}
.rx-num{font-size:clamp(70px,11vw,108px);font-weight:850;line-height:1.1;color:var(--m)}
.rx-num.hit{color:var(--g)}.rx-num.miss{color:var(--r)}
.rx-unit{font-size:17px;color:var(--m);margin-top:10px;font-weight:650}
.rx-divider{height:1px;background:var(--b);margin:18px 0}
.rx-row{display:flex;justify-content:space-around;gap:12px}
.rx-sub{text-align:center}.rx-sv{font-size:27px;font-weight:800;color:var(--t)}
.rx-sl{font-size:16px;color:var(--m);margin-top:4px;font-weight:650}
.stat-grid{display:grid;grid-template-columns:1fr 1fr;gap:10px}
.stat{padding:1.1rem .75rem;text-align:center;display:flex;flex-direction:column;justify-content:center;min-height:124px}
.stat-n{font-size:clamp(38px,6vw,55px);font-weight:850;line-height:1.15}
.stat-n.g{color:var(--g)}.stat-n.r{color:var(--r)}
.stat-l{font-size:16px;color:var(--m);margin-top:7px;font-weight:700}
.gameover{display:none;background:var(--s);border:2px solid var(--g);border-radius:12px;padding:1.5rem;text-align:center}
.gameover.show{display:block}
.go-title{font-size:18px;color:var(--m);font-weight:700}
.go-score{font-size:64px;font-weight:850;color:var(--g)}
.go-sub{font-size:18px;color:var(--m)}
.history-section{display:flex;flex-direction:column;gap:.75rem}
.section-hdr{display:flex;align-items:center;justify-content:space-between;gap:10px;flex-wrap:wrap}
.section-lbl{font-size:19px;font-weight:800;color:var(--t)}
.dl-btn{font:700 15px var(--font);background:transparent;color:var(--g);border:2px solid var(--g);border-radius:8px;padding:.6rem 1rem;text-decoration:none}
.dl-btn.disabled{opacity:.4;pointer-events:none}
.history-wrap{overflow-x:auto}.history-inner{padding:.6rem}
.history-table{width:100%;border-collapse:collapse;font-size:17px;min-width:490px}
.history-table th{text-align:left;padding:.8rem .65rem;color:var(--m);font-size:15px;border-bottom:1px solid var(--b);font-weight:750}
.history-table td{padding:.8rem .65rem;border-bottom:1px solid var(--b);color:var(--t)}
.history-table tr:last-child td{border-bottom:none}
.empty-msg{padding:1.5rem;text-align:center;font-size:18px;color:var(--m)}
.badge{display:inline-block;font-size:15px;font-weight:800;padding:3px 9px;border-radius:5px}
.badge-g{background:#194734;color:var(--g)}.badge-r{background:#4b2930;color:var(--r)}
.ftr{padding:.9rem 1.5rem;border-top:1px solid var(--b);display:flex;justify-content:space-between;gap:10px;font-size:15px;color:var(--m)}
@media(max-width:600px){.hdr{padding:.9rem 1rem}.btn-zone{padding:1rem;gap:8px;grid-template-columns:1.5fr 1fr 1fr}.btn{min-height:58px}.btn-start,.btn-stop,.btn-reset{font-size:16px;padding:.6rem .3rem}.main{padding:1rem}.result-pill{font-size:20px}.stat-l{font-size:15px}.rx-sv{font-size:23px}}
</style>
</head>
<body>
<div class="hdr">
  <div class="h-left"><div class="pip" id="pip"></div><span class="h-title">Reaction Game</span></div>
  <span class="h-right">ESP32</span>
</div>
<div class="btn-zone">
  <button class="btn btn-start" id="btn-start" onclick="startGame()">&#9654; Start Game</button>
  <button class="btn btn-stop" onclick="stopGame()">&#9632; Stop</button>
  <button class="btn btn-reset" onclick="resetGame()">&#8635; Reset</button>
</div>
<div class="main">
  <div class="prog-wrap">
    <div class="prog-hdr"><span class="prog-label">Progress</span><span class="prog-count"><span id="prog-n">0</span> / 10 correct</span></div>
    <div class="prog-track">
      <div class="prog-seg" id="s0"></div><div class="prog-seg" id="s1"></div><div class="prog-seg" id="s2"></div><div class="prog-seg" id="s3"></div><div class="prog-seg" id="s4"></div>
      <div class="prog-seg" id="s5"></div><div class="prog-seg" id="s6"></div><div class="prog-seg" id="s7"></div><div class="prog-seg" id="s8"></div><div class="prog-seg" id="s9"></div>
    </div>
  </div>
  <div class="result-pill" id="result-pill"><div class="result-dot"></div><span id="result-text">Waiting for first round</span></div>
  <div class="rx-card">
    <div class="rx-num" id="rx-num">&#8212;</div>
    <div class="rx-unit">Last reaction &bull; ms</div>
    <div class="rx-divider"></div>
    <div class="rx-row">
      <div class="rx-sub"><div class="rx-sv" id="rx-best">&#8212;</div><div class="rx-sl">Best</div></div>
      <div class="rx-sub"><div class="rx-sv" id="rx-avg">&#8212;</div><div class="rx-sl">Average</div></div>
    </div>
  </div>
  <div class="stat-grid">
    <div class="stat"><div class="stat-n g" id="st-hits">0</div><div class="stat-l">Correct</div></div>
    <div class="stat"><div class="stat-n r" id="st-miss">0</div><div class="stat-l">Misses</div></div>
    <div class="stat"><div class="stat-n" id="st-acc">&#8212;</div><div class="stat-l">Accuracy</div></div>
    <div class="stat"><div class="stat-n" id="st-rounds">0</div><div class="stat-l">Rounds</div></div>
  </div>
  <div class="gameover" id="gameover">
    <div class="go-title">Game complete</div>
    <div class="go-score" id="go-avg">&#8212;</div>
    <div class="go-sub">average reaction time across 10 hits</div>
  </div>
  <div class="history-section">
    <div class="section-hdr">
      <span class="section-lbl">Session history</span>
      <a class="dl-btn disabled" id="dl-btn" href="/download" download="reaction_history.csv">&#8595; Download CSV</a>
    </div>
    <div class="history-wrap"><div class="history-inner">
      <div class="empty-msg" id="history-empty">No games played yet</div>
      <table class="history-table" id="history-table" style="display:none">
        <thead><tr><th>#</th><th>Avg ms</th><th>Best ms</th><th>Accuracy</th><th>Time</th></tr></thead>
        <tbody id="history-body"></tbody>
      </table>
    </div></div>
  </div>
</div>
<div class="ftr"><span>Polling 150ms</span><span id="ftr-time">&#8212;</span></div>
<script>
var segs=[];for(var i=0;i<10;i++)segs.push(document.getElementById('s'+i));
function startGame(){fetch('/start');document.getElementById('btn-start').disabled=true;}
function stopGame(){fetch('/stop');}
function resetGame(){
  fetch('/reset');
  document.getElementById('btn-start').disabled=false;
  document.getElementById('gameover').className='gameover';
  document.getElementById('rx-num').className='rx-num';
  document.getElementById('rx-num').innerHTML='&#8212;';
  document.getElementById('rx-best').innerHTML='&#8212;';
  document.getElementById('rx-avg').innerHTML='&#8212;';
  document.getElementById('st-hits').textContent='0';
  document.getElementById('st-miss').textContent='0';
  document.getElementById('st-acc').innerHTML='&#8212;';
  document.getElementById('st-rounds').textContent='0';
  document.getElementById('prog-n').textContent='0';
  document.getElementById('result-pill').className='result-pill';
  document.getElementById('result-text').textContent='Waiting for first round';
  document.getElementById('pip').className='pip';
  segs.forEach(function(s){s.className='prog-seg';});
}
function updateUI(d){
  var correct=d.correctCount||0,misses=d.missCount||0,total=correct+misses;
  var running=d.gameState==='Running',done=correct>=10;
  document.getElementById('pip').className=running?'pip live':'pip';
  document.getElementById('btn-start').disabled=running||done;
  document.getElementById('prog-n').textContent=correct;
  for(var i=0;i<10;i++)segs[i].className='prog-seg'+(i<correct?' done':'');
  document.getElementById('st-hits').textContent=correct;
  document.getElementById('st-miss').textContent=misses;
  document.getElementById('st-rounds').textContent=d.roundNumber||0;
  document.getElementById('st-acc').textContent=total>0?Math.round((correct/total)*100)+'%':'&#8212;';
  var last=d.lastReactionTime||0,best=d.bestReactionTime||0,avg=d.averageReactionTime||0;
  var rx=document.getElementById('rx-num');
  if(last>0){rx.textContent=last;rx.className='rx-num '+(d.status&&d.status.indexOf('Correct')===0?'hit':'miss');}
  document.getElementById('rx-best').textContent=best>0?best+' ms':'&#8212;';
  document.getElementById('rx-avg').textContent=avg>0?avg+' ms':'&#8212;';
  var pill=document.getElementById('result-pill'),msg=document.getElementById('result-text');
  if(d.status&&d.status.indexOf('Correct')===0){pill.className='result-pill hit';msg.textContent=d.status;}
  else if(d.status&&(d.status.indexOf('Miss')===0||d.status.indexOf('Too slow')===0||d.status.indexOf('Too early')===0)){pill.className='result-pill miss';msg.textContent=d.status;}
  else{pill.className='result-pill';msg.textContent=d.status||'Waiting for first round';}
  document.getElementById('gameover').className=done?'gameover show':'gameover';
  if(done){document.getElementById('go-avg').textContent=avg>0?avg+' ms':'&#8212;';}
  document.getElementById('ftr-time').textContent=new Date().toLocaleTimeString();
}
function updateHistory(rows){
  var empty=document.getElementById('history-empty'),table=document.getElementById('history-table'),body=document.getElementById('history-body'),dl=document.getElementById('dl-btn');
  if(!rows||!rows.length){empty.style.display='block';table.style.display='none';dl.className='dl-btn disabled';return;}
  empty.style.display='none';table.style.display='table';dl.className='dl-btn';
  body.innerHTML='';
  for(var i=rows.length-1;i>=0;i--){
    var r=rows[i],tr=document.createElement('tr');
    tr.innerHTML='<td>#'+r.game+'</td><td>'+r.avg+' ms</td><td>'+r.best+' ms</td><td><span class="badge '+(r.accuracy>=80?'badge-g':'badge-r')+'">'+r.accuracy+'%</span></td><td>'+r.time+'</td>';
    body.appendChild(tr);
  }
}
function pollState(){fetch('/state').then(function(r){return r.json();}).then(function(d){updateUI(d);}).catch(function(){document.getElementById('pip').className='pip';});}
function pollHistory(){fetch('/history').then(function(r){return r.json();}).then(function(d){updateHistory(d.results);}).catch(function(){});}
setInterval(pollState,150);setInterval(pollHistory,2000);
pollState();pollHistory();
</script>
</body>
</html>
)rawliteral";
}

// ============================================================
//  SETUP
// ============================================================
void setup() {
  Serial.begin(115200);

  for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
    pinMode(BTN_PINS[i], INPUT_PULLUP);
    lastBtnReading[i] = HIGH;
    stableBtnState[i] = HIGH;
    lastDebounceMs[i] = 0;
  }
  for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
    rings[i]->begin();
    rings[i]->setBrightness(NEO_BRIGHTNESS);
    rings[i]->clear();
    rings[i]->show();
  }
  neoColorTable[C_OFF]   = ring0.Color(  0,   0,   0);
  neoColorTable[C_RED]   = ring0.Color(255,   0,   0);
  neoColorTable[C_GREEN] = ring0.Color(  0, 255,   0);
  neoColorTable[C_BLUE]  = ring0.Color(  0,   0, 255);
  allOff();  // Also keep lights dark immediately after power-up.

  testLEDsAtStartup();

  randomSeed(analogRead(34));

  WiFi.softAP(ssid, password);
  Serial.print("Connect to WiFi: "); Serial.println(ssid);
  Serial.print("Open: http://");     Serial.println(WiFi.softAPIP());

  server.on("/",         handleRoot);
  server.on("/start",    handleStart);
  server.on("/stop",     handleStop);
  server.on("/reset",    handleReset);
  server.on("/state",    handleState);
  server.on("/history",  handleHistory);
  server.on("/download", handleDownload);
  server.begin();
  Serial.println("Server started.");
}
