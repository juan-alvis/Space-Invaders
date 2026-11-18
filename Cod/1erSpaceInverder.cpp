#include <SFML/Graphics.hpp>
#include <vector>
#include <optional>
#include <cstdlib>
#include <ctime>
#include <algorithm>

struct Bala {
    sf::RectangleShape forma;
    float velocidad;
};

struct Enemigo {
    sf::RectangleShape forma;
    bool vivo = true;
};

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // Ventana en SFML 3 (usa Vector2u con llaves)
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Space Invaders - SFML 3");
    window.setFramerateLimit(60);

    // Jugador (Nave)
    sf::RectangleShape jugador({50.f, 25.f});
    jugador.setFillColor(sf::Color::Green);
    jugador.setPosition({375.f, 540.f});
    float velocidadJugador = 6.f;

    // Balas del jugador
    std::vector<Bala> balas;
    sf::Clock relojDisparo;
    float cooldownDisparo = 0.35f;

    // Oleada de enemigos
    std::vector<Enemigo> enemigos;
    const int filas = 4;
    const int columnas = 9;
    for (int f = 0; f < filas; ++f) {
        for (int c = 0; c < columnas; ++c) {
            Enemigo e;
            e.forma.setSize({40.f, 25.f});
            e.forma.setFillColor(f == 0 ? sf::Color::Red : (f == 1 ? sf::Color::Yellow : sf::Color::Cyan));
            e.forma.setPosition({80.f + c * 70.f, 50.f + f * 45.f});
            enemigos.push_back(e);
        }
    }

    float direccionEnemigos = 1.f;
    float velocidadEnemigos = 2.0f;
    bool juegoTerminado = false;

    while (window.isOpen()) {
        // Manejo de eventos en SFML 3
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        if (!juegoTerminado) {
            // Movimiento del jugador
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
                if (jugador.getPosition().x > 10.f) {
                    jugador.move({-velocidadJugador, 0.f});
                }
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
                if (jugador.getPosition().x + jugador.getSize().x < 790.f) {
                    jugador.move({velocidadJugador, 0.f});
                }
            }

            // Disparo del jugador
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) && relojDisparo.getElapsedTime().asSeconds() >= cooldownDisparo) {
                Bala b;
                b.forma.setSize({5.f, 15.f});
                b.forma.setFillColor(sf::Color::White);
                b.forma.setPosition({
                    jugador.getPosition().x + jugador.getSize().x / 2.f - 2.5f,
                    jugador.getPosition().y - 15.f
                });
                b.velocidad = -8.f;
                balas.push_back(b);
                relojDisparo.restart();
            }

            // Actualizar balas
            for (auto& b : balas) {
                b.forma.move({0.f, b.velocidad});
            }
            balas.erase(
                std::remove_if(balas.begin(), balas.end(), [](const Bala& b) {
                    return b.forma.getPosition().y < -20.f;
                }),
                balas.end()
            );

            // Movimiento de enemigos
            bool tocarBorde = false;
            for (const auto& e : enemigos) {
                if (!e.vivo) continue;
                float x = e.forma.getPosition().x;
                if ((direccionEnemigos > 0 && x + e.forma.getSize().x >= 785.f) ||
                    (direccionEnemigos < 0 && x <= 15.f)) {
                    tocarBorde = true;
                    break;
                }
            }

            if (tocarBorde) {
                direccionEnemigos *= -1.f;
                for (auto& e : enemigos) {
                    if (e.vivo) {
                        e.forma.move({0.f, 20.f});
                        if (e.forma.getPosition().y + e.forma.getSize().y >= jugador.getPosition().y) {
                            juegoTerminado = true;
                        }
                    }
                }
            } else {
                for (auto& e : enemigos) {
                    if (e.vivo) {
                        e.forma.move({velocidadEnemigos * direccionEnemigos, 0.f});
                    }
                }
            }

            // Colisiones balas vs enemigos (SFML 3 usa findIntersection)
            for (auto itBala = balas.begin(); itBala != balas.end();) {
                bool impacto = false;
                for (auto& e : enemigos) {
                    if (e.vivo && itBala->forma.getGlobalBounds().findIntersection(e.forma.getGlobalBounds()).has_value()) {
                        e.vivo = false;
                        impacto = true;
                        break;
                    }
                }
                if (impacto) {
                    itBala = balas.erase(itBala);
                } else {
                    ++itBala;
                }
            }

            // Verificar si todos los enemigos fueron eliminados
            bool quedanVivos = std::any_of(enemigos.begin(), enemigos.end(), [](const Enemigo& e) { return e.vivo; });
            if (!quedanVivos) {
                juegoTerminado = true;
            }
        }

        // Renderizado
        window.clear(sf::Color(15, 15, 25));

        window.draw(jugador);
        for (const auto& b : balas) {
            window.draw(b.forma);
        }
        for (const auto& e : enemigos) {
            if (e.vivo) {
                window.draw(e.forma);
            }
        }

        window.display();
    }

    return 0;
}