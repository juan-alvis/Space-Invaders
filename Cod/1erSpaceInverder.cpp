#include <SFML/Graphics.hpp>
#include <vector>
#include <optional>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <fstream>
#include <string>

struct Bala {
    sf::RectangleShape forma;
    sf::Vector2f velocidad;
    bool esEnemiga;
};

struct Enemigo {
    sf::RectangleShape forma;
    int vida = 1;
    int puntos = 100;
    bool vivo = true;
};

struct Jefe {
    sf::RectangleShape forma;
    sf::RectangleShape barraFondo;
    sf::RectangleShape barraVida;
    int vidaMax = 35;
    int vidaActual = 35;
    float velocidadX = 3.5f;
    bool activo = false;
};

struct Record {
    std::string nombre;
    int puntos;
};

std::vector<Record> cargarRecords(const std::string& archivo) {
    std::vector<Record> lista;
    std::ifstream in(archivo);
    Record r;
    while (in >> r.nombre >> r.puntos) {
        lista.push_back(r);
    }
    std::sort(lista.begin(), lista.end(), [](const Record& a, const Record& b) {
        return a.puntos > b.puntos;
    });
    if (lista.size() > 5) lista.resize(5);
    return lista;
}

void guardarRecord(const std::string& archivo, const std::string& nombre, int puntos) {
    auto lista = cargarRecords(archivo);
    lista.push_back({nombre.empty() ? "AAA" : nombre, puntos});
    std::sort(lista.begin(), lista.end(), [](const Record& a, const Record& b) {
        return a.puntos > b.puntos;
    });
    if (lista.size() > 5) lista.resize(5);

    std::ofstream out(archivo);
    for (const auto& r : lista) {
        out << r.nombre << " " << r.puntos << "\n";
    }
}

void crearOleada(std::vector<Enemigo>& enemigos, int nivel) {
    enemigos.clear();
    int filas = std::min(3 + nivel, 6);
    int columnas = 9;
    for (int f = 0; f < filas; ++f) {
        for (int c = 0; c < columnas; ++c) {
            Enemigo e;
            e.forma.setSize({40.f, 24.f});
            if (f == 0) {
                e.forma.setFillColor(sf::Color(255, 70, 70));
                e.vida = (nivel >= 2) ? 2 : 1;
                e.puntos = 250;
            } else if (f == 1) {
                e.forma.setFillColor(sf::Color(255, 210, 50));
                e.vida = 1;
                e.puntos = 150;
            } else {
                e.forma.setFillColor(sf::Color(60, 220, 255));
                e.vida = 1;
                e.puntos = 100;
            }
            e.forma.setPosition({80.f + c * 70.f, 70.f + f * 42.f});
            enemigos.push_back(e);
        }
    }
}

