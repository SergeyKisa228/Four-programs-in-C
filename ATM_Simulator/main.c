/*ATM machine*/

/*Include*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

/*Define*/
#define MAX_PASS 5 // + '\0'
#define MAX_NAME 50
#define USER_DB "user_pass.txt"

/*Struct*/

/*PROTOTYPE FUNCTION*/
int password_register();
int password_login();
void menu();
void check_balance();
void up_balance();
void take_off_money();
void history();
void space(char *str);
void clear_screen();
unsigned long hash(const unsigned char *str); //djb2

char current_user[MAX_NAME] = ""; // Current user

int main()
{
    int choice;
    do {
        printf("\nATM System\n");
        printf("1. Register\n2. Login\n3. Exit\n");
        printf("Your choice: ");
        scanf("%d", &choice);
        while(getchar() != '\n');

        switch (choice) {
            case 1:
                password_register();
                break;
            case 2: {
                int attempts = 3;
                while (attempts) {
                    int result = password_login();
                    if (result == 1) {
                        menu();
                        break;
                    } else if (result == -1) {
                        break;
                    } else {
                        attempts--;
                        if (attempts > 0) {
                            printf("Attempts left: %d\n\n", attempts);
                        }
                    }
                }
                if (attempts == 0) {
                    printf("Too many failed attempts. Exiting...\n\n");
                    return 0;
                }
                break;
            }
            case 3:
                return 0;
            default:
                printf("Invalid choice!\n\n");
        }
    } while(1);
}

void space(char *str)
{
    if (str == NULL) return;

    // Delete space end
    char *end = str + strlen(str) - 1;
    while (end >= str && isspace((unsigned char)*end)) {
        end--;
    }
    *(end + 1) = '\0';

    // Delete space start
    char *start = str;
    while (isspace((unsigned char)*start)) {
        start++;
    }
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}
int password_register()
{
    char user_name[MAX_NAME];
    char pass[20];
    FILE *file;

    printf("Input username (or 'cancel' to return): ");
    fgets(user_name, sizeof(user_name), stdin);
    user_name[strcspn(user_name, "\n")] = '\0';
    space(user_name);

    if(strlen(user_name) == 0 || strlen(user_name) > MAX_NAME - 1) {
        printf("Invalid username length!\n");
        return 0;
    }

    if (strcmp(user_name, "cancel") == 0) {
        printf("Registration cancelled.\n\n");
        return 0;
    }

    printf("Input password (4 symbols or 'cancel' to return): ");
    fgets(pass, sizeof(pass), stdin);
    pass[strcspn(pass, "\n")] = '\0';

    char pass_temp[20];
    strcpy(pass_temp, pass);
    space(pass_temp);
    if (strcmp(pass_temp, "cancel") == 0) {
        printf("Registration cancelled.\n\n");
        return 0;
    }

    if (strlen(pass) != 4)
    {
        clear_screen();
        printf("Password must be exactly 4 characters\n\n");
        return 0;
    }

    // Hash pass
    unsigned long pass_hash = hash((const unsigned char*)pass);

    // Checking user
    file = fopen(USER_DB, "r");
    if (file) {
        char line[100];
        while (fgets(line, sizeof(line), file)) {
            line[strcspn(line, "\n")] = '\0';
            char *colon = strchr(line, ':');
            if (colon) {
                *colon = '\0';
                char file_name_trimmed[MAX_NAME];
                strcpy(file_name_trimmed, line);
                space(file_name_trimmed);

                if(strcmp(user_name, file_name_trimmed) == 0) {
                    printf("User already exists!\n\n");
                    fclose(file);
                    return 0;
                }
            }
        }
        fclose(file);
    }

    file = fopen(USER_DB, "a");
    if (file == NULL)
    {
        printf("Error: Cannot open user database.\n\n");
        return 0;
    }

    // Saving Hash
    fprintf(file, "%s:%lu\n", user_name, pass_hash);
    fclose(file);

    printf("Registration successful!\n\n");
    return 1;
}

