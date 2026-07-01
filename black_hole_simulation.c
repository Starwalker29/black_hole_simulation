#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// Cross-platform sleep support (Linux/Mac + Windows)
#ifdef _WIN32
    #include <windows.h>
    #define SLEEP(ms) Sleep(ms)
#else
    #include <unistd.h>
    #define SLEEP(ms) usleep((ms) * 1000)
#endif

#define WIDTH 80
#define HEIGHT 40
#define NUM_PARTICLES 120
#define DT 0.08          // Time step (tweak for smoother/faster simulation)
#define EVENT_HORIZON 2.5
#define MASS 800.0       // "Mass" of the black hole — higher = stronger gravity
#define SPAWN_RADIUS_MIN 22.0
#define SPAWN_RADIUS_MAX 35.0

struct Particle {
    double x, y;     // Position
    double vx, vy;   // Velocity
    int active;      // Is this particle currently alive?
};

int main() {
    srand(time(NULL));
    struct Particle particles[NUM_PARTICLES];
    const double cx = WIDTH / 2.0;   // Black hole center
    const double cy = HEIGHT / 2.0;
    const double PI = 3.141592653589793;

    // Initially all particles are inactive (will be spawned in the loop)
    for (int i = 0; i < NUM_PARTICLES; i++) {
        particles[i].active = 0;
    }

    printf("🌌 BLACK HOLE SIMULATOR in C 🌌\n");
    printf("Particles swirl into the accretion disk and vanish past the event horizon!\n");
    printf("Press Ctrl+C to escape the singularity...\n\n");

    while (1) {
        // Screen buffer (prevents flicker - classic technique)
        char screen[HEIGHT][WIDTH];
        for (int i = 0; i < HEIGHT; i++) {
            for (int j = 0; j < WIDTH; j++) {
                screen[i][j] = ' ';
            }
        }

        // Draw the black hole (filled "dark" region using #)
        int bh_r = 3;
        for (int i = -bh_r; i <= bh_r; i++) {
            for (int j = -bh_r; j <= bh_r; j++) {
                if (i * i + j * j <= bh_r * bh_r + 1) {
                    int py = (int)(cy + i);
                    int px = (int)(cx + j);
                    if (py >= 0 && py < HEIGHT && px >= 0 && px < WIDTH) {
                        screen[py][px] = '#';
                    }
                }
            }
        }

        // Update physics and draw active particles
        for (int i = 0; i < NUM_PARTICLES; i++) {
            if (!particles[i].active) continue;
            struct Particle *p = &particles[i];

            double dx = cx - p->x;
            double dy = cy - p->y;
            double dist2 = dx * dx + dy * dy;

            if (dist2 < 0.0001) {  // Avoid division by zero
                p->active = 0;
                continue;
            }

            double dist = sqrt(dist2);

            // Event horizon absorption
            if (dist < EVENT_HORIZON) {
                p->active = 0;
                continue;
            }

            // Newtonian gravity: acceleration = MASS / r² in the direction of the black hole
            double force = MASS / dist2;
            double ax = force * (dx / dist);
            double ay = force * (dy / dist);

            // Euler integration (simple numerical method - good enough for visuals)
            p->vx += ax * DT;
            p->vy += ay * DT;
            p->x += p->vx * DT;
            p->y += p->vy * DT;

            // Remove particles that flew off-screen
            if (p->x < 0 || p->x >= WIDTH || p->y < 0 || p->y >= HEIGHT) {
                p->active = 0;
                continue;
            }

            // Draw particle ( * ) - last-drawn wins on overlaps (simple z-order)
            int px = (int)p->x;
            int py = (int)p->y;
            if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT && screen[py][px] == ' ') {
                screen[py][px] = '*';
            }
        }

        // Clear terminal and render the new frame (ANSI escape codes)
        printf("\033[2J\033[H");
        for (int i = 0; i < HEIGHT; i++) {
            for (int j = 0; j < WIDTH; j++) {
                putchar(screen[i][j]);
            }
            putchar('\n');
        }

        // Respawn inactive particles in a ring (creates the cool accretion disk swirl)
        for (int i = 0; i < NUM_PARTICLES; i++) {
            if (!particles[i].active) {
                double angle = (double)rand() / RAND_MAX * 2.0 * PI;
                double radius = SPAWN_RADIUS_MIN + (rand() % (int)(SPAWN_RADIUS_MAX - SPAWN_RADIUS_MIN));
                particles[i].x = cx + cos(angle) * radius;
                particles[i].y = cy + sin(angle) * radius;

                // Give tangential velocity for near-circular orbits + randomness
                double tang_vel = sqrt(MASS / radius) * 0.6;  // Approximate orbital speed
                particles[i].vx = -sin(angle) * tang_vel + ((rand() % 100) - 50) / 40.0;
                particles[i].vy = cos(angle) * tang_vel + ((rand() % 100) - 50) / 40.0;
                particles[i].active = 1;
            }
        }

        SLEEP(40);  // ~25 FPS - feels smooth without burning CPU
    }

    return 0;
}