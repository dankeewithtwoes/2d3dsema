// Компилируется в VS code в terminal с помощью команды g++ main.cpp -o octree.exe -lglfw3 -lglew32 -lopengl32 -lgdi32 -std=c++17
// Затем запускается путем ввода в терминал команды .\octree.exe
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

// Window dimensions
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Camera parameters
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 10.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float fov = 45.0f;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Mouse control
bool firstMouse = true;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
float yaw = -90.0f;
float pitch = 0.0f;

// Process input
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = 5.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraUp;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraUp;
}

// Mouse callback
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}
// Функция для отрисовки куба одним цветом (для вида сверху)
void drawCubeSingleColor(float size, float r, float g, float b) {
    float half = size / 2.0f;
    
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    
    // Все грани одним цветом
    // Передняя грань
    glVertex3f(-half, -half, half);
    glVertex3f(half, -half, half);
    glVertex3f(half, half, half);
    glVertex3f(-half, half, half);
    
    // Задняя грань
    glVertex3f(-half, -half, -half);
    glVertex3f(-half, half, -half);
    glVertex3f(half, half, -half);
    glVertex3f(half, -half, -half);
    
    // Верхняя грань
    glVertex3f(-half, half, -half);
    glVertex3f(-half, half, half);
    glVertex3f(half, half, half);
    glVertex3f(half, half, -half);
    
    // Нижняя грань
    glVertex3f(-half, -half, -half);
    glVertex3f(half, -half, -half);
    glVertex3f(half, -half, half);
    glVertex3f(-half, -half, half);
    
    // Правая грань
    glVertex3f(half, -half, -half);
    glVertex3f(half, half, -half);
    glVertex3f(half, half, half);
    glVertex3f(half, -half, half);
    
    // Левая грань
    glVertex3f(-half, -half, -half);
    glVertex3f(-half, -half, half);
    glVertex3f(-half, half, half);
    glVertex3f(-half, half, -half);
    glEnd();
}
// Функция для отрисовки куба
void drawCube(float size) {
    float half = size / 2.0f;
    
    // Передняя грань
    glBegin(GL_QUADS);
    glColor3f(1.0f, 0.0f, 0.0f); // Красный
    glVertex3f(-half, -half, half);
    glVertex3f(half, -half, half);
    glVertex3f(half, half, half);
    glVertex3f(-half, half, half);
    
    // Задняя грань
    glColor3f(0.0f, 1.0f, 0.0f); // Зеленый
    glVertex3f(-half, -half, -half);
    glVertex3f(-half, half, -half);
    glVertex3f(half, half, -half);
    glVertex3f(half, -half, -half);
    
    // Верхняя грань
    glColor3f(0.0f, 0.0f, 1.0f); // Синий
    glVertex3f(-half, half, -half);
    glVertex3f(-half, half, half);
    glVertex3f(half, half, half);
    glVertex3f(half, half, -half);
    
    // Нижняя грань
    glColor3f(1.0f, 1.0f, 0.0f); // Желтый
    glVertex3f(-half, -half, -half);
    glVertex3f(half, -half, -half);
    glVertex3f(half, -half, half);
    glVertex3f(-half, -half, half);
    
    // Правая грань
    glColor3f(1.0f, 0.0f, 1.0f); // Пурпурный
    glVertex3f(half, -half, -half);
    glVertex3f(half, half, -half);
    glVertex3f(half, half, half);
    glVertex3f(half, -half, half);
    
    // Левая грань
    glColor3f(0.0f, 1.0f, 1.0f); // Голубой
    glVertex3f(-half, -half, -half);
    glVertex3f(-half, -half, half);
    glVertex3f(-half, half, half);
    glVertex3f(-half, half, -half);
    glEnd();
}

// Plane structure
struct Plane {
    glm::vec3 normal = {0.f, 1.f, 0.f};
    float distance = 0.f;

    Plane() = default;

    Plane(const glm::vec3& p1, const glm::vec3& norm)
        : normal(glm::normalize(norm)), distance(glm::dot(normal, p1)) {}

    float getSignedDistanceToPlane(const glm::vec3& point) const {
        return glm::dot(normal, point) - distance;
    }
};

// Frustum structure
struct Frustum {
    Plane topFace;
    Plane bottomFace;
    Plane rightFace;
    Plane leftFace;
    Plane farFace;
    Plane nearFace;
};

