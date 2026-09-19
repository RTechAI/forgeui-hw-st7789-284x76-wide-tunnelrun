#include <Arduino.h>
#include <TFT_eSPI.h>
#include <math.h>

// ============================================================
// ForgeUI Tunnel Run
// ESP32-S3 + ST7789 284x76 + Analog Joystick
// ============================================================

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite frame = TFT_eSprite(&tft);

constexpr int W = 284;
constexpr int H = 76;

// Physically proven joystick mapping
constexpr int JOY_X  = 6;
constexpr int JOY_Y  = 5;
constexpr int JOY_SW = 4;

// Colours
constexpr uint16_t BLACK    = TFT_BLACK;
constexpr uint16_t WHITE    = TFT_WHITE;
constexpr uint16_t CYAN     = TFT_CYAN;
constexpr uint16_t GREEN    = TFT_GREEN;
constexpr uint16_t YELLOW   = TFT_YELLOW;
constexpr uint16_t RED      = TFT_RED;
constexpr uint16_t BLUE     = TFT_BLUE;
constexpr uint16_t GREY     = 0x8410;
constexpr uint16_t DKGREY   = 0x2104;
constexpr uint16_t DARKCYAN = 0x03EF;

// ============================================================
// Game state
// ============================================================

enum GameState
{
    TITLE,
    PLAYING,
    CRASH,
    GAME_OVER
};

GameState state = TITLE;

int joyCentreX = 2048;
int joyCentreY = 2048;

float shipX = 48.0f;
float shipY = 38.0f;

float gameSpeed = 1.7f;
float boost = 100.0f;
bool boosting = false;

uint32_t distanceScore = 0;
uint32_t bestScore = 0;

unsigned long stateStart = 0;
unsigned long lastFrame = 0;

// ============================================================
// Tunnel
// ============================================================

constexpr int SEGMENT_W = 7;
constexpr int SEGMENTS = (W / SEGMENT_W) + 4;

float tunnelCentre[SEGMENTS];
float tunnelHalf[SEGMENTS];

float tunnelPhase = 0.0f;
float tunnelScroll = 0.0f;

// ============================================================
// Stars
// ============================================================

struct Star
{
    float x;
    float y;
    float speed;
    uint16_t colour;
};

constexpr int STAR_COUNT = 34;
Star stars[STAR_COUNT];

// ============================================================
// Exhaust particles
// ============================================================

struct ExhaustParticle
{
    float x;
    float y;
    float vx;
    float vy;
    int life;
    uint16_t colour;
};

constexpr int EXHAUST_COUNT = 22;
ExhaustParticle exhaustParticles[EXHAUST_COUNT];

// ============================================================
// Explosion particles
// ============================================================

struct ExplosionParticle
{
    float x;
    float y;
    float vx;
    float vy;
    int life;
    uint16_t colour;
};

constexpr int EXPLOSION_COUNT = 28;
ExplosionParticle explosion[EXPLOSION_COUNT];

// ============================================================
// Helpers
// ============================================================

bool buttonPressed()
{
    return digitalRead(JOY_SW) == LOW;
}

void centreText(const String &text, int y, int font, uint16_t colour)
{
    frame.setTextDatum(MC_DATUM);
    frame.setTextFont(font);
    frame.setTextColor(colour, BLACK);
    frame.drawString(text, W / 2, y);
}

float readAxis(int raw, int centre)
{
    constexpr int deadZone = 180;

    int delta = raw - centre;

    if (abs(delta) < deadZone)
        return 0.0f;

    float value = 0.0f;

    if (delta > 0)
    {
        int range = 4095 - centre - deadZone;

        if (range > 0)
            value =
                (float)(delta - deadZone) /
                (float)range;
    }
    else
    {
        int range = centre - deadZone;

        if (range > 0)
            value =
                (float)(delta + deadZone) /
                (float)range;
    }

    return constrain(value, -1.0f, 1.0f);
}

// ============================================================
// Joystick calibration
// ============================================================

