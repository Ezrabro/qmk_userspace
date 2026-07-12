// ============================================================
// Custom "welcome" RGB Matrix boot animation.
//
// TO REMOVE THIS ENTIRELY: just delete this file (or comment out
// the WELCOME_ANIMATION_ACTIVE define in config.h). Nothing else
// needs to change - the whole file compiles to nothing when the
// define is missing.
// ============================================================
#ifdef WELCOME_ANIMATION_ACTIVE

RGB_MATRIX_EFFECT(welcome_animation)

#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

// ---------------- Tunables ----------------
#define WA_SWEEP_STEP_MS    12   // ms per LED during the "one by one" sweep
#define WA_CHECKER_STEP_MS  120  // ms per checkerboard flip
#define WA_CHECKER_FLASHES  12    // number of on/off flips (3 full blinks)
#define WA_LETTER_STEP_MS   220  // ms each letter stays lit
#define WA_LETTER_GAP_MS    80   // ms of "off" between letters

// LED indices for the letters, in the order they light up.
// These come from g_led_config's "Key Matrix to LED Index" table
// in rgb.c - update if you ever change the PCB/layout.
//   A = 63   Z = 80   A = 42   R = 45
static const uint8_t wa_letters[] = {63, 80, 63, 45};
#define WA_LETTER_COUNT (sizeof(wa_letters) / sizeof(wa_letters[0]))

static uint32_t wa_start_time = 0;
static bool     wa_started    = false;

static bool welcome_animation(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    if (params->init || !wa_started) {
        wa_start_time = timer_read32();
        wa_started    = true;
        rgb_matrix_set_color_all(0, 0, 0);
    }

    uint32_t elapsed = timer_elapsed32(wa_start_time);

    uint8_t  led_count   = RGB_MATRIX_LED_COUNT;
    uint32_t sweep_dur   = (uint32_t)led_count * WA_SWEEP_STEP_MS;
    uint32_t checker_dur = (uint32_t)WA_CHECKER_FLASHES * WA_CHECKER_STEP_MS;
    uint32_t letters_dur = (uint32_t)WA_LETTER_COUNT * (WA_LETTER_STEP_MS + WA_LETTER_GAP_MS);
    uint32_t total_dur   = sweep_dur + checker_dur + letters_dur;

    HSV hsv = rgb_matrix_config.hsv;
    RGB rgb = hsv_to_rgb(hsv);

    if (elapsed < sweep_dur) {
        // ---- Phase 1: light LEDs one by one, first to last ----
        uint32_t lit_up_to = elapsed / WA_SWEEP_STEP_MS;
        for (uint8_t i = led_min; i < led_max; i++) {
            if (i <= lit_up_to) {
                rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
            } else {
                rgb_matrix_set_color(i, 0, 0, 0);
            }
        }
    } else if (elapsed < sweep_dur + checker_dur) {
        // ---- Phase 2: checkerboard flash, like an LED test ----
        uint32_t phase_elapsed = elapsed - sweep_dur;
        uint32_t flash_index   = phase_elapsed / WA_CHECKER_STEP_MS;
        bool     even_on       = (flash_index % 2) == 0;

        for (uint8_t i = led_min; i < led_max; i++) {
            bool is_even = (i % 2) == 0;
            if (is_even == even_on) {
                rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
            } else {
                rgb_matrix_set_color(i, 0, 0, 0);
            }
        }
    } else if (elapsed < total_dur) {
        // ---- Phase 3: A - Z - A - R, one at a time ----
        uint32_t phase_elapsed = elapsed - sweep_dur - checker_dur;
        uint32_t step_dur      = WA_LETTER_STEP_MS + WA_LETTER_GAP_MS;
        uint32_t letter_index  = phase_elapsed / step_dur;
        uint32_t within_step   = phase_elapsed % step_dur;

        for (uint8_t i = led_min; i < led_max; i++) {
            rgb_matrix_set_color(i, 0, 0, 0);
        }

        if (letter_index < WA_LETTER_COUNT && within_step < WA_LETTER_STEP_MS) {
            uint8_t led = wa_letters[letter_index];
            if (led >= led_min && led < led_max) {
                rgb_matrix_set_color(led, rgb.r, rgb.g, rgb.b);
            }
        }
    } else {
        // ---- Done: hand off to your normal effect ----
        wa_started = false;
        rgb_matrix_mode_noeeprom(WA_FOLLOWUP_MODE);
        return true;
    }

    return led_max < RGB_MATRIX_LED_COUNT;
}

#endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#endif // WELCOME_ANIMATION_ACTIVE





// ============================================================
// "Snake" RGB Matrix effect.
//
// An autonomous snake slithers around your keyboard following the
// REAL physical layout of the LEDs (not just index order) - it
// builds its own up/down/left/right movement graph at runtime from
// g_led_config's x/y positions, so this works on ANY board without
// hardcoding LED indices.
//
// Features:
//   - Snake grows every time it "eats" a randomly placed food LED
//   - Self-collision avoidance (won't run into its own tail)
//   - Rainbow gradient trail, brightest at the head
//   - Pulsing food indicator
//   - Auto-restarts if it gets stuck, or if it fills the whole board
//
// TO REMOVE: delete this entire block (both #ifdef/#endif included),
// or just comment out "#define SNAKE_ANIMATION_ACTIVE" in config.h.
//
// This block can live in the SAME rgb_matrix_user.inc file as your
// other custom effects (e.g. welcome_animation) - just paste it in
// anywhere outside their #ifdef blocks. Each custom effect is fully
// independent.
// ============================================================
#ifdef SNAKE_ANIMATION_ACTIVE

