#ifndef OPENGLWINDOW_HPP_
#define OPENGLWINDOW_HPP_

#include <random>

#include "abcg.hpp"

class OpenGLWindow : public abcg::OpenGLWindow {
    protected:
    void initializeGL() override;
    void paintGL() override;
    void paintUI() override;
    void resizeGL(int width, int height) override;
    void terminateGL() override;
    void handleEvent(SDL_Event& event) override;

    private:
    GLuint m_vao{};
    GLuint m_vboPositions{};
    GLuint m_vboColors{};
    GLuint m_program{};

    int m_viewportWidth{};
    int m_viewportHeight{};

    std::default_random_engine m_randomEngine;

    float basePositionX = -0.5f;
    float basePositionY = +0.7f;
    float baseScale = 0.1f;

    int size{25};
    int playerSize{3};
    int direction{0};
    bool inputBuffer{true};

    int score{0};
    bool end{false};

    std::vector<int> playerI;
    std::vector<int> playerJ;
    void positionUpdate();
    bool checkPosition(int i, int j);
    int applePositionX;
    int applePositionY;
    void updateApplePosition();
    bool checkApplePosition(int x, int y);
    bool collidedWithApple();
    void gameOver();
    void restart();

    int m_delay{100};
    abcg::ElapsedTimer m_elapsedTimer;

    void setupModel(glm::vec3 color);
};

#endif