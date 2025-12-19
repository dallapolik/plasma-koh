#include <GL/freeglut_std.h>
#include <GL/glut.h>
#include <deque>
#include <iostream>
#include <map>
#include <cmath>
#include <vector>
#include <random>
using namespace std;

float RESOLUTION_H = 1080;
float zoom = 1.0f;
float zoom_speed = 0.01f;
float RESOLUTION_W = 1920;
float offset_x = 0.0f;
float offset_y = 0.0f;
float move_speed = 0.01f;
bool is_koh = 1;

map<int, deque<float>> dp = {
    {0, {-0.5f, -0.289f, 0.5f, -0.289f, 0.0f, 0.577f}}
};

struct Koh_snowflake{
    int q;
    deque<float> vertex;

    Koh_snowflake() : vertex(dp[0]), q(0) {}
    Koh_snowflake(deque<float> vertex) : vertex(vertex), q(0) {}

    void draw() {
        glBegin(GL_LINE_LOOP);
        for(int i = 0; i < vertex.size(); i += 2) {
            glVertex2f(vertex[i], vertex[i + 1]);
        }
        glEnd();
    }

    void update(){
        if(dp.find(q) != dp.end() && !dp[q].empty()){
            vertex = dp[q];
        }
        else{
            deque<float> new_vertex;
            deque<float> prev_vertex;
            if(q > 0) prev_vertex = dp[q - 1];
            else prev_vertex = dp[q + 1];

            for(int i = 0; i < prev_vertex.size(); i += 2) {
                float x1 = prev_vertex[i], y1 = prev_vertex[i + 1];
                float x2 = prev_vertex[(i + 2) % prev_vertex.size()],
                    y2 = prev_vertex[(i + 3) % prev_vertex.size()];

                float dx = x2 - x1;
                float dy = y2 - y1;

                float x3 = x1 + dx / 3;
                float y3 = y1 + dy / 3;

                float x4 = x1 + 2 * dx / 3;
                float y4 = y1 + 2 * dy / 3;

                // Вершина треугольника (поворот на 60 градусов по часовой стрелке)
                float angle = atan2(dy, dx) - M_PI / 3 * (q >= 0) + M_PI / 3 * (q < 0);
                float length = sqrt(dx*dx + dy*dy) / 3;
                float x5 = x3 + length * cos(angle);
                float y5 = y3 + length * sin(angle);

                new_vertex.push_back(x1);
                new_vertex.push_back(y1);
                new_vertex.push_back(x3);
                new_vertex.push_back(y3);
                new_vertex.push_back(x5);
                new_vertex.push_back(y5);
                new_vertex.push_back(x4);
                new_vertex.push_back(y4);
            }

            dp[q] = new_vertex;
            vertex = new_vertex;
        }
        cout << q << '\n';
    }
};



class PlasmaFractal {
    int size;
    float roughness;
    vector<vector<float>> grid;

    float randOffset(float scale) {
        static random_device rd;
        static mt19937 gen(rd());
        uniform_real_distribution<float> dist(-1.0f, 1.0f);
        return dist(gen) * scale;
    }

    void diamondSquare(int step, float scale) {
        if (step < 2) return;
        int half = step / 2;


        for (int y = half; y < size; y += step) {
            for (int x = half; x < size; x += step) {
                float a = grid[y - half][x - half];
                float b = grid[y - half][x + half >= size ? size - 1 : x + half];
                float c = grid[y + half >= size ? size - 1 : y + half][x - half];
                float d = grid[y + half >= size ? size - 1 : y + half][x + half >= size ? size - 1 : x + half];
                float avg = (a + b + c + d) * 0.25f;
                grid[y][x] = avg + randOffset(scale);
            }
        }


        for (int y = 0; y < size; y += half) {
            for (int x = (y + half) % step; x < size; x += step) {
                float a = grid[(y - half + size) % size][x];
                float b = grid[(y + half) % size][x];
                float c = grid[y][(x - half + size) % size];
                float d = grid[y][(x + half) % size];
                float avg = (a + b + c + d) * 0.25f;
                grid[y][x] = avg + randOffset(scale);
            }
        }

        diamondSquare(step / 2, scale * roughness);
    }

public:
    PlasmaFractal(int size = 513, float roughness = 0.55f) : size(size), roughness(roughness) {
        auto isPow2 = [](int v) { return v > 0 && (v & (v - 1)) == 0; };
        int n = size;
        if (!isPow2(n - 1)) n = 513;
        this->size = n;
        grid.assign(this->size, vector<float>(this->size, 0.0f));

        grid[0][0] = 0.0f;
        grid[0][this->size - 1] = 0.0f;
        grid[this->size - 1][0] = 0.0f;
        grid[this->size - 1][this->size - 1] = 0.0f;
        diamondSquare(this->size - 1, 1.0f);
    }

