// Компиляция:
// g++ main.cpp -o octree.exe -lglfw3 -lglew32 -lopengl32 -lgdi32 -std=c++17
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <algorithm>

// Глобальные константы

constexpr unsigned int SCREEN_WIDTH  = 800;
constexpr unsigned int SCREEN_HEIGHT = 600;

constexpr float CAMERA_SPEED = 5.0f;
constexpr float CAMERA_FOV   = 45.0f;

constexpr int   MAX_OBJECTS_PER_NODE = 4;
constexpr float MIN_NODE_SIZE        = 2.0f;

// Параметры камеры

glm::vec3 cameraPosition(0.0f, 0.0f, 10.0f);
glm::vec3 cameraFront   (0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp      (0.0f, 1.0f,  0.0f);

float yaw   = -90.0f;
float pitch =   0.0f;

// Время

float deltaTime  = 0.0f;
float lastFrame  = 0.0f;

// Управление мышью

bool  firstMouse = true;
float lastMouseX = SCREEN_WIDTH  * 0.5f;
float lastMouseY = SCREEN_HEIGHT * 0.5f;

// Ввод с клавиатуры

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    const float velocity = CAMERA_SPEED * deltaTime;
    const glm::vec3 right = glm::normalize(glm::cross(cameraFront, cameraUp));

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPosition += velocity * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPosition -= velocity * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPosition -= velocity * right;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPosition += velocity * right;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        cameraPosition += velocity * cameraUp;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        cameraPosition -= velocity * cameraUp;
}

// Callback мыши

void mouseCallback(GLFWwindow*, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastMouseX = static_cast<float>(xpos);
        lastMouseY = static_cast<float>(ypos);
        firstMouse = false;
    }

    float xOffset = static_cast<float>(xpos) - lastMouseX;
    float yOffset = lastMouseY - static_cast<float>(ypos);

    lastMouseX = static_cast<float>(xpos);
    lastMouseY = static_cast<float>(ypos);

    constexpr float sensitivity = 0.1f;
    xOffset *= sensitivity;
    yOffset *= sensitivity;

    yaw   += xOffset;
    pitch += yOffset;

    pitch = std::clamp(pitch, -89.0f, 89.0f);

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    cameraFront = glm::normalize(direction);
}


// Геометрия куба


void drawColoredCube(float size)
{
    const float h = size * 0.5f;

    glBegin(GL_QUADS);

    glColor3f(1,0,0); glVertex3f(-h,-h, h); glVertex3f( h,-h, h); glVertex3f( h, h, h); glVertex3f(-h, h, h);
    glColor3f(0,1,0); glVertex3f(-h,-h,-h); glVertex3f(-h, h,-h); glVertex3f( h, h,-h); glVertex3f( h,-h,-h);
    glColor3f(0,0,1); glVertex3f(-h, h,-h); glVertex3f(-h, h, h); glVertex3f( h, h, h); glVertex3f( h, h,-h);
    glColor3f(1,1,0); glVertex3f(-h,-h,-h); glVertex3f( h,-h,-h); glVertex3f( h,-h, h); glVertex3f(-h,-h, h);
    glColor3f(1,0,1); glVertex3f( h,-h,-h); glVertex3f( h, h,-h); glVertex3f( h, h, h); glVertex3f( h,-h, h);
    glColor3f(0,1,1); glVertex3f(-h,-h,-h); glVertex3f(-h,-h, h); glVertex3f(-h, h, h); glVertex3f(-h, h,-h);

    glEnd();
}

// Математика плоскости и фрустум

struct Plane
{
    glm::vec3 normal;
    float distance;

    Plane() : normal(0,1,0), distance(0) {}
    Plane(const glm::vec3& point, const glm::vec3& n)
        : normal(glm::normalize(n)), distance(glm::dot(normal, point)) {}

    float signedDistance(const glm::vec3& p) const
    {
        return glm::dot(normal, p) - distance;
    }
};

struct Frustum
{
    Plane left, right, top, bottom, nearP, farP;
};

// AABB 

struct AABB
{
    glm::vec3 center;
    glm::vec3 extents;

