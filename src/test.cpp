#include "raylib.h"
#include <vector>

struct Particle
{
    Vector2 position;
    Vector2 velocity;
    float size;
    // Color color;
};

int main()
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "Circle Blast Effect");

    std::vector<Particle> particles;
    bool explosion = false;
    const int circleRadius = 50;
    const int numParticles = 50;
    const float explosionSpeed = .3f; // Adjust this value for slower/faster explosion

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        // Update
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            explosion = true;
            particles.clear();
            for (int i = 0; i < numParticles; ++i)
            {
                Particle particle;
                particle.position = {GetMouseX(), GetMouseY()};
                particle.velocity = {(float)GetRandomValue(-10, 10), (float)GetRandomValue(-10, 10)};
                // particle.color = (Color){GetRandomValue(50, 255), GetRandomValue(50, 255), GetRandomValue(50, 255), 255};
                particle.size = 9.f;
                particles.push_back(particle);
            }
        }

        if (explosion)
        {
            for (auto &particle : particles)
            {
                particle.position.x += particle.velocity.x * explosionSpeed;
                particle.position.y += particle.velocity.y * explosionSpeed;
                particle.size -= (float)GetRandomValue(30, 70)/100;
            }
        }

        // Draw
        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (explosion)
        {
            int cnt = 0;
            for (auto &particle : particles)
            {
                if (particle.size < 0.01f) {
                    ++cnt;
                }
                DrawCircle(particle.position.x, particle.position.y, particle.size, RED);
            }
            if (cnt ==particles.size()) {explosion = false; particles.clear();}
        }
        // else
        // {
        //     DrawCircle(GetMouseX(), GetMouseY(), circleRadius, RED);
        // }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