RGB_MATRIX_EFFECT(snake)

#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

#include <stdlib.h>
#include <string.h>

// ---------------- Tunables ----------------
#define SNAKE_MOVE_INTERVAL_MS   110  // lower = faster snake
#define SNAKE_START_LENGTH       4
#define SNAKE_GROWTH_PER_FOOD    3
#define SNAKE_NEIGHBOR_TOL       7    // px tolerance for "same row/col"
#define SNAKE_HUE_STEP           6    // hue shift per body segment
#define SNAKE_NO_NEIGHBOR        0xFF
#define SNAKE_MAX_LEN            (RGB_MATRIX_LED_COUNT - 1)

enum snake_dir {
    SNAKE_UP = 0,
    SNAKE_DOWN,
    SNAKE_LEFT,
    SNAKE_RIGHT,
    SNAKE_DIR_COUNT
};

static uint8_t  wa_snake_neighbor[RGB_MATRIX_LED_COUNT][SNAKE_DIR_COUNT];
static bool     wa_snake_graph_built = false;

static uint8_t  wa_snake_body[RGB_MATRIX_LED_COUNT];
static bool     wa_snake_occupied[RGB_MATRIX_LED_COUNT];
static uint8_t  wa_snake_length;
static uint8_t  wa_snake_target_length;
static uint8_t  wa_snake_dir;
static uint8_t  wa_snake_food;
static uint32_t wa_snake_last_move;
static uint8_t  wa_snake_hue_base;

// ---- Build the movement graph once, from the board's real LED layout ----
static void wa_snake_build_graph(void) {
    for (uint16_t i = 0; i < RGB_MATRIX_LED_COUNT; i++) {
        int16_t ix = g_led_config.point[i].x;
        int16_t iy = g_led_config.point[i].y;

        int16_t best_dist[SNAKE_DIR_COUNT];
        for (uint8_t d = 0; d < SNAKE_DIR_COUNT; d++) {
            wa_snake_neighbor[i][d] = SNAKE_NO_NEIGHBOR;
            best_dist[d] = 32767;
        }

        for (uint16_t j = 0; j < RGB_MATRIX_LED_COUNT; j++) {
            if (i == j) continue;
            int16_t jx = g_led_config.point[j].x;
            int16_t jy = g_led_config.point[j].y;
            int16_t dx = jx - ix;
            int16_t dy = jy - iy;

            if (dx > 0 && abs(dy) <= SNAKE_NEIGHBOR_TOL && dx < best_dist[SNAKE_RIGHT]) {
                best_dist[SNAKE_RIGHT] = dx;
                wa_snake_neighbor[i][SNAKE_RIGHT] = (uint8_t)j;
            }
            if (dx < 0 && abs(dy) <= SNAKE_NEIGHBOR_TOL && -dx < best_dist[SNAKE_LEFT]) {
                best_dist[SNAKE_LEFT] = -dx;
                wa_snake_neighbor[i][SNAKE_LEFT] = (uint8_t)j;
            }
            if (dy > 0 && abs(dx) <= SNAKE_NEIGHBOR_TOL && dy < best_dist[SNAKE_DOWN]) {
                best_dist[SNAKE_DOWN] = dy;
                wa_snake_neighbor[i][SNAKE_DOWN] = (uint8_t)j;
            }
            if (dy < 0 && abs(dx) <= SNAKE_NEIGHBOR_TOL && -dy < best_dist[SNAKE_UP]) {
                best_dist[SNAKE_UP] = -dy;
                wa_snake_neighbor[i][SNAKE_UP] = (uint8_t)j;
            }
        }
    }
    wa_snake_graph_built = true;
}

static uint8_t wa_snake_random_free_led(void) {
    // Fast path: a handful of random guesses.
    for (uint8_t tries = 0; tries < 40; tries++) {
        uint8_t candidate = rand() % RGB_MATRIX_LED_COUNT;
        if (!wa_snake_occupied[candidate]) return candidate;
    }
    // Fallback: full scan (only matters when the board is nearly full).
    for (uint16_t i = 0; i < RGB_MATRIX_LED_COUNT; i++) {
        if (!wa_snake_occupied[i]) return (uint8_t)i;
    }
    return 0;
}

static void wa_snake_reset(void) {
    for (uint16_t i = 0; i < RGB_MATRIX_LED_COUNT; i++) {
        wa_snake_occupied[i] = false;
    }
    wa_snake_length        = 1;
    wa_snake_target_length = SNAKE_START_LENGTH;
    wa_snake_body[0]       = rand() % RGB_MATRIX_LED_COUNT;
    wa_snake_occupied[wa_snake_body[0]] = true;
    wa_snake_dir           = rand() % SNAKE_DIR_COUNT;
    wa_snake_food          = wa_snake_random_free_led();
    wa_snake_hue_base      = rand() % 256;
    wa_snake_last_move     = timer_read32();
}