// Create frustum from camera
Frustum createFrustumFromCamera(const glm::vec3& pos, const glm::vec3& front, const glm::vec3& up, 
                                const glm::vec3& right, float aspect, float fovY, float zNear, float zFar) {
    Frustum frustum;
    const float halfVSide = zFar * tanf(glm::radians(fovY) * 0.5f);
    const float halfHSide = halfVSide * aspect;
    const glm::vec3 frontMultFar = zFar * front;

    frustum.nearFace = Plane(pos + zNear * front, front);
    frustum.farFace = Plane(pos + frontMultFar, -front);
    frustum.rightFace = Plane(pos, glm::cross(up, frontMultFar + right * halfHSide));
    frustum.leftFace = Plane(pos, glm::cross(frontMultFar - right * halfHSide, up));
    frustum.topFace = Plane(pos, glm::cross(right, frontMultFar - up * halfVSide));
    frustum.bottomFace = Plane(pos, glm::cross(frontMultFar + up * halfVSide, right));

    return frustum;
}

// AABB structure
struct AABB {
    glm::vec3 center = {0.f, 0.f, 0.f};
    glm::vec3 extents = {0.f, 0.f, 0.f};

    AABB(const glm::vec3& min, const glm::vec3& max)
        : center{(max + min) * 0.5f}, extents{(max - min) * 0.5f} {}

    bool isOnOrForwardPlane(const Plane& plane) const {
        const float r = extents.x * std::abs(plane.normal.x) + 
                       extents.y * std::abs(plane.normal.y) + 
                       extents.z * std::abs(plane.normal.z);
        return -r <= plane.getSignedDistanceToPlane(center);
    }

    bool isOnFrustum(const Frustum& frustum) const {
        return (isOnOrForwardPlane(frustum.leftFace) &&
                isOnOrForwardPlane(frustum.rightFace) &&
                isOnOrForwardPlane(frustum.topFace) &&
                isOnOrForwardPlane(frustum.bottomFace) &&
                isOnOrForwardPlane(frustum.nearFace) &&
                isOnOrForwardPlane(frustum.farFace));
    }
};

// Cube class
class Cube {
public:
    glm::vec3 position;
    float size;
    AABB boundingBox;

    Cube(glm::vec3 pos, float s) : position(pos), size(s), 
        boundingBox(pos - glm::vec3(s/2), pos + glm::vec3(s/2)) {}

    void draw() const {
        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);
        drawCube(size);
        glPopMatrix();
    }
};

// Octree Node
class OctreeNode {
public:
    AABB bounds;
    std::vector<Cube*> cubes;
    OctreeNode* children[8] = {nullptr};
    bool isLeaf = true;

    OctreeNode(const AABB& b) : bounds(b) {}

    ~OctreeNode() {
        for (int i = 0; i < 8; ++i) {
            delete children[i];
        }
    }

    void insert(Cube* cube) {
        // Проверяем, находится ли куб в пределах этого узла
        if (!isPointInBounds(cube->position)) {
            return;
        }

        if (isLeaf) {
            if (cubes.size() < 4 || (bounds.extents.x < 2.0f)) {
                cubes.push_back(cube);
            } else {
                subdivide();
                for (auto* c : cubes) {
                    insertIntoChild(c);
                }
                cubes.clear();
                insertIntoChild(cube);
                isLeaf = false;
            }
        } else {
            insertIntoChild(cube);
        }
    }

    void subdivide() {
        glm::vec3 min = bounds.center - bounds.extents;
        glm::vec3 max = bounds.center + bounds.extents;
        glm::vec3 mid = bounds.center;

        children[0] = new OctreeNode(AABB(glm::vec3(min.x, min.y, min.z), glm::vec3(mid.x, mid.y, mid.z)));
        children[1] = new OctreeNode(AABB(glm::vec3(mid.x, min.y, min.z), glm::vec3(max.x, mid.y, mid.z)));
        children[2] = new OctreeNode(AABB(glm::vec3(mid.x, mid.y, min.z), glm::vec3(max.x, max.y, mid.z)));
        children[3] = new OctreeNode(AABB(glm::vec3(min.x, mid.y, min.z), glm::vec3(mid.x, max.y, mid.z)));
        children[4] = new OctreeNode(AABB(glm::vec3(min.x, min.y, mid.z), glm::vec3(mid.x, mid.y, max.z)));
        children[5] = new OctreeNode(AABB(glm::vec3(mid.x, min.y, mid.z), glm::vec3(max.x, mid.y, max.z)));
        children[6] = new OctreeNode(AABB(glm::vec3(mid.x, mid.y, mid.z), glm::vec3(max.x, max.y, max.z)));
        children[7] = new OctreeNode(AABB(glm::vec3(min.x, mid.y, mid.z), glm::vec3(mid.x, max.y, max.z)));
    }

