#include "openglwindow.hpp"

#include <fmt/core.h>

#include <imgui.h>

#include <cppitertools/itertools.hpp>

#include "abcg.hpp"

void OpenGLWindow::initializeGL() {
    const auto *vertexShader{R"gl(
        #version 410

        layout(location = 0) in vec2 inPosition;
        layout(location = 1) in vec4 inColor;

        uniform vec2 translation;
        uniform float scale;

        out vec4 fragColor;

        void main() {
        vec2 newPosition = inPosition * scale + translation;
        gl_Position = vec4(newPosition, 0, 1);
        fragColor = inColor;
        }
    )gl"};

    const auto *fragmentShader{R"gl(
        #version 410

        in vec4 fragColor;

        out vec4 outColor;

        void main() { outColor = fragColor; }
    )gl"};

    // Create shader program
    m_program = createProgramFromString(vertexShader, fragmentShader);

    // Clear window
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    // Start pseudo-random number generator
    auto seed{std::chrono::steady_clock::now().time_since_epoch().count()};
    m_randomEngine.seed(seed);

    // Initialize matrix board for player positioning
    std::vector<std::vector<int> > boardMatrix(size, std::vector<int>(size, int()));

    for (int i = 0; i < 25; i++) {
        boardMatrix[i].resize(25);
    }

    for (int i = 0; i < 25; i++) {
        for (int j = 0; j < 25; j++) {
            boardMatrix[i][j] = 0;
        }
    }
}

void OpenGLWindow::paintGL() {
    // Check whether to render the next polygon
    if (m_elapsedTimer.elapsed() < m_delay / 1000.0) return;
    m_elapsedTimer.restart();

    for (int i = 0; i < 25; i++) {
        for (int j = 0; j < 25; j++) {
            // Create a regular polygon
            int sides = 4;

            setupModel(glm::vec3 {1.0f, 1.0f, 1.0f});

            glViewport(0, 0, m_viewportWidth, m_viewportHeight);

            glUseProgram(m_program);

            glm::vec2 translation{basePositionX + j * 0.075f, basePositionY - i * 0.075f};
            GLint translationLocation{glGetUniformLocation(m_program, "translation")};
            glUniform2fv(translationLocation, 1, &translation.x);

            // Choose a random scale factor (1% to 25%)
            auto scale{0.05f};
            GLint scaleLocation{glGetUniformLocation(m_program, "scale")};
            glUniform1f(scaleLocation, scale);

            // Render
            glBindVertexArray(m_vao);
            glDrawArrays(GL_TRIANGLE_FAN, 0, sides + 2);
            glBindVertexArray(0);

            glUseProgram(0);
        }
    }
}

void OpenGLWindow::paintUI() {
  


}

void OpenGLWindow::resizeGL(int width, int height) {
    m_viewportWidth = width;
    m_viewportHeight = height;

    glClear(GL_COLOR_BUFFER_BIT);
}

void OpenGLWindow::terminateGL() {
    glDeleteProgram(m_program);
    glDeleteBuffers(1, &m_vboPositions);
    glDeleteBuffers(1, &m_vboColors);
    glDeleteVertexArrays(1, &m_vao);
}

void OpenGLWindow::setupModel(glm::vec3 color) {
    // Release previous resources, if any
    glDeleteBuffers(1, &m_vboPositions);
    glDeleteBuffers(1, &m_vboColors);
    glDeleteVertexArrays(1, &m_vao);

    // Select random colors for the radial gradient
    //   std::uniform_real_distribution<float> rd(0.0f, 1.0f);

    std::vector<glm::vec2> positions(0);
    std::vector<glm::vec3> colors(0);

    // Polygon center
    positions.emplace_back(0, 0);
    colors.push_back(color);

    // Border vertices
    auto step{M_PI / 2};
    for (auto angle : iter::range(M_PI / 4, M_PI * 4, step)) {
        // fmt::print("Angle: {0:.2f} {0:.2f}\n", std::cos(angle), std::sin(angle));
        positions.emplace_back(std::cos(angle), std::sin(angle));
        colors.push_back(color);
    }

    // Duplicate second vertex
    positions.push_back(positions.at(1));
    colors.push_back(color);

    // Generate VBO of positions
    glGenBuffers(1, &m_vboPositions);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboPositions);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec2),
                positions.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Generate VBO of colors
    glGenBuffers(1, &m_vboColors);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboColors);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3),
                colors.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Get location of attributes in the program
    GLint positionAttribute{glGetAttribLocation(m_program, "inPosition")};
    GLint colorAttribute{glGetAttribLocation(m_program, "inColor")};

    // Create VAO
    glGenVertexArrays(1, &m_vao);

    // Bind vertex attributes to current VAO
    glBindVertexArray(m_vao);

    glEnableVertexAttribArray(positionAttribute);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboPositions);
    glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glEnableVertexAttribArray(colorAttribute);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboColors);
    glVertexAttribPointer(colorAttribute, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // End of binding to current VAO
    glBindVertexArray(0);
}