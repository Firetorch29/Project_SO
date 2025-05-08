#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define BUFFER_SIZE 1024

int monitor_running = 0;
HANDLE hMonitorPipeRead = NULL;
HANDLE hMonitorPipeWrite = NULL;
HANDLE hMonitorProcess = NULL;

void start_monitor() {
    if (monitor_running) {
        printf("Monitor already running.\n");
        return;
    }

    SECURITY_ATTRIBUTES sa = {0};
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;

    if (!CreatePipe(&hMonitorPipeRead, &hMonitorPipeWrite, &sa, 0)) {
        printf("Error creating pipe (%d)\n", GetLastError());
        return;
    }

    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(STARTUPINFO);
    si.hStdOutput = hMonitorPipeWrite;
    si.hStdError = hMonitorPipeWrite;
    si.dwFlags |= STARTF_USESTDHANDLES;

    if (!CreateProcess(NULL, "treasure_manager monitor", NULL, NULL, TRUE,
                      CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        printf("Failed to start monitor (%d)\n", GetLastError());
        CloseHandle(hMonitorPipeRead);
        CloseHandle(hMonitorPipeWrite);
        hMonitorPipeRead = hMonitorPipeWrite = NULL;
        return;
    }

    hMonitorProcess = pi.hProcess;
    CloseHandle(pi.hThread);
    monitor_running = 1;
    printf("Monitor started successfully.\n");
}

void stop_monitor() {
    if (!monitor_running) {
        printf("Monitor not running.\n");
        return;
    }

    // Send stop command through pipe
    DWORD bytesWritten;
    const char* cmd = "STOP\n";
    WriteFile(hMonitorPipeWrite, cmd, strlen(cmd), &bytesWritten, NULL);

    WaitForSingleObject(hMonitorProcess, 5000);

    CloseHandle(hMonitorProcess);
    CloseHandle(hMonitorPipeRead);
    CloseHandle(hMonitorPipeWrite);
    hMonitorProcess = hMonitorPipeRead = hMonitorPipeWrite = NULL;
    monitor_running = 0;
    printf("Monitor stopped.\n");
}

void calculate_score() {
    if (!monitor_running) {
        printf("Monitor not running.\n");
        return;
    }

    char hunt_id[100];
    printf("Enter hunt ID: ");
    scanf("%s", hunt_id);

    SECURITY_ATTRIBUTES sa = {0};
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;

    HANDLE hPipeRead, hPipeWrite;
    if (!CreatePipe(&hPipeRead, &hPipeWrite, &sa, 0)) {
        printf("Pipe creation failed (%d)\n", GetLastError());
        return;
    }

    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(STARTUPINFO);
    si.hStdOutput = hPipeWrite;
    si.hStdError = hPipeWrite;
    si.dwFlags |= STARTF_USESTDHANDLES;

    char cmd[256];
    snprintf(cmd, sizeof(cmd), "treasure_manager calculate %s", hunt_id);

    if (!CreateProcess(NULL, cmd, NULL, NULL, TRUE,
                      CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        printf("Failed to start calculation (%d)\n", GetLastError());
        CloseHandle(hPipeRead);
        CloseHandle(hPipeWrite);
        return;
    }

    CloseHandle(hPipeWrite);
    CloseHandle(pi.hThread);

    char buffer[BUFFER_SIZE];
    DWORD bytesRead;
    printf("Scores for hunt %s:\n", hunt_id);

    while (ReadFile(hPipeRead, buffer, BUFFER_SIZE-1, &bytesRead, NULL) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        printf("%s", buffer);
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(hPipeRead);
    CloseHandle(pi.hProcess);
}

int main() {
    char command[100];

    printf("Treasure Hunt Hub System\n");
    printf("Available commands:\n");
    printf("  start_monitor - Start monitoring hunts\n");
    printf("  stop_monitor - Stop monitoring\n");
    printf("  calculate_score - Calculate user scores\n");
    printf("  exit - Exit the program\n");

    while (1) {
        printf("hub> ");
        scanf("%s", command);

        if (strcmp(command, "start_monitor") == 0) {
            start_monitor();
        } else if (strcmp(command, "stop_monitor") == 0) {
            stop_monitor();
        } else if (strcmp(command, "calculate_score") == 0) {
            calculate_score();
        } else if (strcmp(command, "exit") == 0) {
            if (monitor_running) {
                stop_monitor();
            }
            break;
        } else {
            printf("Unknown command. Try again.\n");
        }
    }

    return 0;
}