/*VFS (VIRTUAL FILE SYSTEM)*/

/*Include*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <sys/types.h>

/* Define */
#define BLOCK_SIZE 4096
#define MAX_BLOCKS 1024
#define MAX_FILES 128
#define MAX_NAME_LEN 256
#define INODE_BLOCKS 16
#define MAX_PATH_LEN 1024
#define SAVE_FILE "vfs_save.bin"

/* Struct */
typedef enum { FILE_TYPE, DIR_TYPE } inode_type;

// Inode
typedef struct {
    uint32_t id;
    inode_type type;
    size_t size;
    time_t ctime;
    time_t mtime;
    uint32_t blocks[INODE_BLOCKS]; // Index blocks data
} inode_t;

// Entry in the directory
typedef struct {
    char name[MAX_NAME_LEN];
    uint32_t inode_id;
} dir_entry_t;

// Super-block
typedef struct {
    uint32_t magic;
    uint32_t block_size;
    uint32_t free_blocks[MAX_BLOCKS / 32];
} superblock_t;

// VFS condition
typedef struct {
    superblock_t super;
    inode_t inodes[MAX_FILES];
    uint8_t* blocks[MAX_BLOCKS];
    inode_t* root;
    inode_t* current_dir;
    char current_path[MAX_PATH_LEN];
} vfs_state_t;

/* Prototype */
void vfs_init(vfs_state_t* vfs);
inode_t* vfs_create(vfs_state_t* vfs, const char* name, inode_type type);
inode_t* vfs_lookup(vfs_state_t* vfs, const char* name);
ssize_t vfs_write(vfs_state_t* vfs, inode_t* file, const char* data, size_t size);
void vfs_ls(vfs_state_t* vfs);
int vfs_cd(vfs_state_t* vfs, const char* path);
int vfs_unlink(vfs_state_t* vfs, const char* name);
void clear_input_buffer();
void print_menu();
int vfs_save(vfs_state_t* vfs, const char* filename);
int vfs_load(vfs_state_t* vfs, const char* filename);
int is_name_valid(const char* name);

