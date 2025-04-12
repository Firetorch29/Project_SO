#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <errno.h>

#define TREASURE_FILE "treasures.dat"
#define LOG_FILE "logged_hunt"
#define SYMLINK_PREFIX "logged_hunt-"
#define TREASURE_NAME_LEN 64
#define CLUE_LEN 256

typedef struct {
    char treasure_id[TREASURE_NAME_LEN];
    char user_name[TREASURE_NAME_LEN];
    float latitude;
    float longitude;
    char clue[CLUE_LEN];
    int value;
} Treasure;

void log_operation(const char *hunt_path, const char *operation) {
    char log_path[PATH_MAX];
    snprintf(log_path, sizeof(log_path), "%s\\%s", hunt_path, LOG_FILE);
    int fd = open(log_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0) return;

    // Replacer dprintf par fprintf avec fdopen
    FILE *log_file = fdopen(fd, "a");
    if (log_file != NULL) {
        time_t t = time(NULL);
        struct tm *tm_info = localtime(&t);
        char timestamp[25];
        strftime(timestamp, 25, "%Y-%m-%d %H:%M:%S", tm_info);
        fprintf(log_file, "[%s] %s\n", timestamp, operation);
        fclose(log_file);
    }
    close(fd);

    // Ignorer le symlink sur Windows
    // symlink(log_path, symlink_name);
}

int add_treasure(const char *hunt_id) {
    char dir_path[PATH_MAX];
    snprintf(dir_path, sizeof(dir_path), "%s", hunt_id);
    mkdir(dir_path);  // Sur Windows, mkdir prend un seul argument.

    char file_path[PATH_MAX];
    snprintf(file_path, sizeof(file_path), "%s\\%s", dir_path, TREASURE_FILE);

    Treasure t;
    printf("Enter Treasure ID: ");
    scanf("%s", t.treasure_id);
    printf("Enter User Name: ");
    scanf("%s", t.user_name);
    printf("Enter Latitude: ");
    scanf("%f", &t.latitude);
    printf("Enter Longitude: ");
    scanf("%f", &t.longitude);
    printf("Enter Clue: ");
    getchar(); // consume newline
    fgets(t.clue, CLUE_LEN, stdin);
    t.clue[strcspn(t.clue, "\n")] = '\0';
    printf("Enter Value: ");
    scanf("%d", &t.value);

    int fd = open(file_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0) {
        perror("open");
        return 1;
    }
    write(fd, &t, sizeof(Treasure));
    close(fd);

    log_operation(dir_path, "add");
    return 0;
}

int list_treasures(const char *hunt_id) {
    char dir_path[PATH_MAX], file_path[PATH_MAX];
    snprintf(dir_path, sizeof(dir_path), "%s", hunt_id);
    snprintf(file_path, sizeof(file_path), "%s\\%s", dir_path, TREASURE_FILE);

    struct stat st;
    if (stat(file_path, &st) < 0) {
        perror("stat");
        return 1;
    }
    printf("Hunt: %s\n", hunt_id);
    printf("Total Size: %ld bytes\n", st.st_size);
    printf("Last Modified: %s", ctime(&st.st_mtime));

    int fd = open(file_path, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }
    Treasure t;
    while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) {
        printf("ID: %s, User: %s, Lat: %.2f, Long: %.2f, Value: %d\n",
               t.treasure_id, t.user_name, t.latitude, t.longitude, t.value);
    }
    close(fd);
    log_operation(dir_path, "list");
    return 0;
}

int view_treasure(const char *hunt_id, const char *treasure_id) {
    char file_path[PATH_MAX];
    snprintf(file_path, sizeof(file_path), "%s\\%s", hunt_id, TREASURE_FILE);

    int fd = open(file_path, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }
    Treasure t;
    while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) {
        if (strcmp(t.treasure_id, treasure_id) == 0) {
            printf("ID: %s\nUser: %s\nLat: %.2f\nLong: %.2f\nClue: %s\nValue: %d\n",
                   t.treasure_id, t.user_name, t.latitude, t.longitude, t.clue, t.value);
            close(fd);
            log_operation(hunt_id, "view");
            return 0;
        }
    }
    printf("Treasure not found.\n");
    close(fd);
    return 1;
}

int remove_treasure(const char *hunt_id, const char *treasure_id) {
    char file_path[PATH_MAX], tmp_path[PATH_MAX];
    snprintf(file_path, sizeof(file_path), "%s\\%s", hunt_id, TREASURE_FILE);
    snprintf(tmp_path, sizeof(tmp_path), "%s\\temp.dat", hunt_id);

    int fd = open(file_path, O_RDONLY);
    int fd_tmp = open(tmp_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0 || fd_tmp < 0) {
        perror("open");
        return 1;
    }

    Treasure t;
    int found = 0;
    while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) {
        if (strcmp(t.treasure_id, treasure_id) != 0) {
            write(fd_tmp, &t, sizeof(Treasure));
        } else {
            found = 1;
        }
    }
    close(fd);
    close(fd_tmp);
    rename(tmp_path, file_path);

    if (found) log_operation(hunt_id, "remove_treasure");
    else printf("Treasure not found.\n");
    return 0;
}

int remove_hunt(const char *hunt_id) {
    char path[PATH_MAX];
    snprintf(path, sizeof(path), "%s", hunt_id);

    char file_path[PATH_MAX];
    snprintf(file_path, sizeof(file_path), "%s\\%s", path, TREASURE_FILE);
    unlink(file_path);

    snprintf(file_path, sizeof(file_path), "%s\\%s", path, LOG_FILE);
    unlink(file_path);

    rmdir(path);

    log_operation(".", "remove_hunt");
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <operation> <hunt_id> [<id>]\n", argv[0]);
        return 1;
    }
    if (strcmp(argv[1], "add") == 0) return add_treasure(argv[2]);
    if (strcmp(argv[1], "list") == 0) return list_treasures(argv[2]);
    if (strcmp(argv[1], "view") == 0 && argc == 4) return view_treasure(argv[2], argv[3]);
    if (strcmp(argv[1], "remove_treasure") == 0 && argc == 4) return remove_treasure(argv[2], argv[3]);
    if (strcmp(argv[1], "remove_hunt") == 0) return remove_hunt(argv[2]);

    fprintf(stderr, "Invalid command or parameters.\n");
    return 1;
}