    bool isPointInBounds(const glm::vec3& point) const {
        return (point.x >= bounds.center.x - bounds.extents.x &&
                point.x <= bounds.center.x + bounds.extents.x &&
                point.y >= bounds.center.y - bounds.extents.y &&
                point.y <= bounds.center.y + bounds.extents.y &&
                point.z >= bounds.center.z - bounds.extents.z &&
                point.z <= bounds.center.z + bounds.extents.z);
    }

    void insertIntoChild(Cube* cube) {
        glm::vec3 pos = cube->position;
        glm::vec3 center = bounds.center;
        
        int index = 0;
        if (pos.x >= center.x) index |= 1;
        if (pos.y >= center.y) index |= 2;
        if (pos.z >= center.z) index |= 4;
        
        if (children[index]) {
            children[index]->insert(cube);
        }
    }

    void traverseAndRender(const Frustum& frustum, std::vector<Cube*>& visibleCubes) const {
        if (!bounds.isOnFrustum(frustum)) {
            return;
        }

        for (auto* cube : cubes) {
            if (cube->boundingBox.isOnFrustum(frustum)) {
                cube->draw();
                visibleCubes.push_back(cube);
            }
        }

        if (!isLeaf) {
            for (int i = 0; i < 8; ++i) {
                if (children[i]) {
                    children[i]->traverseAndRender(frustum, visibleCubes);
                }
            }
        }
    }
};

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    
    // Create window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Octree Frustum Culling", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    
    // Set mouse callback
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Print OpenGL version
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    // Configure global OpenGL state
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    // После glewInit добавьте:
glMatrixMode(GL_PROJECTION);
glLoadIdentity();

glMatrixMode(GL_MODELVIEW);
glLoadIdentity();
    
    // Simple lighting (optional)
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    
    float lightPos[] = {0.0f, 10.0f, 10.0f, 1.0f};
    float lightAmbient[] = {0.2f, 0.2f, 0.2f, 1.0f};
    float lightDiffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};
    
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);

    // Create octree
    AABB sceneBounds(glm::vec3(-50, -50, -50), glm::vec3(50, 50, 50));
    OctreeNode* octree = new OctreeNode(sceneBounds);

    // Generate random cubes
    std::cout << "Generating cubes..." << std::endl;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-45.0, 45.0);
    
    std::vector<Cube*> allCubes;
    int numCubes = 200;
    
    for (int i = 0; i < numCubes; ++i) {
        glm::vec3 pos(dis(gen), dis(gen), dis(gen));
        Cube* cube = new Cube(pos, 3.0f);
        allCubes.push_back(cube);
        octree->insert(cube);
    }
    
    std::cout << "Generated " << allCubes.size() << " cubes" << std::endl;

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        // Calculate delta time
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Process input
        processInput(window);

        // Clear buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Calculate camera right vector and frustum
        glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFront, cameraUp));
        float aspectRatio = (float)(SCR_WIDTH / 2) / (float)SCR_HEIGHT;
        Frustum camFrustum = createFrustumFromCamera(cameraPos, cameraFront, cameraUp, 
                                                     cameraRight, aspectRatio, fov, 0.1f, 100.0f);

        std::vector<Cube*> visibleCubes;

        // Left viewport: Main 3D view with frustum culling
        glViewport(0, 0, SCR_WIDTH / 2, SCR_HEIGHT);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glm::mat4 projection = glm::perspective(glm::radians(fov), aspectRatio, 0.1f, 100.0f);
        glMultMatrixf(glm::value_ptr(projection));
        
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glMultMatrixf(glm::value_ptr(view));

        // Draw coordinate axes
        glDisable(GL_LIGHTING);
        glLineWidth(3.0f);
        glBegin(GL_LINES);
        glColor3f(1.0f, 0.0f, 0.0f); // X - Red
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(5.0f, 0.0f, 0.0f);
        glColor3f(0.0f, 1.0f, 0.0f); // Y - Green
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 5.0f, 0.0f);
        glColor3f(0.0f, 0.0f, 1.0f); // Z - Blue
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 5.0f);
        glEnd();
        glEnable(GL_LIGHTING);

        // Render with octree and frustum culling
        octree->traverseAndRender(camFrustum, visibleCubes);

        // Right viewport: Top-down view showing visible cubes
glViewport(SCR_WIDTH / 2, 0, SCR_WIDTH / 2, SCR_HEIGHT);
glMatrixMode(GL_PROJECTION);
glLoadIdentity();