    AABB(const glm::vec3& min, const glm::vec3& max)
        : center((min + max) * 0.5f),
          extents((max - min) * 0.5f) {}

    bool isOnPlane(const Plane& p) const
    {
        const float r =
            extents.x * std::abs(p.normal.x) +
            extents.y * std::abs(p.normal.y) +
            extents.z * std::abs(p.normal.z);

        return -r <= p.signedDistance(center);
    }

    bool isInsideFrustum(const Frustum& f) const
    {
        return isOnPlane(f.left)   && isOnPlane(f.right) &&
               isOnPlane(f.top)    && isOnPlane(f.bottom) &&
               isOnPlane(f.nearP)  && isOnPlane(f.farP);
    }
};

// Объект сцены

class Cube
{
public:
    glm::vec3 position;
    float size;
    AABB bounds;

    Cube(const glm::vec3& pos, float s)
        : position(pos), size(s),
          bounds(pos - glm::vec3(s * 0.5f),
                 pos + glm::vec3(s * 0.5f)) {}

    void draw() const
    {
        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);
        drawColoredCube(size);
        glPopMatrix();
    }
};

// Октарное дерево

class OctreeNode
{
public:
    AABB bounds;
    std::vector<Cube*> objects;
    OctreeNode* children[8]{};
    bool isLeaf = true;

    explicit OctreeNode(const AABB& box) : bounds(box) {}

    ~OctreeNode()
    {
        for (auto* c : children)
            delete c;
    }

    void insert(Cube* obj)
    {
        if (!contains(obj->position))
            return;

        if (isLeaf)
        {
            if (objects.size() < MAX_OBJECTS_PER_NODE ||
                bounds.extents.x < MIN_NODE_SIZE)
            {
                objects.push_back(obj);
                return;
            }

            subdivide();
            for (Cube* existing : objects)
                insertIntoChild(existing);

            objects.clear();
            isLeaf = false;
        }

        insertIntoChild(obj);
    }

    void traverse(const Frustum& frustum,
                  std::vector<Cube*>& visible) const
    {
        if (!bounds.isInsideFrustum(frustum))
            return;

        for (Cube* c : objects)
        {
            if (c->bounds.isInsideFrustum(frustum))
            {
                c->draw();
                visible.push_back(c);
            }
        }

        if (!isLeaf)
            for (const auto* child : children)
                if (child) child->traverse(frustum, visible);
    }

private:
    bool contains(const glm::vec3& p) const
    {
        return (p.x >= bounds.center.x - bounds.extents.x &&
                p.x <= bounds.center.x + bounds.extents.x &&
                p.y >= bounds.center.y - bounds.extents.y &&
                p.y <= bounds.center.y + bounds.extents.y &&
                p.z >= bounds.center.z - bounds.extents.z &&
                p.z <= bounds.center.z + bounds.extents.z);
    }

    void subdivide()
    {
        const glm::vec3 min = bounds.center - bounds.extents;
        const glm::vec3 max = bounds.center + bounds.extents;
        const glm::vec3 mid = bounds.center;

        children[0] = new OctreeNode({min, mid});
        children[1] = new OctreeNode({{mid.x,min.y,min.z},{max.x,mid.y,mid.z}});
        children[2] = new OctreeNode({{mid.x,mid.y,min.z},{max.x,max.y,mid.z}});
        children[3] = new OctreeNode({{min.x,mid.y,min.z},{mid.x,max.y,mid.z}});
        children[4] = new OctreeNode({{min.x,min.y,mid.z},{mid.x,mid.y,max.z}});
        children[5] = new OctreeNode({{mid.x,min.y,mid.z},{max.x,mid.y,max.z}});
        children[6] = new OctreeNode({mid, max});
        children[7] = new OctreeNode({{min.x,mid.y,mid.z},{mid.x,max.y,max.z}});
    }

    void insertIntoChild(Cube* obj)
    {
        int index = 0;
        if (obj->position.x >= bounds.center.x) index |= 1;
        if (obj->position.y >= bounds.center.y) index |= 2;
        if (obj->position.z >= bounds.center.z) index |= 4;

        children[index]->insert(obj);
    }
};