void calibrateJoystick()
{
    long totalX = 0;
    long totalY = 0;

    constexpr int samples = 64;

    frame.fillSprite(BLACK);

    centreText("FORGEUI", 25, 4, CYAN);
    centreText("CALIBRATING FLIGHT CONTROL", 52, 1, WHITE);

    frame.pushSprite(0, 0);

    for (int i = 0; i < samples; i++)
    {
        totalX += analogRead(JOY_X);
        totalY += analogRead(JOY_Y);
        delay(5);
    }

    joyCentreX = totalX / samples;
    joyCentreY = totalY / samples;

    Serial.printf(
        "Joystick centre X=%d Y=%d\n",
        joyCentreX,
        joyCentreY
    );
}

// ============================================================
// Stars
// ============================================================

void resetStar(Star &s, bool randomX)
{
    s.x = randomX ? random(0, W) : W + random(0, 30);
    s.y = random(14, H - 3);

    int layer = random(0, 3);

    if (layer == 0)
    {
        s.speed = 0.5f;
        s.colour = DKGREY;
    }
    else if (layer == 1)
    {
        s.speed = 1.0f;
        s.colour = GREY;
    }
    else
    {
        s.speed = 1.7f;
        s.colour = WHITE;
    }
}

void initStars()
{
    for (int i = 0; i < STAR_COUNT; i++)
        resetStar(stars[i], true);
}

void updateStars(float speedMultiplier)
{
    for (int i = 0; i < STAR_COUNT; i++)
    {
        Star &s = stars[i];

        s.x -= s.speed * speedMultiplier;

        if (s.x < 0)
            resetStar(s, false);

        if (s.speed > 1.5f)
        {
            frame.drawFastHLine(
                (int)s.x,
                (int)s.y,
                boosting ? 5 : 2,
                s.colour
            );
        }
        else
        {
            frame.drawPixel(
                (int)s.x,
                (int)s.y,
                s.colour
            );
        }
    }
}

// ============================================================
// Tunnel generation
// ============================================================

void generateTunnel()
{
    float difficulty =
        constrain(distanceScore / 3000.0f, 0.0f, 1.0f);

    float baseHalfWidth =
        25.0f - difficulty * 8.0f;

    for (int i = 0; i < SEGMENTS; i++)
    {
        float worldX =
            tunnelPhase + i * 0.28f;

        float centre =
            39.0f +
            sinf(worldX) * 8.0f +
            sinf(worldX * 0.43f) * 5.0f +
            sinf(worldX * 0.19f) * 3.0f;

        float widthVariation =
            sinf(worldX * 0.71f) * 3.0f;

        tunnelCentre[i] = centre;

        tunnelHalf[i] =
            constrain(
                baseHalfWidth + widthVariation,
                14.0f,
                27.0f
            );
    }
}

// ============================================================
// Tunnel drawing
// ============================================================

void drawTunnel()
{
    for (int i = 0; i < SEGMENTS - 1; i++)
    {
        int x1 =
            i * SEGMENT_W - (int)tunnelScroll;

        int x2 = x1 + SEGMENT_W;

        if (x2 < 0 || x1 >= W)
            continue;

        int top1 =
            (int)(tunnelCentre[i] - tunnelHalf[i]);

        int top2 =
            (int)(tunnelCentre[i + 1] - tunnelHalf[i + 1]);

        int bottom1 =
            (int)(tunnelCentre[i] + tunnelHalf[i]);

        int bottom2 =
            (int)(tunnelCentre[i + 1] + tunnelHalf[i + 1]);

        top1 = constrain(top1, 13, H - 15);
        top2 = constrain(top2, 13, H - 15);

        bottom1 = constrain(bottom1, 15, H - 2);
        bottom2 = constrain(bottom2, 15, H - 2);

        // Upper wall
        frame.fillTriangle(
            x1, 13,
            x2, 13,
            x1, top1,
            DKGREY
        );

        frame.fillTriangle(
            x2, 13,
            x2, top2,
            x1, top1,
            DKGREY
        );

        // Lower wall
        frame.fillTriangle(
            x1, bottom1,
            x2, bottom2,
            x1, H - 1,
            DKGREY
        );

        frame.fillTriangle(
            x2, bottom2,
            x2, H - 1,
            x1, H - 1,
            DKGREY
        );

        // Bright boundaries
        frame.drawLine(
            x1, top1,
            x2, top2,
            CYAN
        );

        frame.drawLine(
            x1, bottom1,
            x2, bottom2,
            BLUE
        );

        // Upper glow
        frame.drawLine(
            x1, top1 - 1,
            x2, top2 - 1,
            DARKCYAN
        );
    }
}

