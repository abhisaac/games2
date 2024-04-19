#pragma once
struct ExplosionParticle
{
    Vector2 position;
    Vector2 velocity;
    float size;
};
struct Explosion {
    std::vector<ExplosionParticle> particles;
    bool valid = false;
    const int numParticles = 60;
    const float explosionSpeed = .3f; // Adjust this value for slower/faster explosion
    Color tint;
    
    void setup(float x, float y, Color c) {
        tint = c;
        particles.clear();
        valid = true;
        for (int i = 0; i < numParticles; ++i) {
            ExplosionParticle particle;
            particle.position = {x, y}; //{GetMouseX(), GetMouseY()};
            particle.velocity = {(float)GetRandomValue(-10, 10), (float)GetRandomValue(-10, 10)};
            particle.size = 9.f;
            particles.push_back(particle);
        }
    }

    void update() {
        if (valid) {
            for (auto &particle : particles)
            {
                particle.position.x += particle.velocity.x * explosionSpeed;
                particle.position.y += particle.velocity.y * explosionSpeed;
                particle.size -= (float)GetRandomValue(30, 70)/100;
            }
        }
    }

    bool draw() {
        if (valid) {
            int cnt = 0;
            for (auto &particle : particles)
            {
                if (particle.size < 0.01f) {
                    ++cnt;
                }
                DrawCircle(particle.position.x, particle.position.y, particle.size, tint);
            }
            if (cnt == particles.size()) {valid = false; particles.clear(); return false;}
        }
        return true;
    }
};
