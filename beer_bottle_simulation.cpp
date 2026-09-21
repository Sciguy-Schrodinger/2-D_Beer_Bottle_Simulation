#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

using namespace std;
using namespace glm;

int width = 1000;
int height = 1000;

unsigned int shaderProgram, colourShaderProgram, pointShaderProgram, VAO_bottle, VBO_bottle, EBO_bottle, VAO_liquid, VBO_liquid, EBO_liquid, VAO_bubbles, VBO_bubbles;

struct Bubble{
    float x, y;
    float speed;
    float size;
    float phase;
};

std::vector<Bubble> bubbles;

float random_number(float min, float max){
return min + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(max-min)));
}

// Add this function to create a point shader for bubbles
unsigned int createPointShader(){
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec2 pos;
        layout (location = 1) in float size;
        void main(){
            gl_Position = vec4(pos, 0.0, 1.0);
            gl_PointSize = 100 * size; // Scale factor for visibility
        } 
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        void main() {
            // Create a circular point
            vec2 center = gl_PointCoord - vec2(0.5);
            float dist = length(center);
            if(dist > 0.5) discard;
            
            // Soft edge
            float alpha = 1.0 - smoothstep(0.3, 0.5, dist);
            float red = 0.9;
            float green = 0.6;
            float blue = 0.1; 
            FragColor = vec4(red, green, blue, alpha * 0.3);
        }
    )";

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << endl;
    }

    unsigned int program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if(!success){
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return program;
}

unsigned int createColourShader(){
  const char* vertexShaderSource = R"(
      #version 330 core
      layout (location = 0) in vec3 pos;
      void main(){
      gl_Position = vec4(pos,1.0);
      }
)";

  const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        uniform vec4 liquidColor;
        void main() {
            FragColor = liquidColor;
        }
    )";

  // Compile vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
    }

    // Compile fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << endl;
    }

    // Link program
    unsigned int program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if(!success){
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return program;
}

unsigned int shaders(){
const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec2 aTexCoord;

    out vec2 TexCoord;

    void main() {
        gl_Position = vec4(aPos, 1.0);
        TexCoord = aTexCoord;
    }
)";

const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;

    in vec2 TexCoord;

    uniform sampler2D ourTexture;

    void main() {
        FragColor = texture(ourTexture, TexCoord);
    }
)";

     // Compile vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
    }

    // Compile fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << endl;
    }

    // Link program
    unsigned int program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if(!success){
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return program;
}

unsigned int loadTexture(const char* path){
    unsigned int textureID;
    int width, height, channels;
    
    // Load the image
    unsigned char* data = stbi_load(path, &width, &height, &channels, 0);
    
    if(!data){
        cout << "Failed to load texture: " << path;
        return 0;
    }
    
    // Determine the format
    GLenum format;
    if(channels == 1){
      format = GL_RED;
    }
    
    else if(channels == 3){
      format = GL_RGB;
    }
    
    else if(channels == 4){
      	format = GL_RGBA;
      }
      
    // Generate and bind the texture
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Upload to GPU
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    // Free CPU memory
    stbi_image_free(data);
    
    return textureID;
}

void initBottle(){
// --- VERTEX DATA ---
  float x = -1.6f;
  float y = -0.8f;
  float w = 3.6f;
  float h = 1.6f;
  float z = 0.0f;
  float u = 0.0f;
  float du = 1.0f;
  float v = 0.0f;
  float dv = 1.0f;
  
  float bottle_vertices[] = {// x,y,z,u,v
    // positions          // texture coordinates
    x, y, z, u, v,   // bottom-left
    x + w, y, z,  u + du, v,   // bottom-right
    x + w,  y + h, z, u + du, v + dv,   // top-right
    x,  y + h, z, u, v + dv    // top-left
};

unsigned int bottle_indices[] = {
    0, 1, 2,
    0, 2, 3
};

// --- IN YOUR main() ---
 
// Create VAO and VBO and EBO
glGenVertexArrays(1, &VAO_bottle);
glGenBuffers(1, &VBO_bottle);
glGenBuffers(1, &EBO_bottle);

glBindVertexArray(VAO_bottle);

glBindBuffer(GL_ARRAY_BUFFER, VBO_bottle);
glBufferData(GL_ARRAY_BUFFER, sizeof(bottle_vertices), bottle_vertices, GL_STATIC_DRAW);

glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_bottle);
glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(bottle_indices), bottle_indices, GL_STATIC_DRAW);

// Position attribute
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
glEnableVertexAttribArray(0);

// Texture coordinate attribute
glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
glEnableVertexAttribArray(1);

// 5. Unbind
glBindBuffer(GL_ARRAY_BUFFER, 0);
glBindVertexArray(0);
}

