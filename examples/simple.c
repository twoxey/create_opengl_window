#include "create_opengl_window.h"

int main(void) {
    Window* win = create_window();
    gl_swap_interval(1); // enable vsync;

    while (!window_should_close(win)) {
        for (Event ev; window_check_event(win, &ev);) {
            // handle window events here
        }

        // GL calls here
        glClearColor(.2, .2, .2, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        window_swap_buffers(win);
    }

    destroy_window(win);

    return 0;
}