void activarJefe(Jefe& jefe) {
    jefe.activo = true;
    jefe.vidaMax = 35;
    jefe.vidaActual = jefe.vidaMax;
    jefe.forma.setSize({140.f, 55.f});
    jefe.forma.setFillColor(sf::Color(180, 50, 255));
    jefe.forma.setOutlineThickness(3.f);
    jefe.forma.setOutlineColor(sf::Color::White);
    jefe.forma.setPosition({330.f, 80.f});

    jefe.barraFondo.setSize({300.f, 16.f});
    jefe.barraFondo.setFillColor(sf::Color(60, 60, 60));
    jefe.barraFondo.setPosition({250.f, 45.f});

    jefe.barraVida.setSize({300.f, 16.f});
    jefe.barraVida.setFillColor(sf::Color(220, 40, 40));
    jefe.barraVida.setPosition({250.f, 45.f});
}

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    sf::RenderWindow window(sf::VideoMode({800, 600}), "Space Invaders - Tabla de Records");
    window.setFramerateLimit(60);

    // Cargar fuente del sistema Windows
    sf::Font font;
    bool fuenteCargada = font.openFromFile("C:/Windows/Fonts/arial.ttf");

    // Jugador
    sf::RectangleShape jugador({50.f, 25.f});
    jugador.setFillColor(sf::Color::Green);
    jugador.setPosition({375.f, 545.f});
    float velocidadJugador = 6.5f;

    // Estados del juego: 0 = Jugando, 1 = Ingresar Nombre, 2 = Tabla de Records
    int estado = 0;
    bool esVictoria = false;
    int vidas = 3;
    int nivel = 1;
    int puntuacion = 0;
    const int nivelJefe = 3;

    std::string nombreActual = "";
    const std::string archivoScores = "scores.txt";
    std::vector<Record> topScores = cargarRecords(archivoScores);

    std::vector<Bala> balas;
    std::vector<Enemigo> enemigos;
    Jefe jefe;

    sf::Clock relojDisparoJugador;
    sf::Clock relojDisparoEnemigo;
    float cooldownJugador = 0.30f;

    crearOleada(enemigos, nivel);
    float direccionEnemigos = 1.f;
    float velocidadEnemigos = 2.0f;

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            // Captura de teclado al ingresar iniciales en la tabla
            if (estado == 1) {
                if (const auto* textEntered = event->getIf<sf::Event::TextEntered>()) {
                    char32_t c = textEntered->unicode;
                    // Backspace
                    if (c == 8 && !nombreActual.empty()) {
                        nombreActual.pop_back();
                    }
                    // Letras o números (hasta 3 caracteres)
                    else if (((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) && nombreActual.size() < 3) {
                        char letra = static_cast<char>(c);
                        if (letra >= 'a' && letra <= 'z') letra -= 32; // Convertir a mayúscula
                        nombreActual.push_back(letra);
                    }
                }
                if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                    if (keyPressed->code == sf::Keyboard::Key::Enter) {
                        guardarRecord(archivoScores, nombreActual, puntuacion);
                        topScores = cargarRecords(archivoScores);
                        estado = 2; // Pasar a mostrar la tabla
                    }
                }
            }
            // Reiniciar partida desde la tabla de récords con R
            else if (estado == 2) {
                if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                    if (keyPressed->code == sf::Keyboard::Key::R) {
                        vidas = 3;
                        nivel = 1;
                        puntuacion = 0;
                        estado = 0;
                        esVictoria = false;
                        jefe.activo = false;
                        nombreActual.clear();
                        balas.clear();
                        jugador.setPosition({375.f, 545.f});
                        velocidadEnemigos = 2.0f;
                        crearOleada(enemigos, nivel);
                    }
                }
            }
        }

        if (estado == 0) {
            // Movimiento jugador
            if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) &&
                jugador.getPosition().x > 10.f) {
                jugador.move({-velocidadJugador, 0.f});
            }
            if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) &&
                jugador.getPosition().x + jugador.getSize().x < 790.f) {
                jugador.move({velocidadJugador, 0.f});
            }

            // Disparo jugador
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) &&
                relojDisparoJugador.getElapsedTime().asSeconds() >= cooldownJugador) {
                Bala b;
                b.forma.setSize({5.f, 15.f});
                b.forma.setFillColor(sf::Color::White);
                b.forma.setPosition({jugador.getPosition().x + jugador.getSize().x / 2.f - 2.5f, jugador.getPosition().y - 15.f});
                b.velocidad = {0.f, -9.f};
                b.esEnemiga = false;
                balas.push_back(b);
                relojDisparoJugador.restart();
            }

            if (!jefe.activo) {
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
                            e.forma.move({0.f, 18.f});
                            if (e.forma.getPosition().y + e.forma.getSize().y >= jugador.getPosition().y) {
                                estado = 1; // Ir a registro de récord
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

                float intervaloEnemigo = std::max(0.45f, 1.2f - nivel * 0.25f);
                if (relojDisparoEnemigo.getElapsedTime().asSeconds() >= intervaloEnemigo) {
                    std::vector<size_t> vivos;
                    for (size_t i = 0; i < enemigos.size(); ++i) {
                        if (enemigos[i].vivo) vivos.push_back(i);
                    }
                    if (!vivos.empty()) {
                        size_t idx = vivos[std::rand() % vivos.size()];
                        Bala b;
                        b.forma.setSize({6.f, 14.f});
                        b.forma.setFillColor(sf::Color(255, 100, 100));
                        b.forma.setPosition({
                            enemigos[idx].forma.getPosition().x + enemigos[idx].forma.getSize().x / 2.f,
                            enemigos[idx].forma.getPosition().y + enemigos[idx].forma.getSize().y
                        });
                        b.velocidad = {0.f, 5.f + nivel * 0.6f};
                        b.esEnemiga = true;
                        balas.push_back(b);
                    }
                    relojDisparoEnemigo.restart();
                }

                bool quedanVivos = std::any_of(enemigos.begin(), enemigos.end(), [](const Enemigo& e) { return e.vivo; });
                if (!quedanVivos) {
                    nivel++;
                    balas.clear();
                    if (nivel < nivelJefe) {
                        velocidadEnemigos += 0.7f;
                        crearOleada(enemigos, nivel);
                    } else {
                        activarJefe(jefe);
                    }
                }
            } else {
                jefe.forma.move({jefe.velocidadX, 0.f});
                if (jefe.forma.getPosition().x <= 30.f || jefe.forma.getPosition().x + jefe.forma.getSize().x >= 770.f) {
                    jefe.velocidadX *= -1.f;
                }

                if (relojDisparoEnemigo.getElapsedTime().asSeconds() >= 0.85f) {
                    float centroX = jefe.forma.getPosition().x + jefe.forma.getSize().x / 2.f;
                    float fondoY = jefe.forma.getPosition().y + jefe.forma.getSize().y;
                    float vx[3] = {-2.2f, 0.f, 2.2f};

                    for (int i = 0; i < 3; ++i) {
                        Bala b;
                        b.forma.setSize({8.f, 16.f});
                        b.forma.setFillColor(sf::Color(255, 50, 220));
                        b.forma.setPosition({centroX, fondoY});
                        b.velocidad = {vx[i], 6.0f};
                        b.esEnemiga = true;
                        balas.push_back(b);
                    }
                    relojDisparoEnemigo.restart();
                }
            }

            for (auto& b : balas) {
                b.forma.move(b.velocidad);
            }

            for (auto it = balas.begin(); it != balas.end();) {
                bool eliminarBala = false;

                if (it->forma.getPosition().y < -20.f || it->forma.getPosition().y > 620.f) {
                    eliminarBala = true;
                } else if (it->esEnemiga) {
                    if (it->forma.getGlobalBounds().findIntersection(jugador.getGlobalBounds()).has_value()) {
                        vidas--;
                        eliminarBala = true;
                        if (vidas <= 0) {
                            estado = 1; // Ir a registro de récord
                        }
                    }
                } else {
                    if (!jefe.activo) {
                        for (auto& e : enemigos) {
                            if (e.vivo && it->forma.getGlobalBounds().findIntersection(e.forma.getGlobalBounds()).has_value()) {
                                e.vida--;
                                if (e.vida <= 0) {
                                    e.vivo = false;
                                    puntuacion += e.puntos;
                                }
                                eliminarBala = true;
                                break;
                            }
                        }
                    } else {
                        if (it->forma.getGlobalBounds().findIntersection(jefe.forma.getGlobalBounds()).has_value()) {
                            jefe.vidaActual--;
                            puntuacion += 50;
                            eliminarBala = true;
                            float porcentaje = std::max(0.f, static_cast<float>(jefe.vidaActual) / jefe.vidaMax);
                            jefe.barraVida.setSize({300.f * porcentaje, 16.f});
                            if (jefe.vidaActual <= 0) {
                                puntuacion += 2000;
                                jefe.activo = false;
                                esVictoria = true;
                                estado = 1; // Ir a registro de récord
                            }
                        }
                    }
                }

                if (eliminarBala) {
                    it = balas.erase(it);
                } else {
                    ++it;
                }
            }
        }

        // Renderizado
        window.clear(sf::Color(15, 15, 25));

        if (estado == 0) {
            // HUD Superior
            for (int i = 0; i < vidas; ++i) {
                sf::RectangleShape iconoVida({22.f, 12.f});
                iconoVida.setFillColor(sf::Color::Green);
                iconoVida.setPosition({15.f + i * 30.f, 15.f});
                window.draw(iconoVida);
            }

            if (fuenteCargada) {
                sf::Text txtInfo(font, "NIVEL: " + std::to_string(nivel) + "    PUNTOS: " + std::to_string(puntuacion), 20);
                txtInfo.setFillColor(sf::Color::White);
                txtInfo.setPosition({270.f, 10.f});
                window.draw(txtInfo);
            }

            window.draw(jugador);

            if (!jefe.activo) {
                for (const auto& e : enemigos) {
                    if (e.vivo) window.draw(e.forma);
                }
            } else {
                window.draw(jefe.barraFondo);
                window.draw(jefe.barraVida);
                window.draw(jefe.forma);
            }

            for (const auto& b : balas) {
                window.draw(b.forma);
            }
        } else if (estado == 1 && fuenteCargada) {
            // Pantalla de registro en la tabla
            sf::Text titulo(font, esVictoria ? "VICTORIA! DERROTASTE AL JEFE" : "GAME OVER", 36);
            titulo.setFillColor(esVictoria ? sf::Color::Green : sf::Color(255, 70, 70));
            titulo.setPosition({170.f, 110.f});
            window.draw(titulo);

            sf::Text txtPuntos(font, "Puntuacion Final: " + std::to_string(puntuacion), 26);
            txtPuntos.setFillColor(sf::Color::Yellow);
            txtPuntos.setPosition({250.f, 190.f});
            window.draw(txtPuntos);

            sf::Text instruccion(font, "Escribe tus iniciales (3 letras) y presiona ENTER:", 22);
            instruccion.setFillColor(sf::Color::White);
            instruccion.setPosition({160.f, 270.f});
            window.draw(instruccion);

            std::string mostrarNombre = nombreActual;
            while (mostrarNombre.size() < 3) mostrarNombre += "_";

            sf::Text cajaNombre(font, mostrarNombre, 48);
            cajaNombre.setFillColor(sf::Color::Cyan);
            cajaNombre.setPosition({345.f, 330.f});
            window.draw(cajaNombre);
        } else if (estado == 2 && fuenteCargada) {
            // Pantalla de Tabla de Récords (Top 5)
            sf::Text titulo(font, "TABLA DE MEJORES PUNTAJES", 32);
            titulo.setFillColor(sf::Color::Yellow);
            titulo.setPosition({175.f, 70.f});
            window.draw(titulo);

            for (size_t i = 0; i < topScores.size(); ++i) {
                std::string linea = std::to_string(i + 1) + ".   " + topScores[i].nombre + "   -   " + std::to_string(topScores[i].puntos) + " pts";
                sf::Text fila(font, linea, 26);
                fila.setFillColor(i == 0 ? sf::Color::Green : sf::Color::White);
                fila.setPosition({250.f, 150.f + i * 50.f});
                window.draw(fila);
            }

            sf::Text reiniciar(font, "Presiona R para volver a jugar", 22);
            reiniciar.setFillColor(sf::Color::Cyan);
            reiniciar.setPosition({245.f, 470.f});
            window.draw(reiniciar);
        }

        window.display();
    }

    return 0;
}