static bool wa_snake_try_move(uint8_t dir) {
    uint8_t head = wa_snake_body[0];
    uint8_t next = wa_snake_neighbor[head][dir];
    if (next == SNAKE_NO_NEIGHBOR || wa_snake_occupied[next]) return false;

    bool ate = (next == wa_snake_food);

    memmove(&wa_snake_body[1], &wa_snake_body[0], wa_snake_length);
    wa_snake_body[0] = next;
    wa_snake_occupied[next] = true;

    if (ate) {
        wa_snake_target_length += SNAKE_GROWTH_PER_FOOD;
        if (wa_snake_target_length > SNAKE_MAX_LEN) wa_snake_target_length = SNAKE_MAX_LEN;
        wa_snake_food = wa_snake_random_free_led();
    }

    if (wa_snake_length < wa_snake_target_length) {
        wa_snake_length++;
    } else {
        uint8_t tail = wa_snake_body[wa_snake_length];
        wa_snake_occupied[tail] = false;
    }

    wa_snake_dir = dir;
    return true;
}

static void wa_snake_step(void) {
    if (wa_snake_length >= SNAKE_MAX_LEN) {
        // Snake filled the whole board - "won". Start a new game.
        wa_snake_reset();
        return;
    }

    // Figure out which direction actually gets us closer to the food.
    uint8_t head = wa_snake_body[0];
    int16_t hx = g_led_config.point[head].x;
    int16_t hy = g_led_config.point[head].y;
    int16_t fx = g_led_config.point[wa_snake_food].x;
    int16_t fy = g_led_config.point[wa_snake_food].y;
    int16_t dx = fx - hx;
    int16_t dy = fy - hy;

    uint8_t primary, secondary;
    if (abs(dx) >= abs(dy)) {
        primary   = (dx >= 0) ? SNAKE_RIGHT : SNAKE_LEFT;
        secondary = (dy >= 0) ? SNAKE_DOWN  : SNAKE_UP;
    } else {
        primary   = (dy >= 0) ? SNAKE_DOWN  : SNAKE_UP;
        secondary = (dx >= 0) ? SNAKE_RIGHT : SNAKE_LEFT;
    }

    uint8_t order[SNAKE_DIR_COUNT];
    uint8_t idx = 0;
    order[idx++] = primary;
    order[idx++] = secondary;
    for (uint8_t d = 0; d < SNAKE_DIR_COUNT; d++) {
        if (d != primary && d != secondary) order[idx++] = d;
    }

    // Occasional random detour so it doesn't move like a robot.
    if (rand() % 6 == 0) {
        uint8_t a = rand() % SNAKE_DIR_COUNT;
        uint8_t b = rand() % SNAKE_DIR_COUNT;
        uint8_t tmp = order[a];
        order[a] = order[b];
        order[b] = tmp;
    }

    for (uint8_t k = 0; k < SNAKE_DIR_COUNT; k++) {
        if (wa_snake_try_move(order[k])) return;
    }

    // Totally boxed in - start over.
    wa_snake_reset();
}

static bool snake(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    if (!wa_snake_graph_built) {
        wa_snake_build_graph();
    }
    if (params->init) {
        srand(timer_read32());
        wa_snake_reset();
    }

    if (timer_elapsed32(wa_snake_last_move) > SNAKE_MOVE_INTERVAL_MS) {
        wa_snake_step();
        wa_snake_last_move = timer_read32();
    }

    for (uint8_t i = led_min; i < led_max; i++) {
        rgb_matrix_set_color(i, 0, 0, 0);
    }

    // Food: gentle pulse between dim and bright red.
    if (wa_snake_food >= led_min && wa_snake_food < led_max) {
        uint16_t t = timer_read32() % 1000;
        uint8_t  pulse = (t < 500) ? (155 + (t * 100) / 500)
                                   : (255 - ((t - 500) * 100) / 500);
        HSV food_hsv = {0, 255, pulse};
        RGB food_rgb = hsv_to_rgb(food_hsv);
        rgb_matrix_set_color(wa_snake_food, food_rgb.r, food_rgb.g, food_rgb.b);
    }

    // Body: rainbow gradient, brightest at the head, fading to the tail.
    for (uint8_t seg = 0; seg < wa_snake_length; seg++) {
        uint8_t led = wa_snake_body[seg];
        if (led < led_min || led >= led_max) continue;

        uint8_t val = 255 - (seg * (200 / (wa_snake_length + 1)));
        HSV hsv = {(uint8_t)(wa_snake_hue_base + seg * SNAKE_HUE_STEP), 255, val};
        RGB rgb = hsv_to_rgb(hsv);
        rgb_matrix_set_color(led, rgb.r, rgb.g, rgb.b);
    }

    return led_max < RGB_MATRIX_LED_COUNT;
}

#endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#endif // SNAKE_ANIMATION_ACTIVE


