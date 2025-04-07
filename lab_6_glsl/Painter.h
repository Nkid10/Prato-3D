#ifndef PAINTER_H
#define PAINTER_H
#define STB_IMAGE_IMPLEMENTATION

#include "Matrix.h"
#include "Vertex.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "gl/glew.h"
#include <vector>
#include <corecrt_math_defines.h>
#include <memory>
#include "stb_image.h"

// Variáveis globais
float running_time = 0.0f;
struct {
    int x, y;
} mousePosition;

Matrix rotateMatrix;
GLuint textureID;
GLuint dryTextureID;
GLuint shaderProgram;

// Parâmetros de secagem
float dry_radius = 0.9f;
float dry_center_x = 0.0f;
float dry_center_y = 0.5f;
float dry_intensity = 0.5f;
float dry_edge_smoothness = 0.2f;

void changeRotateMatrix(int dx, int dy) {
    rotateMatrix = rotateMatrix * Matrix::RotationY(static_cast<float>(dx) / 50.0f) *
        Matrix::RotationX(static_cast<float>(dy) / 50.0f);
}

GLuint CreateShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        printf("Erro de compilação do shader: %s\n", infoLog);
    }
    return shader;
}

void LoadTextures() {
    int width, height, nrChannels;
    unsigned char* data = stbi_load("C:/Users/tihdi/Documents/lab_6_glsl/lab_6_glsl/lab_6_glsl/porcelain_texture.jpg",
        &width, &height, &nrChannels, 0);
    if (!data) {
        printf("Falha ao carregar a textura principal\n");
        return;
    }

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    unsigned char dryColor[3] = { 200, 180, 160 };
    unsigned char* dryData = new unsigned char[width * height * 3];
    for (int i = 0; i < width * height * 3; i += 3) {
        dryData[i] = dryColor[0];
        dryData[i + 1] = dryColor[1];
        dryData[i + 2] = dryColor[2];
    }

    glGenTextures(1, &dryTextureID);
    glBindTexture(GL_TEXTURE_2D, dryTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, dryData);
    glGenerateMipmap(GL_TEXTURE_2D);
    delete[] dryData;
}

void InitOpenGL() {
    const char* vertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec3 position;
        layout(location = 1) in vec3 normal;
        layout(location = 2) in vec2 texCoord;
        uniform mat4 general_matrix;
        out vec3 FragPos;
        out vec3 Normal;
        out vec2 TexCoord;
        void main() {
            gl_Position = general_matrix * vec4(position, 1.0);
            FragPos = vec3(general_matrix * vec4(position, 1.0));
            Normal = mat3(transpose(inverse(general_matrix))) * normal;
            TexCoord = texCoord;
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        in vec3 FragPos;
        in vec3 Normal;
        in vec2 TexCoord;
        out vec4 color;
        
        uniform sampler2D wetTexture;
        uniform sampler2D dryTexture;
        uniform vec3 lightPos = vec3(3.0, 3.0, 3.0);
        uniform vec3 lightColor = vec3(1.0, 1.0, 1.0);
        uniform vec3 viewPos = vec3(0.0, 0.0, 5.0);
        
        uniform vec2 dryCenter = vec2(0.0, 0.5);
        uniform float dryRadius = 0.7;
        uniform float dryIntensity = 0.5;
        uniform float dryEdgeSmooth = 0.2;
        
        void main() {
            float distToDryCenter = distance(TexCoord, dryCenter);
            float dryFactor = smoothstep(dryRadius - dryEdgeSmooth, dryRadius + dryEdgeSmooth, distToDryCenter);
            dryFactor = mix(1.0 - dryIntensity, 1.0, dryFactor);
            
            vec4 wetColor = texture(wetTexture, TexCoord);
            vec4 dryColor = texture(dryTexture, TexCoord);
            vec4 finalColor = mix(dryColor, wetColor, dryFactor);
            
            float ambientStrength = 0.5;
            vec3 ambient = ambientStrength * lightColor;
            vec3 norm = normalize(Normal);
            vec3 lightDir = normalize(lightPos - FragPos);
            float diff = max(dot(norm, lightDir), 0.0);
            vec3 diffuse = diff * lightColor;
            float specularStrength = 0.5;
            vec3 viewDir = normalize(viewPos - FragPos);
            vec3 reflectDir = reflect(-lightDir, norm);  
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
            vec3 specular = specularStrength * spec * lightColor;
            
            vec3 result = (ambient + diffuse + specular) * finalColor.rgb;
            color = vec4(result, 1.0);
        }
    )";

    GLuint vertexShader = CreateShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = CreateShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        printf("Erro de ligação do shader: %s\n", infoLog);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glEnable(GL_DEPTH_TEST);
    LoadTextures();
}

