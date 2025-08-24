/*Shif-dev program*/

/*Include*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*Prepross*/
#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

/*Define*/
#define SIGNATURE "CIPH"
#define VERSION 1
#define HEADER_SIZE 5
#define MAX_FILES 100
#define BUFFER_SIZE 4096
#define CHECK_STRING "VERIFY"  // Verification string for key validation
#define CHECK_SIZE 6           // Length of verification string

/*Prototype*/
void process_data(FILE* input, FILE* output, const char* key); // Data encryption/decryption function
int is_encrypted(FILE* f); // Checks if the file is encrypted
void encrypt_file_inplace(const char* filename, const char* key); // Encrypts the file in place with the addition of a header
void decrypt_file_inplace(const char* filename, const char* key); // Decrypts the file in place with the header removed
int create_directory(const char* path); // Create directory
int get_files_count(); // Counts files in the projects folder
void list_files(); // Displays a list of files in the "projects" folder
char* get_filename_by_index(int index); // Retrieves the file name by its number in the list
void interactive_menu(); // Menu

int main(int argc, char* argv[])
{
    if (argc == 1) {
        interactive_menu();
        return 0;
    }

    if (argc < 3 || argc > 5) {
        fprintf(stderr, "Usage: %s -e|-d key [input] [output]\n", argv[0]);
        return 1;
    }

    const char* mode = argv[1];
    const char* key = argv[2];
    const char* input_file = (argc > 3) ? argv[3] : NULL;
    const char* output_file = (argc > 4) ? argv[4] : NULL;

    if (strcmp(mode, "-e") != 0 && strcmp(mode, "-d") != 0) {
        fprintf(stderr, "Invalid mode. Use -e for encryption or -d for decryption\n");
        return 1;
    }

    FILE* input = stdin;
    FILE* output = stdout;

    if (input_file) {
        input = fopen(input_file, "rb");
        if (!input) {
            perror("Error opening input file");
            return 1;
        }
    }

    if (output_file) {
        output = fopen(output_file, "wb");
        if (!output) {
            perror("Error opening output file");
            if (input != stdin) fclose(input);
            return 1;
        }
    }

    process_data(input, output, key);

    if (input != stdin) fclose(input);
    if (output != stdout) fclose(output);

    return 0;
}

/*Function*/

void process_data(FILE* input, FILE* output, const char* key)
{
    int key_len = strlen(key);
    if (key_len == 0) {
        int ch;
        while ((ch = fgetc(input)) != EOF) {
            fputc(ch, output);
        }
        return;
    }

    int key_index = 0;
    int ch;
    while ((ch = fgetc(input)) != EOF) {
        fputc(ch ^ key[key_index], output);
        key_index = (key_index + 1) % key_len;
    }
}

int is_encrypted(FILE* f)
{
    char header[HEADER_SIZE];
    if (fread(header, 1, HEADER_SIZE, f) != HEADER_SIZE) {
        return 0;
    }
    rewind(f);
    return memcmp(header, SIGNATURE, 4) == 0 && header[4] == VERSION;
}

void encrypt_file_inplace(const char* filename, const char* key)
{
    // Open file for reading
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening file");
        return;
    }

    // Check if file is already encrypted
    if (is_encrypted(file)) {
        printf("Error: File is already encrypted\n");
        fclose(file);
        return;
    }
    rewind(file);

    // Create temporary file for writing
    char temp_filename[256];
    snprintf(temp_filename, sizeof(temp_filename), "%s.tmp", filename);
    FILE* temp_file = fopen(temp_filename, "wb");
    if (!temp_file) {
        perror("Error creating temporary file");
        fclose(file);
        return;
    }

    // Write signature and version
    fwrite(SIGNATURE, 1, 4, temp_file);
    fputc(VERSION, temp_file);

    // Write verification string (encrypted)
    int key_len = strlen(key);
    int key_index = 0;
    for (int i = 0; i < CHECK_SIZE; i++) {
        char c = CHECK_STRING[i];
        if (key_len > 0) {
            c = c ^ key[key_index];
            key_index = (key_index + 1) % key_len;
        }
        fputc(c, temp_file);
    }

    // Encrypt data
    int ch;
    while ((ch = fgetc(file)) != EOF) {
        if (key_len > 0) {
            ch = ch ^ key[key_index];
            key_index = (key_index + 1) % key_len;
        }
        fputc(ch, temp_file);
    }

    // Close files
    fclose(file);
    fclose(temp_file);

    // Replace original file with encrypted version
    remove(filename);
    rename(temp_filename, filename);

    printf("File encrypted successfully: %s\n", filename);
}

