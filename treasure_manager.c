#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#include <direct.h>

#define TREASURE_FILE "treasures.dat"
#define LOG_FILE "logged_hunt"
#define TREASURE_NAME_LEN 64
#define CLUE_LEN 256
#define MAX_USERS 100

typedef struct {
    char treasure_id[TREASURE_NAME_LEN];
    char user_name[TREASURE_NAME_LEN];
    float latitude;
    float longitude;
    char clue[CLUE_LEN];
    int value;
} Treasure;

typedef struct {
    char name[TREASURE_NAME_LEN];
    int total;
} UserScore;

void log_operation(const char *hunt_path, const char *operation) {
    char log_path[MAX_PATH];
    snprintf(log_path, sizeof(log_path), "%s\\%s", hunt_path, LOG_FILE);

    HANDLE hFile = CreateFile(log_path, FILE_APPEND_DATA, FILE_SHARE_READ, NULL,
                             OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;

    SYSTEMTIME st;
    GetLocalTime(&st);
    char log_entry[256];
    snprintf(log_entry, sizeof(log_entry), "[%04d-%02d-%02d %02d:%02d:%02d] %s\r\n",
             st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, operation);

    DWORD bytesWritten;
    WriteFile(hFile, log_entry, strlen(log_entry), &bytesWritten, NULL);
    CloseHandle(hFile);
}

void calculate_scores(const char *hunt_id) {
    char file_path[MAX_PATH];
    snprintf(file_path, sizeof(file_path), "%s\\%s", hunt_id, TREASURE_FILE);

    HANDLE hFile = CreateFile(file_path, GENERIC_READ, FILE_SHARE_READ, NULL,
                            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        printf("Error opening treasure file\n");
        return;
    }

    UserScore scores[MAX_USERS] = {0};
    int user_count = 0;
    Treasure treasure;
    DWORD bytesRead;

    while (ReadFile(hFile, &treasure, sizeof(Treasure), &bytesRead, NULL) && bytesRead > 0) {
        int found = 0;
        for (int i = 0; i < user_count; i++) {
            if (strcmp(scores[i].name, treasure.user_name) == 0) {
                scores[i].total += treasure.value;
                found = 1;
                break;
            }
        }

        if (!found && user_count < MAX_USERS) {
            strncpy(scores[user_count].name, treasure.user_name, TREASURE_NAME_LEN);
            scores[user_count].total = treasure.value;
            user_count++;
        }
    }

    CloseHandle(hFile);

    for (int i = 0; i < user_count; i++) {
        printf("%s: %d\n", scores[i].name, scores[i].total);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <command> [options]\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "calculate") == 0 && argc == 3) {
        calculate_scores(argv[2]);
        return 0;
    }

    // 💡 AJOUTE CE BLOC POUR LA COMMANDE "monitor"
    else if (strcmp(argv[1], "monitor") == 0) {
        printf("Monitoring... Press Ctrl+C to stop.\n");
        while (1) {
            Sleep(1000); // Simule une activité de fond
        }
    }

    // [Tu peux aussi gérer d'autres commandes ici comme add, list, view, etc.]

    fprintf(stderr, "Invalid command\n");
    return 1;
}