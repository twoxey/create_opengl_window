#define MULTI_(...) #__VA_ARGS__
#define MULTI(...) MULTI_(__VA_ARGS__)

#define QUAD_rect    0
#define QUAD_ellipse 1
#define QUAD_image   2
#define QUAD_char    3

#define UNIFORMS \
Uniform(vec2, screen_size) \
Uniform(vec2, size) \
Uniform(vec2, offset) \
Uniform(vec4, color) \
Uniform(int, type) \
// UNIFORMS

static const char quad_header[] =
    "#version 330\n"
#define Uniform(type, name) uniform type name;
    MULTI(UNIFORMS)
#undef Uniform
    ;

static const char quad_vertex_source[] = MULTI(
    out vec2 frag_uv;

    void main() {
        vec2 uv = vec2(gl_VertexID & 1, (gl_VertexID >> 1) & 1);
        vec2 pos = uv;
        pos *= size;
        pos += offset;
        pos *= vec2(2) / screen_size;
        gl_Position = vec4(pos, 0, 1);
        frag_uv = uv;
    }
);

static const char quad_fragment_source[] = MULTI(
    in vec2 frag_uv;
    out vec4 out_color;

    void main() {
        out_color = color;
        switch (type) {
            case QUAD_ellipse:
            if (length(frag_uv - vec2(.5)) > .5) {
                out_color.a = 0;
            }
            break;
        }
    }
);

void init_quad_shader() {
    if (vertex_source.len && fragment_source.len) {
        GLuint quad_shader_program = create_shader_program(quad_header, quad_vertex_source, quad_fragment_source);
#define Uniform(type, name) GLuint u_##type_##name = glGetUniformLocation(quad_shader_program, #name);
        UNIFORMS
#undef Uniform
    } else {
        fprintf(stderr, "Error: Failed to load shader files\n");
    }
}

void draw_ellipse(float x, float y, float w, float h, color_v4 c) {
    glUniform1i(u_int_type, QUAD_ellipse);
    glUniform2f(u_vec2_offset, x, y);
    glUniform2f(u_vec2_size, w, h);
    glUniform4f(u_vec4_color, c.r, c.g, c.b, c.a);
    glDrawArrays(GL_TRIANGLE_STRIPS, 0, 4);
}