// ============================================================
// Ship
// ============================================================

void drawShip()
{
    int x = (int)shipX;
    int y = (int)shipY;

    frame.fillTriangle(
        x + 9, y,
        x - 6, y - 5,
        x - 6, y + 5,
        CYAN
    );

    frame.fillRect(
        x - 5,
        y - 2,
        8,
        5,
        WHITE
    );

    frame.fillRect(
        x,
        y - 1,
        4,
        3,
        BLUE
    );

    frame.drawLine(
        x - 4, y - 4,
        x - 9, y - 7,
        WHITE
    );

    frame.drawLine(
        x - 4, y + 4,
        x - 9, y + 7,
        WHITE
    );

    frame.fillRect(
        x - 8,
        y - 2,
        3,
        5,
        boosting ? YELLOW : GREEN
    );
}

// ============================================================
// Exhaust
// ============================================================

void spawnExhaust()
{
    for (int i = 0; i < EXHAUST_COUNT; i++)
    {
        if (exhaustParticles[i].life > 0)
            continue;

        ExhaustParticle &p = exhaustParticles[i];

        p.x = shipX - 9;
        p.y = shipY + random(-2, 3);

        p.vx =
            boosting
                ? random(-45, -20) * 0.10f
                : random(-25, -10) * 0.10f;

        p.vy =
            random(-8, 9) * 0.05f;

        p.life =
            boosting
                ? random(8, 18)
                : random(5, 11);

        p.colour =
            boosting
                ? (random(0, 2) ? YELLOW : RED)
                : CYAN;

        break;
    }
}

void updateExhaust()
{
    spawnExhaust();

    if (boosting)
        spawnExhaust();

    for (int i = 0; i < EXHAUST_COUNT; i++)
    {
        ExhaustParticle &p = exhaustParticles[i];

        if (p.life <= 0)
            continue;

        p.x += p.vx;
        p.y += p.vy;

        p.life--;

        frame.drawFastHLine(
            (int)p.x,
            (int)p.y,
            boosting ? 4 : 2,
            p.colour
        );
    }
}

// ============================================================
// Collision
// ============================================================

bool shipHitTunnel()
{
    int segment =
        constrain(
            ((int)shipX + (int)tunnelScroll) / SEGMENT_W,
            0,
            SEGMENTS - 1
        );

    float top =
        tunnelCentre[segment] -
        tunnelHalf[segment];

    float bottom =
        tunnelCentre[segment] +
        tunnelHalf[segment];

    constexpr float shipRadius = 6.0f;

    return
        shipY - shipRadius < top ||
        shipY + shipRadius > bottom;
}

// ============================================================
// Crash
// ============================================================

void startCrash()
{
    state = CRASH;
    stateStart = millis();

    boosting = false;

    if (distanceScore > bestScore)
        bestScore = distanceScore;

    for (int i = 0; i < EXPLOSION_COUNT; i++)
    {
        ExplosionParticle &p = explosion[i];

        p.x = shipX;
        p.y = shipY;

        p.vx =
            random(-40, 41) * 0.11f;

        p.vy =
            random(-35, 36) * 0.11f;

        p.life =
            random(12, 30);

        int choice = random(0, 3);

        if (choice == 0)
            p.colour = RED;
        else if (choice == 1)
            p.colour = YELLOW;
        else
            p.colour = WHITE;
    }
}

