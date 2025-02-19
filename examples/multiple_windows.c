#include "stdio.h"
#include "create_opengl_window.h"

uint64_t query_performance_frequency() {
    LARGE_INTEGER frequency;
    assert(QueryPerformanceFrequency(&frequency));
    return frequency.QuadPart;
}
uint64_t query_performance_counter() {
    LARGE_INTEGER ticks;
    assert(QueryPerformanceCounter(&ticks));
    return ticks.QuadPart;
}

int main() {
    Window* main_window = create_window("Main window");
    Window* sub_windows[5] = {};

    gl_swap_interval(1); // enable vsync;

    uint8_t r = 0;
    uint8_t b = 0;
    uint64_t perf_freq = query_performance_frequency();
    uint64_t start_time = query_performance_counter();
    while (!window_should_close(main_window)) {
        for (Event event; window_check_event(main_window, &event);) {
            if (event.type == Event_key_down && event.key == 'A') {
                for (size_t i = 0; i < ARRAY_LEN(sub_windows); ++i) {
                    if (!sub_windows[i]) {
                        sub_windows[i] = create_window("Sub window");
                        break;
                    }
                }
            }
        }

        uint64_t current_time = query_performance_counter();

        window_make_current(main_window);

        glClearColor(r++ / 255.0, 0, b-- / 255.0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        window_swap_buffers(main_window);

        float seconds_ellapesd = (float)(current_time - start_time)/perf_freq;
        static float last_time = 0;
        static uint64_t frame_count = 0;
        ++frame_count;
        if (seconds_ellapesd > last_time + 1) {
            int fps = frame_count / (seconds_ellapesd - last_time);
            char buff[128];
            snprintf(buff, sizeof(buff), "FPS: %d\n", fps);
            SetWindowTextA(main_window->hwnd, buff);
            last_time = seconds_ellapesd;
            frame_count = 0;
        }

        for (size_t i = 0; i < ARRAY_LEN(sub_windows); ++i) {
            Window* win = sub_windows[i];
            if (!win) continue;
            for (Event ev; window_check_event(win, &ev);) {
                if (ev.type == Event_close) {
                    destroy_window(win);
                    sub_windows[i] = 0;
                }
            }

            window_make_current(win);
            glClearColor(0.2, 0.7, 0.2, 1);
            glClear(GL_COLOR_BUFFER_BIT);
            window_swap_buffers(win);
        }
    }

    return 0;
}