void initLiquid(){
    // Create a closed shape using a center point + surrounding vertices
    // --- DEFINE YOUR PARAMETERS CLEARLY ---
    // --- Center should match the bottle's center ---
    float cx = 0.2f;
    float cy = -0.2f;
    float cz = 0.0f;
    
    float halfWidth = 0.19f;
    float bottomY = -0.55f;
    float topY = 0.15f;
    
    float dx = 0.03f;
    float dy = 0.0f;
    
    float liquid_vertices[] = {
        // Center point (index 0)
        cx, cy, cz,
        
        // Bottom curve (indices 1-7)
        cx - halfWidth,             bottomY,                 cz,
        cx - halfWidth + 3*dx,      bottomY - 3*dy,          cz,
        cx - halfWidth + 5*dx,      bottomY - 5*dy,          cz,
        cx,                         bottomY - 7*dy,          cz,
        cx + halfWidth - 5*dx,      bottomY - 5*dy,          cz,
        cx + halfWidth - 2*dx,      bottomY - 3*dy,          cz,
        cx + halfWidth,             bottomY,                 cz,
                
        // Top surface (indices 11-19)
        cx + halfWidth,             topY,                    cz,
        cx + halfWidth - 2*dx,      topY,                    cz,
        cx + halfWidth - 4*dx,      topY,                    cz,
        cx + halfWidth - 6*dx,      topY,                    cz,
        cx,                         topY,                    cz,
        cx - halfWidth + 6*dx,      topY,                    cz,
        cx - halfWidth + 4*dx,      topY,                    cz,
        cx - halfWidth + 2*dx,      topY,                    cz,
        cx - halfWidth,             topY,                    cz,
    };

    // Triangle fan from center (index 0) to all surrounding vertices
    unsigned int liquid_indices[] = {
        // Fan from center to bottom
        0, 1, 2,
        0, 2, 3,
        0, 3, 4,
        0, 4, 5,
        0, 5, 6,
        0, 6, 7,
        0, 7, 8,
        0, 8, 9,
        0, 9, 10,
        0, 10, 11,
        0, 11, 12,
        0, 12, 13,
        0, 13, 14,
        0, 14, 15,
        0, 15, 16,
        0, 16, 17,
        0, 17, 18,
        0, 18, 19,
        0, 19, 20,
        0, 20, 21,
        0, 21, 22,
        0, 22, 1,  // Close the loop
    };

    glGenVertexArrays(1, &VAO_liquid);
    glGenBuffers(1, &VBO_liquid);
    glGenBuffers(1, &EBO_liquid);

    glBindVertexArray(VAO_liquid);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_liquid);
    glBufferData(GL_ARRAY_BUFFER, sizeof(liquid_vertices), liquid_vertices, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_liquid);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(liquid_indices), liquid_indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void updateLiquid(){
    float time = glfwGetTime();
    float waveAmplitude = 0.009f;
    float waveSpeed = 2.0f;
    
    float cx = 0.2f;
    float cy = -0.2f;
    float cz = 0.0f;
    
    float halfWidth = 0.19f;
    float bottomY = -0.55f;
    float topY = 0.15f;
    
    float dx = 0.03f;
    float dy = 0.0f;
    
    float liquid_vertices[] = {
        cx, cy, cz,
        
        // Bottom curve (stays flat)
        cx - halfWidth,             bottomY,                 cz,
        cx - halfWidth + 3*dx,      bottomY - 3*dy,          cz,
        cx - halfWidth + 5*dx,      bottomY - 5*dy,          cz,
        cx,                         bottomY - 7*dy,          cz,
        cx + halfWidth - 5*dx,      bottomY - 5*dy,          cz,
        cx + halfWidth - 2*dx,      bottomY - 3*dy,          cz,
        cx + halfWidth,             bottomY,                 cz,
        
        // Top surface (WAVY)
        cx + halfWidth,             topY + waveAmplitude * sin(time * waveSpeed + 0.0f), cz,
        cx + halfWidth - 2*dx,      topY + waveAmplitude * sin(time * waveSpeed + 0.5f), cz,
        cx + halfWidth - 4*dx,      topY + waveAmplitude * sin(time * waveSpeed + 1.0f), cz,
        cx + halfWidth - 6*dx,      topY + waveAmplitude * sin(time * waveSpeed + 1.5f), cz,
        cx,                         topY + waveAmplitude * sin(time * waveSpeed + 2.0f), cz,
        cx - halfWidth + 6*dx,      topY + waveAmplitude * sin(time * waveSpeed + 2.5f), cz,
        cx - halfWidth + 4*dx,      topY + waveAmplitude * sin(time * waveSpeed + 3.0f), cz,
        cx - halfWidth + 2*dx,      topY + waveAmplitude * sin(time * waveSpeed + 3.5f), cz,
        cx - halfWidth,             topY + waveAmplitude * sin(time * waveSpeed + 4.0f), cz,
    };
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO_liquid);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(liquid_vertices), liquid_vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void initBubbles(int num_bubbles){
  bubbles.clear();
  vector<float> bubbleData;
  float cx = 0.2f;
  float halfWidth = 0.19f;
  
  for(int i = 0; i < num_bubbles; i++){
    Bubble b;
    b.x = random_number(cx - halfWidth, cx + halfWidth);
    b.y = random_number(-0.50f, 0.10f);
    b.speed = random_number(0.002f, 0.008f);
    b.size = random_number(0.1f, 0.2f);
    b.phase = random_number(0.0f, 6.28f);
    bubbles.push_back(b);

    bubbleData.push_back(b.x);
    bubbleData.push_back(b.y);
    bubbleData.push_back(b.size);
    }
    
    // Create VBO with positions and sizes
    glGenBuffers(1, &VBO_bubbles);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_bubbles);
    glBufferData(GL_ARRAY_BUFFER, bubbleData.size() * sizeof(float), bubbleData.data(), GL_DYNAMIC_DRAW);
    
    // Setup VAO for bubbles
    glGenVertexArrays(1, &VAO_bubbles);
    glBindVertexArray(VAO_bubbles);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void updateBubbles(){
  vector<float> bubbleData;
  float cx = 0.2f;
  float halfWidth = 0.19f;
  
  for(auto &b : bubbles){
    b.y += b.speed;
    b.x += 0.001*sin(b.phase + 10.0f*b.y);

    if(b.y >= 0.12f){
      b.y = -0.50f;
      b.x = random_number(cx - halfWidth, cx + halfWidth);
      b.speed = random_number(0.002f, 0.008f);
      b.size = random_number(0.1f, 0.2f);
      b.phase = random_number(0.0f, 6.28f);
    }

    if(b.x <= cx - halfWidth){
      b.x = cx - halfWidth;
    }

    if(b.x >= cx + halfWidth){
      b.x = cx + halfWidth;
    }
    
   // Store position and size
        bubbleData.push_back(b.x);
        bubbleData.push_back(b.y);
        bubbleData.push_back(b.size);
    }
    
    // Update the VBO with new data
    glBindBuffer(GL_ARRAY_BUFFER, VBO_bubbles);
    glBufferSubData(GL_ARRAY_BUFFER, 0, bubbleData.size() * sizeof(float), bubbleData.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void drawBubbles(){
  if(bubbles.empty()){
    return;
  }
  
    glUseProgram(pointShaderProgram);

    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    
    glBindVertexArray(VAO_bubbles);
    glDrawArrays(GL_POINTS, 0, bubbles.size());
    glBindVertexArray(0);

    glDisable(GL_PROGRAM_POINT_SIZE);
}

int main(){

  glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
  glewExperimental = true;

  if(!glfwInit()){
    fprintf(stderr, "Failed to initialize GLFW\n");
    return -1;
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
  glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow* window;
  window = glfwCreateWindow(width,height,"Beer Bottle Simulation!",NULL,NULL);
  if(window == NULL){
    fprintf(stderr,"Failed to open GLFW window\n");
    glfwTerminate();
    return -1;
  }
  
  glfwMakeContextCurrent(window);
  glewExperimental = true;

  if(glewInit() != GLEW_OK){
    fprintf(stderr,"Failed to initalize GLEW\n");
    return -1;
  }

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  shaderProgram = shaders();
  colourShaderProgram = createColourShader();
  pointShaderProgram = createPointShader();
  
  initBottle();
  initLiquid();
  initBubbles(50);
  stbi_set_flip_vertically_on_load(true);
  unsigned int bottleTexture = loadTexture("beer_bottle.png");
  
    while(!glfwWindowShouldClose(window)){
      if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
        glfwSetWindowShouldClose(window, true);
      }

      glClearColor(0.0f,0.0f,0.2f,1.0f);
      glClear(GL_COLOR_BUFFER_BIT);

      updateBubbles();
      updateLiquid();
      
      glUseProgram(shaderProgram);
      
      // Draw bottle
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, bottleTexture);
      glBindVertexArray(VAO_bottle);
      glDrawElements(GL_TRIANGLE_STRIP, 14, GL_UNSIGNED_INT, 0);

      // Draw liquid with colour shader
      glUseProgram(colourShaderProgram);
      // Set the liquid colour (amber/gold for beer)
      glUniform4f(glGetUniformLocation(colourShaderProgram, "liquidColor"),0.9f, 0.6f, 0.1f, 0.1f); // R,G,B,A - amber with some transparency
      glBindVertexArray(VAO_liquid);
      glDrawElements(GL_TRIANGLES, 66, GL_UNSIGNED_INT, 0); // 22 triangles 3
      drawBubbles();
      
      glfwSwapBuffers(window);
      glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
