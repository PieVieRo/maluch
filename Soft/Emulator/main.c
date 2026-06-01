#include <stdio.h>
#include <stdlib.h>

#include "maluch.h"

#define GLAD_GL_IMPLEMENTATION
#include "glad.h"

#define RGFW_IMPLEMENTATION
#define RGFW_OPENGL
#include "RGFW.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define FONT_WIDTH 8
#define FONT_HEIGHT 16

#define CELLS (ROWS * COLS)

#define SCREEN_WIDTH (FONT_WIDTH * COLS)
#define SCREEN_HEIGHT (FONT_HEIGHT * ROWS)

#define CHAR_NUM 256

#define RGB_TO_332(r,g,b) ( (((uint8_t)(r) >> 5) << 5) | (((uint8_t)(g) >> 5) << 2) | ((uint8_t)(b) >> 6))

// font: http://xyzzy.freeshell.org/cp437/

GLuint compileShader(const char* path, GLenum type) {
    GLuint shader = glCreateShader(type);

    { 
        FILE *file = fopen(path, "rb");
        fseek(file, 0, SEEK_END);
        const size_t file_size = ftell(file);
        rewind(file);
        char* shader_source = malloc(file_size+1);
        fread(shader_source, 1, file_size, file);
        shader_source[file_size] = '\0';
        fclose(file);
        glShaderSource(shader, 1, (const GLchar * const *)&shader_source, NULL);
        glCompileShader(shader);
        free(shader_source);
    }
    {
        int success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if(!success) {
            char info_log[1024];
            glGetShaderInfoLog(shader, sizeof(info_log), NULL, info_log);
            printf("%s SHADER (%s) COMPILATION ERROR - %s\n", type == GL_VERTEX_SHADER? "VERTEX" : "FRAGMENT", path, info_log);
            exit(EXIT_FAILURE);
        }
    }
    return shader;
}

Maluch maluch;
int main() {
    RGFW_glHints* hints = RGFW_getGlobalHints_OpenGL();
    hints->major = 4;
    hints->minor = 6;
    RGFW_setGlobalHints_OpenGL(hints);

    RGFW_window *win = RGFW_createWindow("mALUch emulator", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 
            RGFW_windowCenter | RGFW_windowOpenGL | RGFW_windowNoResize);
    int version = gladLoadGL(RGFW_getProcAddress_OpenGL);
    if(version == 0) {
        printf("Could not initialize OpenGL.\n");
        return EXIT_FAILURE;
    }

    RGFW_window_swapInterval_OpenGL(win, 1);
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    GLuint vao;
    glCreateVertexArrays(1, &vao);

    // LOAD THE FONT
    GLuint font;
    {
        int width, height;
        const char* font_texture_name = "font.png";
        uint8_t *font_tex = stbi_load(font_texture_name, &width, &height, NULL, 1);
        if(font_tex == NULL || width == 0 || height == 0) {
            printf("Could not load font %s.\n", font_texture_name);
            return EXIT_FAILURE;
        }

        glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &font);
        glTextureStorage3D(font, 1, GL_R8, width, height/CHAR_NUM, CHAR_NUM);
        glTextureSubImage3D(font, 0, 0, 0, 0, width, height/CHAR_NUM, CHAR_NUM, GL_RED,
                GL_UNSIGNED_BYTE, font_tex);

        glTextureParameteri(font, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(font, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTextureParameteri(font, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(font, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        stbi_image_free(font_tex);
    }
    glBindTextureUnit(0, font);

    // CREATE THE SSBO
    GLuint ssbo;
    const uint32_t ssbo_size = CELLS * 2 * sizeof(uint16_t);
    glCreateBuffers(1, &ssbo);
    glNamedBufferStorage(ssbo, ssbo_size, NULL, GL_DYNAMIC_STORAGE_BIT);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
    
    // SET UP THE SHADER
    const GLuint sp = glCreateProgram();
    {
        GLuint vs = compileShader("shader.vert", GL_VERTEX_SHADER);
        GLuint fs = compileShader("shader.frag", GL_FRAGMENT_SHADER);
        glAttachShader(sp, vs);
        glAttachShader(sp, fs);
        glLinkProgram(sp);
        glDeleteShader(vs);
        glDeleteShader(fs);
    }
    glUseProgram(sp);
    glUniform2f(glGetUniformLocation(sp, "window_size"), SCREEN_WIDTH, SCREEN_HEIGHT);

    for(int i = 0; i < ROWS * COLS; i++) {
        maluch.colors[i].fg = 0xFF;
    }
    maluch.vram[0] = 'a';
    maluch.vram[1] = 'b';
    maluch.vram[2] = '\2';
    maluch.colors[0].bg = RGB_TO_332(0x00, 0x6B, 0x38);
    maluch.colors[2].bg = RGB_TO_332(0xB2, 0x08, 0x23);
    while(!RGFW_window_shouldClose(win)) {
        RGFW_pollEvents();
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f); 
        glClear(GL_COLOR_BUFFER_BIT);

        glNamedBufferSubData(ssbo, 0, CELLS * sizeof(uint16_t), maluch.vram);
        glNamedBufferSubData(ssbo, CELLS * sizeof(uint16_t), CELLS * sizeof(uint16_t), maluch.colors);
        glBindVertexArray(vao);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, ROWS * COLS);

        RGFW_window_swapBuffers_OpenGL(win);
    }
    return EXIT_SUCCESS;
}