void decrypt_file_inplace(const char* filename, const char* key)
{
    // Open file for reading
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening file");
        return;
    }

    // Check if file is encrypted
    if (!is_encrypted(file)) {
        printf("Error: File is not encrypted\n");
        fclose(file);
        return;
    }

    // Skip header
    fseek(file, HEADER_SIZE, SEEK_SET);

    // Read and verify the encrypted verification string
    char check_buf[CHECK_SIZE];
    if (fread(check_buf, 1, CHECK_SIZE, file) != CHECK_SIZE) {
        printf("Error: File too short for verification\n");
        fclose(file);
        return;
    }

    int key_len = strlen(key);
    int key_index = 0;
    char decrypted_check[CHECK_SIZE + 1] = {0};

    // Decrypt verification string
    for (int i = 0; i < CHECK_SIZE; i++) {
        decrypted_check[i] = check_buf[i];
        if (key_len > 0) {
            decrypted_check[i] = decrypted_check[i] ^ key[key_index];
            key_index = (key_index + 1) % key_len;
        }
    }

    // Verify decrypted string
    if (strcmp(decrypted_check, CHECK_STRING) != 0) {
        printf("Error: Wrong key! File cannot be decrypted.\n");
        fclose(file);
        return;
    }

    // Create temporary file for writing
    char temp_filename[256];
    snprintf(temp_filename, sizeof(temp_filename), "%s.tmp", filename);
    FILE* temp_file = fopen(temp_filename, "wb");
    if (!temp_file) {
        perror("Error creating temporary file");
        fclose(file);
        return;
    }

    // Decrypt data
    int ch;
    while ((ch = fgetc(file)) != EOF) {
        if (key_len > 0) {
            ch = ch ^ key[key_index];
            key_index = (key_index + 1) % key_len;
        }
        fputc(ch, temp_file);
    }

    // Close files
    fclose(file);
    fclose(temp_file);

    // Replace original file with decrypted version
    remove(filename);
    rename(temp_filename, filename);

    printf("File decrypted successfully: %s\n", filename);
}

int create_directory(const char* path)
{
#ifdef _WIN32
    return _mkdir(path);
#else
    return mkdir(path, 0777);
#endif
}

int get_files_count()
{
    int count = 0;
#ifdef _WIN32
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile("projects\\*", &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) return 0;

    do {
        if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            count++;
        }
    } while (FindNextFile(hFind, &findFileData) != 0);
    FindClose(hFind);
#else
    DIR* dir = opendir("projects");
    if (!dir) return 0;

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            count++;
        }
    }
    closedir(dir);
#endif
    return count;
}

void list_files()
{
    int count = 1;
#ifdef _WIN32
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile("projects\\*", &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) return;

    do {
        if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            printf("%d. %s\n", count++, findFileData.cFileName);
        }
    } while (FindNextFile(hFind, &findFileData) != 0);
    FindClose(hFind);
#else
    DIR* dir = opendir("projects");
    if (!dir) return;

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            printf("%d. %s\n", count++, entry->d_name);
        }
    }
    closedir(dir);
#endif
}

char* get_filename_by_index(int index)
{
    int count = 0;
    static char filename[256];
#ifdef _WIN32
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile("projects\\*", &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) return NULL;

    do {
        if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            if (++count == index) {
                strcpy(filename, findFileData.cFileName);
                FindClose(hFind);
                return filename;
            }
        }
    } while (FindNextFile(hFind, &findFileData) != 0);
    FindClose(hFind);
#else
    DIR* dir = opendir("projects");
    if (!dir) return NULL;

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            if (++count == index) {
                strcpy(filename, entry->d_name);
                closedir(dir);
                return filename;
            }
        }
    }
    closedir(dir);
#endif
    return NULL;
}

void interactive_menu() 
{
    if (create_directory("projects") != 0) {
        #ifdef _WIN32
        if (GetLastError() != ERROR_ALREADY_EXISTS)
        #endif
        printf("Using existing projects directory\n");
    }

    while (1) {
        printf("\nFile Manager Menu:\n");
        printf("1. List files\n");
        printf("2. Encrypt file\n");
        printf("3. Decrypt file\n");
        printf("4. Exit\n");
        printf("Select option: ");

        int choice;
        scanf("%d", &choice);
        while (getchar() != '\n');

        if (choice == 4) break;

        int file_count = get_files_count();

        switch (choice) {
            case 1:
                printf("\nFiles in projects directory:\n");
                if (file_count == 0) {
                    printf("No files found\n");
                } else {
                    list_files();
                }
                break;

            case 2: {
                if (file_count == 0) {
                    printf("No files found in projects directory\n");
                    break;
                }

                printf("\nAvailable files:\n");
                list_files();
                printf("Select file number: ");
                int enc_index;
                scanf("%d", &enc_index);
                while (getchar() != '\n');

                if (enc_index < 1 || enc_index > file_count) {
                    printf("Invalid file number\n");
                    break;
                }

                char* filename = get_filename_by_index(enc_index);
                if (!filename) {
                    printf("File not found\n");
                    break;
                }

                char fullpath[256];
                snprintf(fullpath, sizeof(fullpath), "projects/%s", filename);

                printf("Enter encryption key: ");
                char key[256];
                fgets(key, sizeof(key), stdin);
                key[strcspn(key, "\n")] = '\0';

                encrypt_file_inplace(fullpath, key);
                break;
            }

            case 3: {
                if (file_count == 0) {
                    printf("No files found in projects directory\n");
                    break;
                }

                printf("\nAvailable files:\n");
                list_files();
                printf("Select file number: ");
                int dec_index;
                scanf("%d", &dec_index);
                while (getchar() != '\n');

                if (dec_index < 1 || dec_index > file_count) {
                    printf("Invalid file number\n");
                    break;
                }

                char* filename = get_filename_by_index(dec_index);
                if (!filename) {
                    printf("File not found\n");
                    break;
                }

                char fullpath[256];
                snprintf(fullpath, sizeof(fullpath), "projects/%s", filename);

                printf("Enter decryption key: ");
                char key[256];
                fgets(key, sizeof(key), stdin);
                key[strcspn(key, "\n")] = '\0';

                decrypt_file_inplace(fullpath, key);
                break;
            }

            default:
                printf("Invalid option\n");
        }
    }
}