int main() {
    vfs_state_t vfs;
    vfs_init(&vfs);

    // Upload the saved state, if available
    if (vfs_load(&vfs, SAVE_FILE) == 0) {
        printf("VFS state loaded successfully from %s\n", SAVE_FILE);
    } else {
        printf("Starting with new VFS. No saved state found.\n");
    }

    int choice;
    char name[MAX_NAME_LEN];
    char content[BLOCK_SIZE];
    char path[MAX_PATH_LEN];

    while (1) {
        print_menu();
        printf("\nVFS [%s] > ", vfs.current_path);
        if (scanf("%d", &choice) != 1) {
            clear_input_buffer();
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        clear_input_buffer();

        switch (choice) {
            case 1: // Create file
                printf("Enter file name: ");
                if (!fgets(name, MAX_NAME_LEN, stdin)) {
                    printf("Error reading input\n");
                    break;
                }
                name[strcspn(name, "\n")] = '\0';

                if (!is_name_valid(name)) {
                    printf("Error: File name cannot:\n"
                           " - Start with space or tab\n"
                           " - Contain special characters (/\\:*?\"<>|)\n"
                           " - Be empty or consist only of whitespace\n");
                    break;
                }

                if (vfs_create(&vfs, name, FILE_TYPE)) {
                    printf("File '%s' created successfully.\n", name);
                } else {
                    printf("Error: Failed to create file '%s'\n", name);
                }
                break;

            case 2: // Write to file
                printf("Enter file name: ");
                if (!fgets(name, MAX_NAME_LEN, stdin)) {
                    printf("Error reading input\n");
                    break;
                }
                name[strcspn(name, "\n")] = '\0';

                inode_t* file = vfs_lookup(&vfs, name);
                if (file && file->type == FILE_TYPE) {
                    printf("Enter content (max %d chars): ", BLOCK_SIZE - 1);
                    if (!fgets(content, BLOCK_SIZE, stdin)) {
                        printf("Error reading content\n");
                        break;
                    }
                    content[strcspn(content, "\n")] = '\0';

                    ssize_t written = vfs_write(&vfs, file, content, strlen(content));
                    if (written >= 0) {
                        printf("Wrote %ld bytes to '%s'\n", (long)written, name);
                    } else {
                        printf("Error writing to file\n");
                    }
                } else {
                    printf("File not found or is a directory\n");
                }
                break;

            case 3: // Delete file
                printf("Enter file name: ");
                if (!fgets(name, MAX_NAME_LEN, stdin)) {
                    printf("Error reading input\n");
                    break;
                }
                name[strcspn(name, "\n")] = '\0';

                int result = vfs_unlink(&vfs, name);
                if (result == 0) {
                    printf("File '%s' deleted\n", name);
                } else if (result == -1) {
                    printf("File not found\n");
                } else if (result == -2) {
                    printf("Cannot delete non-empty directory\n");
                }
                break;

            case 4: // List directory
                vfs_ls(&vfs);
                break;

            case 5: // Create directory
                printf("Enter directory name: ");
                if (!fgets(name, MAX_NAME_LEN, stdin)) {
                    printf("Error reading input\n");
                    break;
                }
                name[strcspn(name, "\n")] = '\0';

                if (!is_name_valid(name)) {
                    printf("Error: Directory name cannot:\n"
                           " - Start with space or tab\n"
                           " - Contain special characters (/\\:*?\"<>|)\n"
                           " - Be empty or consist only of whitespace\n");
                    break;
                }

                if (vfs_create(&vfs, name, DIR_TYPE)) {
                    printf("Directory '%s' created\n", name);
                } else {
                    printf("Error creating directory\n");
                }
                break;

            case 6: // Change directory
                printf("Enter path (or '..' for parent): ");
                if (!fgets(path, MAX_PATH_LEN, stdin)) {
                    printf("Error reading input\n");
                    break;
                }
                path[strcspn(path, "\n")] = '\0';

                if (vfs_cd(&vfs, path) == 0) {
                    printf("Current directory: %s\n", vfs.current_path);
                } else {
                    printf("Directory not found\n");
                }
                break;

            case 7: // Back to parent directory
                if (vfs_cd(&vfs, "..") == 0) {
                    printf("Back to parent directory: %s\n", vfs.current_path);
                } else {
                    printf("Already at root directory\n");
                }
                break;

            case 8: // Exit
                // Save the VFS state before exiting
                if (vfs_save(&vfs, SAVE_FILE) == 0) {
                    printf("VFS state saved to %s\n", SAVE_FILE);
                } else {
                    printf("Failed to save VFS state\n");
                }

                // Free resources
                for (int i = 0; i < MAX_BLOCKS; i++) {
                    if (vfs.blocks[i]) free(vfs.blocks[i]);
                }
                printf("Exiting VFS. Goodbye!\n");
                return 0;

            default:
                printf("Invalid choice. Please try again.\n");
        }
    }
    return 0;
}

/* Function Implementations */
void vfs_init(vfs_state_t* vfs) {
    memset(vfs, 0, sizeof(vfs_state_t));
    vfs->super.magic = 0xDEADBEEF;
    vfs->super.block_size = BLOCK_SIZE;

    // Initialize root directory
    vfs->root = &vfs->inodes[0];
    vfs->root->id = 1;
    vfs->root->type = DIR_TYPE;
    vfs->root->ctime = time(NULL);
    vfs->current_dir = vfs->root;
    strcpy(vfs->current_path, "/");

    // Allocate root directory block
    vfs->blocks[0] = malloc(BLOCK_SIZE);
    if (!vfs->blocks[0]) {
        fprintf(stderr, "FATAL: Failed to allocate root block\n");
        exit(EXIT_FAILURE);
    }
    memset(vfs->blocks[0], 0, BLOCK_SIZE);
    vfs->root->blocks[0] = 0;
    vfs->super.free_blocks[0] |= 1; // Mark block 0 as used

    // Initialize root directory entries
    dir_entry_t* root_dir = (dir_entry_t*)vfs->blocks[0];
    strncpy(root_dir[0].name, ".", MAX_NAME_LEN);
    root_dir[0].inode_id = 1;
    strncpy(root_dir[1].name, "..", MAX_NAME_LEN);
    root_dir[1].inode_id = 1;
    vfs->root->size = 2 * sizeof(dir_entry_t);
}

int is_name_valid(const char* name) {
    // Check for empty name
    if (name == NULL || name[0] == '\0') {
        return 0;
    }

    // Check if name starts with space or tab
    if (name[0] == ' ' || name[0] == '\t') {
        return 0;
    }

    // Check each character in the name
    for (int i = 0; name[i] != '\0'; i++) {
        unsigned char c = name[i];

        // Allow only printable characters (ASCII 32-126)
        if (c < 32 || c > 126) {
            return 0;
        }

        // Additional forbidden characters for safety
        if (c == '/' || c == '\\' || c == ':' ||
            c == '*' || c == '?' || c == '"' ||
            c == '<' || c == '>' || c == '|') {
            return 0;
        }
    }

    // Check if name consists only of whitespace
    for (int i = 0; name[i] != '\0'; i++) {
        if (name[i] != ' ' && name[i] != '\t') {
            return 1;
        }
    }

    return 0;
}

inode_t* vfs_create(vfs_state_t* vfs, const char* name, inode_type type) {
    // Validate name
    if (!is_name_valid(name)) {
        printf("Invalid name: cannot be empty\n");
        return NULL;
    }

    // Check if name exists
    if (vfs_lookup(vfs, name)) {
        printf("Name '%s' already exists\n", name);
        return NULL;
    }

    // Find free inode
    uint32_t inode_id = 0;
    for (; inode_id < MAX_FILES; inode_id++) {
        if (vfs->inodes[inode_id].id == 0) break;
    }
    if (inode_id >= MAX_FILES) {
        printf("No free inodes\n");
        return NULL;
    }

    // Find free block
    uint32_t block_id = 0;
    for (; block_id < MAX_BLOCKS; block_id++) {
        if (!(vfs->super.free_blocks[block_id / 32] & (1 << (block_id % 32)))) break;
    }
    if (block_id >= MAX_BLOCKS) {
        printf("No free blocks\n");
        return NULL;
    }

    // Initialize inode
    inode_t* inode = &vfs->inodes[inode_id];
    inode->id = inode_id + 1;
    inode->type = type;
    inode->ctime = time(NULL);
    inode->size = 0;
    inode->blocks[0] = block_id;

    // Allocate block
    vfs->blocks[block_id] = malloc(BLOCK_SIZE);
    if (!vfs->blocks[block_id]) {
        printf("Failed to allocate block\n");
        memset(inode, 0, sizeof(inode_t));
        return NULL;
    }
    memset(vfs->blocks[block_id], 0, BLOCK_SIZE);
    vfs->super.free_blocks[block_id / 32] |= (1 << (block_id % 32));

    // Initialize directory entries
    if (type == DIR_TYPE) {
        dir_entry_t* dir = (dir_entry_t*)vfs->blocks[block_id];
        strncpy(dir[0].name, ".", MAX_NAME_LEN);
        dir[0].inode_id = inode->id;
        strncpy(dir[1].name, "..", MAX_NAME_LEN);
        dir[1].inode_id = vfs->current_dir->id;
        inode->size = 2 * sizeof(dir_entry_t);
    }

    // Add to current directory
    uint32_t dir_block = vfs->current_dir->blocks[0];
    if (dir_block >= MAX_BLOCKS || !vfs->blocks[dir_block]) {
        printf("Current directory invalid\n");
        free(vfs->blocks[block_id]);
        memset(inode, 0, sizeof(inode_t));
        return NULL;
    }

    dir_entry_t* current_dir = (dir_entry_t*)vfs->blocks[dir_block];
    uint32_t entry_count = vfs->current_dir->size / sizeof(dir_entry_t);
    if ((entry_count + 1) * sizeof(dir_entry_t) >= BLOCK_SIZE) {
        printf("Directory full\n");
        free(vfs->blocks[block_id]);
        memset(inode, 0, sizeof(inode_t));
        return NULL;
    }

    strncpy(current_dir[entry_count].name, name, MAX_NAME_LEN - 1);
    current_dir[entry_count].name[MAX_NAME_LEN - 1] = '\0';
    current_dir[entry_count].inode_id = inode->id;
    vfs->current_dir->size += sizeof(dir_entry_t);

    return inode;
}

inode_t* vfs_lookup(vfs_state_t* vfs, const char* name) {
    if (!name || !*name) return NULL;

    // Special directories
    if (strcmp(name, ".") == 0) return vfs->current_dir;
    if (strcmp(name, "..") == 0) {
        if (vfs->current_dir == vfs->root) return vfs->root;
        uint32_t block_id = vfs->current_dir->blocks[0];
        if (block_id >= MAX_BLOCKS || !vfs->blocks[block_id]) return NULL;
        dir_entry_t* dir = (dir_entry_t*)vfs->blocks[block_id];
        return &vfs->inodes[dir[1].inode_id - 1];
    }

    // Regular lookup
    uint32_t block_id = vfs->current_dir->blocks[0];
    if (block_id >= MAX_BLOCKS || !vfs->blocks[block_id]) return NULL;

    dir_entry_t* dir = (dir_entry_t*)vfs->blocks[block_id];
    uint32_t entry_count = vfs->current_dir->size / sizeof(dir_entry_t);

    for (uint32_t i = 0; i < entry_count; i++) {
        if (strcmp(dir[i].name, name) == 0) {
            return &vfs->inodes[dir[i].inode_id - 1];
        }
    }
    return NULL;
}

int vfs_unlink(vfs_state_t* vfs, const char* name) {
    if (!name || strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
        printf("Invalid name\n");
        return -1;
    }

    uint32_t dir_block = vfs->current_dir->blocks[0];
    if (dir_block >= MAX_BLOCKS || !vfs->blocks[dir_block]) {
        printf("Directory invalid\n");
        return -1;
    }

    dir_entry_t* dir = (dir_entry_t*)vfs->blocks[dir_block];
    uint32_t entry_count = vfs->current_dir->size / sizeof(dir_entry_t);
    uint32_t index = 0;
    inode_t* target = NULL;

    // Find entry
    for (; index < entry_count; index++) {
        if (strcmp(dir[index].name, name) == 0) {
            target = &vfs->inodes[dir[index].inode_id - 1];
            break;
        }
    }

    if (!target) return -1; // Not found

    // Validate directory
    if (target->type == DIR_TYPE) {
        if (target->size > 2 * sizeof(dir_entry_t)) {
            printf("Directory not empty\n");
            return -2;
        }
    }

    // Free blocks
    for (int i = 0; i < INODE_BLOCKS; i++) {
        uint32_t block_id = target->blocks[i];
        if (block_id && block_id < MAX_BLOCKS && vfs->blocks[block_id]) {
            free(vfs->blocks[block_id]);
            vfs->blocks[block_id] = NULL;
            vfs->super.free_blocks[block_id / 32] &= ~(1 << (block_id % 32));
        }
    }

    // Remove from directory
    memset(target, 0, sizeof(inode_t));
    if (index < entry_count - 1) {
        memmove(&dir[index], &dir[index + 1],
                (entry_count - index - 1) * sizeof(dir_entry_t));
    }
    vfs->current_dir->size -= sizeof(dir_entry_t);
    memset(&dir[entry_count - 1], 0, sizeof(dir_entry_t));

    return 0;
}

ssize_t vfs_write(vfs_state_t* vfs, inode_t* file, const char* data, size_t size) {
    if (!file || file->type != FILE_TYPE || !data || size == 0) return -1;

    size_t offset = 0;
    size_t remaining = size;

    // We find the first free block for writing
    int start_block = 0;
    while (start_block < INODE_BLOCKS && file->blocks[start_block] != 0) {
        start_block++;
    }

    // Recording data starting from the current position
    for (int i = start_block; i < INODE_BLOCKS && remaining > 0; i++) {
        uint32_t block_id = file->blocks[i];

        // Allocation of a new block (if necessary)
        if (block_id == 0) {
            // Search for a free block
            for (block_id = 0; block_id < MAX_BLOCKS; block_id++) {
                uint32_t block_idx = block_id / 32;
                uint32_t bit_mask = 1 << (block_id % 32);

                if (!(vfs->super.free_blocks[block_idx] & bit_mask)) {
                    // block as occupied
                    vfs->super.free_blocks[block_idx] |= bit_mask;
                    file->blocks[i] = block_id;

                    // Memory is allocated for the block
                    vfs->blocks[block_id] = malloc(BLOCK_SIZE);
                    if (!vfs->blocks[block_id]) {
                        printf("Failed to allocate block %u\n", block_id);
                        file->blocks[i] = 0;
                        vfs->super.free_blocks[block_idx] &= ~bit_mask;
                        break;
                    }
                    memset(vfs->blocks[block_id], 0, BLOCK_SIZE);
                    break;
                }
            }
            if (block_id >= MAX_BLOCKS) {
                printf("No free blocks available\n");
                break;
            }
        }

        // Checking the validity of the block
        if (block_id >= MAX_BLOCKS || !vfs->blocks[block_id]) {
            printf("Invalid block %u\n", block_id);
            break;
        }

        // Calculating the size of the data to write to the current block
        size_t block_offset = file->size % BLOCK_SIZE;
        size_t space_in_block = BLOCK_SIZE - block_offset;
        size_t to_copy = (remaining < space_in_block) ? remaining : space_in_block;

        // Coping data in block
        memcpy(vfs->blocks[block_id] + block_offset, data + offset, to_copy);

        offset += to_copy;
        remaining -= to_copy;

        // Update size data
        if (block_offset + to_copy > file->size % BLOCK_SIZE) {
            file->size += to_copy;
        }
    }

    file->mtime = time(NULL);
    return offset; // Return the number of bytes written
}

void vfs_ls(vfs_state_t* vfs) {
    if (!vfs->current_dir) {
        printf("No current directory\n");
        return;
    }

    uint32_t block_id = vfs->current_dir->blocks[0];
    if (block_id >= MAX_BLOCKS || !vfs->blocks[block_id]) {
        printf("Directory data invalid\n");
        return;
    }

    dir_entry_t* dir = (dir_entry_t*)vfs->blocks[block_id];
    uint32_t entry_count = vfs->current_dir->size / sizeof(dir_entry_t);

    printf("\nContents of %s:\n", vfs->current_path);
    printf("%-20s %-8s %s\n", "Name", "Type", "Created");
    printf("----------------------------------------\n");

    for (uint32_t i = 0; i < entry_count; i++) {
        inode_t* inode = &vfs->inodes[dir[i].inode_id - 1];
        char* type = (inode->type == DIR_TYPE) ? "DIR" : "FILE";
        char time_buf[32];
        strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", localtime(&inode->ctime));
        printf("%-20s %-8s %s\n", dir[i].name, type, time_buf);
    }
    printf("Total: %u items\n", entry_count);
}

int vfs_cd(vfs_state_t* vfs, const char* path) {
    if (!path) return -1;

    // Root directory
    if (strcmp(path, "/") == 0) {
        vfs->current_dir = vfs->root;
        strcpy(vfs->current_path, "/");
        return 0;
    }

    // Parent directory
    if (strcmp(path, "..") == 0) {
        if (vfs->current_dir == vfs->root) return 0;

        uint32_t block_id = vfs->current_dir->blocks[0];
        if (block_id >= MAX_BLOCKS || !vfs->blocks[block_id]) return -1;

        dir_entry_t* dir = (dir_entry_t*)vfs->blocks[block_id];
        vfs->current_dir = &vfs->inodes[dir[1].inode_id - 1];

        // Update path
        char* last_slash = strrchr(vfs->current_path, '/');
        if (last_slash) {
            if (last_slash == vfs->current_path) {
                // Located in the root directory
                vfs->current_path[1] = '\0';
            } else {
                *last_slash = '\0';
            }
        }
        return 0;
    }

    // Regular directory
    inode_t* dir_inode = vfs_lookup(vfs, path);
    if (!dir_inode || dir_inode->type != DIR_TYPE) return -1;

    vfs->current_dir = dir_inode;
    if (strcmp(vfs->current_path, "/") != 0) {
        strcat(vfs->current_path, "/");
    }
    strcat(vfs->current_path, path);
    return 0;
}

int vfs_save(vfs_state_t* vfs, const char* filename) {
    FILE* f = fopen(filename, "wb");
    if (!f) {
        perror("Failed to open save file");
        return -1;
    }

    // Save super-block
    if (fwrite(&vfs->super, sizeof(superblock_t), 1, f) != 1) {
        perror("Failed to write superblock");
        fclose(f);
        return -1;
    }

    // Save inodes
    if (fwrite(vfs->inodes, sizeof(inode_t), MAX_FILES, f) != MAX_FILES) {
        perror("Failed to write inodes");
        fclose(f);
        return -1;
    }

    // Save data blocks
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (vfs->blocks[i]) {
            if (fwrite(vfs->blocks[i], BLOCK_SIZE, 1, f) != 1) {
                perror("Failed to write data block");
                fclose(f);
                return -1;
            }
        }
    }

    // Save current path
    size_t path_len = strlen(vfs->current_path) + 1;
    if (fwrite(vfs->current_path, path_len, 1, f) != 1) {
        perror("Failed to write current path");
        fclose(f);
        return -1;
    }

    fclose(f);
    return 0;
}

