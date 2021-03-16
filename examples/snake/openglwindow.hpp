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

    private:
    GLuint m_vao{};
    GLuint m_vboPositions{};
    GLuint m_vboColors{};
    GLuint m_program{};

    int m_viewportWidth{};
    int m_viewportHeight{};

    std::default_random_engine m_randomEngine;

    float basePositionX = -0.9f;
    float basePositionY = +0.9f;
    float baseScale = 0.1f;

    int size{25};
    std::vector<std::vector<int>> boardMatrix;

    int m_delay{500};
    abcg::ElapsedTimer m_elapsedTimer;

    void setupModel(glm::vec3 color);
};

#endif