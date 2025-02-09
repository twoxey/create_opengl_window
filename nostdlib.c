// gcc -nostdlib nostdlib.c -lkernel32

#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include "stdbool.h"

#define STRINGIFY_(a) # a
#define STRINGIFY(a) STRINGIFY_(a)

#define ARRAY_LEN(a) (sizeof(a)/sizeof*(a))

typedef struct {
    char* ptr;
    size_t len;
} buffer, string;
#define S(s_literal) ({char s[] = s_literal; (string){s, sizeof(s)-1};})

INT64 write_string(HANDLE handle, string s) {
    DWORD bytes_written;
    if(!WriteFile(handle, s.ptr, s.len, &bytes_written, 0)) {
        return -GetLastError();
    }
    return bytes_written;
}

static HANDLE stdout = INVALID_HANDLE_VALUE;
static HANDLE stderr = INVALID_HANDLE_VALUE; 

#define assert(expr) (void)((expr) || ((stderr != INVALID_HANDLE_VALUE && write_string(stderr, S(__FILE__ ":" STRINGIFY(__LINE__) ": Assertion failed: '" # expr "'\n"))), ExitProcess(1), 0))

void* memcpy(void* dst, const void* src, size_t count) {
    void* res = dst;
    while (count--) *(char*)dst++ = *(char*)src++;
    return res;
}

size_t wstr_byte_len(wchar_t* wstr) {
    size_t count = 0;
    while (*wstr++) ++count;
    return count*sizeof(wchar_t);
}

// This function overwrites the wchar string to its corresponding ascii version,
// the returned string points to the same memory as the wstr that's passed in
string zwstr_to_string(wchar_t* wstr) {
    string s = {(char*)wstr, 0};
    while (*wstr) {
        assert(*wstr <= 0xFF);
        s.ptr[s.len++] = *wstr++;
    }
    return s;
}

string format_string_v(buffer b, const char* fmt, va_list args) {
    string s = {b.ptr, 0};
    for (char c; (c = *fmt++);) {
        if (c == '%') {
            switch (*fmt++) {
                case 's': { // null terminated string
                    char* z_str = va_arg(args, char*);
                    while ((c = *z_str++)) {
                        assert(s.len < b.len - 1);
                        s.ptr[s.len++] = c;
                    }
                } break;

                case 'w': { // null terminated wchar string
                    wchar_t* wstr = va_arg(args, wchar_t*);
                    size_t byte_len = wstr_byte_len(wstr);
                    assert(s.len + byte_len <= b.len - 1);
                    memcpy(s.ptr + s.len, wstr, byte_len);
                    s.len += byte_len;
                } break;

                default: assert(!"Unsupported format specifier");
            }
        } else {
            assert(s.len < b.len - 1);
            s.ptr[s.len++] = c;
        }
    }
    s.ptr[s.len] = 0;
    return s;
}
string format_string(buffer b, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    string s = format_string_v(b, fmt, args);
    va_end(args);
    return s;
}

intptr_t write_formated_string(HANDLE f, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char b[1024]; // fixed buffer for now
    string s = format_string_v((buffer){b, sizeof(b)}, fmt, args);
    va_end(args);
    return write_string(f, s);
}

bool run_command(char* command_line) {
    STARTUPINFOA startup_info = {};
    startup_info.cb = sizeof(startup_info);
    PROCESS_INFORMATION process_info;
    if (!CreateProcessA(0, command_line, 0, 0, TRUE, 0, 0, 0, &startup_info, &process_info))
        return false;

    WaitForSingleObject(process_info.hProcess, INFINITE);
    DWORD exit_code;
    assert(GetExitCodeProcess(process_info.hProcess, &exit_code));
    CloseHandle(process_info.hThread);
    CloseHandle(process_info.hProcess);
    return !exit_code;
}

bool get_file_last_write_time(const char* file_path, FILETIME* last_write_time) {
    WIN32_FIND_DATAA find_data;
    HANDLE find = FindFirstFileA(file_path, &find_data);
    if (find == INVALID_HANDLE_VALUE) return false;
    *last_write_time = find_data.ftLastWriteTime;
    return true;
}

#include "shellapi.h"

int mainCRTStartup() {
    stdout = GetStdHandle(STD_OUTPUT_HANDLE);
    stderr = GetStdHandle(STD_ERROR_HANDLE);

    check_and_rebuild();

    write_string(stdout, S("Hello, world\n"));

    return 0;


    write_string(stdout, S("Building\n"));
    if (!run_command("gcc -Wall -Wextra -nostdlib -o main nostdlib.c -lkernel32"))
        write_string(stdout, S("Command failed\n"));

    return 0;
}