int password_login()
{
    char input_name[MAX_NAME];
    char input_pass[20];
    FILE *file;
    int authenticated = 0;

    printf("Username (or 'cancel' to return): ");
    fgets(input_name, sizeof(input_name), stdin);
    input_name[strcspn(input_name, "\n")] = '\0';;
    space(input_name);

    if (strcmp(input_name, "cancel") == 0) {
        printf("Login cancelled.\n\n");
        return -1;
    }

    printf("Password (4 symbols or 'cancel' to return): ");
    fgets(input_pass, sizeof(input_pass), stdin);
    input_pass[strcspn(input_pass, "\n")] = '\0';

    char pass_temp[20];
    strcpy(pass_temp, input_pass);
    space(pass_temp);
    if (strcmp(pass_temp, "cancel") == 0) {
        printf("Login cancelled.\n\n");
        return -1;
    }

    // Checking length password
    if(strlen(input_pass) != 4) {
        printf("Error: Password must be exactly 4 characters!\n\n");
        return 0;
    }

    // Hash pass
    unsigned long input_hash = hash((const unsigned char*)input_pass);

    file = fopen(USER_DB, "r");
    if (file == NULL)
    {
        printf("Error: User database not found.\n\n");
        return 0;
    }

    char line[100];
    while(fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0';

        char *colon = strchr(line, ':');
        if(!colon) continue;

        *colon = '\0';
        char file_name_trimmed[MAX_NAME];
        strcpy(file_name_trimmed, line);
        space(file_name_trimmed);

        // Read Hash
        unsigned long file_hash = strtoul(colon + 1, NULL, 10);

        if(strcmp(input_name, file_name_trimmed) == 0 &&
           input_hash == file_hash) {
            authenticated = 1;
            strcpy(current_user, input_name);
            break;
        }
    }

    fclose(file);

    if(authenticated) {
        printf("Login successful!\n\n");
        clear_screen();
        return 1;
    }
    else {
        printf("Error: Invalid username or password.\n\n");
        return 0;
    }
}

void menu()
{
    int your_choice;
    do {
        printf("\nWelcome, %s!\n", current_user);
        printf("1. Check the balance\n");
        printf("2. Top up your account\n");
        printf("3. Withdraw cash\n");
        printf("4. Operation history\n");
        printf("5. Exit\n\n");
        printf("Input operation: ");
        scanf("%d", &your_choice);
        while(getchar() != '\n');

        switch(your_choice)
        {
            case 1: check_balance(); break;
            case 2: up_balance(); break;
            case 3: take_off_money(); break;
            case 4: history(); break;
            case 5: clear_screen(); return;  // Come back in menu
            default: printf("Error: invalid operation!\n\n");
        }
    } while (1);
}

void check_balance()
{
    char filename[100];
    sprintf(filename, "%s_balance.txt", current_user);

    FILE *file = fopen(filename, "r");
    if(!file)
    {
        printf("Balance: 0.00 Rub\n\n");
        return;
    }

    double balance;
    if(fscanf(file, "%lf", &balance) != 1) {
        balance = 0.0;
    }
    fclose(file);

    printf("Your current balance: %.2lf Rub.\n\n", balance);
}

void up_balance()
{
    double money;
    printf("Enter amount to deposit: ");
    scanf("%lf", &money);
    while(getchar() != '\n');

    if(money <= 0)
    {
        printf("Error: Invalid amount\n\n");
        return;
    }

    char bal_file[100];
    sprintf(bal_file, "%s_balance.txt", current_user);

    char hist_file[100];
    sprintf(hist_file, "%s_history.txt", current_user);

    // Current balance
    double balance = 0.0;
    FILE *file = fopen(bal_file, "r");
    if(file) {
        fscanf(file, "%lf", &balance);
        fclose(file);
    }

    // Update Balance
    balance += money;
    file = fopen(bal_file, "w");
    fprintf(file, "%.2lf", balance);
    fclose(file);

    // Record in history
    file = fopen(hist_file, "a");
    if(file) {
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        fprintf(file, "%04d-%02d-%02d Deposit: +%.2lf\n",
                t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, money);
        fclose(file);
    }

    printf("Successfully deposited %.2lf Rub.\n\n", money);
}

void take_off_money()
{
    double money;
    printf("Enter amount to withdraw: ");
    scanf("%lf", &money);
    while(getchar() != '\n');

    if(money <= 0)
    {
        printf("Error: Amount must be positive!\n\n");
        return;
    }

    char bal_file[100];
    sprintf(bal_file, "%s_balance.txt", current_user);

    char hist_file[100];
    sprintf(hist_file, "%s_history.txt", current_user);

    // Current balance
    double balance = 0.0;
    FILE *file = fopen(bal_file, "r");
    if(file) {
        fscanf(file, "%lf", &balance);
        fclose(file);
    }

    if(money > balance)
    {
        printf("Error: Insufficient funds\n\n");
        return;
    }

    // Update Balance
    balance -= money;
    file = fopen(bal_file, "w");
    fprintf(file, "%.2lf", balance);
    fclose(file);

    // Record for history
    file = fopen(hist_file, "a");
    if(file) {
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        fprintf(file, "%04d-%02d-%02d Withdrawal: -%.2lf\n",
                t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, money);
        fclose(file);
    }

    printf("Successfully withdrawn %.2lf Rub.\n\n", money);
}

void history()
{
    char filename[100];
    sprintf(filename, "%s_history.txt", current_user);

    FILE *file = fopen(filename, "r");
    if(!file)
    {
        printf("No operations found\n\n");
        return;
    }

    printf("\nOperation History:\n");
    char line[100];
    while(fgets(line, sizeof(line), file)) {
        printf("%s", line);
    }
    fclose(file);
    printf("\n");
}

void clear_screen()
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

unsigned long hash(const unsigned char *str)
{
    unsigned long hash = 5381;
    int c;
    while((c = *str++))
    {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}
