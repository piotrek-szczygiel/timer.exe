#define WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <tchar.h>
#include <windows.h>

SYSTEMTIME SystemTimeDiff(const SYSTEMTIME* a, const SYSTEMTIME* b) {
    SYSTEMTIME t_res;
    FILETIME v_ftime;
    ULARGE_INTEGER v_ui;
    __int64 v_right, v_left, v_res;
    SystemTimeToFileTime(a, &v_ftime);
    v_ui.LowPart = v_ftime.dwLowDateTime;
    v_ui.HighPart = v_ftime.dwHighDateTime;
    v_right = v_ui.QuadPart;

    SystemTimeToFileTime(b, &v_ftime);
    v_ui.LowPart = v_ftime.dwLowDateTime;
    v_ui.HighPart = v_ftime.dwHighDateTime;
    v_left = v_ui.QuadPart;

    v_res = v_right - v_left;

    v_ui.QuadPart = v_res;
    v_ftime.dwLowDateTime = v_ui.LowPart;
    v_ftime.dwHighDateTime = v_ui.HighPart;
    FileTimeToSystemTime(&v_ftime, &t_res);

    return t_res;
}

void PrintfSystemTime(const SYSTEMTIME* t) {
    if (t->wHour > 0) {
        fprintf(stderr, "%huh ", t->wHour);
    }

    if (t->wMinute > 0) {
        fprintf(stderr, "%hum ", t->wMinute);
    }

    if (t->wSecond > 0) {
        fprintf(stderr, "%hus ", t->wSecond);
    }

    fprintf(stderr, "%hums", t->wMilliseconds);
}

int main(int argc, char** argv) {
    char cmd[MAX_PATH] = {0};
    strcpy(cmd, argv[1]);

    for (int i = 2; i < argc; ++i) {
        strcat(cmd, " ");
        strcat(cmd, argv[i]);
    }

    STARTUPINFO startup_info = {0};
    PROCESS_INFORMATION process_info = {0};

    if (!CreateProcess(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &startup_info, &process_info)) {
        fprintf(stderr, "unable to create process: '%s'!\n", cmd);
        return -1;
    }

    WaitForSingleObject(process_info.hProcess, INFINITE);

    DWORD exit_code = 0;
    if (!GetExitCodeProcess(process_info.hProcess, &exit_code)) {
        fprintf(stderr, "unable to get exit code from process!\n");
        return -1;
    }

    FILETIME ft_creation = {0};
    FILETIME ft_exit = {0};
    FILETIME ft_kernel = {0};
    FILETIME ft_user = {0};

    if (!GetProcessTimes(process_info.hProcess, &ft_creation, &ft_exit, &ft_kernel, &ft_user)) {
        fprintf(stderr, "unable to query process times!\n");
        return -1;
    }

    CloseHandle(process_info.hThread);
    CloseHandle(process_info.hProcess);

    SYSTEMTIME creation_time = {0};
    SYSTEMTIME exit_time = {0};
    SYSTEMTIME kernel_time = {0};
    SYSTEMTIME user_time = {0};

    if (!FileTimeToSystemTime(&ft_creation, &creation_time) || !FileTimeToSystemTime(&ft_exit, &exit_time) ||
        !FileTimeToSystemTime(&ft_kernel, &kernel_time) || !FileTimeToSystemTime(&ft_user, &user_time)) {
        fprintf(stderr, "unable to convert FILETIME to SYSTEMTIME\n");
        return -1;
    }

    SYSTEMTIME elapsed_time = SystemTimeDiff(&exit_time, &creation_time);

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);

    fprintf(stderr, "\033[0m\n");

    fprintf(stderr, "real\t\033[92m");
    PrintfSystemTime(&elapsed_time);

    fprintf(stderr, "\033[0m\nuser\t");
    PrintfSystemTime(&user_time);

    fprintf(stderr, "\nkernel\t");
    PrintfSystemTime(&kernel_time);

    fprintf(stderr, "\n");

    return exit_code;
}
