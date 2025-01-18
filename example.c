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
    Window* main_window = create_window();
    Window* sub_windows[5] = {};

    uint8_t r = 0;
    uint8_t b = 0;
    uint64_t perf_freq = query_performance_frequency();
    uint64_t start_time = query_performance_counter();
    for (bool running = true; running;) {
        Event event;
        while (window_check_event(main_window, &event)) {
            switch (event.type) {
            case Event_close:
                running = false;
                break;
            case Event_key_down:
                switch (event.key) {
                case VK_ESCAPE:
                    running = false;
                    break;
                case 'A': {
                    for (size_t i = 0; i < ARRAY_LEN(sub_windows); ++i) {
                        if (!sub_windows[i]) {
                            sub_windows[i] = create_window();
                            break;
                        }
                    }
                } break;
                }
                break;
            default: break;
            }
        }

        uint64_t current_time = query_performance_counter();

        begin_drawing(main_window);

        glClearColor(r++ / 255.0, 0, b-- / 255.0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        // other GL calls here

        end_drawing(main_window);

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
                switch (ev.type) {
                case Event_close:
                    destroy_window(win);
                    sub_windows[i] = 0;
                    break;
                default:
                    break;
                }
            }

            begin_drawing(win);

            glClearColor(0.2, 0.7, 0.2, 1);
            glClear(GL_COLOR_BUFFER_BIT);

            end_drawing(win);
        }
    }

    return 0;
}