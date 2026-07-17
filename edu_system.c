#include <stdio.h>
#include <string.h>
#include <ctype.h>

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

static Student students[MAX_STUDENTS];
static Faculty faculty_members[MAX_FACULTY];
static Course courses[MAX_COURSES];

static int student_count=0;
static int faculty_count=0;
static int course_count=0;

static void copy_str(char *destination, const char *source, size_t size);

static int find_student_index(const char *student_id);
static int find_faculty_index(const char *faculty_id);
static int find_course_index(const char *course_id);

static void initialize_sample_data(void);
static void show_data_summary(void);

static void trim(char *text);
static void read_line(const char *prompt, char *output, size_t size);
static int read_int(const char *prompt);
static void show_main_menu(void);

static void copy_str(char *destination,const char *source,size_t size)
{
    size_t index=0;

    if (size==0)
    {
        return;
    }

    if (source==NULL)
    {
        source="";
    }

    while (index+1<size && source[index]!='\0')
    {
        destination[index]=source[index];
        index++;
    }
    destination[index]='\0';
}

static int find_student_index(const char *student_id)
{
    int index;

    for (index=0; index<student_count; index++)
    {
        if (strcmp(students[index].student_id, student_id)==0)
        {
            return index;
        }
    }
    return -1;
}

static int find_faculty_index(const char *faculty_id)
{
    int index;

    for (index=0; index<faculty_count; index++)
    {
        if (strcmp(faculty_members[index].faculty_id,faculty_id)==0)
        {
            return index;
        }
    }
    return -1;
}

static int find_course_index(const char *course_id)
{
    int index;

    for (index=0; index<course_count; index++)
    {
        if (strcmp(courses[index].course_id, course_id)==0)
        {
            return index;
        }
    }
    return -1;
}

static void initialize_sample_data(void)
{
    Student *student;
    Faculty *faculty;
    Course *course;

    if (student_count < MAX_STUDENTS && find_student_index("404123456")==-1)
    {
        student = &students[student_count];

        memset(student, 0, sizeof(*student));

        copy_str(student->first_name,"Ali",sizeof(student->first_name));

        copy_str(student->last_name,"Ahmadi",sizeof(student->last_name));

        copy_str(student->student_id,"404123456",sizeof(student->student_id));

        copy_str(student->field,"Computer Engineering",sizeof(student->field));

        copy_str(student->section,"BSc",sizeof(student->section));

        copy_str(student->password,"123456",sizeof(student->password));

        student->entrance_year=1404;
        student_count++;
    }

    if (faculty_count < MAX_FACULTY && find_faculty_index("FCS105")==-1)
    {
        faculty=&faculty_members[faculty_count];

        memset(faculty, 0, sizeof(*faculty));

        copy_str(faculty->first_name,"Hossein",sizeof(faculty->first_name));

        copy_str(faculty->last_name,"Asadi",sizeof(faculty->last_name));

        copy_str(faculty->faculty_id,"FCS105",sizeof(faculty->faculty_id));

        copy_str(faculty->field,"Computer Engineering",sizeof(faculty->field));

        copy_str(faculty->degree,"PhD",sizeof(faculty->degree));

        copy_str(faculty->password,"123456",sizeof(faculty->password));

        faculty_count++;
    }

    if (course_count < MAX_COURSES && find_course_index("CS101")==-1)
    {
        course=&courses[course_count];

        memset(course, 0, sizeof(*course));

        copy_str(course->name,"Fundamentals of Programming",sizeof(course->name));

        copy_str(course->course_id,"CS101",sizeof(course->course_id));

        copy_str(course->prerequisites,"-",sizeof(course->prerequisites));

        copy_str(course->section,"BSc",sizeof(course->section));

        copy_str(course->field,"Computer Engineering",sizeof(course->field));

        course->units=3;
        course_count++;
    }
}

static void show_data_summary(void)
{
    printf("\nSample data loaded successfully.\n");

    printf(
        "Students: %d | Faculty: %d | Courses: %d\n",
        student_count,
        faculty_count,
        course_count
    );
}

int main(void)
{
    int option;

        initialize_sample_data();
    show_data_summary();

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

static void trim(char *text)
{
    size_t start=0;
    size_t length;

    if (text==NULL)
    {
        return;
    }

    while (text[start]!='\0' && isspace((unsigned char)text[start]))
    {
        start++;
    }

       if (start>0)
    {
        memmove(text, text+start, strlen(text+start)+1);
    }

    length=strlen(text);

    while (length>0 && isspace((unsigned char)text[length-1]))
    {
        text[length-1]='\0';
        length--;
    }
}

static void read_line(const char *prompt,char *output,size_t size)
{
    printf("%s", prompt);

    if (fgets(output, (int)size, stdin)==NULL)
    {
        output[0]='\0';
        clearerr(stdin);
        return;
}
    output[strcspn(output, "\n")]='\0';
    trim(output);
}

static int read_int(const char *prompt)
{
    char input[100];
    char extra_character;
    int value;

    while (1)
    {
        read_line(prompt, input, sizeof(input));

               if (sscanf(input, "%d %c", &value, &extra_character)==1)
        {
            return value;
        }

        printf("Please enter a valid integer.\n");
    }
}
