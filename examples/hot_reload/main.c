#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include "stdbool.h"
#include "stdio.h"
#include "assert.h"

typedef struct Window Window;
typedef Window* fn_on_load(Window* win);
typedef bool fn_draw_frame(Window* win);

typedef struct {
    HINSTANCE handle;
    FILETIME last_write_time;
} DLL;

bool check_and_reload_dll(const char* dll_path, DLL* dll) {
    WIN32_FIND_DATAA find_data;
    HANDLE find = FindFirstFileA(dll_path, &find_data);
    if (find == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error: failed to find dll: %s\n", dll_path);
        // assert(false);
        return false;
    }

    if (CompareFileTime(&dll->last_write_time, &find_data.ftLastWriteTime) != 0) {
        // NOTE: gcc writes the file twice when creating the dll, wait for it to
        // finish writing.
        Sleep(300);

        if (dll->handle) FreeLibrary(dll->handle);
        const char* tmp_file_name = "__tmp.dll";
        assert(CopyFileA(dll_path, tmp_file_name, FALSE));
        dll->handle = LoadLibraryA(tmp_file_name);
        dll->last_write_time = find_data.ftLastWriteTime;
        assert(dll->handle);
        return true;
    }

    return false;
}

char* take_first_n(char* s, size_t count) {
    size_t len = strlen(s);
    assert(count <= len);
    s[count] = 0;
    return s;
}

int find_last_index(const char* s, char c) {
    size_t len = strlen(s);
    for (const char* p = s + len - 1; p != s; --p) {
        if (*p == c) return p - s;
    }
    return -1;
}

typedef struct {
    char* base;
    size_t cap;
    size_t used;
} Arena;

#define arena_on_stack(size) ((Arena){(char[size]){}, size, 0})

char* copy_string(Arena* a, const char* s) {
    size_t len = strlen(s);
    assert(a->used + len < a->cap);
    char* res = memcpy(a->base + a->used, s, len + 1);
    a->used += len + 1;
    return res;
}

int main(int argc, char* argv[]) {
    Arena a = arena_on_stack(1024);

    char* prog_path = copy_string(&a, argv[0]);
    int idx = find_last_index(prog_path, '\\');
    if (idx < 0) {
        // command line doesn't contain path seperator, which means the program
        // is called in the same folder that contains the executable
    } else {
        char* dirname = take_first_n(prog_path, idx);
        assert(SetCurrentDirectoryA(dirname));
    }

    Window* win = 0;
    fn_draw_frame* draw_frame = 0;
    DLL dll = {};
    while (1) {
        if (check_and_reload_dll("hot_reload.dll", &dll)) {
            fn_on_load* on_load = (void*)GetProcAddress(dll.handle, "on_load");
            assert(on_load);
            draw_frame = (void*)GetProcAddress(dll.handle, "draw_frame");
            assert(draw_frame);
            win = on_load(win);
        }

        if (draw_frame) {
            if (!draw_frame(win)) break;
        }
    }

    return 0;
}