void drawExplosion()
{
    int shakeX = random(-2, 3);
    int shakeY = random(-2, 3);

    frame.fillSprite(BLACK);

    updateStars(1.5f);

    generateTunnel();
    drawTunnel();

    for (int i = 0; i < EXPLOSION_COUNT; i++)
    {
        ExplosionParticle &p = explosion[i];

        if (p.life <= 0)
            continue;

        p.x += p.vx;
        p.y += p.vy;

        p.vx *= 0.97f;
        p.vy *= 0.97f;

        p.life--;

        frame.fillRect(
            (int)p.x + shakeX,
            (int)p.y + shakeY,
            2,
            2,
            p.colour
        );
    }

    centreText(
        "CRITICAL IMPACT",
        38 + shakeY,
        2,
        RED
    );

    frame.pushSprite(0, 0);

    if (millis() - stateStart > 1400)
    {
        state = GAME_OVER;
        stateStart = millis();
    }
}
// ============================================================
// HUD
// ============================================================

void drawHUD()
{
    frame.fillRect(0, 0, W, 13, BLACK);

    frame.setTextDatum(TL_DATUM);
    frame.setTextFont(1);

    frame.setTextColor(CYAN, BLACK);
    frame.drawString("FORGEUI", 3, 3);

    char dist[24];

    snprintf(
        dist,
        sizeof(dist),
        "DIST %05lu",
        (unsigned long)distanceScore
    );

    frame.setTextColor(WHITE, BLACK);
    frame.drawString(dist, 55, 3);

    int shownSpeed =
        (int)(gameSpeed * 100.0f);

    char speedText[16];

    snprintf(
        speedText,
        sizeof(speedText),
        "%03d",
        shownSpeed
    );

    frame.setTextColor(YELLOW, BLACK);
    frame.drawString(speedText, 145, 3);

    frame.setTextColor(WHITE, BLACK);
    frame.drawString("BOOST", 187, 3);

    frame.drawRect(
        226,
        3,
        54,
        7,
        GREY
    );

    int boostWidth =
        map(
            (int)boost,
            0,
            100,
            0,
            50
        );

    uint16_t boostColour =
        boost > 25.0f
            ? GREEN
            : RED;

    frame.fillRect(
        228,
        5,
        boostWidth,
        3,
        boostColour
    );
}

// ============================================================
// Title screen
// ============================================================

void drawTitle()
{
    frame.fillSprite(BLACK);

    updateStars(0.6f);

    frame.drawFastHLine(
        24,
        13,
        236,
        CYAN
    );

    frame.drawFastHLine(
        24,
        64,
        236,
        BLUE
    );

    centreText(
        "FORGEUI",
        28,
        4,
        CYAN
    );

    centreText(
        "TUNNEL RUN",
        48,
        2,
        WHITE
    );

    if (((millis() / 450) % 2) == 0)
    {
        centreText(
            "PRESS STICK TO LAUNCH",
            63,
            1,
            GREEN
        );
    }

    frame.pushSprite(0, 0);
}

// ============================================================
// Game over
// ============================================================

void drawGameOver()
{
    frame.fillSprite(BLACK);

    updateStars(0.4f);

    centreText(
        "MISSION LOST",
        19,
        4,
        RED
    );

    char scoreLine[48];

    snprintf(
        scoreLine,
        sizeof(scoreLine),
        "DIST %lu   BEST %lu",
        (unsigned long)distanceScore,
        (unsigned long)bestScore
    );

    centreText(
        scoreLine,
        45,
        2,
        WHITE
    );

    if (((millis() / 450) % 2) == 0)
    {
        centreText(
            "PRESS TO RELAUNCH",
            65,
            1,
            GREEN
        );
    }

    frame.pushSprite(0, 0);
}

// ============================================================
// Start / reset game
// ============================================================

void startGame()
{
    shipX = 48.0f;
    shipY = 38.0f;

    gameSpeed = 1.7f;
    boost = 100.0f;
    boosting = false;

    distanceScore = 0;

    tunnelPhase = 0.0f;
    tunnelScroll = 0.0f;

    for (int i = 0; i < EXHAUST_COUNT; i++)
        exhaustParticles[i].life = 0;

    for (int i = 0; i < EXPLOSION_COUNT; i++)
        explosion[i].life = 0;

    generateTunnel();

    state = PLAYING;
    stateStart = millis();
}

