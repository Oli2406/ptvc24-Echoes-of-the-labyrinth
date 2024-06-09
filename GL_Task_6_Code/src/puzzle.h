#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"

enum Mode { MODE_2D };
Mode currentMode = MODE_2D;

struct Tile {
    int number;
    bool empty;
};

const int gridSize = 4;
std::vector<std::vector<Tile>> grid(gridSize, std::vector<Tile>(gridSize));
GLuint puzzleVao, puzzleVbo, puzzleEbo;
Shader* shaderProgram;

class puzzle {
public:
    
    puzzle(Shader* shader) {
        shaderProgram = shader;
    }
    
    void initGrid() {
    int counter = 1;
    for (int i = 0; i < gridSize; ++i) {
        for (int j = 0; j < gridSize; ++j) {
            grid[i][j].number = counter++;
            grid[i][j].empty = false;
        }
    }
    grid[gridSize - 1][gridSize - 1].empty = true; 
    grid[gridSize - 1][gridSize - 1].number = 0;
}


    void shuffleGrid() {
        srand(static_cast<unsigned int>(time(0)));
        for (int i = 0; i < 1000; ++i) {
            int x1 = rand() % gridSize;
            int y1 = rand() % gridSize;
            int x2 = rand() % gridSize;
            int y2 = rand() % gridSize;
            std::swap(grid[x1][y1], grid[x2][y2]);
        }
    }

    void render(int width, int height) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        drawGrid(width, height);
    }
    

    glm::mat4 setup2DProjection(int width, int height) {
        return glm::ortho(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, -1.0f, 1.0f);
    }

    void createBuffers() {
        float vertices[] = {

             0.0f,  1.0f,     0.0f, 1.0f,
             1.0f,  1.0f,     1.0f, 1.0f,
             1.0f,  0.0f,     1.0f, 0.0f,
             0.0f,  0.0f,     0.0f, 0.0f,
        };

        unsigned int indices[] = {
            0, 1, 2,
            2, 3, 0
        };

        glGenVertexArrays(1, &puzzleVao);
        glGenBuffers(1, &puzzleVbo);
        glGenBuffers(1, &puzzleEbo);

        glBindVertexArray(puzzleVao);

        glBindBuffer(GL_ARRAY_BUFFER, puzzleVbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, puzzleEbo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }


    void drawGrid(int width, int height) {
        shaderProgram->use();

        glm::mat4 projection = setup2DProjection(width, height);
        shaderProgram->setUniform("projection", projection);

        float tileWidth = width / gridSize;
        float tileHeight = height / gridSize;

        for (int i = 0; i < gridSize; ++i) {
            for (int j = 0; j < gridSize; ++j) {
                if (!grid[i][j].empty) {
                    float x = j * tileWidth;
                    float y = i * tileHeight;

                    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
                    shaderProgram->setUniform("model", model);

                    glBindVertexArray(puzzleVao);
                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                    glBindVertexArray(0);
                }
            }
        }
    }

    void moveTile(int x, int y) {
        int emptyX, emptyY;
        bool found = false;
        for (int i = 0; i < gridSize; ++i) {
            for (int j = 0; j < gridSize; ++j) {
                if (grid[i][j].empty) {
                    emptyX = i;
                    emptyY = j;
                    found = true;
                    break;
                }
            }
            if (found) break;
        }

        if ((abs(emptyX - x) == 1 && emptyY == y) || (abs(emptyY - y) == 1 && emptyX == x)) {
            std::swap(grid[emptyX][emptyY], grid[x][y]);
        }
    }

    void handleMouseClick(int button, int action, int x, int y, int width, int height) {
        if (currentMode == MODE_2D && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            int gridX = x / (width / gridSize);
            int gridY = y / (height / gridSize);
            moveTile(gridY, gridX);
        }
    }

    void initOpenGL() {
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    }
};


