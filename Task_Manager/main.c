/* Task Manager(Console) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>  // For isspace()

#define MAX_TASKS 100       // The limit for a simple application
#define TASK_LEN 256        // Max length for task description
#define FILENAME "tasks.dat" // Data file name

// The main data structure for tasks
typedef struct {
    char text[TASK_LEN];    // Task description
    int done;               // Completion status (0 = pending, 1 = done)
} TodoTask;

// Global state
TodoTask tasks[MAX_TASKS];  // Task storage
int task_count = 0;         // Current number of tasks

/* FUNCTION PROTOTYPE */
void trim_newline(char* str);
int is_empty(const char* str);
void add_task();
void show_tasks();
void complete_task();
void remove_task();
void save_tasks();
void load_tasks();
void show_menu();

int main() {
    int choice;

    // Load existing tasks at startup
    load_tasks();

    // The main cycle
    do {
        show_menu();
        scanf("%d", &choice);
        getchar();  // Always clear input buffer after scanf

        switch(choice) {
            case 1: add_task(); break;
            case 2: show_tasks(); break;
            case 3: complete_task(); break;
            case 4: remove_task(); break;
            case 5: save_tasks(); break;
            case 6: load_tasks(); break;
            case 0:
                save_tasks();  // Auto-save on exit
                printf("\nGoodbye! Your tasks are saved.\n");
                break;
            default:
                printf("\nInvalid option! Try again.\n");
        }
    } while (choice != 0);

    return 0;
}

// Helper: Remove newline characters from input
void trim_newline(char* str) {
    int len = strlen(str);
    if (len > 0 && str[len-1] == '\n') {
        str[len-1] = '\0';
    }
}

// Check if string is empty or whitespace
int is_empty(const char* str) {
    while (*str) {
        if (!isspace((unsigned char)*str)) {
            return 0;
        }
        str++;
    }
    return 1;
}

// Adding a new task to the list
void add_task() {
    // Check if we have an empty seat
    if (task_count >= MAX_TASKS) {
        printf("\nTask list is full! Can't add more.\n");
        return;
    }

    // Getting task description
    printf("\nEnter task description: ");
    fgets(tasks[task_count].text, TASK_LEN, stdin);

    // Remove newline and check if input is empty
    trim_newline(tasks[task_count].text);

    // Validate input - check for empty or whitespace-only tasks
    if (is_empty(tasks[task_count].text)) {
        printf("Error: Task description cannot be empty!\n");
        return;
    }

    // Initialize as not done
    tasks[task_count].done = 0;
    task_count++;

    printf("Task added successfully!\n");
}

// Displaying all tasks with their status
void show_tasks() {
    if (task_count == 0) {
        printf("\nNo tasks found. Your list is empty!\n");
        return;
    }

    printf("\n------ YOUR TASKS ------\n");
    for (int i = 0; i < task_count; i++) {
        printf("%3d. [%c] %s\n",
            i + 1,
            tasks[i].done ? 'X' : ' ',  // Show X for completed
            tasks[i].text);
    }
    printf("-----------------------\n");
}

// Mark the task as completed
void complete_task() {
    show_tasks();
    if (task_count == 0) return;

    int num;
    printf("\nEnter task number to complete: ");
    scanf("%d", &num);
    getchar();  // Consume leftover newline

    // Validate input
    if (num < 1 || num > task_count) {
        printf("Invalid task number!\n");
        return;
    }

    // Update status (array is 0)
    tasks[num - 1].done = 1;
    printf("Task marked as completed!\n");
}

// Remove a task from the list
void remove_task() {
    show_tasks();
    if (task_count == 0) return;

    int num;
    printf("\nEnter task number to remove: ");
    scanf("%d", &num);
    getchar();  // Consume leftover newline

    if (num < 1 || num > task_count) {
        printf("Invalid task number!\n");
        return;
    }

    // Shift tasks down to fill the gap
    for (int i = num - 1; i < task_count - 1; i++) {
        tasks[i] = tasks[i + 1];
    }

    task_count--;
    printf("Task removed successfully!\n");
}

// Save tasks to file
void save_tasks() {
    FILE *file = fopen(FILENAME, "w");
    if (!file) {
        printf("\nERROR: Couldn't open file for writing!\n");
        return;
    }

    // Format: status|description
    for (int i = 0; i < task_count; i++) {
        fprintf(file, "%d|%s\n", tasks[i].done, tasks[i].text);
    }

    fclose(file);
    printf("\nTasks saved to disk!\n");
}

// Load tasks from file
void load_tasks() {
    FILE *file = fopen(FILENAME, "r");
    if (!file) {
        printf("\nNo saved tasks found. Starting fresh.\n");
        return;
    }

    task_count = 0;  // Reset before loading
    char line[TASK_LEN + 10];

    // Read until we hit MAX_TASKS or end of file
    while (fgets(line, sizeof(line), file) && task_count < MAX_TASKS) {
        char *status_str = strtok(line, "|");
        char *desc = strtok(NULL, "\n");

        if (desc) {
            tasks[task_count].done = atoi(status_str);
            strncpy(tasks[task_count].text, desc, TASK_LEN);
            task_count++;
        }
    }

    fclose(file);
    printf("\nTasks loaded from disk!\n");
}

// Display main menu
void show_menu() {
    printf("\n==== TODO MANAGER ====\n");
    printf("1. Add New Task\n");
    printf("2. View All Tasks\n");
    printf("3. Complete Task\n");
    printf("4. Remove Task\n");
    printf("5. Save to File\n");
    printf("6. Load from File\n");
    printf("0. Exit\n");
    printf("======================\n");
    printf("Choose an option: ");
}