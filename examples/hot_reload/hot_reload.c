#include "create_opengl_window.h"
#include "stdio.h"

typedef struct buffer {
    char* ptr;
    size_t len;
} buffer, string;

buffer read_entire_file_and_null_terminate(const char* file_path) {
    buffer result = {};

    HANDLE file = CreateFileA(file_path, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (file == INVALID_HANDLE_VALUE) return result;

    LARGE_INTEGER file_size;
    if (!GetFileSizeEx(file, &file_size)) goto defer;

    char* mem = (char*)malloc(file_size.QuadPart + 1);
    if (!mem) goto defer;

    DWORD bytes_read;
    if (ReadFile(file, mem, file_size.QuadPart, &bytes_read, 0) && bytes_read == file_size.QuadPart) {
        result.len = bytes_read;
        result.ptr = mem;
        result.ptr[result.len] = 0;
    }
    else free(mem);

defer:
    CloseHandle(file);
    return result;
}

GLuint create_shader_program(const char* header, const char* vertex_source_string, const char* fragment_source_string) {
    char buff[512];
    GLint success;

    const GLchar* vertex_shader_source[] = {header, vertex_source_string};
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 2, vertex_shader_source, 0);
    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex_shader, sizeof(buff), NULL, buff);
        fprintf(stderr, "Error: vertex shader:\n%s\n", buff);
        return 0;
    }

    const GLchar* fragment_shader_source[] = {header, fragment_source_string};
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 2, fragment_shader_source, 0);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment_shader, sizeof(buff), NULL, buff);
        fprintf(stderr, "Error: fragment shader:\n%s\n", buff);
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, sizeof(buff), NULL, buff);
        fprintf(stderr, "Error: linking program:\n%s\n", buff);
        glDeleteProgram(program);
        return 0;
    }

    return program;
}

typedef union {
    struct { float x, y, z, w; };
    struct { float r, g, b, a; };
} vec4, color_v4;

color_v4 Color_v4(float r, float g, float b, float a) {
    return (color_v4){{r, g, b, a}};
}

#include "quad_shader.c"

Window* on_load(Window* win) {
    if (!win) {
        win = create_window();
    } else {
        // we already have a window, just load the gl procs to call them in the dll
        load_gl_procs();
    }

    init_quad_shader();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return win; // pass the window handle to main executable to preserve it between reloads
}

bool draw_frame(Window* win) {
    assert(win);
    for (Event ev; window_check_event(win, &ev);) {
        if (ev.type == Event_close || (ev.type == Event_key_down && ev.key == 'Q')) {
            return false;
        }
    }

    glViewport(0, 0, win->width, win->height);
    glClearColor(1, 1, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    if (quad_shader_program) {
        glUseProgram(quad_shader_program);
        glUniform2f(u_vec2_screen_size, win->width, win->height);
        draw_ellipse(-100, 0, 200, 120, Color_v4(.8, .1, .7, 1));
    }

    window_swap_buffers(win);

    return true;
}