struct PlateData {
    std::vector<GLfloat> vertices;
    std::vector<GLfloat> normals;
    std::vector<GLfloat> texCoords;
    std::vector<GLuint> indices;
};

std::unique_ptr<PlateData> CreatePlate() {
    auto data = std::make_unique<PlateData>();
    const int sectors = 300;
    const int rings = 100;
    const float radius = 2.0f;

    // Reserva espaço
    data->vertices.reserve((sectors + 1) * (rings + 1) * 3);
    data->normals.reserve((sectors + 1) * (rings + 1) * 3);
    data->texCoords.reserve((sectors + 1) * (rings + 1) * 2);
    data->indices.reserve(rings * sectors * 6);

    // Gera vértices
    for (int ring = 0; ring <= rings; ++ring) {
        float r = static_cast<float>(ring) / rings;
        float z = -0.6f + pow(r, 1.7f) * 1.0f;

        for (int seg = 0; seg <= sectors; ++seg) {
            float angle = 2.0f * static_cast<float>(M_PI) * static_cast<float>(seg) / sectors;
            float x = r * radius * cosf(angle);
            float y = r * radius * sinf(angle);

            data->vertices.push_back(x);
            data->vertices.push_back(y);
            data->vertices.push_back(z);

            glm::vec3 normal = glm::normalize(glm::vec3(x, y, -z));
            data->normals.push_back(normal.x);
            data->normals.push_back(normal.y);
            data->normals.push_back(normal.z);

            float u = 0.5f + (x / (2.0f * radius));
            float v = 0.5f + (y / (2.0f * radius));
            data->texCoords.push_back(u);
            data->texCoords.push_back(v);
        }
    }

    // Gera índices dos triângulos
    for (int ring = 0; ring < rings; ++ring) {
        for (int seg = 0; seg < sectors; ++seg) {
            int current = ring * (sectors + 1) + seg;
            int next = current + 1;
            int below = (ring + 1) * (sectors + 1) + seg;
            int belowNext = below + 1;

            // Triângulo 1
            data->indices.push_back(current);
            data->indices.push_back(next);
            data->indices.push_back(below);

            // Triângulo 2
            data->indices.push_back(next);
            data->indices.push_back(belowNext);
            data->indices.push_back(below);
        }
    }

    return data;
}

void Draw(int width, int height) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glUseProgram(shaderProgram);

    auto plateData = CreatePlate();

    // Matriz de transformação
    float angle = running_time;
    Matrix general_matrix =
        (Matrix::RotationX(angle / 8.0f) *
            Matrix::RotationY(angle / 16.0f) *
            rotateMatrix *
            Matrix::Translation(0, 0, -4)) *
        Matrix::Perspective(45.0f, static_cast<float>(width) / height, 0.1f, 100.0f);

    GLuint matrixLocation = glGetUniformLocation(shaderProgram, "general_matrix");
    glUniformMatrix4fv(matrixLocation, 1, GL_FALSE, &general_matrix.M[0][0]);

    // Configura texturas
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glUniform1i(glGetUniformLocation(shaderProgram, "wetTexture"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, dryTextureID);
    glUniform1i(glGetUniformLocation(shaderProgram, "dryTexture"), 1);

    // Configura parâmetros de secagem
    glUniform2f(glGetUniformLocation(shaderProgram, "dryCenter"), dry_center_x, dry_center_y);
    glUniform1f(glGetUniformLocation(shaderProgram, "dryRadius"), dry_radius);
    glUniform1f(glGetUniformLocation(shaderProgram, "dryIntensity"), dry_intensity);
    glUniform1f(glGetUniformLocation(shaderProgram, "dryEdgeSmooth"), dry_edge_smoothness);

    // Configura buffers
    GLuint VAO, VBO, normalVBO, texCoordVBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &normalVBO);
    glGenBuffers(1, &texCoordVBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // Vértices
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, plateData->vertices.size() * sizeof(GLfloat), plateData->vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    // Normais
    glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
    glBufferData(GL_ARRAY_BUFFER, plateData->normals.size() * sizeof(GLfloat), plateData->normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(1);

    // Coordenadas de textura
    glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO);
    glBufferData(GL_ARRAY_BUFFER, plateData->texCoords.size() * sizeof(GLfloat), plateData->texCoords.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(2);

    // Índices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, plateData->indices.size() * sizeof(GLuint), plateData->indices.data(), GL_STATIC_DRAW);

    // Renderização
    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLES, plateData->indices.size(), GL_UNSIGNED_INT, 0);

    // Limpeza
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &normalVBO);
    glDeleteBuffers(1, &texCoordVBO);
    glDeleteBuffers(1, &EBO);
}

#endif // PAINTER_H