    void regenerate() {
        for (int y = 0; y < size; ++y) for (int x = 0; x < size; ++x) grid[y][x] = 0.0f;
        grid[0][0] = 0.0f;
        grid[0][size - 1] = 0.0f;
        grid[size - 1][0] = 0.0f;
        grid[size - 1][size - 1] = 0.0f;
        diamondSquare(size - 1, 1.0f);
    }

    void draw() const {
        glBegin(GL_POINTS);
        for (int y = 0; y < size; ++y) {
            for (int x = 0; x < size; ++x) {
                float v = grid[y][x];
                float t = (v + 1.0f) * 0.5f;

                float r = sinf(3.14159f * t);
                float g = sinf(3.14159f * (t + 0.33f));
                float b = sinf(3.14159f * (t + 0.66f));
                r = r * r; g = g * g; b = b * b;
                glColor3f(r, g, b);
                float xf = -1.0f + 2.0f * (float)x / (float)(size - 1);
                float yf = -1.0f + 2.0f * (float)y / (float)(size - 1);
                glVertex2f(xf, yf);
            }
        }
        glEnd();
    }
};

Koh_snowflake koh;
PlasmaFractal plasma(1025, 0.6f);

void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-zoom + offset_x, zoom + offset_x,
            -zoom * height/width + offset_y, zoom * height/width + offset_y,
            -1, 1);
    glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y) {
    if (key == 'q' || key == 'Q') {
        is_koh = !is_koh;
        if (is_koh) glColor3f(0.0f, 0.0f, 1.0f);
        reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
        glutPostRedisplay();
        return;
    }

    if(!is_koh){
        if (key == 'r' || key == 'R') {
            plasma.regenerate();
            glutPostRedisplay();
        }
        if (key == 27) {
            exit(0);
        }
    } else{
        if (key == 'n' || key == 'N') {
            koh.q++;
            koh.update();
            glutPostRedisplay();
        }
        if (key == 'p' || key == 'P') {
            koh.q--;
            koh.update();
            glutPostRedisplay();
        }
        if (key == '+' || key == '=') {
            zoom -= zoom_speed;
            if (zoom < 0.000001f) zoom = 0.01f;
            glutPostRedisplay();
            reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
        }
        if (key == '-' || key == '_') {
            zoom += zoom_speed;
            glutPostRedisplay();
            reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
        }
        if (key == 27) {
            exit(0);
        }
    }

}

void specialKeys(int key, int x, int y) {
    if(is_koh) switch(key) {
        case GLUT_KEY_UP:
            offset_y += move_speed;
            glutPostRedisplay();
            reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
            break;
        case GLUT_KEY_DOWN:
            offset_y -= move_speed;
            glutPostRedisplay();
            reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
            break;
        case GLUT_KEY_LEFT:
            offset_x -= move_speed;
            glutPostRedisplay();
            reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
            break;
        case GLUT_KEY_RIGHT:
            offset_x += move_speed;
            glutPostRedisplay();
            reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
            break;
        }
}


void display() {

    glClear(GL_COLOR_BUFFER_BIT);
    if(is_koh){
        koh.draw();
    } else{
        plasma.draw();
    }
    glutSwapBuffers();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitWindowSize(RESOLUTION_W, RESOLUTION_H);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutCreateWindow("Plasma or Koh");
    glutReshapeWindow(RESOLUTION_W, RESOLUTION_H);
    if (is_koh) glColor3f(0.0f, 0.0f, 1.0f);
    glClearColor(0.02f, 0.02f, 0.04f, 1.0f);
    glPointSize(1.0f);
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutReshapeFunc(reshape);
    glutMainLoop();
    return 0;
}
