#include "create_opengl_window.h"

int main(void) {
    Window* win = create_window();
    while (!window_should_close(win)) {
        for (Event ev; window_check_event(win, &ev);) {
        }

        glClearColor(.2, .2, .2, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        window_swap_buffers(win);
    }
    destroy_window(win);

    return 0;
}