int vfs_load(vfs_state_t* vfs, const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        return -1; // File not exist yet
    }

    // Load super-block
    if (fread(&vfs->super, sizeof(superblock_t), 1, f) != 1) {
        fclose(f);
        return -1;
    }

    // Load inodes
    if (fread(vfs->inodes, sizeof(inode_t), MAX_FILES, f) != MAX_FILES) {
        fclose(f);
        return -1;
    }

    // Free existing blocks if any
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (vfs->blocks[i]) {
            free(vfs->blocks[i]);
            vfs->blocks[i] = NULL;
        }
    }

    // Load data blocks
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (vfs->super.free_blocks[i / 32] & (1 << (i % 32))) {
            vfs->blocks[i] = malloc(BLOCK_SIZE);
            if (!vfs->blocks[i]) {
                perror("Failed to allocate memory for block");
                fclose(f);
                return -1;
            }
            if (fread(vfs->blocks[i], BLOCK_SIZE, 1, f) != 1) {
                free(vfs->blocks[i]);
                vfs->blocks[i] = NULL;
                fclose(f);
                return -1;
            }
        }
    }

    // Load current path
    if (fread(vfs->current_path, MAX_PATH_LEN, 1, f) != 1) {
        strcpy(vfs->current_path, "/");
    }

    // Set root and current directory pointers
    vfs->root = &vfs->inodes[0];
    if (vfs_cd(vfs, vfs->current_path) != 0) {
        // Fall-back to root if path is invalid
        strcpy(vfs->current_path, "/");
        vfs->current_dir = vfs->root;
    }

    fclose(f);
    return 0;
}

void print_menu() {
    printf("\n=== Virtual File System Menu ===\n");
    printf("1. Create file\n");
    printf("2. Write to file\n");
    printf("3. Delete file/directory\n");
    printf("4. List directory contents\n");
    printf("5. Create directory\n");
    printf("6. Change directory\n");
    printf("7. Back to parent directory\n");
    printf("8. Exit\n");
}

void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}
