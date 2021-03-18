#include "openglwindow.hpp"

#include <fmt/core.h>

#include <imgui.h>

#include <cppitertools/itertools.hpp>

#include "abcg.hpp"

void OpenGLWindow::handleEvent(SDL_Event &event) {
    // Keyboard events
    if (event.type == SDL_KEYDOWN && inputBuffer) {
        if (event.key.keysym.sym == SDLK_UP && direction != 1)
            direction = 3;
        if (event.key.keysym.sym == SDLK_LEFT && direction != 0)
            direction = 2;
        if (event.key.keysym.sym == SDLK_DOWN && direction != 3)
            direction = 1;
        if (event.key.keysym.sym == SDLK_RIGHT && direction != 2)
            direction = 0;
        
        inputBuffer = false;
    }
}

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

    playerSize = 3;
    playerI = {2, 2, 2};
    playerJ = {6, 5, 4};

}

void OpenGLWindow::gameOver() {
    playerI = {
        5, 5, 5, 5, 6, 7, 8, 9, 10, 10, 10, 10, 9, 8, 8, 
        10, 9, 8, 7, 6, 5, 5, 5, 5, 6, 7, 8, 9, 10, 8, 8, 
        10, 9, 8, 7, 6, 5, 6, 6, 5, 6, 7, 8, 9, 10, 
        10, 9, 8, 7, 6, 5, 5, 5, 5, 8, 8, 8, 10, 10, 10, 
        13, 13, 13, 13, 14, 15, 16, 17, 18, 18, 18, 18, 17, 16, 15, 14, 13, 
        13, 14, 15, 16, 17, 18, 18, 17, 16, 15, 14, 13, 
        18, 17, 16, 15, 14, 13, 13, 13, 13, 16, 16, 16, 18, 18, 18, 
        18, 17, 16, 15, 14, 13, 13, 13, 13, 14, 15, 16, 16, 16, 17, 18
    };
    playerJ = {
        6, 5, 4, 3, 3, 3, 3, 3, 3, 4, 5, 6, 6, 6, 5, 
        8, 8, 8, 8, 8, 8, 9, 10, 11, 11, 11, 11, 11, 11, 9, 10, 
        13, 13, 13, 13, 13, 13, 14, 15, 16, 16, 16, 16, 16, 16, 
        18, 18, 18, 18, 18, 18, 19, 20, 21, 19, 20, 21, 19, 20, 21,
        6, 5, 4, 3, 3, 3, 3, 3, 3, 4, 5, 6, 6, 6, 6, 6, 6,
        8, 8, 8, 8, 8, 9, 10, 11, 11, 11, 11, 11,
        13, 13, 13, 13, 13, 13, 14, 15, 16, 14, 15, 16, 14, 15, 16,
        18, 18, 18, 18, 18, 18, 19, 20, 21, 21, 21, 21, 20, 19, 20, 21
    };

    playerSize = playerI.size();
}

void OpenGLWindow::restart() {
    playerSize = 3;
    playerI = {2, 2, 2};
    playerJ = {6, 5, 4};

    end = false;
}

void OpenGLWindow::positionUpdate() {
    int increment = -1;

    if (direction == 0 || direction == 1)
        increment = 1;

    for (int i = playerSize - 1; i > 0; i--) {
        playerJ[i] = playerJ[i - 1];
        playerI[i] = playerI[i - 1];
    }

    if (direction == 0 || direction == 2) {
        playerJ[0] = (playerJ[0] + increment) % 25;
        if (playerJ[0] == -1)
            playerJ[0] = 24;    
    } else if (direction == 1 || direction == 3) {
        playerI[0] = (playerI[0] + increment) % 25;
        if (playerI[0] == -1)
            playerI[0] = 24;    
    }   

    inputBuffer = true;
}

bool OpenGLWindow::checkPosition(int i, int j) {
    for (int k = 0; k < playerSize; k++)
        if (playerI[k] == i && playerJ[k] == j)
            return true;  

    return false;
}

void OpenGLWindow::paintGL() {
    // Check whether to render the next polygon
    if (m_elapsedTimer.elapsed() < m_delay / 1000.0) return;
    m_elapsedTimer.restart();
    
    int sides = 4;

    if (!end)
        positionUpdate();
    else
        gameOver();

    for (int i = 0; i < 25; i++) {
        for (int j = 0; j < 25; j++) {
            // Set square model based on color
            if (checkPosition(i, j))
                setupModel(glm::vec3 {0.3f, 0.3f, 0.3f});
            else
                setupModel(glm::vec3 {1.0f, 1.0f, 1.0f});

            glViewport(0, 0, m_viewportWidth, m_viewportHeight);

            glUseProgram(m_program);

            // Set square position
            glm::vec2 translation{basePositionX + j * 0.065f, basePositionY - i * 0.065f};
            GLint translationLocation{glGetUniformLocation(m_program, "translation")};
            glUniform2fv(translationLocation, 1, &translation.x);
        
            // Scale square
            auto scale{0.04f};
            GLint scaleLocation{glGetUniformLocation(m_program, "scale")};
            glUniform1f(scaleLocation, scale);

            // Render
            glBindVertexArray(m_vao);
            glDrawArrays(GL_TRIANGLE_FAN, 0, sides + 2);
            glBindVertexArray(0);   
        }
    }
    glUseProgram(0);
}

void OpenGLWindow::paintUI() {
    abcg::OpenGLWindow::paintUI();

    static bool firstTime{true};
    if (firstTime) {
      ImGui::SetNextWindowPos(ImVec2(5, 75));
      firstTime = false;
    }

    ImGuiWindowFlags flags{
        ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_NoTitleBar | 
        ImGuiWindowFlags_NoResize
    };

    ImGui::Begin(" ", nullptr, flags);
        ImGui::Text("Score: %d", score);

        if (end) {
            ImGui::Button("Retry", ImVec2(-1, 50));
            if (ImGui::IsItemClicked())
                restart();
        } else {
            ImGui::Button("", ImVec2(-1, 50));
        }

    ImGui::End();
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