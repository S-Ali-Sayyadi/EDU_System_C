#include <stdio.h>

static int read_int(const char *prompt);
static void show_main_menu(void);

int main(void)
{
    int option;

    while (1)
    {
        show_main_menu();
        option = read_int("Enter an option: ");

        if (option==1)
        {
            printf("Student login will be added later.\n");
        }
        else if (option==2)
        {
            printf("Faculty login will be added later.\n");
        }
        else if (option==3)
        {
            printf("Admin login will be added later.\n");
        }
        else if (option==4)
        {
            printf("Forgot password will be added later.\n");
        }
        else if (option==5)
        {
            printf("Goodbye.\n");
            break;
        }
        else
        {
            printf("Invalid option. Please try again.\n");
        }
    }
    return 0;
}

static void show_main_menu(void)
{
    printf("\n");
    printf("----------------------------------------\n");
    printf("Educational Management System\n");
    printf("----------------------------------------\n");
    printf("1. Login as student\n");
    printf("2. Login as faculty\n");
    printf("3. Login as admin\n");
    printf("4. Forgot password\n");
    printf("5. Exit\n");
}

static int read_int(const char *prompt)
{
    char input[100];
    int value;

    while (1)
    {
        printf("%s", prompt);

        if (fgets(input, sizeof(input), stdin)==NULL)
        {
            return 5;
        }

        if (sscanf(input, "%d", &value)==1)
        {
            return value;
        }

        printf("Please enter a valid number.\n");
    }
}