glOrtho(-60.0f, 60.0f, -60.0f, 60.0f, -100.0f, 100.0f);

glMatrixMode(GL_MODELVIEW);
glLoadIdentity();

// Вид сверху: камера над сценой, смотрит вниз
glm::mat4 topDownViewMatrix = glm::lookAt(
    glm::vec3(0.0f, 100.0f, 0.0f),  // Камера сверху
    glm::vec3(0.0f, 0.0f, 0.0f),    // Смотрим в центр
    glm::vec3(0.0f, 0.0f, -1.0f)    // Вектор "вверх"
);
glMultMatrixf(glm::value_ptr(topDownViewMatrix));

glEnable(GL_COLOR_MATERIAL);
glDisable(GL_LIGHTING);

// Draw ground plane
glColor3f(0.2f, 0.2f, 0.2f);
glBegin(GL_QUADS);
glVertex3f(-100.0f, 0.0f, -100.0f);
glVertex3f(100.0f, 0.0f, -100.0f);
glVertex3f(100.0f, 0.0f, 100.0f);
glVertex3f(-100.0f, 0.0f, 100.0f);
glEnd();

// Draw coordinate axes
glLineWidth(2.0f);
glBegin(GL_LINES);
glColor3f(1.0f, 0.0f, 0.0f); // X - Red
glVertex3f(0.0f, 0.0f, 0.0f);
glVertex3f(20.0f, 0.0f, 0.0f);
glColor3f(0.0f, 0.0f, 1.0f); // Z - Blue
glVertex3f(0.0f, 0.0f, 0.0f);
glVertex3f(0.0f, 0.0f, 20.0f);
glEnd();

// Draw camera position in top-down view
glColor3f(1.0f, 0.0f, 1.0f);
glPointSize(10.0f);
glBegin(GL_POINTS);
glVertex3f(cameraPos.x, 0.0f, cameraPos.z);
glEnd();

// Draw camera direction
glLineWidth(2.0f);
glBegin(GL_LINES);
glVertex3f(cameraPos.x, 0.0f, cameraPos.z);
glVertex3f(cameraPos.x + cameraFront.x * 10.0f, 0.0f, cameraPos.z + cameraFront.z * 10.0f);
glEnd();

// Draw all cubes
for (auto* cube : allCubes) {
    // Проверяем, виден ли куб из основной камеры
    bool isVisible = false;
    for (auto* visibleCube : visibleCubes) {
        if (visibleCube == cube) {
            isVisible = true;
            break;
        }
    }
    
    if (isVisible) {
        glColor3f(0.0f, 1.0f, 0.0f); // Зеленый - видимые кубы
    } else {
        glColor3f(0.5f, 0.5f, 0.5f); // Серый - невидимые кубы
    }
    
    // Рисуем куб как квадрат (для вида сверху)
    float half = cube->size / 2.0f;
    glBegin(GL_QUADS);
    glVertex3f(cube->position.x - half, 0.01f, cube->position.z - half);
    glVertex3f(cube->position.x + half, 0.01f, cube->position.z - half);
    glVertex3f(cube->position.x + half, 0.01f, cube->position.z + half);
    glVertex3f(cube->position.x - half, 0.01f, cube->position.z + half);
    glEnd();
}

// Включите освещение обратно если нужно
glEnable(GL_LIGHTING);

// Draw camera frustum visualization in top-down view
glColor3f(1.0f, 1.0f, 0.0f);
glLineWidth(1.0f);
glBegin(GL_LINES);

// Для вида сверху нам нужно спроецировать позицию камеры и frustum на плоскость XZ
glm::vec3 cameraPos2D = glm::vec3(cameraPos.x, 0.0f, cameraPos.z);
glm::vec3 cameraFront2D = glm::normalize(glm::vec3(cameraFront.x, 0.0f, cameraFront.z));

// Создаем точки frustum для 2D вида сверху
float nearDist = 0.1f;
float farDist = 10.0f;
float fovRad = glm::radians(fov);
float aspect = (float)SCR_WIDTH / (float)SCR_HEIGHT;

float nearHeight = nearDist * tan(fovRad / 2.0f);
float nearWidth = nearHeight * aspect;
float farHeight = farDist * tan(fovRad / 2.0f);
float farWidth = farHeight * aspect;

// Боковые векторы для frustum
glm::vec3 right = glm::normalize(glm::cross(cameraFront, cameraUp));
glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

