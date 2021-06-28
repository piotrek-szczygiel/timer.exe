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
        printf("%huh ", t->wHour);
    }

    if (t->wMinute > 0) {
        printf("%hum ", t->wMinute);
    }

    if (t->wSecond > 0) {
        printf("%hus ", t->wSecond);
    }

    printf("%hums", t->wMilliseconds);
}

int SystemTimeToMilliseconds(const SYSTEMTIME* t) {
    int ms = t->wMilliseconds;
    ms += t->wSecond * 1000;
    ms += t->wMinute * 1000 * 60;
    ms += t->wHour * 1000 * 60 * 60;
    return ms;
}

void Usage() { fprintf(stderr, "usage: timer.exe [--only-real-ms|-o] COMMAND\n"); }

int main(int argc, char** argv) {
    if (argc < 2) {
        Usage();
        return -1;
    }

    int current_argv = 1;
    int only_real_ms = 0;

    if ((strcmp(argv[1], "--only-real-ms") == 0) || (strcmp(argv[1], "-o") == 0)) {
        only_real_ms = 1;

        if (argc < 3) {
            Usage();
            return -1;
        }

        ++current_argv;
    }

    char cmd[MAX_PATH] = {0};
    strcpy(cmd, argv[current_argv]);
    ++current_argv;

    for (int i = current_argv; i < argc; ++i) {
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

    if (only_real_ms == 1) {
        printf("%d\n", SystemTimeToMilliseconds(&elapsed_time));
        return exit_code;
    }

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);

    printf("\033[90m--- timer.exe results ---\033[0m\n");

    printf("real\t\033[92m");
    PrintfSystemTime(&elapsed_time);

    printf("\033[0m\nuser\t");
    PrintfSystemTime(&user_time);

    printf("\nkernel\t");
    PrintfSystemTime(&kernel_time);

    printf("\n");

    return exit_code;
}