// ============================================================
// Gameplay
// ============================================================

void updateGame()
{
    int rawX = analogRead(JOY_X);
    int rawY = analogRead(JOY_Y);

    float inputX =
        readAxis(
            rawX,
            joyCentreX
        );

    float inputY =
        readAxis(
            rawY,
            joyCentreY
        );

    // Ship can move around the left portion of the tunnel.
    shipX += inputX * 1.8f;
    shipY += inputY * 2.1f;

    shipX =
        constrain(
            shipX,
            25.0f,
            95.0f
        );

    shipY =
        constrain(
            shipY,
            18.0f,
            H - 7.0f
        );

    boosting =
        buttonPressed() &&
        boost > 1.0f;

    float currentSpeed =
        gameSpeed;

    if (boosting)
    {
        currentSpeed *= 1.75f;

        boost -= 1.25f;
    }
    else
    {
        boost += 0.20f;
    }

    boost =
        constrain(
            boost,
            0.0f,
            100.0f
        );

    // Gradually increase difficulty.
    gameSpeed += 0.0007f;

    if (gameSpeed > 4.2f)
        gameSpeed = 4.2f;

    // Move through procedural tunnel.
    tunnelScroll +=
        currentSpeed * 1.5f;

    while (tunnelScroll >= SEGMENT_W)
    {
        tunnelScroll -= SEGMENT_W;

        tunnelPhase += 0.28f;
    }

    generateTunnel();

    // Distance rises faster during boost.
    distanceScore +=
        boosting ? 3 : 1;

    // Draw complete frame off-screen.
    frame.fillSprite(BLACK);

    updateStars(
        boosting
            ? currentSpeed * 1.5f
            : currentSpeed
    );

    drawTunnel();

    updateExhaust();

    drawShip();

    drawHUD();

    // Physical collision check after the tunnel is updated.
    if (shipHitTunnel())
    {
        startCrash();
        return;
    }

    frame.pushSprite(0, 0);
}

// ============================================================
// Setup
// ============================================================

void setup()
{
    Serial.begin(115200);
    delay(300);

    Serial.println();
    Serial.println("==============================");
    Serial.println("FORGEUI TUNNEL RUN");
    Serial.println("ESP32-S3 + ST7789 284x76");
    Serial.println("JOY X=6 Y=5 SW=4");
    Serial.println("==============================");

    pinMode(
        JOY_SW,
        INPUT_PULLUP
    );

    analogReadResolution(12);

    // Physically proven ST7789 configuration.
    tft.init();
    tft.invertDisplay(false);
    tft.setRotation(1);

    // Full-screen RGB565 framebuffer.
    frame.setColorDepth(16);

    if (frame.createSprite(W, H) == nullptr)
    {
        Serial.println(
            "ERROR: framebuffer allocation failed"
        );

        while (true)
            delay(1000);
    }

    frame.fillSprite(BLACK);
    frame.pushSprite(0, 0);

    randomSeed(
        analogRead(JOY_X) ^
        analogRead(JOY_Y) ^
        micros()
    );

    initStars();

    // Keep joystick untouched/centred during startup.
    calibrateJoystick();

    generateTunnel();

    state = TITLE;
    stateStart = millis();

    // Prevent a held button from instantly starting.
    while (buttonPressed())
        delay(10);
}

// ============================================================
// Main loop
// ============================================================

void loop()
{
    // Approximately 30 FPS.
    if (millis() - lastFrame < 33)
        return;

    lastFrame = millis();

    static bool previousButton = false;

    bool currentButton =
        buttonPressed();

    bool buttonEdge =
        currentButton &&
        !previousButton;

    previousButton =
        currentButton;

    switch (state)
    {
        case TITLE:
        {
            drawTitle();

            if (buttonEdge)
                startGame();

            break;
        }

        case PLAYING:
        {
            updateGame();
            break;
        }

        case CRASH:
        {
            drawExplosion();
            break;
        }

        case GAME_OVER:
        {
            drawGameOver();

            if (buttonEdge)
                startGame();

            break;
        }
    }
}