// Создаем 4 угла near plane
glm::vec3 nearCenter = cameraPos + cameraFront * nearDist;
glm::vec3 nearTL = nearCenter + up * nearHeight - right * nearWidth;
glm::vec3 nearTR = nearCenter + up * nearHeight + right * nearWidth;
glm::vec3 nearBL = nearCenter - up * nearHeight - right * nearWidth;
glm::vec3 nearBR = nearCenter - up * nearHeight + right * nearWidth;

// Создаем 4 угла far plane
glm::vec3 farCenter = cameraPos + cameraFront * farDist;
glm::vec3 farTL = farCenter + up * farHeight - right * farWidth;
glm::vec3 farTR = farCenter + up * farHeight + right * farWidth;
glm::vec3 farBL = farCenter - up * farHeight - right * farWidth;
glm::vec3 farBR = farCenter - up * farHeight + right * farWidth;

// Проецируем точки на плоскость XZ (вид сверху)
auto projectTo2D = [](const glm::vec3& p) -> glm::vec3 {
    return glm::vec3(p.x, 0.0f, p.z);
};

// Рисуем линии frustum в 2D
glm::vec3 cam2D = projectTo2D(cameraPos);
glm::vec3 nTL = projectTo2D(nearTL);
glm::vec3 nTR = projectTo2D(nearTR);
glm::vec3 nBL = projectTo2D(nearBL);
glm::vec3 nBR = projectTo2D(nearBR);
glm::vec3 fTL = projectTo2D(farTL);
glm::vec3 fTR = projectTo2D(farTR);
glm::vec3 fBL = projectTo2D(farBL);
glm::vec3 fBR = projectTo2D(farBR);

// От камеры до near plane
glVertex3f(cam2D.x, 0.0f, cam2D.z); glVertex3f(nTL.x, 0.0f, nTL.z);
glVertex3f(cam2D.x, 0.0f, cam2D.z); glVertex3f(nTR.x, 0.0f, nTR.z);
glVertex3f(cam2D.x, 0.0f, cam2D.z); glVertex3f(nBL.x, 0.0f, nBL.z);
glVertex3f(cam2D.x, 0.0f, cam2D.z); glVertex3f(nBR.x, 0.0f, nBR.z);

// Near plane
glVertex3f(nTL.x, 0.0f, nTL.z); glVertex3f(nTR.x, 0.0f, nTR.z);
glVertex3f(nTR.x, 0.0f, nTR.z); glVertex3f(nBR.x, 0.0f, nBR.z);
glVertex3f(nBR.x, 0.0f, nBR.z); glVertex3f(nBL.x, 0.0f, nBL.z);
glVertex3f(nBL.x, 0.0f, nBL.z); glVertex3f(nTL.x, 0.0f, nTL.z);

// Far plane
glVertex3f(fTL.x, 0.0f, fTL.z); glVertex3f(fTR.x, 0.0f, fTR.z);
glVertex3f(fTR.x, 0.0f, fTR.z); glVertex3f(fBR.x, 0.0f, fBR.z);
glVertex3f(fBR.x, 0.0f, fBR.z); glVertex3f(fBL.x, 0.0f, fBL.z);
glVertex3f(fBL.x, 0.0f, fBL.z); glVertex3f(fTL.x, 0.0f, fTL.z);

// Соединяем near и far planes
glVertex3f(nTL.x, 0.0f, nTL.z); glVertex3f(fTL.x, 0.0f, fTL.z);
glVertex3f(nTR.x, 0.0f, nTR.z); glVertex3f(fTR.x, 0.0f, fTR.z);
glVertex3f(nBL.x, 0.0f, nBL.z); glVertex3f(fBL.x, 0.0f, fBL.z);
glVertex3f(nBR.x, 0.0f, nBR.z); glVertex3f(fBR.x, 0.0f, fBR.z);

glEnd();

// Draw camera position and direction in top-down view
glColor3f(1.0f, 0.0f, 1.0f); // Пурпурный
glPointSize(8.0f);
glBegin(GL_POINTS);
glVertex3f(cameraPos.x, 0.0f, cameraPos.z); // Позиция камеры
glEnd();

// Направление камеры
glBegin(GL_LINES);
glVertex3f(cameraPos.x, 0.0f, cameraPos.z);
glVertex3f(cameraPos.x + cameraFront.x * 5.0f, 0.0f, cameraPos.z + cameraFront.z * 5.0f);
glEnd();

glEnable(GL_LIGHTING);

        // Display visible cubes count
        std::cout << "Visible cubes: " << visibleCubes.size() << "/" << numCubes << "\r" << std::flush;

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    delete octree;
    for (auto* cube : allCubes) {
        delete cube;
    }
    
    glfwTerminate();
    return 0;
}
