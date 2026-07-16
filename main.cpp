#include <SFML/Graphics.hpp>
#include <omp.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>

// Konstanta Jendela
const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 700;
const int JUMLAH_PARTIKEL = 600; // Memenuhi syarat minimal 500 partikel

// Fungsi utilitas untuk menghitung jarak
float hitungJarak(sf::Vector2f a, sf::Vector2f b) {
    return std::sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y));
}

// Struktur Boid (Partikel)
struct Boid {
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Vector2f acceleration;
    sf::Color color;

    Boid(float x, float y) {
        position = sf::Vector2f(x, y);
        velocity = sf::Vector2f((rand() % 100 - 50) / 10.0f, (rand() % 100 - 50) / 10.0f);
        acceleration = sf::Vector2f(0.0f, 0.0f);
        color = sf::Color(100 + rand() % 155, 100 + rand() % 155, 255); // Nuansa biru
    }

    // Membatasi kecepatan maksimum agar tidak terbang tak terkendali
    void limitVelocity(float maxSpeed) {
        float speed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
        if (speed > maxSpeed) {
            velocity.x = (velocity.x / speed) * maxSpeed;
            velocity.y = (velocity.y / speed) * maxSpeed;
        }
    }

    // Update posisi berdasarkan fisika
    void update(float dt) {
        velocity += acceleration * dt;
        limitVelocity(4.0f); // Kecepatan maksimal
        position += velocity * dt;
        acceleration = sf::Vector2f(0.0f, 0.0f); // Reset akselerasi
        
        // Memantul jika menabrak dinding (interaksi lingkungan)
        if (position.x < 0) { position.x = 0; velocity.x *= -1; }
        if (position.x > WINDOW_WIDTH) { position.x = WINDOW_WIDTH; velocity.x *= -1; }
        if (position.y < 0) { position.y = 0; velocity.y *= -1; }
        if (position.y > WINDOW_HEIGHT) { position.y = WINDOW_HEIGHT; velocity.y *= -1; }
    }
};

int main() {
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Simulasi Boids - Tugas Rancang Pempar");
    window.setFramerateLimit(60);
    srand(time(NULL));

    std::vector<Boid> boids;
    for (int i = 0; i < JUMLAH_PARTIKEL; i++) {
        boids.push_back(Boid(rand() % WINDOW_WIDTH, rand() % WINDOW_HEIGHT));
    }

    // Menggunakan bentuk segitiga agar arah hadap partikel terlihat
    sf::CircleShape shape(4.0f, 3); 
    float dt = 1.0f;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
        }

        // Menghitung interaksi boids (Separation, Alignment, Cohesion)
        #pragma omp parallel for schedule(dynamic)
        for (int i = 0; i < JUMLAH_PARTIKEL; i++) {
            sf::Vector2f separation(0, 0);
            sf::Vector2f alignment(0, 0);
            sf::Vector2f cohesion(0, 0);
            int totalTetangga = 0;

            for (int j = 0; j < JUMLAH_PARTIKEL; j++) {
                if (i != j) {
                    float jarak = hitungJarak(boids[i].position, boids[j].position);
                    
                    if (jarak < 50.0f) { // Jarak pandang boid
                        // 1. Separation (Menjauh agar tidak tabrakan)
                        if (jarak < 20.0f) {
                            separation += (boids[i].position - boids[j].position) / jarak;
                        }
                        // 2. Alignment (Menyamakan arah gerak)
                        alignment += boids[j].velocity;
                        // 3. Cohesion (Mendekat ke pusat kerumunan)
                        cohesion += boids[j].position;
                        totalTetangga++;
                    }
                }
            }

            if (totalTetangga > 0) {
                alignment.x /= totalTetangga; alignment.y /= totalTetangga;
                cohesion.x /= totalTetangga; cohesion.y /= totalTetangga;
                cohesion = (cohesion - boids[i].position); // Vektor menuju pusat
                
                // Menggabungkan semua gaya (dikalikan bobot agar seimbang)
                boids[i].acceleration += (separation * 1.5f) + (alignment * 0.05f) + (cohesion * 0.005f);
            }
        }

        // Update posisi (dilakukan setelah semua gaya selesai dihitung)
        for (int i = 0; i < JUMLAH_PARTIKEL; i++) {
            boids[i].update(dt);
        }

        window.clear(sf::Color(20, 20, 30)); // Background biru gelap

        for (int i = 0; i < JUMLAH_PARTIKEL; i++) {
            shape.setPosition(boids[i].position);
            shape.setFillColor(boids[i].color);
            
            // Memutar segitiga agar menghadap arah gerak
            float angle = atan2(boids[i].velocity.y, boids[i].velocity.x) * 180 / 3.14159265f;
            shape.setRotation(angle + 90.0f);
            
            window.draw(shape);
        }

        window.display();
    }

    return 0;
}
