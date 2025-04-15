#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

int monitor_running = 0;

void start_monitor() {
    if (monitor_running) {
        printf("Monitor already running.\n");
        return;
    }
    monitor_running = 1;
    printf("Monitor started. (Simulation)\n");
}

void stop_monitor() {
    if (!monitor_running) {
        printf("Monitor is not running.\n");
        return;
    }
    printf("Stopping monitor...\n");
    Sleep(2000); // simulate delay
    monitor_running = 0;
    printf("Monitor stopped.\n");
}

void list_hunts() {
    if (!monitor_running) {
        printf("Monitor not running. Cannot list hunts.\n");
        return;
    }
    system("dir"); // simulate list with directory listing
}

void list_treasures() {
    if (!monitor_running) {
        printf("Monitor not running. Cannot list treasures.\n");
        return;
    }
    char hunt[100];
    printf("Enter hunt ID: ");
    scanf("%s", hunt);

    char command[256];
    snprintf(command, sizeof(command), "treasure_manager list %s", hunt);
    system(command);
}

void view_treasure() {
    if (!monitor_running) {
        printf("Monitor not running. Cannot view treasure.\n");
        return;
    }
    char hunt[100], tid[100];
    printf("Enter hunt ID: ");
    scanf("%s", hunt);
    printf("Enter treasure ID: ");
    scanf("%s", tid);

    char command[256];
    snprintf(command, sizeof(command), "treasure_manager view %s %s", hunt, tid);
    system(command);
}

int main() {
    char command[100];

    while (1) {
        printf("treasure_hub> ");
        scanf("%s", command);

        if (strcmp(command, "start_monitor") == 0) {
            start_monitor();
        } else if (strcmp(command, "stop_monitor") == 0) {
            stop_monitor();
        } else if (strcmp(command, "list_hunts") == 0) {
            list_hunts();
        } else if (strcmp(command, "list_treasures") == 0) {
            list_treasures();
        } else if (strcmp(command, "view_treasure") == 0) {
            view_treasure();
        } else if (strcmp(command, "exit") == 0) {
            if (monitor_running) {
                printf("Cannot exit: monitor is still running.\n");
            } else {
                printf("Goodbye!\n");
                break;
            }
        } else {
            printf("Unknown command.\n");
        }
    }

    return 0;
}
