#include <stdio.h>

#define MAX_STUDENTS 1000
#define MAX_FACULTY 500
#define MAX_COURSES 500
#define MAX_OFFERINGS 1000
#define MAX_REQUESTS 1000
#define MAX_ENROLLED 400
#define STR_SIZE 128
#define SMALL_SIZE 64

typedef struct
{
    char student_id[SMALL_SIZE];
    double grade;
    int survey_score;
} Enrollment;

typedef struct
{
    char first_name[STR_SIZE];
    char last_name[STR_SIZE];
    char student_id[SMALL_SIZE];
    char national_code[SMALL_SIZE];
    char field[STR_SIZE];
    int entrance_year;
    char section[SMALL_SIZE];
    char mentor[STR_SIZE];
    char department[STR_SIZE];
    char answer_birth[STR_SIZE];
    char answer_book[STR_SIZE];
    char answer_bike[STR_SIZE];
    char password[STR_SIZE];
} Student;

typedef struct
{
    char first_name[STR_SIZE];
    char last_name[STR_SIZE];
    char faculty_id[SMALL_SIZE];
    char national_code[SMALL_SIZE];
    char field[STR_SIZE];
    int entrance_year;
    char degree[STR_SIZE];
    char department[STR_SIZE];
    char password[STR_SIZE];
    char answer_birth[STR_SIZE];
    char answer_book[STR_SIZE];
    char answer_bike[STR_SIZE];
} Faculty;

typedef struct
{
    char name[STR_SIZE];
    char course_id[SMALL_SIZE];
    int units;
    char prerequisites[STR_SIZE];
    char section[SMALL_SIZE];
    char field[STR_SIZE];
    char department[STR_SIZE];
} Course;

typedef struct
{
    char course_id[SMALL_SIZE];
    char faculty_id[SMALL_SIZE];
    int semester;
    int capacity;
    int enrolled_count;
    char department[STR_SIZE];
    char place[STR_SIZE];
    Enrollment enrollments[MAX_ENROLLED];
} Offering;

typedef struct
{
    int id;
    char type[SMALL_SIZE];
    char course_id[SMALL_SIZE];
    char faculty_id[SMALL_SIZE];
    int semester;
    int capacity;
    int amount;
    int offering_index;
    char department[STR_SIZE];
    char place[STR_SIZE];
    char status[SMALL_SIZE];
} Request;

typedef struct
{
    int offering;
    int unit_selection;
    int classes_exams;
    int grade_recording;
    int course_survey;
} CalendarState;

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