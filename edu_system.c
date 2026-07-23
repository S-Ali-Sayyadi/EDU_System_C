#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_STUDENTS 1000
#define MAX_FACULTY 500
#define MAX_COURSES 500
#define MAX_OFFERINGS 1000
#define MAX_REQUESTS 1000
#define MAX_ENROLLED 400
#define STR_SIZE 128
#define SMALL_SIZE 64
#define ADMIN_USERNAME "admin"
#define ADMIN_PASSWORD "admin123"
#define PASSING_GRADE 10.0
#define LINE_SIZE 8192
#define DATA_FILE "edu_data.json"
#define DATA_TEMP_FILE "edu_data.json.tmp"
#define COURSE_AVAILABILITY_NEXT_SEMESTER -1
#define COURSE_AVAILABILITY_UNRESTRICTED 0

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
    char answer_school[STR_SIZE];
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
    char answer_school[STR_SIZE];
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
    int available_from_semester;
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

typedef enum
{
    PHASE_NOT_STARTED=0,
    PHASE_ACTIVE=1,
    PHASE_FINISHED=2
} PhaseState;

typedef struct
{
    int current_semester;
    PhaseState offering;
    PhaseState unit_selection;
    PhaseState classes_exams;
    PhaseState grade_recording;
} CalendarState;

static void json_write_string(FILE *file,const char *text);
static void json_write_key_string(FILE *file,const char *key,const char *value);
static void begin_json_record(FILE *file,int *first_record);
static const char *skip_json_spaces(const char *text);
static int next_json_string(const char **cursor,char *output,size_t size);
static int next_json_int(const char **cursor,int *output);
static int next_json_double(const char **cursor,double *output);
static int save_all(void);
static int load_all(void);

static Student students[MAX_STUDENTS];
static Faculty faculty_members[MAX_FACULTY];
static Course courses[MAX_COURSES];
static Offering offerings[MAX_OFFERINGS];
static Request requests[MAX_REQUESTS];

static CalendarState calendar_state={
    14042,
    PHASE_NOT_STARTED,
    PHASE_NOT_STARTED,
    PHASE_NOT_STARTED,
    PHASE_NOT_STARTED
};

static int student_count = 0;
static int faculty_count = 0;
static int course_count = 0;
static int offering_count = 0;
static int request_count = 0;
static int next_request_id = 1;

static int strings_equal_ignore_case(
    const char *first,
    const char *second
);

static int contains_ignore_case(
    const char *text,
    const char *key
);

static int is_digits_only(const char *text);
static int is_valid_section(const char *section);
static int national_code_exists(const char *national_code);

static int prerequisites_exist(
    const char *prerequisites,
    const char *course_id
);

static int student_record_is_valid(
    const Student *student
);

static int faculty_record_is_valid(
    const Faculty *faculty
);

static int verify_security_answers(
    const char *expected_birth,
    const char *expected_school,
    const char *expected_book,
    const char *expected_bike
);

static int ask_retry_or_back(const char *back_label);

static int read_new_password(
    char *output,
    size_t size
);

static void recover_student_password(void);
static void recover_faculty_password(void);
static void forgot_password_menu(void);

static void copy_str(char *destination, const char *source, size_t size);

static int find_student_index(const char *student_id);
static int find_faculty_index(const char *faculty_id);
static int find_course_index(const char *course_id);
static int find_request_index(int request_id);

static int collect_offering_indices(
    int semester,
    const char *faculty_id,
    int output[],
    int max_output
);

static void sort_offering_indices_by_semester(
    int indices[],
    int count
);

static void print_offering(const Offering *offering, int number);
static void list_offerings_by_semester(int semester);
static void list_faculty_offerings(int faculty_index);

static void search_students(void);
static void search_faculty(void);
static void search_courses(void);
static void search_offerings(
    int semester_filter,
    const char *faculty_filter
);

static void course_catalog_menu(void);

static void add_student_to_offering_admin(int offering_index);
static void remove_student_from_offering_admin(int offering_index);
static void add_capacity_admin_direct(int offering_index);
static void admin_offerings_menu(void);

static void faculty_offer_course_request(int faculty_index);

static void faculty_request_capacity(
    int faculty_index,
    int offering_index
);

static void faculty_request_removal(
    int faculty_index,
    int offering_index
);

static void faculty_view_surveys(
    int faculty_index,
    int offering_index
);

static void faculty_record_grade_for_offering(
    int faculty_index,
    int offering_index
);

static void record_grades_from_file(
    int offering_index
);

static void faculty_record_grades(
    int faculty_index,
    int offering_index
);

static void faculty_manage_offering(int faculty_index);

static void list_requests(void);
static void approve_request(void);
static void reject_request(void);
static void admin_requests_menu(void);

static const char *phase_status(PhaseState state);

static int transition_calendar_phase(
    PhaseState *phase,
    int can_start,
    int can_finish,
    const char *phase_name,
    const char *start_error,
    const char *finish_error
);

static void start_next_semester(void);
static void admin_calendar_menu(void);

static void student_course_survey(int student_index);

static int course_is_available_for_semester(
    const Course *course,
    int semester
);
static int course_is_in_use(const char *course_id);
static int faculty_is_in_use(const char *faculty_id);

static void add_student_seed(
    const char *first_name,
    const char *last_name,
    const char *student_id,
    const char *national_code,
    const char *field,
    int entrance_year,
    const char *section,
    const char *mentor,
    const char *department,
    const char *answer_birth,
    const char *answer_school,
    const char *answer_book,
    const char *answer_bike,
    const char *password
);

static void add_faculty_seed(
    const char *first_name,
    const char *last_name,
    const char *faculty_id,
    const char *national_code,
    const char *field,
    int entrance_year,
    const char *degree,
    const char *department,
    const char *password
);

static void add_course_seed(
    const char *name,
    const char *course_id,
    int units,
    const char *prerequisites,
    const char *section,
    const char *field,
    const char *department
);

static void add_offering_seed(
    const char *course_id,
    const char *faculty_id,
    int semester,
    int capacity,
    const char *department,
    const char *place
);

static void enroll_seed(
    int offering_index,
    const char *student_id,
    double grade
);

static void initialize_sample_data(void);
static void show_data_summary(void);

static void list_students(void);
static void register_student(void);
static void import_students_file(void);
static void delete_student(void);
static void admin_students_menu(void);

static void list_faculty(void);
static void register_faculty(void);
static void import_faculty_file(void);
static void delete_faculty(void);
static void admin_faculty_menu(void);

static void list_courses(void);
static void register_course(void);
static void delete_course(void);
static void admin_courses_menu(void);

static int offering_has_student(
    int offering_index,
    const char *student_id
);

static int student_is_enrolled(const char *student_id);

static int is_student_passed_course(
    const char *student_id,
    const char *course_id
);

static int prerequisites_satisfied(
    const Student *student,
    const Course *course
);

static void student_enroll_course(
    int student_index,
    int selected_semester,
    int offering_index
);

static void student_withdraw_course(
    int student_index,
    int selected_semester,
    int offering_index
);
static void student_offerings_menu(int student_index);

static void login_student(void);
static void login_faculty(void);
static void login_admin(void);

static void student_dashboard(int student_index);
static void faculty_dashboard(int faculty_index);
static void admin_dashboard(void);

static void trim(char *text);

static int parse_csv_line(
    char *line,
    char *fields[],
    int max_fields
);

static void read_line(
    const char *prompt,
    char *output,
    size_t size
);
static int read_int(const char *prompt);
static double read_double(const char *prompt)
{
    char input[100];
    char extra_character;
    double value;

    while (1)
    {
        read_line(prompt, input, sizeof(input));

        if (sscanf(
                input,
                "%lf %c",
                &value,
                &extra_character
            )==1)
        {
            return value;
        }

        printf("Please enter a valid number.\n");
    }
}

static void show_main_menu(void);

static void list_offering_students(int offering_index);

static void calculate_student_gpa(
    const char *student_id,
    int semester_filter,
    int *enrolled_count_output,
    int *passed_count_output,
    int *failed_count_output,
    double *gpa_output
);

static void show_semester_report(
    int student_index,
    int semester
);

static void search_passed_courses(int student_index);
static void student_report_card(int student_index);

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

static int strings_equal_ignore_case(
    const char *first,
    const char *second
)
{
    size_t index=0;

    if (first==NULL || second==NULL)
    {
        return 0;
    }

    while (first[index]!='\0' &&
           second[index]!='\0')
    {
        if (tolower((unsigned char)first[index])!=tolower((unsigned char)second[index]))
        {
            return 0;
        }
        index++;
    }
    return first[index]=='\0' && second[index]=='\0';
}

static int contains_ignore_case(
    const char *text,
    const char *key
)
{
    size_t text_length;
    size_t key_length;
    size_t start;
    size_t index;

    if (text==NULL || key==NULL)
    {
        return 0;
    }

    text_length=strlen(text);
    key_length=strlen(key);

    if (key_length==0)
    {
        return 1;
    }

    if (key_length>text_length)
    {
        return 0;
    }

    for (start=0; start<=text_length-key_length; start++)
    {
        for (index=0; index<key_length; index++)
        {
            if (
                tolower((unsigned char)text[start+index]) !=
                tolower((unsigned char)key[index]))
            {
                break;
            }
        }

        if (index==key_length)
        {
            return 1;
        }
    }
    return 0;
}

static int is_digits_only(const char *text)
{
    size_t index;

    if (text==NULL || text[0]=='\0')
    {
        return 0;
    }

    for (index=0; text[index]!='\0'; index++)
    {
        if (!isdigit((unsigned char)text[index]))
        {
            return 0;
        }
    }

    return 1;
}

static int is_valid_section(const char *section)
{
    return strcmp(section,"BSc")==0 ||
        strcmp(section,"MSc")==0 ||
        strcmp(section,"PhD")==0;
}

static int national_code_exists(const char *national_code)
{
    int index;

    for (index=0; index<student_count; index++)
    {
        if (strcmp(
                students[index].national_code,
                national_code
            )==0)
        {
            return 1;
        }
    }

    for (index=0; index<faculty_count; index++)
    {
        if (strcmp(
                faculty_members[index].national_code,
                national_code
            )==0)
        {
            return 1;
        }
    }

    return 0;
}

static int prerequisites_exist(
    const char *prerequisites,
    const char *course_id
)
{
    char copy[STR_SIZE];
    char *prerequisite;

    if (prerequisites==NULL ||
        prerequisites[0]=='\0' ||
        strcmp(prerequisites,"-")==0)
    {
        return 1;
    }

    copy_str(
        copy,
        prerequisites,
        sizeof(copy)
    );

    prerequisite=strtok(copy,",");

    while (prerequisite!=NULL)
    {
        trim(prerequisite);

        if (prerequisite[0]=='\0' ||
            strcmp(prerequisite,course_id)==0 ||
            find_course_index(prerequisite)==-1)
        {
            return 0;
        }

        prerequisite=strtok(NULL,",");
    }

    return 1;
}

static int student_record_is_valid(
    const Student *student
)
{
    if (student==NULL)
    {
        return 0;
    }

    if (student->first_name[0]=='\0' ||
        student->last_name[0]=='\0' ||
        student->student_id[0]=='\0' ||
        student->national_code[0]=='\0' ||
        student->field[0]=='\0' ||
        student->section[0]=='\0' ||
        student->mentor[0]=='\0' ||
        student->department[0]=='\0' ||
        student->answer_birth[0]=='\0' ||
        student->answer_school[0]=='\0' ||
        student->answer_book[0]=='\0' ||
        student->answer_bike[0]=='\0' ||
        student->password[0]=='\0')
    {
        return 0;
    }

    if (!is_digits_only(student->student_id) ||
        !is_digits_only(student->national_code) ||
        strlen(student->national_code)!=10 ||
        student->entrance_year<=0 ||
        !is_valid_section(student->section))
    {
        return 0;
    }

    return 1;
}

static int faculty_record_is_valid(
    const Faculty *faculty
)
{
    if (faculty==NULL)
    {
        return 0;
    }

    if (faculty->first_name[0]=='\0' ||
        faculty->last_name[0]=='\0' ||
        faculty->faculty_id[0]=='\0' ||
        faculty->national_code[0]=='\0' ||
        faculty->field[0]=='\0' ||
        faculty->degree[0]=='\0' ||
        faculty->department[0]=='\0' ||
        faculty->password[0]=='\0' ||
        faculty->answer_birth[0]=='\0' ||
        faculty->answer_school[0]=='\0' ||
        faculty->answer_book[0]=='\0' ||
        faculty->answer_bike[0]=='\0')
    {
        return 0;
    }

    if (!is_digits_only(faculty->national_code) ||
        strlen(faculty->national_code)!=10 ||
        faculty->entrance_year<=0)
    {
        return 0;
    }

    return 1;
}


static int verify_security_answers(
    const char *expected_birth,
    const char *expected_school,
    const char *expected_book,
    const char *expected_bike
)
{
    char birth[STR_SIZE];
    char school[STR_SIZE];
    char book[STR_SIZE];
    char bike[STR_SIZE];

    if (expected_birth[0]=='\0' ||
        expected_school[0]=='\0' ||
        expected_book[0]=='\0' ||
        expected_bike[0]=='\0')
    {
        printf(
            "Security answers have not been configured "
            "for this account.\n"
        );

        return 0;
    }

    read_line(
        "Where were you born? ",
        birth,
        sizeof(birth)
    );

    read_line(
        "What was the name of your first school? ",
        school,
        sizeof(school)
    );

    read_line(
        "What was the title of the first book you read? ",
        book,
        sizeof(book)
    );

    read_line(
        "What was the color of your first bicycle? ",
        bike,
        sizeof(bike)
    );

    if (!strings_equal_ignore_case(
            birth,
            expected_birth
        ) ||
        !strings_equal_ignore_case(
            school,
            expected_school
        ) ||
        !strings_equal_ignore_case(
            book,
            expected_book
        ) ||
        !strings_equal_ignore_case(
            bike,
            expected_bike
        ))
    {
        printf(
            "One or more security answers are incorrect.\n"
        );

        return 0;
    }

    return 1;
}

static int ask_retry_or_back(const char *back_label)
{
    int option;

    while (1)
    {
        printf("1. Retry\n");
        printf("2. %s\n", back_label);

        option=read_int("Enter an option: ");

        if (option==1)
        {
            return 1;
        }

        if (option==2)
        {
            return 0;
        }

        printf(
            "Invalid option. Please try again.\n"
        );
    }
}

static int read_new_password(
    char *output,
    size_t size
)
{
    char new_password[STR_SIZE];
    char confirmation[STR_SIZE];

    while (1)
    {
        read_line(
            "Enter your new password: ",
            new_password,
            sizeof(new_password)
        );

        if (new_password[0]=='\0')
        {
            printf("Password cannot be empty.\n");

            if (!ask_retry_or_back("Cancel"))
            {
                return 0;
            }

            continue;
        }

        read_line(
            "Confirm your password: ",
            confirmation,
            sizeof(confirmation)
        );

        if (strcmp(
                new_password,
                confirmation
            )!=0)
        {
            printf("Passwords do not match.\n");

            if (!ask_retry_or_back("Cancel"))
            {
                return 0;
            }

            continue;
        }

        copy_str(
            output,
            new_password,
            size
        );

        return 1;
    }
}

static void recover_student_password(void)
{
    Student *student;
    char student_id[SMALL_SIZE];
    char new_password[STR_SIZE];
    int student_index;

    printf("\n");
    printf("----------------------------------------\n");
    printf("Student Password Recovery\n");
    printf("----------------------------------------\n");

    while (1)
    {
        read_line(
            "Enter your username: ",
            student_id,
            sizeof(student_id)
        );

        student_index=
            find_student_index(student_id);

        if (student_index==-1)
        {
            printf("Username not found.\n");

            if (!ask_retry_or_back(
                    "Go back"
                ))
            {
                return;
            }

            continue;
        }

        student=&students[student_index];
        break;
    }

    while (!verify_security_answers(
        student->answer_birth,
        student->answer_school,
        student->answer_book,
        student->answer_bike
    ))
    {
        if (!ask_retry_or_back(
                "Go back"
            ))
        {
            return;
        }
    }

    printf("Authentication successful.\n");

    if (!read_new_password(
        new_password,
        sizeof(new_password)
    ))
    {
        return;
    }

    copy_str(
        student->password,
        new_password,
        sizeof(student->password)
    );

    save_all();

    printf("Password changed successfully.\n");
}

static void recover_faculty_password(void)
{
    Faculty *faculty;
    char faculty_id[SMALL_SIZE];
    char new_password[STR_SIZE];
    int faculty_index;

    printf("\n");
    printf("----------------------------------------\n");
    printf("Faculty Password Recovery\n");
    printf("----------------------------------------\n");

    while (1)
    {
        read_line(
            "Enter your username: ",
            faculty_id,
            sizeof(faculty_id)
        );

        faculty_index=
            find_faculty_index(faculty_id);

        if (faculty_index==-1)
        {
            printf("Username not found.\n");

            if (!ask_retry_or_back(
                    "Go back"
                ))
            {
                return;
            }

            continue;
        }

        faculty=&faculty_members[faculty_index];
        break;
    }

    while (!verify_security_answers(
        faculty->answer_birth,
        faculty->answer_school,
        faculty->answer_book,
        faculty->answer_bike
    ))
    {
        if (!ask_retry_or_back(
                "Go back"
            ))
        {
            return;
        }
    }

    printf("Authentication successful.\n");

    if (!read_new_password(
        new_password,
        sizeof(new_password)
    ))
    {
        return;
    }

    copy_str(
        faculty->password,
        new_password,
        sizeof(faculty->password)
    );

    save_all();

    printf("Password changed successfully.\n");
}

static void forgot_password_menu(void)
{
    int option;

    while (1)
    {
        printf("\n");
        printf("----------------------------------------\n");
        printf("Password Recovery\n");
        printf("----------------------------------------\n");
        printf("1. Recover student password\n");
        printf("2. Recover faculty password\n");
        printf("3. Go back\n");

        option=read_int("Enter an option: ");

        if (option==1)
        {
            recover_student_password();
        }
        else if (option==2)
        {
            recover_faculty_password();
        }
        else if (option==3)
        {
            return;
        }
        else
        {
            printf("Invalid option. Please try again.\n");
        }
    }
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

static int find_request_index(int request_id)
{
    int index;

    for (index=0; index<request_count; index++)
    {
        if (requests[index].id==request_id)
        {
            return index;
        }
    }
    return -1;
}

static int offering_has_student(
    int offering_index,
    const char *student_id
)
{
    int index;

    if (offering_index<0 ||
        offering_index>=offering_count)
    {
        return -1;
    }

    for (
        index=0;
        index<offerings[offering_index].enrolled_count;
        index++
    )
    {
        if (strcmp(
                offerings[offering_index]
                    .enrollments[index]
                    .student_id,
                student_id
            )==0)
        {
            return index;
        }
    }
    return -1;
}

static int student_is_enrolled(const char *student_id)
{
    int offering_index;

    for (
        offering_index=0;
        offering_index<offering_count;
        offering_index++
    )
    {
        if (offering_has_student(
                offering_index,
                student_id
            )!=-1)
        {
            return 1;
        }
    }

    return 0;
}

static int is_student_passed_course(
    const char *student_id,
    const char *course_id
)
{
    int offering_index;
    int enrollment_index;

    for (
        offering_index=0;
        offering_index<offering_count;
        offering_index++
    )
    {
        if (strcmp(
                offerings[offering_index].course_id,
                course_id
            )!=0)
        {
            continue;
        }

        for (
            enrollment_index=0;
            enrollment_index<offerings[offering_index].enrolled_count;
            enrollment_index++
        )
        {
            Enrollment *enrollment=&offerings[offering_index]
		.enrollments[enrollment_index];

            if (strcmp(
                    enrollment->student_id,
                    student_id
                )==0 &&
                enrollment->grade>=PASSING_GRADE)
            {
                return 1;
            }
        }
    }
    return 0;
}

static int prerequisites_satisfied(
    const Student *student,
    const Course *course
)
{
    char prerequisites[STR_SIZE];
    char *course_id;

    if (course->prerequisites[0]=='\0' ||
        strcmp(course->prerequisites, "-")==0)
    {
        return 1;
    }

    copy_str(
        prerequisites,
        course->prerequisites,
        sizeof(prerequisites)
    );

    course_id=strtok(prerequisites, ",");

    while (course_id!=NULL)
    {
        trim(course_id);

        if (course_id[0]!='\0' &&
            !is_student_passed_course(
                student->student_id,
                course_id
            ))
        {
            printf(
                "Prerequisite not satisfied: %s\n",
                course_id
            );
            return 0;
        }

        course_id=strtok(NULL, ",");
    }
    return 1;
}

static int course_is_available_for_semester(
    const Course *course,
    int semester
)
{
    if (course==NULL || semester<=0)
    {
        return 0;
    }

    if (course->available_from_semester==
        COURSE_AVAILABILITY_NEXT_SEMESTER)
    {
        return 0;
    }

    if (course->available_from_semester==
        COURSE_AVAILABILITY_UNRESTRICTED)
    {
        return 1;
    }

    return semester>=course->available_from_semester;
}

static int course_is_in_use(const char *course_id)
{
    int index;

    for (index=0; index<offering_count; index++)
    {
        if (strcmp(offerings[index].course_id, course_id)==0)
        {
            return 1;
        }
    }

    for (index=0; index<request_count; index++)
    {
        if (strcmp(requests[index].course_id, course_id)==0 &&
            strcmp(requests[index].status, "pending")==0)
        {
            return 1;
        }
    }
    return 0;
}

static int faculty_is_in_use(const char *faculty_id)
{
    int index;

    for (index=0; index<offering_count; index++)
    {
        if (strcmp(offerings[index].faculty_id, faculty_id)==0)
        {
            return 1;
        }
    }

    for (index=0; index<request_count; index++)
    {
        if (strcmp(requests[index].faculty_id, faculty_id)==0 &&
            strcmp(requests[index].status, "pending")==0)
        {
            return 1;
        }
    }
    return 0;
}

static void json_write_string(FILE *file,const char *text)
{
    const unsigned char *cursor;

    if (text==NULL)
    {
        text="";
    }

    cursor=(const unsigned char *)text;
    fputc('"', file);

    while (*cursor!='\0')
    {
        if (*cursor=='"' || *cursor=='\\')
        {
            fputc('\\', file);
            fputc(*cursor, file);
        }
        else if (*cursor=='\n')
        {
            fputs("\\n", file);
        }
        else if (*cursor=='\r')
        {
            fputs("\\r", file);
        }
        else if (*cursor=='\t')
        {
            fputs("\\t", file);
        }
        else if (*cursor<32)
        {
            fprintf(
                file,
                "\\u%04x",
                (unsigned int)*cursor
            );
        }
        else
        {
            fputc(*cursor, file);
        }
        cursor++;
    }

    fputc('"', file);
}

static void json_write_key_string(FILE *file,const char *key,const char *value)
{
    json_write_string(file, key);
    fputc(':', file);
    json_write_string(file, value);
}

static void begin_json_record(FILE *file,int *first_record)
{
    if (!*first_record)
    {
        fputs(",\n", file);
    }

    *first_record=0;
    fputs("  {", file);
}

static int save_all(void)
{
    FILE *file;
    Student *student;
    Faculty *faculty;
    Course *course;
    Offering *offering;
    Enrollment *enrollment;
    Request *request;
    int first_record=1;
    int index;
    int enrollment_index;

    file=fopen(DATA_TEMP_FILE, "w");

    if (file==NULL)
    {
        printf("Could not save %s.\n", DATA_FILE);
        return 0;
    }

    fputs("[\n", file);

    begin_json_record(file, &first_record);

    fprintf(
        file,
        "\"record\":\"calendar\","
        "\"current_semester\":%d,"
        "\"offering\":%d,"
        "\"unit_selection\":%d,"
        "\"classes_exams\":%d,"
        "\"grade_recording\":%d}",
        calendar_state.current_semester,
        (int)calendar_state.offering,
        (int)calendar_state.unit_selection,
        (int)calendar_state.classes_exams,
        (int)calendar_state.grade_recording
    );

    for (index=0; index<student_count; index++)
    {
        student=&students[index];

        begin_json_record(file, &first_record);

        fputs("\"record\":\"student\",", file);

        json_write_key_string(file,"first_name",student->first_name);

        fputc(',', file);

        json_write_key_string(file,"last_name",student->last_name);

        fputc(',', file);

        json_write_key_string(file,"student_id",student->student_id);

        fputc(',', file);

        json_write_key_string(file,"national_code",student->national_code);

        fputc(',', file);

        json_write_key_string(file,"field",student->field);

        fprintf(file,",\"entrance_year\":%d,",student->entrance_year);

        json_write_key_string(file,"section",student->section);

        fputc(',', file);

        json_write_key_string(file,"mentor",student->mentor);

        fputc(',', file);

        json_write_key_string(file,"department",student->department);

        fputc(',', file);

        json_write_key_string(file,"answer_birth",student->answer_birth);

        fputc(',', file);

        json_write_key_string(file,"answer_school",student->answer_school);

        fputc(',', file);

        json_write_key_string(file,"answer_book",student->answer_book);

        fputc(',', file);

        json_write_key_string(file,"answer_bike",student->answer_bike);
        
	    fputc(',', file);

        json_write_key_string(file,"password",student->password);

        fputc('}', file);
    }


        for (index=0; index<faculty_count; index++)
    {
        faculty=&faculty_members[index];

        begin_json_record(file, &first_record);

        fputs("\"record\":\"faculty\",", file);

        json_write_key_string(file,"first_name",faculty->first_name);

        fputc(',', file);

        json_write_key_string(file,"last_name",faculty->last_name);

        fputc(',', file);

        json_write_key_string(file,"faculty_id",faculty->faculty_id);

        fputc(',', file);

        json_write_key_string(file,"national_code",faculty->national_code);

        fputc(',', file);

        json_write_key_string(file,"field",faculty->field);

        fprintf(file,",\"entrance_year\":%d,",faculty->entrance_year);

        json_write_key_string(file,"degree",faculty->degree);

        fputc(',', file);

        json_write_key_string(file,"department",faculty->department);

        fputc(',', file);

        json_write_key_string(file,"password",faculty->password);

        fputc(',', file);

        json_write_key_string(file,"answer_birth",faculty->answer_birth);

        fputc(',', file);

        json_write_key_string(file,"answer_school",faculty->answer_school);

        fputc(',', file);

        json_write_key_string(file,"answer_book",faculty->answer_book);

        fputc(',', file);

        json_write_key_string(file,"answer_bike",faculty->answer_bike);

        fputc('}', file);
    }

    for (index=0; index<course_count; index++)
    {
        course=&courses[index];

        begin_json_record(file, &first_record);

        fputs("\"record\":\"course\",", file);

        json_write_key_string(file,"name",course->name);

        fputc(',', file);

        json_write_key_string(file,"course_id",course->course_id);

        fprintf(file,",\"units\":%d,",course->units);

        json_write_key_string(file,"prerequisites",course->prerequisites);

        fputc(',', file);

        json_write_key_string(file,"section",course->section);

        fputc(',', file);

        json_write_key_string(file,"field",course->field);

        fputc(',', file);

        json_write_key_string(file,"department",course->department);

        fprintf(file,",\"available_from_semester\":%d",course->available_from_semester);

        fputc('}', file);
    }


    for (index=0; index<offering_count; index++)
    {
        offering=&offerings[index];

        begin_json_record(file, &first_record);

        fprintf(file,"\"record\":\"offering\",""\"index\":%d,", index);

        json_write_key_string(file,"course_id",offering->course_id);

        fputc(',', file);

        json_write_key_string(file,"faculty_id",offering->faculty_id);

        fprintf(file,",\"semester\":%d,""\"capacity\":%d,",offering->semester,offering->capacity);

        json_write_key_string(file,"department",offering->department);

        fputc(',', file);

        json_write_key_string(file,"place",offering->place);

        fputc('}', file);

        for (
            enrollment_index=0;
            enrollment_index<offering->enrolled_count;
            enrollment_index++)
        {
            enrollment=&offering->enrollments[enrollment_index];

            begin_json_record(file, &first_record);

            fprintf(file,"\"record\":\"enrollment\",""\"offering_index\":%d,",index);

            json_write_key_string(file, "student_id", enrollment->student_id);

            fprintf(file,",\"grade\":%.17g," "\"survey_score\":%d}", enrollment->grade,enrollment->survey_score);
        }
    }

    for (index=0; index<request_count; index++)
    {
        request=&requests[index];

        begin_json_record(file, &first_record);

        fprintf(file,"\"record\":\"request\",""\"id\":%d,",request->id);

        json_write_key_string(file,"type",request->type);

        fputc(',', file);

        json_write_key_string(file,"course_id",request->course_id);

        fputc(',', file);

        json_write_key_string(file,"faculty_id", request->faculty_id);

        fprintf(
            file,
            ",\"semester\":%d,"
            "\"capacity\":%d,"
            "\"amount\":%d,"
            "\"offering_index\":%d,",
            request->semester,
            request->capacity,
            request->amount,
            request->offering_index
        );

        json_write_key_string(file,"department",request->department);

        fputc(',', file);

        json_write_key_string(file,"place",request->place);

        fputc(',', file);

        json_write_key_string(file,"status",request->status);

        fputc('}', file);
    }

    fputs("\n]\n", file);

    if (fclose(file)!=0)
    {
        remove(DATA_TEMP_FILE);
        printf("Could not finish writing %s.\n", DATA_FILE);
        return 0;
    }

    remove(DATA_FILE);

    if (rename(DATA_TEMP_FILE, DATA_FILE)!=0)
    {
        printf("Could not finalize %s.\n", DATA_FILE);
        remove(DATA_TEMP_FILE);
        return 0;
    }
    return 1;
}

static const char *skip_json_spaces(
    const char *text
)
{
    while (
        *text!='\0' &&
        isspace((unsigned char)*text)
    )
    {
        text++;
    }

    return text;
}

static int next_json_string(
    const char **cursor,
    char *output,
    size_t size
)
{
    const char *position;
    size_t used=0;

    if (
        cursor==NULL ||
        *cursor==NULL ||
        output==NULL ||
        size==0
    )
    {
        return 0;
    }

    position=strchr(*cursor, ':');

    if (position==NULL)
    {
        return 0;
    }

    position=skip_json_spaces(position+1);

    if (*position!='"')
    {
        return 0;
    }

    position++;

    while (
        *position!='\0' &&
        *position!='"'
    )
    {
        char character=*position++;

        if (character=='\\')
        {
            if (*position=='\0')
            {
                return 0;
            }

            character=*position++;

            if (character=='n')
            {
                character='\n';
            }
            else if (character=='r')
            {
                character='\r';
            }
            else if (character=='t')
            {
                character='\t';
            }
            else if (character=='u')
            {
                int digit;

                for (
                    digit=0;
                    digit<4 && isxdigit((unsigned char)*position);
                    digit++)
                
                {
                    position++;
                }

                character='?';
            }
        }

        if (used+1<size)
        {
            output[used]=character;
            used++;
        }
    }

    if (*position!='"')
    {
        return 0;
    }

    output[used]='\0';

    *cursor=position+1;

    return 1;
}

static int next_json_int(
    const char **cursor,
    int *output
)
{
    const char *position;
    char *end;
    long value;

    position=strchr(*cursor, ':');

    if (position==NULL)
    {
        return 0;
    }

    position=skip_json_spaces(position+1);

    value=strtol(position, &end, 10);

    if (end==position)
    {
        return 0;
    }

    *output=(int)value;
    *cursor=end;
    return 1;
}

static int next_json_double(
    const char **cursor,
    double *output
)
{
    const char *position;
    char *end;
    double value;

    position=strchr(*cursor, ':');

    if (position==NULL)
    {
        return 0;
    }

    position=skip_json_spaces(position+1);

    value=strtod(position, &end);

    if (end==position)
    {
        return 0;
    }

    *output=value;
    *cursor=end;

    return 1;
}

static int load_all(void)
{
    FILE *file;
    char line[LINE_SIZE];
    int loaded_any=0;
    int invalid_lines=0;

    file=fopen(DATA_FILE, "r");

    if (file==NULL)
    {
        return 0;
    }

    memset(students, 0, sizeof(students));
    memset(faculty_members,0,sizeof(faculty_members));
    memset(courses, 0, sizeof(courses));
    memset(offerings, 0, sizeof(offerings));
    memset(requests, 0, sizeof(requests));
    memset(&calendar_state,0,sizeof(calendar_state));

    calendar_state.current_semester=14042;

    student_count=0;
    faculty_count=0;
    course_count=0;
    offering_count=0;
    request_count=0;
    next_request_id=1;

    while (fgets(line, sizeof(line), file)!=NULL)
    {
        const char *cursor=line;
        char record_type[SMALL_SIZE];

        if (strchr(line, '{')==NULL)
        {
            continue;
        }

        if (!next_json_string(&cursor,record_type,sizeof(record_type)))
        {
            invalid_lines++;
            continue;
        }

        if (strcmp(record_type, "calendar")==0)
        {
            int current_semester;
            int offering_state;
            int unit_selection_state;
            int classes_exams_state;
            int grade_recording_state;

            if (strstr(line, "\"current_semester\"")!=NULL)
            {
                if (next_json_int(&cursor,&current_semester) &&
                    next_json_int(&cursor,&offering_state) &&
                    next_json_int(&cursor,&unit_selection_state) &&
                    next_json_int(&cursor,&classes_exams_state) &&
                    next_json_int(&cursor,&grade_recording_state) &&
                    current_semester>0 &&
                    offering_state>=PHASE_NOT_STARTED &&
                    offering_state<=PHASE_FINISHED &&
                    unit_selection_state>=PHASE_NOT_STARTED &&
                    unit_selection_state<=PHASE_FINISHED &&
                    classes_exams_state>=PHASE_NOT_STARTED &&
                    classes_exams_state<=PHASE_FINISHED &&
                    grade_recording_state>=PHASE_NOT_STARTED &&
                    grade_recording_state<=PHASE_FINISHED)
                {
                    calendar_state.current_semester=
                        current_semester;

                    calendar_state.offering=
                        (PhaseState)offering_state;

                    calendar_state.unit_selection=
                        (PhaseState)unit_selection_state;

                    calendar_state.classes_exams=
                        (PhaseState)classes_exams_state;

                    calendar_state.grade_recording=
                        (PhaseState)grade_recording_state;

                    loaded_any=1;
                }
                else
                {
                    invalid_lines++;
                }
            }
            else
            {
                int old_course_survey;

                if (next_json_int(&cursor,&offering_state) &&
                next_json_int(&cursor,&unit_selection_state) &&
                next_json_int(&cursor,&classes_exams_state) &&
                next_json_int(&cursor,&grade_recording_state) &&
                next_json_int(&cursor,&old_course_survey))
                {
                    (void)old_course_survey;

                    calendar_state.current_semester=14042;

                    calendar_state.offering=offering_state? PHASE_ACTIVE: PHASE_NOT_STARTED;

                    calendar_state.unit_selection=unit_selection_state? PHASE_ACTIVE: PHASE_NOT_STARTED;

                    calendar_state.classes_exams=classes_exams_state? PHASE_ACTIVE: PHASE_NOT_STARTED;

                    calendar_state.grade_recording=grade_recording_state? PHASE_ACTIVE: PHASE_NOT_STARTED;

                    loaded_any=1;
                }
                else
                {
                    invalid_lines++;
                }
            }
        }
        else if (strcmp(record_type, "student")==0 &&student_count<MAX_STUDENTS)
        {
            Student student;
            int student_fields_loaded;

            memset(&student,0,sizeof(student));

            copy_str(student.answer_school,"Unknown",sizeof(student.answer_school));

            student_fields_loaded=
                next_json_string(&cursor,student.first_name,sizeof(student.first_name)) &&
                    next_json_string(&cursor,student.last_name,sizeof(student.last_name)) &&
                        next_json_string(&cursor,student.student_id,sizeof(student.student_id)) &&
                            next_json_string(&cursor,student.national_code,sizeof(student.national_code)) &&
                                next_json_string(&cursor,student.field,sizeof(student.field)) &&
                                    next_json_int(&cursor,&student.entrance_year) &&
                                        next_json_string(&cursor,student.section,sizeof(student.section)) &&
                                            next_json_string(&cursor,student.mentor,sizeof(student.mentor)) &&
                                                next_json_string(&cursor,student.department,sizeof(student.department)) &&
                                                    next_json_string(&cursor,student.answer_birth,sizeof(student.answer_birth));

            if (student_fields_loaded && strstr(line,"\"answer_school\"")!=NULL)
            {
                student_fields_loaded=
                    next_json_string(&cursor,student.answer_school,sizeof(student.answer_school));
            }

            student_fields_loaded=student_fields_loaded &&next_json_string(&cursor,student.answer_book,sizeof(student.answer_book)) &&
                next_json_string(&cursor,student.answer_bike,sizeof(student.answer_bike)) &&
                    next_json_string(&cursor,student.password,sizeof(student.password));

            if (student_fields_loaded)
            {
                students[student_count]=student;
                student_count++;
                loaded_any=1;
            }
            else
            {
                invalid_lines++;
            }
        }

        else if (strcmp(record_type, "faculty")==0 &&faculty_count<MAX_FACULTY)
        {
            Faculty faculty;
            int faculty_fields_loaded;
            memset(&faculty,0,sizeof(faculty));

            copy_str(faculty.answer_school,"Unknown",sizeof(faculty.answer_school));

            faculty_fields_loaded=next_json_string(&cursor,faculty.first_name,sizeof(faculty.first_name)) &&
                next_json_string(&cursor,faculty.last_name,sizeof(faculty.last_name)) &&
                    next_json_string(&cursor,faculty.faculty_id,sizeof(faculty.faculty_id)) &&
                        next_json_string(&cursor,faculty.national_code,sizeof(faculty.national_code)) &&
                            next_json_string(&cursor,faculty.field,sizeof(faculty.field)) &&
                                next_json_int(&cursor,&faculty.entrance_year) &&
                                    next_json_string(&cursor,faculty.degree,sizeof(faculty.degree)) &&
                                        next_json_string(&cursor,faculty.department,sizeof(faculty.department)) &&
                                            next_json_string(&cursor,faculty.password,sizeof(faculty.password)) &&
                                                next_json_string(&cursor,faculty.answer_birth,sizeof(faculty.answer_birth));

            if (faculty_fields_loaded &&
            strstr(line,"\"answer_school\"")!=NULL)
            {
                faculty_fields_loaded=next_json_string(
                &cursor,
                faculty.answer_school,
                sizeof(faculty.answer_school)
                );
            }

            faculty_fields_loaded=
                faculty_fields_loaded &&
                    next_json_string(
                        &cursor,
                        faculty.answer_book,
                        sizeof(faculty.answer_book)
                        ) &&
                            next_json_string(
                                &cursor,
                                faculty.answer_bike,
                                sizeof(faculty.answer_bike));

            if (faculty_fields_loaded)
            {
                faculty_members[faculty_count]=
                    faculty;
                faculty_count++;
                loaded_any=1;
            }
            else
                {
                invalid_lines++;
                }
}
        else if (strcmp(record_type, "course")==0 && course_count<MAX_COURSES)
        {
            Course course;
            int course_fields_loaded;

            memset(&course, 0, sizeof(course));

            course.available_from_semester=
                COURSE_AVAILABILITY_UNRESTRICTED;

            course_fields_loaded=
                next_json_string(
                    &cursor,
                    course.name,
                    sizeof(course.name)
                ) &&
                next_json_string(
                    &cursor,
                    course.course_id,
                    sizeof(course.course_id)
                ) &&
                next_json_int(
                    &cursor,
                    &course.units
                ) &&
                next_json_string(
                    &cursor,
                    course.prerequisites,
                    sizeof(course.prerequisites)
                ) &&
                next_json_string(
                    &cursor,
                    course.section,
                    sizeof(course.section)
                ) &&
                next_json_string(
                    &cursor,
                    course.field,
                    sizeof(course.field)
                ) &&
                next_json_string(
                    &cursor,
                    course.department,
                    sizeof(course.department)
                );

            if (course_fields_loaded &&
                strstr(
                    line,
                    "\"available_from_semester\""
                )!=NULL)
            {
                course_fields_loaded=next_json_int(
                    &cursor,
                    &course.available_from_semester
                );
            }

            if (course_fields_loaded &&
                course.available_from_semester>=
                    COURSE_AVAILABILITY_NEXT_SEMESTER)
            {
                courses[course_count]=course;
                course_count++;
                loaded_any=1;
            }
            else
            {
                invalid_lines++;
            }
        }

        else if (strcmp(record_type, "offering")==0 && offering_count<MAX_OFFERINGS)
        {
            Offering offering;
            int stored_index;

            memset(&offering, 0, sizeof(offering));

            if (next_json_int(&cursor,&stored_index) &&
                next_json_string(&cursor,offering.course_id,sizeof(offering.course_id)) &&
                next_json_string(&cursor,offering.faculty_id,sizeof(offering.faculty_id)) &&
                next_json_int(&cursor,&offering.semester) &&
                next_json_int(&cursor,&offering.capacity) &&
                next_json_string(&cursor,offering.department,sizeof(offering.department)) &&
                next_json_string(&cursor,offering.place,sizeof(offering.place)))
            {
                (void)stored_index;

                offerings[offering_count]=offering;
                offering_count++;
                loaded_any=1;
            }
            else
            {
                invalid_lines++;
            }
        }
        else if (strcmp(record_type, "enrollment")==0)
        {
            char student_id[SMALL_SIZE];
            int offering_index;
            int survey_score;
            double grade;

            if (next_json_int(&cursor,&offering_index) &&
                next_json_string(&cursor,student_id,sizeof(student_id)) &&
                next_json_double(&cursor,&grade) &&
                next_json_int(&cursor,&survey_score) &&
                offering_index>=0 &&
                offering_index<offering_count &&
                offerings[offering_index].enrolled_count<MAX_ENROLLED &&
                offerings[offering_index].enrolled_count<offerings[offering_index].capacity)
            {
                Enrollment *enrollment;

                enrollment= &offerings[offering_index].enrollments[offerings[offering_index].enrolled_count];

                copy_str(
                    enrollment->student_id,
                    student_id,
                    sizeof(enrollment->student_id)
                );

                enrollment->grade=grade;
                enrollment->survey_score=survey_score;

                offerings[offering_index].enrolled_count++;

                loaded_any=1;
            }
            else
            {
                invalid_lines++;
            }
        }
        else if (strcmp(record_type, "request")==0 && request_count<MAX_REQUESTS)
        {
            Request request;

            memset(&request, 0, sizeof(request));

            if (next_json_int(&cursor,&request.id) &&
                next_json_string(&cursor,request.type,sizeof(request.type)) &&
                next_json_string(&cursor,request.course_id,sizeof(request.course_id)) &&
                next_json_string(&cursor,request.faculty_id,sizeof(request.faculty_id)) &&
                next_json_int(&cursor,&request.semester) &&
                next_json_int(&cursor,&request.capacity) &&
                next_json_int(&cursor,&request.amount) &&
                next_json_int(&cursor,&request.offering_index) &&
                next_json_string(&cursor,request.department,sizeof(request.department)) &&
                next_json_string(&cursor,request.place,sizeof(request.place)) &&
                next_json_string(&cursor,request.status,sizeof(request.status)))
            {
                requests[request_count]=request;
                request_count++;

                if (request.id>=next_request_id)
                {
                    next_request_id=request.id+1;
                }

                loaded_any=1;
            }
            else
            {
                invalid_lines++;
            }
        }
    }

    fclose(file);

    if (invalid_lines>0)
    {
        printf(
            "Warning: %d invalid data line(s) "
            "were ignored.\n",
            invalid_lines
        );
    }

    return loaded_any;
}

static void add_student_seed(
    const char *first_name,
    const char *last_name,
    const char *student_id,
    const char *national_code,
    const char *field,
    int entrance_year,
    const char *section,
    const char *mentor,
    const char *department,
    const char *answer_birth,
    const char *answer_school,
    const char *answer_book,
    const char *answer_bike,
    const char *password
)
{
    Student *student;

    if (student_count>=MAX_STUDENTS)
    {
        return;
    }

    student=&students[student_count];

    memset(
        student,
        0,
        sizeof(*student)
    );

    copy_str(
        student->first_name,
        first_name,
        sizeof(student->first_name)
    );

    copy_str(
        student->last_name,
        last_name,
        sizeof(student->last_name)
    );

    copy_str(
        student->student_id,
        student_id,
        sizeof(student->student_id)
    );

    copy_str(
        student->national_code,
        national_code,
        sizeof(student->national_code)
    );

    copy_str(
        student->field,
        field,
        sizeof(student->field)
    );

    student->entrance_year=entrance_year;

    copy_str(
        student->section,
        section,
        sizeof(student->section)
    );

    copy_str(
        student->mentor,
        mentor,
        sizeof(student->mentor)
    );

    copy_str(
        student->department,
        department,
        sizeof(student->department)
    );

    copy_str(
        student->answer_birth,
        answer_birth,
        sizeof(student->answer_birth)
    );

    copy_str(
    student->answer_school,
    answer_school,
    sizeof(student->answer_school));

    copy_str(
        student->answer_book,
        answer_book,
        sizeof(student->answer_book)
    );

    copy_str(
        student->answer_bike,
        answer_bike,
        sizeof(student->answer_bike)
    );

    copy_str(
        student->password,
        password,
        sizeof(student->password)
    );

    student_count++;
}

static void add_faculty_seed(
    const char *first_name,
    const char *last_name,
    const char *faculty_id,
    const char *national_code,
    const char *field,
    int entrance_year,
    const char *degree,
    const char *department,
    const char *password
)
{
    Faculty *faculty;

    if (faculty_count>=MAX_FACULTY)
    {
        return;
    }

    faculty=&faculty_members[faculty_count];

    memset(
        faculty,
        0,
        sizeof(*faculty)
    );

    copy_str(
        faculty->first_name,
        first_name,
        sizeof(faculty->first_name)
    );

    copy_str(
        faculty->last_name,
        last_name,
        sizeof(faculty->last_name)
    );

    copy_str(
        faculty->faculty_id,
        faculty_id,
        sizeof(faculty->faculty_id)
    );

    copy_str(
        faculty->national_code,
        national_code,
        sizeof(faculty->national_code)
    );

    copy_str(
        faculty->field,
        field,
        sizeof(faculty->field)
    );

    faculty->entrance_year=entrance_year;

    copy_str(
        faculty->degree,
        degree,
        sizeof(faculty->degree)
    );

    copy_str(
        faculty->department,
        department,
        sizeof(faculty->department)
    );

    copy_str(
        faculty->password,
        password,
        sizeof(faculty->password)
    );

    copy_str(
        faculty->answer_birth,
        "Tehran",
        sizeof(faculty->answer_birth)
    );

    copy_str(
    faculty->answer_school,
    "Sharif High School",
    sizeof(faculty->answer_school));

    copy_str(
        faculty->answer_book,
        "1984",
        sizeof(faculty->answer_book)
    );

    copy_str(
        faculty->answer_bike,
        "Blue",
        sizeof(faculty->answer_bike)
    );

    faculty_count++;
}

static void add_course_seed(
    const char *name,
    const char *course_id,
    int units,
    const char *prerequisites,
    const char *section,
    const char *field,
    const char *department
)
{
    Course *course;

    if (course_count>=MAX_COURSES)
    {
        return;
    }

    course=&courses[course_count];

    memset(
        course,
        0,
        sizeof(*course)
    );

    copy_str(
        course->name,
        name,
        sizeof(course->name)
    );

    copy_str(
        course->course_id,
        course_id,
        sizeof(course->course_id)
    );

    course->units=units;

    copy_str(
        course->prerequisites,
        prerequisites,
        sizeof(course->prerequisites)
    );

    copy_str(
        course->section,
        section,
        sizeof(course->section)
    );

    copy_str(
        course->field,
        field,
        sizeof(course->field)
    );

    copy_str(
        course->department,
        department,
        sizeof(course->department)
    );

    course->available_from_semester=
    COURSE_AVAILABILITY_UNRESTRICTED;

    course_count++;
}

static void add_offering_seed(
    const char *course_id,
    const char *faculty_id,
    int semester,
    int capacity,
    const char *department,
    const char *place
)
{
    Offering *offering;

    if (offering_count>=MAX_OFFERINGS)
    {
        return;
    }

    offering=&offerings[offering_count];

    memset(
        offering,
        0,
        sizeof(*offering)
    );

    copy_str(
        offering->course_id,
        course_id,
        sizeof(offering->course_id)
    );

    copy_str(
        offering->faculty_id,
        faculty_id,
        sizeof(offering->faculty_id)
    );

    offering->semester=semester;
    offering->capacity=capacity;
    offering->enrolled_count=0;

    copy_str(
        offering->department,
        department,
        sizeof(offering->department)
    );

    copy_str(
        offering->place,
        place,
        sizeof(offering->place)
    );

    offering_count++;
}

static void enroll_seed(
    int offering_index,
    const char *student_id,
    double grade
)
{
    Offering *offering;
    Enrollment *enrollment;

    if (
        offering_index<0 ||
        offering_index>=offering_count
    )
    {
        return;
    }

    offering=&offerings[offering_index];

    if (offering->enrolled_count>=MAX_ENROLLED)
    {
        return;
    }

    enrollment=
        &offering->enrollments[
            offering->enrolled_count
        ];

    memset(
        enrollment,
        0,
        sizeof(*enrollment)
    );

    copy_str(
        enrollment->student_id,
        student_id,
        sizeof(enrollment->student_id)
    );

    enrollment->grade=grade;
    enrollment->survey_score=-1;

    offering->enrolled_count++;
}

static void initialize_sample_data(void)
{
    memset(
        students,
        0,
        sizeof(students)
    );

    memset(
        faculty_members,
        0,
        sizeof(faculty_members)
    );

    memset(
        courses,
        0,
        sizeof(courses)
    );

    memset(
        offerings,
        0,
        sizeof(offerings)
    );

    memset(
        requests,
        0,
        sizeof(requests)
    );

    student_count=0;
    faculty_count=0;
    course_count=0;
    offering_count=0;
    request_count=0;
    next_request_id=1;

    add_student_seed(
        "Ali",
        "Ahmadi",
        "404123456",
        "0123456789",
        "Computer Engineering",
        404,
        "BSc",
        "Hossein Asadi",
        "Computer Engineering",
        "Karaj",
        "Alborz High School",
        "Anne Shirley",
        "White",
        "123456"
    );

    add_student_seed(
        "Sara",
        "Karimi",
        "403234567",
        "1234567890",
        "Electrical Engineering",
        403,
        "MSc",
        "Mehdi Rezaei",
        "Electrical Engineering",
        "Isfahan",
        "Beheshti High School",
        "The Little Prince",
        "Blue",
        "123456"
    );

    add_student_seed(
        "Reza",
        "Nouri",
        "404345678",
        "2345678901",
        "Mechanical Engineering",
        404,
        "BSc",
        "Leila Ahmadi",
        "Mechanical Engineering",
        "Shiraz",
        "Hafez High School",
        "Pride and Prejudice",
        "Red",
        "123456"
    );

    add_student_seed(
        "Rozhan",
        "Azizi",
        "402456789",
        "3456789012",
        "Civil Engineering",
        402,
        "PhD",
        "Saeed Jamali",
        "Civil Engineering",
        "Sanandaj",
        "Kurdistan High School",
        "The Alchemist",
        "Green",
        "123456"
    );

    add_student_seed(
        "Diako",
        "Gholami",
        "403567890",
        "4567890123",
        "Software Engineering",
        403,
        "MSc",
        "Parisa Moradi",
        "Computer Engineering",
        "Kermanshah",
        "Razi High School",
        "1984",
        "Black",
        "123456"
    );

    add_faculty_seed(
        "Hossein",
        "Asadi",
        "FCS105",
        "9911111111",
        "Computer Engineering",
        1395,
        "PhD",
        "Computer Engineering",
        "123456"
    );

    add_faculty_seed(
        "Maryam",
        "Ghafari",
        "FEE203",
        "9922222222",
        "Electrical Engineering",
        1396,
        "PhD",
        "Electrical Engineering",
        "123456"
    );

    add_faculty_seed(
        "Reza",
        "Karimi",
        "FME315",
        "9933333333",
        "Mechanical Engineering",
        1394,
        "PhD",
        "Mechanical Engineering",
        "123456"
    );

    add_faculty_seed(
        "Sara",
        "Nikpour",
        "FCS107",
        "9944444444",
        "Software Engineering",
        1397,
        "PhD",
        "Computer Engineering",
        "123456"
    );

    add_faculty_seed(
        "Hasan",
        "Rezaei",
        "FCS101",
        "9955555555",
        "Computer Engineering",
        1393,
        "PhD",
        "Computer Engineering",
        "123456"
    );

    add_course_seed(
        "Fundamentals of Programming",
        "CS101",
        3,
        "-",
        "BSc",
        "Computer Engineering",
        "Computer Engineering"
    );

    add_course_seed(
        "Data Structures",
        "CS201",
        3,
        "CS101",
        "BSc",
        "Computer Engineering",
        "Computer Engineering"
    );

    add_course_seed(
        "Advanced Programming",
        "CS305",
        4,
        "CS201",
        "BSc",
        "Computer Engineering",
        "Computer Engineering"
    );

    add_course_seed(
        "Digital Logic Design",
        "EE112",
        3,
        "EE101",
        "BSc",
        "Electrical Engineering",
        "Electrical Engineering"
    );

    add_course_seed(
        "Thermodynamics I",
        "ME301",
        3,
        "-",
        "BSc",
        "Mechanical Engineering",
        "Mechanical Engineering"
    );

    add_course_seed(
        "Software Engineering",
        "CS401",
        3,
        "CS305",
        "MSc",
        "Computer Engineering",
        "Computer Engineering"
    );

    add_course_seed(
        "General Physics 2",
        "PHY102",
        3,
        "PHY101",
        "BSc",
        "General",
        "Physics"
    );

    add_course_seed(
        "Logical Design",
        "CS103",
        3,
        "-",
        "BSc",
        "Computer Engineering",
        "Computer Engineering"
    );

    add_course_seed(
        "Calculus 2",
        "MATH102",
        4,
        "MATH101",
        "BSc",
        "General",
        "Mathematics"
    );

    add_course_seed(
        "Discrete Mathematics",
        "CS104",
        3,
        "-",
        "BSc",
        "Computer Engineering",
        "Computer Engineering"
    );

    add_offering_seed(
        "CS101",
        "FCS105",
        14042,
        40,
        "Computer Engineering",
        "Room 201, Science Building"
    );

    add_offering_seed(
        "CS201",
        "FCS105",
        14042,
        35,
        "Computer Engineering",
        "Room 405, Science Building"
    );

    add_offering_seed(
        "EE112",
        "FEE203",
        14042,
        30,
        "Electrical Engineering",
        "Lab 201, Engineering Hall"
    );

    add_offering_seed(
        "ME301",
        "FME315",
        14042,
        40,
        "Mechanical Engineering",
        "Room 102, Main Hall"
    );

    add_offering_seed(
        "CS401",
        "FCS107",
        14042,
        25,
        "Computer Engineering",
        "Room 305, IT Center"
    );

    add_offering_seed(
        "PHY102",
        "FCS101",
        14042,
        45,
        "Physics",
        "Room 301, Main Hall"
    );

    add_offering_seed(
        "CS103",
        "FCS101",
        14042,
        35,
        "Computer Engineering",
        "Room 204, Science Building"
    );

    add_offering_seed(
        "MATH102",
        "FCS101",
        14042,
        50,
        "Mathematics",
        "Room 110, Main Hall"
    );

    add_offering_seed(
        "CS104",
        "FCS101",
        14042,
        30,
        "Computer Engineering",
        "Room 207, Science Building"
    );

    add_offering_seed(
        "CS201",
        "FCS105",
        14041,
        35,
        "Computer Engineering",
        "Room 203, Science Building"
    );

    add_offering_seed(
        "CS305",
        "FCS105",
        14032,
        30,
        "Computer Engineering",
        "Room 205, Science Building"
    );

    add_offering_seed(
        "CS401",
        "FCS105",
        14031,
        25,
        "Computer Engineering",
        "Room 207, Science Building"
    );

    enroll_seed(
        0,
        "404123456",
        18.25
    );

    enroll_seed(
        5,
        "404123456",
        14.00
    );

    enroll_seed(
        6,
        "404123456",
        16.75
    );

    enroll_seed(
        7,
        "404123456",
        12.50
    );

    enroll_seed(
        8,
        "404123456",
        9.75
    );

    enroll_seed(
        1,
        "404123456",
        -1.00
    );

    enroll_seed(
        2,
        "403234567",
        -1.00
    );

    enroll_seed(
        3,
        "404345678",
        -1.00
    );

    enroll_seed(
        4,
        "403567890",
        -1.00
    );

    calendar_state.current_semester=14042;
    calendar_state.offering=PHASE_NOT_STARTED;
    calendar_state.unit_selection=PHASE_NOT_STARTED;
    calendar_state.classes_exams=PHASE_NOT_STARTED;
    calendar_state.grade_recording=PHASE_NOT_STARTED;
}

static void show_data_summary(void)
{
    printf("\nCurrent data summary:\n");

    printf(
        "Students: %d | Faculty: %d | Courses: %d\n",
        student_count,
        faculty_count,
        course_count
    );

    printf(
        "Offerings: %d | Requests: %d\n",
        offering_count,
        request_count
    );
}

static int collect_offering_indices(
    int semester,
    const char *faculty_id,
    int output[],
    int max_output
)
{
    int index;
    int count=0;

    if (output==NULL || max_output<=0)
    {
        return 0;
    }

    for (index=0; index<offering_count; index++)
    {
        if (semester>0 &&
            offerings[index].semester!=semester)
        {
            continue;
        }

        if (faculty_id!=NULL &&
            faculty_id[0]!='\0' &&
            strcmp(
                offerings[index].faculty_id,
                faculty_id
            )!=0)
        {
            continue;
        }

        output[count]=index;
        count++;

        if (count>=max_output)
        {
            break;
        }
    }

    return count;
}

static void sort_offering_indices_by_semester(
    int indices[],
    int count
)
{
    int first;
    int second;
    int temporary;

    for (first=0; first<count-1; first++)
    {
        for (second=first+1; second<count; second++)
        {
            if (offerings[indices[first]].semester<
                offerings[indices[second]].semester)
            {
                temporary=indices[first];
                indices[first]=indices[second];
                indices[second]=temporary;
            }
        }
    }
}

static void print_offering(
    const Offering *offering,
    int number
)
{
    int course_index;
    int faculty_index;

    course_index=
        find_course_index(offering->course_id);

    faculty_index=
        find_faculty_index(offering->faculty_id);

    printf("\nOffering number %d\n", number);

    printf(
        "Course ID: %s\n",
        offering->course_id
    );

    if (course_index!=-1)
    {
        printf(
            "Course name: %s\n",
            courses[course_index].name
        );
    }

    printf(
        "Faculty ID: %s\n",
        offering->faculty_id
    );

    if (faculty_index!=-1)
    {
        printf(
            "Faculty name: %s %s\n",
            faculty_members[faculty_index].first_name,
            faculty_members[faculty_index].last_name
        );
    }

    printf(
        "Semester: %d\n",
        offering->semester
    );

    printf(
        "Capacity: %d\n",
        offering->capacity
    );

    printf(
        "Enrolled students: %d\n",
        offering->enrolled_count
    );

    printf(
        "Department: %s\n",
        offering->department
    );

    printf(
        "Place: %s\n",
        offering->place
    );
}

static void list_offerings_by_semester(int semester)
{
    int offering_indices[MAX_OFFERINGS];
    int count;
    int index;

    printf("\n");
    printf("----------------------------------------\n");
    printf("Course Offerings in Semester %d\n", semester);
    printf("----------------------------------------\n");

    if (semester<=0)
    {
        printf("Semester number must be greater than zero.\n");
        return;
    }

    count=collect_offering_indices(
        semester,
        NULL,
        offering_indices,
        MAX_OFFERINGS
    );

    for (index=0; index<count; index++)
    {
        print_offering(
            &offerings[offering_indices[index]],
            index+1
        );
    }

    if (count==0)
    {
        printf("No offerings were found for this semester.\n");
    }
}

static void search_students(void)
{
    char key[STR_SIZE];
    int option;
    int index;
    int found=0;
    int matches;

    printf("\n");
    printf("----------------------------------------\n");
    printf("Search Students\n");
    printf("----------------------------------------\n");
    printf("1. First name\n");
    printf("2. Last name\n");
    printf("3. Student ID\n");
    printf("4. Field\n");
    printf("5. Department\n");

    option=read_int("Enter an option: ");

    if (option<1 || option>5)
    {
        printf("Invalid search option.\n");
        return;
    }

    read_line("Enter search phrase: ",key,sizeof(key));

    if (key[0]=='\0')
    {
        printf("Search phrase cannot be empty.\n");
        return;
    }

    for (index=0; index<student_count; index++)
    {
        matches=0;

        if (option==1)
        {
            matches=contains_ignore_case(students[index].first_name,key);
        }
        else if (option==2)
        {
            matches=contains_ignore_case(students[index].last_name,key);
        }
        else if (option==3)
        {
            matches=contains_ignore_case(students[index].student_id,key);
        }
        else if (option==4)
        {
            matches=contains_ignore_case(students[index].field,key);
        }
        else if (option==5)
        {
            matches=contains_ignore_case(students[index].department,key);
        }

        if (matches)
        {
            printf("\nStudent number %d\n",index+1);

            printf("Name: %s %s\n",students[index].first_name,students[index].last_name);

            printf("Student ID: %s\n",students[index].student_id);

            printf("Field: %s\n",students[index].field);

            printf("Section: %s\n",students[index].section);

            printf("Department: %s\n",students[index].department);
            found=1;
        }
    }

    if (!found)
    {
        printf("No matching student was found.\n");
    }
}

static void search_faculty(void)
{
    char key[STR_SIZE];
    int option;
    int index;
    int found=0;
    int matches;

    printf("\n");
    printf("----------------------------------------\n");
    printf("Search Faculty Members\n");
    printf("----------------------------------------\n");
    printf("1. First name\n");
    printf("2. Last name\n");
    printf("3. Faculty ID\n");
    printf("4. Field\n");
    printf("5. Department\n");

    option=read_int("Enter an option: ");

    if (option<1 || option>5)
    {
        printf("Invalid search option.\n");
        return;
    }

    read_line("Enter search phrase: ",key,sizeof(key));

    if (key[0]=='\0')
    {
        printf("Search phrase cannot be empty.\n");
        return;
    }

    for (index=0; index<faculty_count; index++)
    {
        matches=0;

        if (option==1)
        {
            matches=contains_ignore_case(faculty_members[index].first_name,key);
        }
        else if (option==2)
        {
            matches=contains_ignore_case(faculty_members[index].last_name,key);
        }
        else if (option==3)
        {
            matches=contains_ignore_case(faculty_members[index].faculty_id,key);
        }
        else if (option==4)
        {
            matches=contains_ignore_case(faculty_members[index].field,key);
        }
        else if (option==5)
        {
            matches=contains_ignore_case(faculty_members[index].department,key);
        }

        if (matches)
        {
            printf("\nFaculty member number %d\n",index+1);

            printf("Name: %s %s\n",faculty_members[index].first_name,faculty_members[index].last_name);

            printf("Faculty ID: %s\n",faculty_members[index].faculty_id);

            printf("Field: %s\n",faculty_members[index].field);

            printf("Degree: %s\n",faculty_members[index].degree);

            printf("Department: %s\n",faculty_members[index].department);
            found=1;
        }
    }

    if (!found)
    {
        printf("No matching faculty member was found.\n");
    }
}

static void search_courses(void)
{
    char key[STR_SIZE];
    int option;
    int index;
    int found=0;
    int matches;

    printf("\n");
    printf("----------------------------------------\n");
    printf("Search Courses\n");
    printf("----------------------------------------\n");
    printf("1. Course name\n");
    printf("2. Course ID\n");
    printf("3. Field\n");
    printf("4. Department\n");

    option=read_int("Enter an option: ");

    if (option<1 || option>4)
    {
        printf("Invalid search option.\n");
        return;
    }

    read_line("Enter search phrase: ",key,sizeof(key));

    if (key[0]=='\0')
    {
        printf("Search phrase cannot be empty.\n");
        return;
    }

    for (index=0; index<course_count; index++)
    {
        matches=0;

        if (option==1)
        {
            matches=contains_ignore_case(courses[index].name,key);
        }
        else if (option==2)
        {
            matches=contains_ignore_case(courses[index].course_id,key);
        }
        else if (option==3)
        {
            matches=contains_ignore_case(courses[index].field,key);
        }
        else if (option==4)
        {
            matches=contains_ignore_case(courses[index].department,key);
        }

        if (matches)
        {
            printf("\nCourse number %d\n",index+1);

            printf("Course name: %s\n",courses[index].name);

            printf("Course ID: %s\n",courses[index].course_id);

            printf("Units: %d\n",courses[index].units);

            printf("Prerequisites: %s\n",courses[index].prerequisites);

            printf("Section: %s\n",courses[index].section);

            printf("Field: %s\n",courses[index].field);

            printf("Department: %s\n",courses[index].department);
            found=1;
        }
    }

    if (!found)
    {
        printf("No matching course was found.\n");
    }
}

static void search_offerings(
    int semester_filter,
    const char *faculty_filter
)
{
    char key[STR_SIZE];
    char faculty_name[STR_SIZE*2+2];
    int option;
    int index;
    int course_index;
    int faculty_index;
    int found=0;
    int matches;
    int result_number=0;

    printf("\n");
    printf("----------------------------------------\n");
    printf("Search Course Offerings\n");
    printf("----------------------------------------\n");

    if (semester_filter>0)
    {
        printf("Semester: %d\n", semester_filter);
    }

    printf("1. Course name\n");
    printf("2. Course ID\n");
    printf("3. Faculty ID or name\n");
    printf("4. Department\n");
    printf("5. Place\n");

    option=read_int("Enter an option: ");

    if (option<1 || option>5)
    {
        printf("Invalid search option.\n");
        return;
    }

    read_line("Enter search phrase: ",key,sizeof(key));

    if (key[0]=='\0')
    {
        printf("Search phrase cannot be empty.\n");
        return;
    }

    for (index=0; index<offering_count; index++)
    {
        if (semester_filter>0 &&
            offerings[index].semester!=semester_filter)
        {
            continue;
        }

        if (faculty_filter!=NULL &&
            faculty_filter[0]!='\0' &&
            strcmp(
                offerings[index].faculty_id,
                faculty_filter
            )!=0)
        {
            continue;
        }

        matches=0;

        course_index=find_course_index(offerings[index].course_id);

        faculty_index=find_faculty_index(offerings[index].faculty_id);

        faculty_name[0]='\0';

        if (faculty_index!=-1)
        {
            snprintf(faculty_name,
                sizeof(faculty_name),
                "%s %s",
                faculty_members[faculty_index].first_name,
                faculty_members[faculty_index].last_name);
        }

        if (option==1 && course_index!=-1)
        {
            matches=contains_ignore_case(courses[course_index].name,key);
        }
        else if (option==2)
        {
            matches=contains_ignore_case(offerings[index].course_id,key);
        }
        else if (option==3)
        {
            matches=contains_ignore_case(offerings[index].faculty_id,key) ||
                contains_ignore_case(faculty_name,key);
        }
        else if (option==4)
        {
            matches=contains_ignore_case(offerings[index].department,key);
        }
        else if (option==5)
        {
            matches=contains_ignore_case(offerings[index].place,key);
        }

        if (matches)
        {
            result_number++;

            print_offering(
                &offerings[index],
                result_number
            );

            found=1;
        }
    }

    if (!found)
    {
        printf("No matching offering was found.\n");
    }
}

static void course_catalog_menu(void)
{
    int option;

    while (1)
    {
        printf("\n");
        printf("----------------------------------------\n");
        printf("Course Catalog\n");
        printf("----------------------------------------\n");
        printf("1. List courses\n");
        printf("2. Search courses\n");
        printf("3. Go back\n");

        option=read_int("Enter an option: ");

        if (option==1)
        {
            list_courses();
        }
        else if (option==2)
        {
            search_courses();
        }
        else if (option==3)
        {
            return;
        }
        else
        {
            printf("Invalid option. Please try again.\n");
        }
    }
}

static void add_student_to_offering_admin(int offering_index)
{
    Offering *offering;
    Enrollment *enrollment;
    char student_id[SMALL_SIZE];

    if (offering_index<0 ||
        offering_index>=offering_count)
    {
        printf("Offering not found.\n");
        return;
    }

    read_line(
        "Enter student ID: ",
        student_id,
        sizeof(student_id)
    );

    if (find_student_index(student_id)==-1)
    {
        printf("Student not found.\n");
        return;
    }

    if (offering_has_student(
            offering_index,
            student_id
        )!=-1)
    {
        printf(
            "Student is already enrolled "
            "in this offering.\n"
        );
        return;
    }

    offering=&offerings[offering_index];

    if (offering->enrolled_count>=
        offering->capacity)
    {
        printf("Offering is full.\n");
        return;
    }

    if (offering->enrolled_count>=MAX_ENROLLED)
    {
        printf(
            "Enrollment storage for this "
            "offering is full.\n"
        );
        return;
    }

    enrollment=
        &offering->enrollments[
            offering->enrolled_count
        ];

    memset(
        enrollment,
        0,
        sizeof(*enrollment)
    );

    copy_str(
        enrollment->student_id,
        student_id,
        sizeof(enrollment->student_id)
    );

    enrollment->grade=-1.0;
    enrollment->survey_score=-1;

    offering->enrolled_count++;

    save_all();

    printf(
        "Student added to offering successfully.\n"
    );
}

static void remove_student_from_offering_admin(int offering_index)
{
    Offering *offering;
    char student_id[SMALL_SIZE];
    int enrollment_index;
    int index;

    if (offering_index<0 ||
        offering_index>=offering_count)
    {
        printf("Offering not found.\n");
        return;
    }

    read_line(
        "Enter student ID: ",
        student_id,
        sizeof(student_id)
    );

    enrollment_index=offering_has_student(
        offering_index,
        student_id
    );

    if (enrollment_index==-1)
    {
        printf(
            "Student is not enrolled "
            "in this offering.\n"
        );
        return;
    }

    offering=&offerings[offering_index];

    for (
        index=enrollment_index;
        index<offering->enrolled_count-1;
        index++
    )
    {
        offering->enrollments[index]=
            offering->enrollments[index+1];
    }

    offering->enrolled_count--;

    memset(
        &offering->enrollments[
            offering->enrolled_count
        ],
        0,
        sizeof(
            offering->enrollments[
                offering->enrolled_count
            ]
        )
    );

    save_all();

    printf(
        "Student removed from offering successfully.\n"
    );
}

static void add_capacity_admin_direct(int offering_index)
{
    Offering *offering;
    int amount;

    if (offering_index<0 ||
        offering_index>=offering_count)
    {
        printf("Offering not found.\n");
        return;
    }

    amount=
        read_int("Enter capacity increment: ");

    if (amount<=0)
    {
        printf(
            "Capacity increment must be "
            "greater than zero.\n"
        );
        return;
    }

    offering=&offerings[offering_index];

    if (offering->capacity+amount>MAX_ENROLLED)
    {
        printf(
            "The resulting capacity cannot "
            "be greater than %d.\n",
            MAX_ENROLLED
        );
        return;
    }

    offering->capacity+=amount;

    save_all();

    printf(
        "Capacity updated successfully. "
        "New capacity: %d\n",
        offering->capacity
    );
}

static void admin_offerings_menu(void)
{
    int offering_indices[MAX_OFFERINGS];
    int displayed_count;
    int offering_number;
    int offering_index;
    int semester;
    int option;

    while (1)
    {
        printf("\n");
        printf("----------------------------------------\n");
        printf("Admin: Offering Management\n");
        printf("----------------------------------------\n");

        semester=read_int(
            "Enter semester number "
            "(0 to go back): "
        );

        if (semester==0)
        {
            return;
        }

        if (semester<0)
        {
            printf(
                "Semester number must be "
                "greater than zero.\n"
            );
            continue;
        }

        while (1)
        {
            displayed_count=collect_offering_indices(
                semester,
                NULL,
                offering_indices,
                MAX_OFFERINGS
            );

            list_offerings_by_semester(semester);

            printf("\n");
            printf("1. Search offerings\n");
            printf(
                "2. Add student to an offering\n"
            );
            printf(
                "3. Remove student from an offering\n"
            );
            printf(
                "4. Add capacity to an offering\n"
            );
            printf("5. Go back\n");

            option=read_int("Enter an option: ");

            if (option==1)
            {
                search_offerings(semester,NULL);
            }
            else if (option>=2 && option<=4)
            {
                if (displayed_count==0)
                {
                    printf(
                        "No course offerings are available "
                        "in this semester.\n"
                    );
                    continue;
                }

                offering_number=
                    read_int("Enter offering number: ");

                if (offering_number<1 ||
                    offering_number>displayed_count)
                {
                    printf("Offering not found.\n");
                    continue;
                }

                offering_index=
                    offering_indices[offering_number-1];

                if (option==2)
                {
                    add_student_to_offering_admin(
                        offering_index
                    );
                }
                else if (option==3)
                {
                    remove_student_from_offering_admin(
                        offering_index
                    );
                }
                else
                {
                    add_capacity_admin_direct(
                        offering_index
                    );
                }
            }
            else if (option==5)
            {
                break;
            }
            else
            {
                printf(
                    "Invalid option. "
                    "Please try again.\n"
                );
            }
        }
    }
}

static void list_faculty_offerings(int faculty_index)
{
    int offering_indices[MAX_OFFERINGS];
    int count;
    int index;
    Faculty *faculty=&faculty_members[faculty_index];

    printf("\n");
    printf("----------------------------------------\n");
    printf("My Course Offerings\n");
    printf("----------------------------------------\n");

    count=collect_offering_indices(
        0,
        faculty->faculty_id,
        offering_indices,
        MAX_OFFERINGS
    );

    sort_offering_indices_by_semester(
        offering_indices,
        count
    );

    for (index=0; index<count; index++)
    {
        print_offering(
            &offerings[offering_indices[index]],
            index+1
        );
    }

    if (count==0)
    {
        printf("You do not have any approved offerings.\n");
    }
}

static void faculty_offer_course_request(
    int faculty_index
)
{
    Faculty *faculty;
    Course *course;
    Request *request;
    char course_id[SMALL_SIZE];
    char place[STR_SIZE];
    int course_index;
    int capacity;
    int index;

    faculty=&faculty_members[faculty_index];

    if (calendar_state.offering!=PHASE_ACTIVE)
    {
        printf("Course offering time is not active.\n");
        return;
    }

    if (request_count>=MAX_REQUESTS)
    {
        printf("Request storage is full.\n");
        return;
    }

    printf("\n");
    printf("----------------------------------------\n");
    printf("Request a Course Offering\n");
    printf("----------------------------------------\n");

    list_courses();

    read_line("Enter course ID: ",course_id,sizeof(course_id));

    course_index=find_course_index(course_id);

    if (course_index==-1)
    {
        printf("Course ID not found.\n");
        return;
    }

    course=&courses[course_index];

    if (!course_is_available_for_semester(
        course,
        calendar_state.current_semester))
    {
        if (course->available_from_semester==
            COURSE_AVAILABILITY_NEXT_SEMESTER)
        {
            printf(
                "This course was registered after course "
                "offering started and can be offered from "
                "the next semester.\n"
            );
        }
        else
        {
            printf(
                "This course is available from semester %d.\n",
                course->available_from_semester
            );
        }

        return;
    }

    capacity=read_int("Enter capacity: ");

    if (capacity<=0)
    {
        printf("Capacity must be greater than zero.\n");
        return;
    }

    if (capacity>MAX_ENROLLED)
    {
        printf("Capacity cannot be greater than %d.\n",MAX_ENROLLED);
        return;
    }

    read_line("Enter class place: ",place,sizeof(place));

    if (place[0]=='\0')
    {
        copy_str(place,"TBD",sizeof(place));
    }

    for (index=0; index<request_count; index++)
    {
        if (strcmp(requests[index].type,"offer")==0 &&
            strcmp(requests[index].course_id,course_id)==0 &&
            strcmp(requests[index].faculty_id,faculty->faculty_id)==0 &&
            requests[index].semester==calendar_state.current_semester &&
            strcmp(requests[index].status,"pending")==0)
        {
            printf("A pending request for this offering ""already exists.\n");
            return;
        }
    }

    for (index=0; index<offering_count; index++)
    {
        if (strcmp(offerings[index].course_id,course_id)==0 &&
            strcmp(offerings[index].faculty_id,faculty->faculty_id)==0 &&
            offerings[index].semester==calendar_state.current_semester)
        {
            printf("This course offering already exists.\n");return;
        }
    }

    request=&requests[request_count];

    memset(request,0,sizeof(*request));

    request->id=next_request_id++;

    copy_str(
        request->type,
        "offer",
        sizeof(request->type)
    );

    copy_str(
        request->course_id,
        course_id,
        sizeof(request->course_id)
    );

    copy_str(
        request->faculty_id,
        faculty->faculty_id,
        sizeof(request->faculty_id)
    );

    request->semester=
    calendar_state.current_semester;
    request->capacity=capacity;
    request->amount=0;
    request->offering_index = -1;

    copy_str(
        request->department,
        course->department,
        sizeof(request->department)
    );

    copy_str(
        request->place,
        place,
        sizeof(request->place)
    );

    copy_str(
        request->status,
        "pending",
        sizeof(request->status)
    );

    request_count++;

    save_all();

    printf("\nRequest sent successfully.\n");
    printf("Request ID: %d\n", request->id);
    printf("Semester: %d\n", request->semester);
    printf("Status: %s\n", request->status);
}

static int pending_offering_request_exists(
    const char *type,
    int offering_index
)
{
    int index;

    for (index=0; index<request_count; index++)
    {
        if (strcmp(requests[index].type, type)==0 &&
            requests[index].offering_index==offering_index &&
            strcmp(requests[index].status, "pending")==0)
        {
            return 1;
        }
    }

    return 0;
}

static void faculty_request_capacity(
    int faculty_index,
    int offering_index
)
{
    Faculty *faculty;
    Offering *offering;
    Request *request;
    int amount;

    if (offering_index<0 || offering_index>=offering_count)
    {
        printf("Offering not found.\n");
        return;
    }

    faculty=&faculty_members[faculty_index];
    offering=&offerings[offering_index];

    if (strcmp(offering->faculty_id, faculty->faculty_id)!=0)
    {
        printf("This offering does not belong to you.\n");
        return;
    }

    if (offering->semester!=
    calendar_state.current_semester)
    {
        printf(
            "Capacity can be changed only for offerings "
            "in the current semester (%d).\n",
            calendar_state.current_semester
        );
        return;
    }

    if (request_count>=MAX_REQUESTS)
    {
        printf("Request storage is full.\n");
        return;
    }

    if (pending_offering_request_exists("capacity",offering_index))
    {
        printf(
            "A pending capacity request already exists "
            "for this offering.\n"
        );
        return;
    }

    amount=read_int("Enter capacity increment: ");

    if (amount<=0)
    {
        printf(
            "Capacity increment must be greater than zero.\n"
        );
        return;
    }

    if (offering->capacity+amount>MAX_ENROLLED)
    {
        printf(
            "The resulting capacity cannot be greater than %d.\n",
            MAX_ENROLLED
        );
        return;
    }

    request=&requests[request_count];

    memset(
        request,
        0,
        sizeof(*request)
    );

    request->id=next_request_id++;

    copy_str(
        request->type,
        "capacity",
        sizeof(request->type)
    );

    copy_str(
        request->course_id,
        offering->course_id,
        sizeof(request->course_id)
    );

    copy_str(
        request->faculty_id,
        faculty->faculty_id,
        sizeof(request->faculty_id)
    );

    request->semester=offering->semester;
    request->capacity=offering->capacity;
    request->amount=amount;
    request->offering_index=offering_index;

    copy_str(
        request->department,
        offering->department,
        sizeof(request->department)
    );

    copy_str(
        request->place,
        offering->place,
        sizeof(request->place)
    );

    copy_str(
        request->status,
        "pending",
        sizeof(request->status)
    );

    request_count++;

    save_all();

    printf("Capacity request sent successfully.\n");
    printf("Request ID: %d\n", request->id);
    printf("Requested increment: %d\n", amount);
}

static void faculty_request_removal(
    int faculty_index,
    int offering_index
)
{
    Faculty *faculty;
    Offering *offering;
    Request *request;

    if (offering_index<0 || offering_index>=offering_count)
    {
        printf("Offering not found.\n");
        return;
    }

    faculty=&faculty_members[faculty_index];
    offering=&offerings[offering_index];

    if (strcmp(offering->faculty_id, faculty->faculty_id)!=0)
    {
        printf("This offering does not belong to you.\n");
        return;
    }

    if (offering->semester!=
    calendar_state.current_semester)
    {
        printf(
            "Only an offering from the current semester "
            "(%d) can be removed.\n",
            calendar_state.current_semester
        );
        return;
    }

    if (calendar_state.offering!=PHASE_ACTIVE)
    {
        printf(
    "Removing an offering is allowed only while "
             "the offering period is active.\n");
        return;
    }

    if (request_count>=MAX_REQUESTS)
    {
        printf("Request storage is full.\n");
        return;
    }

    if (pending_offering_request_exists(
            "remove",
            offering_index
        ))
    {
        printf(
            "A pending removal request already exists "
            "for this offering.\n"
        );
        return;
    }

    request=&requests[request_count];

    memset(
        request,
        0,
        sizeof(*request)
    );

    request->id=next_request_id++;

    copy_str(
        request->type,
        "remove",
        sizeof(request->type)
    );

    copy_str(
        request->course_id,
        offering->course_id,
        sizeof(request->course_id)
    );

    copy_str(
        request->faculty_id,
        faculty->faculty_id,
        sizeof(request->faculty_id)
    );

    request->semester=offering->semester;
    request->capacity=offering->capacity;
    request->amount=0;
    request->offering_index=offering_index;

    copy_str(
        request->department,
        offering->department,
        sizeof(request->department)
    );

    copy_str(
        request->place,
        offering->place,
        sizeof(request->place)
    );

    copy_str(
        request->status,
        "pending",
        sizeof(request->status)
    );

    request_count++;

    save_all();

    printf(
        "Offering removal request sent successfully.\n"
    );

    printf("Request ID: %d\n", request->id);
}

static void list_requests(void)
{
    int index;

    printf("\n");
    printf("----------------------------------------\n");
    printf("Course Offering Requests\n");
    printf("----------------------------------------\n");

    if (request_count==0)
    {
        printf("No requests have been submitted.\n");
        return;
    }

    for (index=0; index<request_count; index++)
    {
        printf(
            "\nRequest number %d\n",
            index+1
        );

        printf(
            "Request ID: %d\n",
            requests[index].id
        );

        printf(
            "Type: %s\n",
            requests[index].type
        );

        printf(
            "Course ID: %s\n",
            requests[index].course_id
        );

        printf(
            "Faculty ID: %s\n",
            requests[index].faculty_id
        );

        printf(
            "Semester: %d\n",
            requests[index].semester
        );

        printf(
            "Capacity: %d\n",
            requests[index].capacity
        );

        if (strcmp(requests[index].type, "capacity")==0)
        {
            printf(
                "Requested increment: %d\n",
                requests[index].amount
            );
        }

        if (requests[index].offering_index>=0)
        {
            printf(
                "Offering number: %d\n",
                requests[index].offering_index+1
            );
        }

        printf(
            "Department: %s\n",
            requests[index].department
        );

        printf(
            "Place: %s\n",
            requests[index].place
        );

        printf(
            "Status: %s\n",
            requests[index].status
        );
    }

    printf(
        "\nTotal requests: %d\n",
        request_count
    );
}

static void approve_request(void)
{
    Request *request;
    Course *course;
    Offering *offering;
    int request_id;
    int request_index;
    int course_index;
    int faculty_index;
    int offering_index;
    int index;

    if (request_count==0)
    {
        printf("No requests have been submitted.\n");
        return;
    }

    request_id=read_int("Enter request ID: ");

    request_index=find_request_index(request_id);

    if (request_index==-1)
    {
        printf("Request ID not found.\n");
        return;
    }

    request=&requests[request_index];

    if (strcmp(request->status, "pending")!=0)
    {
        printf(
            "This request has already been processed.\n"
        );
        return;
    }

    if (request->semester!=
    calendar_state.current_semester)
    {
        printf(
            "This request belongs to semester %d and "
            "cannot be approved in semester %d.\n",
            request->semester,
            calendar_state.current_semester
        );
        return;
    }

    if (strcmp(request->type, "offer")==0)
    {
        course_index=find_course_index(
            request->course_id
        );

        faculty_index=find_faculty_index(
            request->faculty_id
        );

        if (course_index==-1)
        {
            printf(
                "The requested course no longer exists.\n"
            );
            return;
        }

        course=&courses[course_index];

        if (!course_is_available_for_semester(
                course,
                request->semester))
        {
            printf(
                "This course is not available for offering "
                "in semester %d.\n",
                request->semester
            );
            return;
        }

        if (faculty_index==-1)
        {
            printf(
                "The faculty member no longer exists.\n"
            );
            return;
        }

        if (offering_count>=MAX_OFFERINGS)
        {
            printf("Offering storage is full.\n");
            return;
        }

        for (index=0; index<offering_count; index++)
        {
            if (strcmp(
                    offerings[index].course_id,
                    request->course_id
                )==0 &&
                strcmp(
                    offerings[index].faculty_id,
                    request->faculty_id
                )==0 &&
                offerings[index].semester==
                    request->semester)
            {
                printf(
                    "This offering already exists.\n"
                );
                return;
            }
        }

        offering=&offerings[offering_count];

        memset(
            offering,
            0,
            sizeof(*offering)
        );

        copy_str(
            offering->course_id,
            request->course_id,
            sizeof(offering->course_id)
        );

        copy_str(
            offering->faculty_id,
            request->faculty_id,
            sizeof(offering->faculty_id)
        );

        offering->semester=request->semester;
        offering->capacity=request->capacity;
        offering->enrolled_count=0;

        copy_str(
            offering->department,
            request->department,
            sizeof(offering->department)
        );

        copy_str(
            offering->place,
            request->place,
            sizeof(offering->place)
        );

        request->offering_index=offering_count;

        offering_count++;

        printf(
            "A new course offering was created.\n"
        );
    }
    else if (strcmp(request->type, "capacity")==0)
    {
        offering_index=request->offering_index;

        if (offering_index<0 ||
            offering_index>=offering_count)
        {
            printf(
                "The offering no longer exists.\n"
            );
            return;
        }

        offering=&offerings[offering_index];

        if (strcmp(
                offering->course_id,
                request->course_id
            )!=0 ||
            strcmp(
                offering->faculty_id,
                request->faculty_id
            )!=0 ||
            offering->semester!=request->semester)
        {
            printf(
                "The offering data no longer matches "
                "this request.\n"
            );
            return;
        }

        if (request->amount<=0 ||
            offering->capacity+request->amount>
                MAX_ENROLLED)
        {
            printf(
                "The requested capacity increment "
                "is invalid.\n"
            );
            return;
        }

        offering->capacity+=request->amount;

        printf(
            "Offering capacity increased to %d.\n",
            offering->capacity
        );
    }
    else if (strcmp(request->type, "remove")==0)
    {
        offering_index=request->offering_index;

        if (offering_index<0 ||
            offering_index>=offering_count)
        {
            printf(
                "The offering no longer exists.\n"
            );
            return;
        }

        offering=&offerings[offering_index];

        if (strcmp(
                offering->course_id,
                request->course_id
            )!=0 ||
            strcmp(
                offering->faculty_id,
                request->faculty_id
            )!=0 ||
            offering->semester!=request->semester)
        {
            printf(
                "The offering data no longer matches "
                "this request.\n"
            );
            return;
        }

        for (
            index=offering_index;
            index<offering_count-1;
            index++
        )
        {
            offerings[index]=offerings[index+1];
        }

        offering_count--;

        memset(
            &offerings[offering_count],
            0,
            sizeof(offerings[offering_count])
        );

        for (index=0; index<request_count; index++)
        {
            if (index==request_index)
            {
                continue;
            }

            if (
                requests[index].offering_index==
                    offering_index &&
                strcmp(
                    requests[index].status,
                    "pending"
                )==0
            )
            {
                copy_str(
                    requests[index].status,
                    "rejected",
                    sizeof(requests[index].status)
                );

                requests[index].offering_index=-1;
            }
            else if (
                requests[index].offering_index>
                    offering_index
            )
            {
                requests[index].offering_index--;
            }
        }

        request->offering_index=-1;

        printf(
            "The course offering was removed.\n"
        );
    }
    else
    {
        printf("Unsupported request type.\n");
        return;
    }

    copy_str(
        request->status,
        "approved",
        sizeof(request->status)
    );

    save_all();

    printf("Request approved successfully.\n");
}
static void reject_request(void)
{
    Request *request;
    int request_id;
    int request_index;

    if (request_count==0)
    {
        printf("No requests have been submitted.\n");
        return;
    }

    request_id=
        read_int("Enter request ID: ");

    request_index=
        find_request_index(request_id);

    if (request_index==-1)
    {
        printf("Request ID not found.\n");
        return;
    }

    request=
        &requests[request_index];

    if (strcmp(request->status, "pending")!=0)
    {
        printf(
            "This request has already been processed.\n"
        );
        return;
    }

    copy_str(
        request->status,
        "rejected",
        sizeof(request->status)
    );

    save_all();

    printf("Request rejected successfully.\n");
}

static void admin_requests_menu(void)
{
    int option;

    while (1)
    {
        printf("\n");
        printf("----------------------------------------\n");
        printf("Admin: Request Management\n");
        printf("----------------------------------------\n");
        printf("1. List requests\n");
        printf("2. Approve a request\n");
        printf("3. Reject a request\n");
        printf("4. Go back\n");

        option =
            read_int("Enter an option: ");

        if (option==1)
        {
            list_requests();
        }
        else if (option==2)
        {
            list_requests();
            approve_request();
        }
        else if (option==3)
        {
            list_requests();
            reject_request();
        }
        else if (option==4)
        {
            return;
        }
        else
        {
            printf(
                "Invalid option. Please try again.\n"
            );
        }
    }
}

static void student_enroll_course(
    int student_index,
    int selected_semester,
    int offering_index
)
{
    Student *student;
    Course *course;
    Offering *offering;
    Enrollment *enrollment;
    int course_index;

    student=&students[student_index];

    if (selected_semester!=
    calendar_state.current_semester)
    {
        printf(
            "Enrollment is available only for the current "
            "semester (%d).\n",
            calendar_state.current_semester
        );
        return;
    }

    if (calendar_state.unit_selection!=PHASE_ACTIVE)
        {
        printf("Unit selection is not active. ""You cannot enroll now.\n");
        return;
        }

    if (offering_index<0 ||
        offering_index>=offering_count)
    {
        printf("Offering not found.\n");
        return;
    }

    offering=&offerings[offering_index];

    if (offering->semester!=
    calendar_state.current_semester)
    {
        printf(
            "The selected offering is not in the current "
            "semester (%d).\n",
            calendar_state.current_semester
        );
        return;
    }

    course_index=
        find_course_index(offering->course_id);

    if (course_index==-1)
    {
        printf("Course data not found.\n");
        return;
    }

    course=&courses[course_index];

    if (offering_has_student(
            offering_index,
            student->student_id
        )!=-1)
    {
        printf(
            "You are already enrolled in this offering.\n"
        );
        return;
    }

    if (offering->enrolled_count>=offering->capacity ||
        offering->enrolled_count>=MAX_ENROLLED)
    {
        printf("This offering is full.\n");
        return;
    }

    if (course->section[0]!='\0' &&
        strcmp(course->section, "General")!=0 &&
        strcmp(course->section, student->section)!=0)
    {
        printf(
            "This course is for section %s, "
            "but your section is %s.\n",
            course->section,
            student->section
        );

        return;
    }

    if (!prerequisites_satisfied(student, course))
    {
        return;
    }

    enrollment=
        &offering->enrollments[offering->enrolled_count];

    memset(
        enrollment,
        0,
        sizeof(*enrollment)
    );

    copy_str(
        enrollment->student_id,
        student->student_id,
        sizeof(enrollment->student_id)
    );

    enrollment->grade=-1.0;

    enrollment->survey_score=-1;

    offering->enrolled_count++;

    save_all();

    printf("\nEnrollment successful.\n");
    printf("Course ID: %s\n", offering->course_id);
    printf(
        "Remaining capacity: %d\n",
        offering->capacity - offering->enrolled_count
    );
}

static void student_withdraw_course(
    int student_index,
    int selected_semester,
    int offering_index
)
{
    Student *student;
    Offering *offering;
    int enrollment_index;
    int index;

    student=&students[student_index];

    if (selected_semester!=
    calendar_state.current_semester)
    {
        printf(
            "Withdrawal is available only for the current "
            "semester (%d).\n",
            calendar_state.current_semester
        );
        return;
    }

    if (calendar_state.unit_selection!=PHASE_ACTIVE)
        {
        printf("Unit selection is not active. ""You cannot withdraw now.\n");
        return;
        }
 
    if (!student_is_enrolled(student->student_id))
    {
        printf(
            "You have not enrolled in any course offerings.\n"
        );

        return;
    }

    if (offering_index<0 ||
        offering_index>=offering_count)
    {
        printf("Offering not found.\n");
        return;
    }

    offering=&offerings[offering_index];

    if (offering->semester!=
    calendar_state.current_semester)
    {
        printf(
            "The selected offering is not in the current "
            "semester (%d).\n",
            calendar_state.current_semester
        );
        return;
    }

    enrollment_index=offering_has_student(
        offering_index,
        student->student_id
    );

    if (enrollment_index==-1)
    {
        printf(
            "You are not enrolled in this offering.\n"
        );
        return;
    }

    if (offering->enrollments[enrollment_index].grade>=0)
    {
        printf(
            "You cannot withdraw after the grade "
            "has been recorded.\n"
        );
        return;
    }

    for (
        index=enrollment_index;
        index<offering->enrolled_count-1;
        index++
    )
    {
        offering->enrollments[index]=
            offering->enrollments[index+1];
    }

    offering->enrolled_count--;

    memset(
        &offering->enrollments[offering->enrolled_count],
        0,
        sizeof(offering->enrollments[offering->enrolled_count])
    );

    save_all();

    printf("Withdrawal successful.\n");
}

static void student_offerings_menu(int student_index)
{
    int offering_indices[MAX_OFFERINGS];
    int displayed_count;
    int offering_number;
    int offering_index;
    int semester;
    int option;

    while (1)
    {
        printf("\n");
        printf("----------------------------------------\n");
        printf("Student: Offerings\n");
        printf("----------------------------------------\n");

        semester=read_int(
            "Enter semester number (0 to go back): "
        );

        if (semester==0)
        {
            return;
        }

        if (semester<0)
        {
            printf(
                "Semester number must be greater than zero.\n"
            );
            continue;
        }

        while (1)
        {
            displayed_count=collect_offering_indices(
                semester,
                NULL,
                offering_indices,
                MAX_OFFERINGS
            );

            list_offerings_by_semester(semester);

            printf("\n");
            printf("1. Search offerings\n");
            printf("2. Enroll in an offering\n");
            printf("3. Withdraw from an offering\n");
            printf("4. Choose another semester\n");

            option=read_int("Enter an option: ");

            if (option==1)
            {
                search_offerings(semester,NULL);
            }
            else if (option==2 || option==3)
            {
                if (displayed_count==0)
                {
                    printf(
                        "No course offerings are available "
                        "in this semester.\n"
                    );
                    continue;
                }

                offering_number=
                    read_int("Enter offering number: ");

                if (offering_number<1 ||
                    offering_number>displayed_count)
                {
                    printf("Offering not found.\n");
                    continue;
                }

                offering_index=
                    offering_indices[offering_number-1];

                if (option==2)
                {
                    student_enroll_course(
                        student_index,
                        semester,
                        offering_index
                    );
                }
                else
                {
                    student_withdraw_course(
                        student_index,
                        semester,
                        offering_index
                    );
                }
            }
            else if (option==4)
            {
                break;
            }
            else
            {
                printf(
                    "Invalid option. Please try again.\n"
                );
            }
        }
    }
}

static void student_course_survey(int student_index)
{
    Student *student;
    Offering *offering;
    Enrollment *enrollment;
    char confirmation[SMALL_SIZE];
    int semester;
    int offering_number;
    int offering_index;
    int enrollment_index;
    int course_index;
    int faculty_index;
    int score;
    int index;
    int found=0;

    student=&students[student_index];

    printf("\n");
    printf("----------------------------------------\n");
    printf("Student: Course Survey\n");
    printf("----------------------------------------\n");

    semester=calendar_state.current_semester;

    printf(
        "\nGraded courses available for survey "
        "in semester %d:\n",
        semester
    );

    for (index=0; index<offering_count; index++)
    {
        if (offerings[index].semester!=semester)
        {
            continue;
        }

        enrollment_index=offering_has_student(
            index,
            student->student_id
        );

        if (enrollment_index==-1)
        {
            continue;
        }

        enrollment=
            &offerings[index]
                .enrollments[enrollment_index];

        if (enrollment->grade<0)
        {
            continue;
        }

        course_index=find_course_index(
            offerings[index].course_id
        );

        faculty_index=find_faculty_index(
            offerings[index].faculty_id
        );

        printf("\nOffering number: %d\n", index + 1);
        printf(
            "Course ID: %s\n",
            offerings[index].course_id
        );

        if (course_index!=-1)
        {
            printf(
                "Course name: %s\n",
                courses[course_index].name
            );
        }

        if (faculty_index!=-1)
        {
            printf(
                "Faculty: %s %s\n",
                faculty_members[faculty_index].first_name,
                faculty_members[faculty_index].last_name
            );
        }

        printf(
            "Grade: %.2f\n",
            enrollment->grade
        );

        if (enrollment->survey_score<0)
        {
            printf("Survey score: Not submitted\n");
        }
        else
        {
            printf(
                "Survey score: %d\n",
                enrollment->survey_score
            );
        }

        found=1;
    }

    if (!found)
    {
        printf(
            "\nNo graded enrolled course is available "
            "for survey in this semester.\n"
        );
        return;
    }

    offering_number=
        read_int("Enter offering number: ");

    offering_index=offering_number-1;

    if (offering_index<0 ||
        offering_index>=offering_count)
    {
        printf("Offering not found.\n");
        return;
    }

    offering=&offerings[offering_index];

    if (offering->semester!=semester)
    {
        printf(
            "The selected offering is not "
            "in this semester.\n"
        );
        return;
    }

    enrollment_index=offering_has_student(
        offering_index,
        student->student_id
    );

    if (enrollment_index==-1)
    {
        printf(
            "You are not enrolled in this offering.\n"
        );
        return;
    }

    enrollment=
        &offering->enrollments[enrollment_index];

    if (enrollment->grade<0)
    {
        printf(
            "You cannot survey a course before "
            "its grade is recorded.\n"
        );
        return;
    }

    if (enrollment->survey_score>=0)
    {
        printf(
            "Your current survey score is %d.\n",
            enrollment->survey_score
        );

        read_line(
            "Replace the previous survey score? "
            "(yes/no): ",
            confirmation,
            sizeof(confirmation)
        );

        if (strcmp(confirmation, "yes")!=0 &&
            strcmp(confirmation, "Yes")!=0 &&
            strcmp(confirmation, "YES")!=0)
        {
            printf(
                "Survey replacement was cancelled.\n"
            );
            return;
        }
    }

    score=
        read_int("Enter survey score (1 to 10): ");

    if (score<1 || score>10)
    {
        printf(
            "Survey score must be between 1 and 10.\n"
        );
        return;
    }

    enrollment->survey_score=score;

    save_all();

    printf("\nSurvey submitted successfully.\n");
    printf("Course ID: %s\n", offering->course_id);
    printf("Survey score: %d\n", score);
}

static void student_dashboard(int student_index)
{
    int option;
    Student *student=&students[student_index];

    while (1)
    {
        printf("\n");
        printf("----------------------------------------\n");
        printf("Student Dashboard\n");
        printf("----------------------------------------\n");
        printf(
            "Welcome %s %s\n",
            student->first_name,
            student->last_name
        );
        printf("Student ID: %s\n", student->student_id);
        printf("\n");
        printf("1. Offerings\n");
        printf("2. Courses\n");
        printf("3. Report card\n");
        printf("4. Course survey\n");
        printf("5. Log out\n");

        option=read_int("Enter an option: ");

        if (option==1)
        {
             student_offerings_menu(student_index);
        }
        else if (option==2)
        {
            course_catalog_menu();
        }
        else if (option==3)
        {
            student_report_card(student_index);
        }
        else if (option==4)
        {
            student_course_survey(student_index);
        }
        else if (option==5)
        {
            printf("Student logged out successfully.\n");
            return;
        }
        else
        {
            printf("Invalid option. Please try again.\n");
        }
    }
}

static void list_offering_students(int offering_index)
{
    Offering *offering;
    int enrollment_index;
    int student_index;

    if (offering_index<0 ||
        offering_index>=offering_count)
    {
        printf("Offering not found.\n");
        return;
    }

    offering=&offerings[offering_index];

    printf("\n");
    printf("----------------------------------------\n");
    printf("Students in Offering\n");
    printf("----------------------------------------\n");

    printf("Course ID: %s\n", offering->course_id);
    printf("Semester: %d\n", offering->semester);

    if (offering->enrolled_count==0)
    {
        printf("No students are enrolled in this offering.\n");
        return;
    }

    for (
        enrollment_index=0;
        enrollment_index<offering->enrolled_count;
        enrollment_index++
    )
    {
        Enrollment *enrollment=
            &offering->enrollments[enrollment_index];

        student_index=find_student_index(
            enrollment->student_id
        );

        printf(
            "\nStudent number %d\n",
            enrollment_index+1
        );

        printf(
            "Student ID: %s\n",
            enrollment->student_id
        );

        if (student_index!=-1)
        {
            printf(
                "Name: %s %s\n",
                students[student_index].first_name,
                students[student_index].last_name
            );
        }

        if (enrollment->grade<0)
        {
            printf("Grade: Not recorded\n");
        }
        else
        {
            printf(
                "Grade: %.2f\n",
                enrollment->grade
            );
        }
    }
}

static void faculty_record_grade_for_offering(
    int faculty_index,
    int offering_index
)
{
    Faculty *faculty;
    Offering *offering;
    char student_id[SMALL_SIZE];
    int enrollment_index;
    double grade;

    if (calendar_state.grade_recording!=PHASE_ACTIVE)
    {
        printf(
        "Grade recording time is not active.\n"
        );
        return;
    }

    if (offering_index<0 ||
        offering_index>=offering_count)
    {
        printf("Offering not found.\n");
        return;
    }

    faculty=&faculty_members[faculty_index];
    offering=&offerings[offering_index];

    if (offering->semester!=
    calendar_state.current_semester)
    {
        printf(
            "Grades can be recorded only for the current "
            "semester (%d).\n",
            calendar_state.current_semester
        );
        return;
    }

    if (strcmp(
            offering->faculty_id,
            faculty->faculty_id
        )!=0)
    {
        printf(
            "This offering does not belong to you.\n"
        );
        return;
    }

    if (offering->enrolled_count==0)
    {
        printf(
            "No students are enrolled in this offering.\n"
        );
        return;
    }

    read_line(
        "Enter student ID: ",
        student_id,
        sizeof(student_id)
    );

    enrollment_index=offering_has_student(
        offering_index,
        student_id
    );

    if (enrollment_index==-1)
    {
        printf(
            "This student is not enrolled "
            "in the selected offering.\n"
        );
        return;
    }

    grade=read_double("Enter grade (0 to 20): ");

    if (grade<0.0 || grade>20.0)
    {
        printf(
            "Grade must be between 0 and 20.\n"
        );
        return;
    }

    offering
        ->enrollments[enrollment_index]
        .grade=grade;

    save_all();

    printf("Grade recorded successfully.\n");
    printf("Student ID: %s\n", student_id);
    printf("Grade: %.2f\n", grade);
}

static void record_grades_from_file(
    int offering_index
)
{
    Offering *offering;
    char path[STR_SIZE];
    char line[LINE_SIZE];
    FILE *file;

    int updated_count=0;
    int invalid_count=0;
    int not_enrolled_count=0;

    if (
        offering_index<0 ||
        offering_index>=offering_count
    )
    {
        printf("Offering not found.\n");
        return;
    }

    offering=&offerings[offering_index];

    if (calendar_state.grade_recording!=PHASE_ACTIVE)
    {
        printf(
            "Grade recording time is not active.\n"
        );
        return;
    }

    if (offering->semester!=
        calendar_state.current_semester)
    {
        printf(
            "Grades can be imported only for the current "
            "semester (%d).\n",
            calendar_state.current_semester
        );
        return;
    }

    read_line(
        "Enter CSV file path (student_id,grade): ",
        path,
        sizeof(path)
    );

    file=fopen(path, "r");

    if (file==NULL)
    {
        printf("Could not open the CSV file.\n");
        return;
    }

    while (fgets(line, sizeof(line), file)!=NULL)
    {
        char *comma;
        char *student_id;
        char *grade_text;
        char *end;

        double grade;
        int enrollment_index;

        line[strcspn(line, "\r\n")]='\0';
        trim(line);

        if (line[0]=='\0')
        {
            continue;
        }

        comma=strchr(line, ',');

        if (
            comma==NULL ||
            strchr(comma+1, ',')!=NULL
        )
        {
            invalid_count++;
            continue;
        }

        *comma='\0';

        student_id=line;
        grade_text=comma+1;

        trim(student_id);
        trim(grade_text);

        if (
            strings_equal_ignore_case(
                student_id,
                "student_id"
            ) &&
            strings_equal_ignore_case(
                grade_text,
                "grade"
            )
        )
        {
            continue;
        }

        if (
            student_id[0]=='\0' ||
            grade_text[0]=='\0'
        )
        {
            invalid_count++;
            continue;
        }

        grade=strtod(grade_text, &end);

        while (
            *end!='\0' &&
            isspace((unsigned char)*end)
        )
        {
            end++;
        }

        if (
            end==grade_text ||
            *end!='\0' ||
            grade!=grade ||
            grade<0.0 ||
            grade>20.0
        )
        {
            invalid_count++;
            continue;
        }

        enrollment_index=offering_has_student(
            offering_index,
            student_id
        );

        if (enrollment_index==-1)
        {
            not_enrolled_count++;
            continue;
        }

        offering
            ->enrollments[enrollment_index]
            .grade=grade;

        updated_count++;
    }

    fclose(file);

    if (updated_count>0)
    {
        save_all();
    }

    printf(
        "Updated %d grade(s).\n",
        updated_count
    );

    if (invalid_count>0)
    {
        printf(
            "Skipped invalid CSV lines: %d\n",
            invalid_count
        );
    }

    if (not_enrolled_count>0)
    {
        printf(
            "Skipped students not enrolled "
            "in this offering: %d\n",
            not_enrolled_count
        );
    }
}

static void faculty_record_grades(
    int faculty_index,
    int offering_index
)
{
    Faculty *faculty;
    Offering *offering;
    int option;

    if (calendar_state.grade_recording!=PHASE_ACTIVE)
    {
        printf(
            "Grade recording time is not active.\n"
        );
        return;
    }

    if (
        offering_index<0 ||
        offering_index>=offering_count
    )
    {
        printf("Offering not found.\n");
        return;
    }

    faculty=&faculty_members[faculty_index];
    offering=&offerings[offering_index];

    if (offering->semester!=
    calendar_state.current_semester)
    {
        printf(
            "Grades can be recorded only for the current "
            "semester (%d).\n",
            calendar_state.current_semester
        );
        return;
    }

    if (
        strcmp(
            offering->faculty_id,
            faculty->faculty_id
        )!=0
    )
    {
        printf(
            "This offering does not belong to you.\n"
        );
        return;
    }

    if (offering->enrolled_count==0)
    {
        printf(
            "No students are enrolled "
            "in this offering.\n"
        );
        return;
    }

    while (1)
    {
        list_offering_students(offering_index);

        printf("\n");
        printf("1. Record one grade\n");
        printf("2. Import grades from CSV\n");
        printf("3. Go back\n");

        option=read_int("Enter an option: ");

        if (option==1)
        {
            faculty_record_grade_for_offering(
                faculty_index,
                offering_index
            );
        }
        else if (option==2)
        {
            record_grades_from_file(
                offering_index
            );
        }
        else if (option==3)
        {
            return;
        }
        else
        {
            printf(
                "Invalid option. Please try again.\n"
            );
        }
    }
}

static void faculty_view_surveys(
    int faculty_index,
    int offering_index
)
{
    Faculty *faculty;
    Offering *offering;
    Enrollment *enrollment;
    int enrollment_index;
    int student_index;
    int survey_count=0;
    double survey_sum=0.0;

    if (offering_index<0 ||
        offering_index>=offering_count)
    {
        printf("Offering not found.\n");
        return;
    }

    faculty=&faculty_members[faculty_index];
    offering=&offerings[offering_index];

    if (strcmp(
            offering->faculty_id,
            faculty->faculty_id
        )!=0)
    {
        printf(
            "This offering does not belong to you.\n"
        );
        return;
    }

    printf("\n");
    printf("----------------------------------------\n");
    printf("Survey Results\n");
    printf("----------------------------------------\n");
    printf("Course ID: %s\n", offering->course_id);
    printf("Semester: %d\n", offering->semester);

    for (
        enrollment_index=0;
        enrollment_index<offering->enrolled_count;
        enrollment_index++
    )
    {
        enrollment=
            &offering->enrollments[enrollment_index];

        if (enrollment->survey_score<0)
        {
            continue;
        }

        student_index=find_student_index(
            enrollment->student_id
        );

        printf(
            "\nStudent ID: %s\n",
            enrollment->student_id
        );

        if (student_index!=-1)
        {
            printf(
                "Student name: %s %s\n",
                students[student_index].first_name,
                students[student_index].last_name
            );
        }
        else
        {
            printf("Student name: Unknown\n");
        }

        printf(
            "Survey score: %d / 10\n",
            enrollment->survey_score
        );

        survey_sum+=enrollment->survey_score;
        survey_count++;
    }

    if (survey_count==0)
    {
        printf(
            "No survey has been submitted "
            "for this offering yet.\n"
        );
        return;
    }

    printf(
        "\nSubmitted surveys: %d\n",
        survey_count
    );

    printf(
        "Average survey score: %.2f / 10\n",
        survey_sum/survey_count
    );
}

static void faculty_manage_offering(int faculty_index)
{
    Faculty *faculty;
    int offering_indices[MAX_OFFERINGS];
    int displayed_count;
    int offering_number;
    int offering_index;
    int option;

    faculty=&faculty_members[faculty_index];

    displayed_count=collect_offering_indices(
        0,
        faculty->faculty_id,
        offering_indices,
        MAX_OFFERINGS
    );

    sort_offering_indices_by_semester(
        offering_indices,
        displayed_count
    );

    list_faculty_offerings(faculty_index);

    if (displayed_count==0)
    {
        return;
    }

    offering_number=
        read_int("Enter offering number: ");

    if (offering_number<1 ||
        offering_number>displayed_count)
    {
        printf("Offering not found.\n");
        return;
    }

    offering_index=
        offering_indices[offering_number-1];

    while (1)
    {
        print_offering(
            &offerings[offering_index],
            offering_number
        );

        printf("\n");
        printf("1. Add capacity\n");
        printf("2. Record grades\n");
        printf("3. Remove offering\n");
        printf("4. Publish a homework\n");
        printf("5. Publish an exam\n");
        printf("6. View survey results\n");
        printf("7. Go back\n");

        option=read_int("Enter an option: ");

        if (option==1)
        {
            faculty_request_capacity(
                faculty_index,
                offering_index
            );
        }
        else if (option==2)
        {
            faculty_record_grades(
                faculty_index,
                offering_index);
        }
        else if (option==3)
        {
            faculty_request_removal(
                faculty_index,
                offering_index
            );
        }
        else if (option==4)
        {
            printf(
                "Homework publishing is an optional "
                "LMS feature and is not implemented.\n"
            );
        }
        else if (option==5)
        {
            printf(
                "Exam publishing is an optional "
                "LMS feature and is not implemented.\n"
            );
        }
        else if (option==6)
        {
            faculty_view_surveys(
                faculty_index,
                offering_index
            );
        }
        else if (option==7)
        {
            return;
        }
        else
        {
            printf(
                "Invalid option. Please try again.\n"
            );
        }
    }
}

static void calculate_student_gpa(
    const char *student_id,
    int semester_filter,
    int *enrolled_count_output,
    int *passed_count_output,
    int *failed_count_output,
    double *gpa_output
)
{
    int offering_index;
    int enrollment_index;
    int course_index;
    int units_sum=0;
    double weighted_sum=0.0;

    *enrolled_count_output=0;
    *passed_count_output=0;
    *failed_count_output=0;
    *gpa_output=0.0;

    for (
        offering_index=0;
        offering_index<offering_count;
        offering_index++
    )
    {
        Offering *offering=
            &offerings[offering_index];

        Enrollment *enrollment;

        if (
            semester_filter!=0 &&
            offering->semester!=semester_filter
        )
        {
            continue;
        }

        enrollment_index=offering_has_student(
            offering_index,
            student_id
        );

        if (enrollment_index==-1)
        {
            continue;
        }

        (*enrolled_count_output)++;

        enrollment=
            &offering->enrollments[enrollment_index];

        if (enrollment->grade<0)
        {
            continue;
        }

        if (enrollment->grade>=PASSING_GRADE)
        {
            (*passed_count_output)++;
        }
        else
        {
            (*failed_count_output)++;
        }

        course_index=find_course_index(
            offering->course_id
        );

        if (
            course_index==-1 ||
            courses[course_index].units<=0
        )
        {
            continue;
        }

        weighted_sum+=
            enrollment->grade *
            courses[course_index].units;

        units_sum+=courses[course_index].units;
    }

    if (units_sum>0)
    {
        *gpa_output=
            weighted_sum/units_sum;
    }
}

static void show_semester_report(
    int student_index,
    int semester
)
{
    Student *student;
    Offering *offering;
    Enrollment *enrollment;

    int offering_index;
    int enrollment_index;
    int course_index;
    int faculty_index;

    int enrolled_count;
    int passed_count;
    int failed_count;
    int found=0;

    double gpa;

    if (semester<=0)
    {
        printf(
            "Semester number must be greater than zero.\n"
        );

        return;
    }

    student=&students[student_index];

    printf("\n");
    printf("----------------------------------------\n");
    printf("Semester Report Card\n");
    printf("----------------------------------------\n");

    printf(
        "Student: %s %s\n",
        student->first_name,
        student->last_name
    );

    printf(
        "Student ID: %s\n",
        student->student_id
    );

    printf(
        "Semester: %d\n",
        semester
    );

    for (
        offering_index=0;
        offering_index<offering_count;
        offering_index++
    )
    {
        offering=&offerings[offering_index];

        if (offering->semester!=semester)
        {
            continue;
        }

        enrollment_index=offering_has_student(
            offering_index,
            student->student_id
        );

        if (enrollment_index==-1)
        {
            continue;
        }

        enrollment=
            &offering->enrollments[enrollment_index];

        course_index=find_course_index(
            offering->course_id
        );

        faculty_index=find_faculty_index(
            offering->faculty_id
        );

        printf("\n");
        printf(
            "Course ID: %s\n",
            offering->course_id
        );

        if (course_index!=-1)
        {
            printf(
                "Course name: %s\n",
                courses[course_index].name
            );

            printf(
                "Units: %d\n",
                courses[course_index].units
            );
        }
        else
        {
            printf("Course name: Unknown\n");
            printf("Units: Unknown\n");
        }

        if (faculty_index!=-1)
        {
            printf(
                "Instructor: Dr. %s %s\n",
                faculty_members[
                    faculty_index
                ].first_name,
                faculty_members[
                    faculty_index
                ].last_name
            );
        }
        else
        {
            printf("Instructor: Unknown\n");
        }

        if (enrollment->grade<0)
        {
            printf("Grade: Not recorded\n");
            printf("Passed: Not available\n");
        }
        else
        {
            printf(
                "Grade: %.2f\n",
                enrollment->grade
            );

            printf(
                "Passed: %s\n",
                enrollment->grade>=PASSING_GRADE
                    ? "Yes"
                    : "No"
            );
        }

        found=1;
    }

    if (!found)
    {
        printf(
            "\nNo enrolled courses were found "
            "for this semester.\n"
        );

        return;
    }

    calculate_student_gpa(
        student->student_id,
        semester,
        &enrolled_count,
        &passed_count,
        &failed_count,
        &gpa
    );

    printf("\nSemester summary:\n");

    printf(
        "Enrolled courses: %d\n",
        enrolled_count
    );

    printf(
        "Passed courses: %d\n",
        passed_count
    );

    printf(
        "Failed courses: %d\n",
        failed_count
    );

    printf(
        "Semester GPA: %.2f\n",
        gpa
    );
}

static void search_passed_courses(int student_index)
{
    Student *student;
    Offering *offering;
    Enrollment *enrollment;
    Course *course;
    Faculty *faculty;
    char key[STR_SIZE];
    char faculty_name[STR_SIZE*2+2];
    int option;
    int semester=0;
    int offering_index;
    int enrollment_index;
    int course_index;
    int faculty_index;
    int found=0;
    int matches;

    student=&students[student_index];

    printf("\n");
    printf("----------------------------------------\n");
    printf("Search Passed Courses\n");
    printf("----------------------------------------\n");
    printf("1. Course name\n");
    printf("2. Course ID\n");
    printf("3. Semester\n");
    printf("4. Faculty ID or name\n");

    option=read_int("Enter an option: ");

    if (option<1 || option>4)
    {
        printf("Invalid search option.\n");
        return;
    }

    if (option==3)
    {
        semester=read_int("Enter semester number: ");

        if (semester<=0)
        {
            printf(
                "Semester number must be greater than zero.\n"
            );
            return;
        }
    }
    else
    {
        read_line(
            "Enter search phrase: ",
            key,
            sizeof(key)
        );

        if (key[0]=='\0')
        {
            printf("Search phrase cannot be empty.\n");
            return;
        }
    }

    for (
        offering_index=0;
        offering_index<offering_count;
        offering_index++
    )
    {
        offering=&offerings[offering_index];

        enrollment_index=offering_has_student(
            offering_index,
            student->student_id
        );

        if (enrollment_index==-1)
        {
            continue;
        }

        enrollment=
            &offering->enrollments[enrollment_index];

        if (enrollment->grade<PASSING_GRADE)
        {
            continue;
        }

        course_index=find_course_index(
            offering->course_id
        );

        faculty_index=find_faculty_index(
            offering->faculty_id
        );

        course=NULL;
        faculty=NULL;
        faculty_name[0]='\0';

        if (course_index!=-1)
        {
            course=&courses[course_index];
        }

        if (faculty_index!=-1)
        {
            faculty=&faculty_members[faculty_index];

            snprintf(
                faculty_name,
                sizeof(faculty_name),
                "%s %s",
                faculty->first_name,
                faculty->last_name
            );
        }

        matches=0;

        if (option==1 && course!=NULL)
        {
            matches=contains_ignore_case(
                course->name,
                key
            );
        }
        else if (option==2)
        {
            matches=contains_ignore_case(
                offering->course_id,
                key
            );
        }
        else if (option==3)
        {
            matches=offering->semester==semester;
        }
        else if (option==4)
        {
            matches=contains_ignore_case(
                    offering->faculty_id,
                    key
                ) ||
                contains_ignore_case(
                    faculty_name,
                    key
                );
        }

        if (!matches)
        {
            continue;
        }

        printf("\nPassed course number %d\n",found+1);
        printf("Course ID: %s\n",offering->course_id);

        if (course!=NULL)
        {
            printf("Course name: %s\n",course->name);
            printf("Units: %d\n",course->units);
        }
        else
        {
            printf("Course name: Unknown\n");
            printf("Units: Unknown\n");
        }

        printf("Semester: %d\n",offering->semester);
        printf("Grade: %.2f\n",enrollment->grade);
        printf("Faculty ID: %s\n",offering->faculty_id);

        if (faculty!=NULL)
        {
            printf(
                "Faculty name: %s %s\n",
                faculty->first_name,
                faculty->last_name
            );
        }
        else
        {
            printf("Faculty name: Unknown\n");
        }

        found++;
    }

    if (!found)
    {
        printf("No matching passed course was found.\n");
    }
}

static void student_report_card(int student_index)
{
    Student *student=
        &students[student_index];

    int enrolled_count;
    int passed_count;
    int failed_count;
    int option;
    int semester;

    double gpa;

    while (1)
    {
        calculate_student_gpa(
            student->student_id,
            0,
            &enrolled_count,
            &passed_count,
            &failed_count,
            &gpa
        );

        printf("\n");
        printf("----------------------------------------\n");
        printf("Student Report Card\n");
        printf("----------------------------------------\n");

        printf(
            "Student ID: %s\n",
            student->student_id
        );

        printf(
            "First name: %s\n",
            student->first_name
        );

        printf(
            "Last name: %s\n",
            student->last_name
        );

        printf(
            "National code: %s\n",
            student->national_code
        );

        printf(
            "Field: %s\n",
            student->field
        );

        printf(
            "Entrance year: %d\n",
            student->entrance_year
        );

        printf(
            "Section: %s\n",
            student->section
        );

        printf(
            "Mentor: %s\n",
            student->mentor
        );

        printf(
            "Department: %s\n",
            student->department
        );

        printf(
            "Overall enrolled courses: %d\n",
            enrolled_count
        );

        printf(
            "Overall passed courses: %d\n",
            passed_count
        );

        printf(
            "Overall failed courses: %d\n",
            failed_count
        );

        printf(
            "Overall GPA: %.2f\n",
            gpa
        );

        printf("\n");
        printf("1. View a semester report\n");
        printf("2. Search passed courses\n");
        printf("3. Go back\n");

        option=read_int("Enter an option: ");

        if (option==1)
        {
            semester=
                read_int("Enter semester number: ");

            show_semester_report(
                student_index,
                semester
            );
        }
        else if (option==2)
        {
            search_passed_courses(student_index);
        }
        else if (option==3)
        {
            return;
        }
        else
        {
            printf(
                "Invalid option. Please try again.\n"
            );
        }
    }
}

static void faculty_dashboard(int faculty_index)
{
    int option;
    int submenu_option;
    int semester;

    Faculty *faculty=
        &faculty_members[faculty_index];

    while (1)
    {
        printf("\n");
        printf("----------------------------------------\n");
        printf("Faculty Dashboard\n");
        printf("----------------------------------------\n");

        printf(
            "Welcome Professor %s %s\n",
            faculty->first_name,
            faculty->last_name
        );

        printf(
            "Faculty ID: %s\n",
            faculty->faculty_id
        );

        printf("\n");
        printf("1. My offerings\n");
        printf("2. List of offerings in semester\n");
        printf("3. Courses\n");
        printf("4. Offer a course\n");
        printf("5. Log out\n");

        option=read_int("Enter an option: ");

        if (option==1)
        {
            list_faculty_offerings(faculty_index);

            printf("\n1. Manage an offering\n");
            printf("2. Search offerings\n");
            printf("3. Go back\n");

            submenu_option=
                read_int("Enter an option: ");

            if (submenu_option==1)
            {
                faculty_manage_offering(
                    faculty_index
                );
            }
            else if (submenu_option==2)
            {
                search_offerings(0,faculty->faculty_id);
            }
            else if (submenu_option!=3)
            {
                printf("Invalid option.\n");
            }
        }
        else if (option==2)
        {
            semester=read_int("Enter semester number: ");

            list_offerings_by_semester(semester);

            printf("\n1. Search offerings\n");
            printf("2. Go back\n");

            submenu_option=read_int("Enter an option: ");

            if (submenu_option==1)
            {
                search_offerings(semester,NULL);
            }
            else if (submenu_option!=2)
            {
                printf("Invalid option.\n");
            }
        }
        else if (option==3)
        {
            course_catalog_menu();
        }
        else if (option==4)
        {
            faculty_offer_course_request(
                faculty_index
            );
        }
        else if (option==5)
        {
            printf(
                "Faculty member logged out successfully.\n"
            );
            return;
        }
        else
        {
            printf(
                "Invalid option. Please try again.\n"
            );
        }
    }
}

static void list_students(void)
{
    int index;

    printf("\n");
    printf("----------------------------------------\n");
    printf("Students List\n");
    printf("----------------------------------------\n");

    if (student_count==0)
    {
        printf("No students have been registered.\n");
        return;
    }

    for (index=0; index<student_count; index++)
    {
        printf("\nStudent number %d\n", index+1);
        printf(
            "Name: %s %s\n",
            students[index].first_name,
            students[index].last_name
        );
        printf(
            "Student ID: %s\n",
            students[index].student_id
        );
        printf(
            "National code: %s\n",
            students[index].national_code
        );
        printf(
            "Field: %s\n",
            students[index].field
        );
        printf(
            "Entrance year: %d\n",
            students[index].entrance_year
        );
        printf(
            "Section: %s\n",
            students[index].section
        );
        printf(
            "Mentor: %s\n",
            students[index].mentor
        );
        printf(
            "Department: %s\n",
            students[index].department
        );
    }

    printf("\nTotal students: %d\n", student_count);
}

static void register_student(void)
{
    Student *student;
    char student_id[SMALL_SIZE];

    if (student_count>=MAX_STUDENTS)
    {
        printf("Student storage is full.\n");
        return;
    }

    printf("\n");
    printf("----------------------------------------\n");
    printf("Register New Student\n");
    printf("----------------------------------------\n");

    read_line(
        "Student ID: ",
        student_id,
        sizeof(student_id)
    );

    if (student_id[0]=='\0')
    {
        printf("Student ID cannot be empty.\n");
        return;
    }

    if (!is_digits_only(student_id))
    {
        printf("Student ID must contain only digits.\n");
        return;
    }

    if (find_student_index(student_id)!=-1)
    {
        printf("This student ID already exists.\n");
        return;
    }

    student=&students[student_count];

    memset(student, 0, sizeof(*student));

    copy_str(
        student->student_id,
        student_id,
        sizeof(student->student_id)
    );

    read_line(
        "First name: ",
        student->first_name,
        sizeof(student->first_name)
    );

    read_line(
        "Last name: ",
        student->last_name,
        sizeof(student->last_name)
    );

    read_line(
        "National code: ",
        student->national_code,
        sizeof(student->national_code)
    );

    read_line(
        "Field: ",
        student->field,
        sizeof(student->field)
    );

    student->entrance_year=
        read_int("Entrance year: ");

    read_line(
        "Section: ",
        student->section,
        sizeof(student->section)
    );

    read_line(
        "Mentor: ",
        student->mentor,
        sizeof(student->mentor)
    );

    read_line(
        "Department: ",
        student->department,
        sizeof(student->department)
    );

    read_line(
        "Password: ",
        student->password,
        sizeof(student->password)
    );

    read_line(
        "Where were you born? ",
        student->answer_birth,
        sizeof(student->answer_birth)
    );

    read_line(
    "What was the name of your first school? ",
    student->answer_school,
    sizeof(student->answer_school)
    );

    read_line(
        "What was the title of the first book you read? ",
        student->answer_book,
        sizeof(student->answer_book)
    );

    read_line(
        "What was the color of your first bicycle? ",
        student->answer_bike,
        sizeof(student->answer_bike)
    );

    if (!student_record_is_valid(student))
    {
        printf(
            "All fields must be valid and section must be "
            "BSc, MSc or PhD.\n"
        );
        memset(student,0,sizeof(*student));
        return;
    }

    if (national_code_exists(student->national_code))
    {
        printf("This national code already exists.\n");
        memset(student,0,sizeof(*student));
        return;
    }

    student_count++;

        save_all();

    printf("\nStudent registered successfully.\n");
    printf("Student ID: %s\n", student->student_id);
}

static void import_students_file(void)
{
    char path[STR_SIZE];
    char line[LINE_SIZE];
    FILE *file;
    int imported=0;
    int duplicate_count=0;
    int invalid_count=0;

    read_line(
        "Enter student CSV file path: ",
        path,
        sizeof(path)
    );

    file=fopen(path, "r");

    if (file==NULL)
    {
        printf("Could not open the CSV file.\n");
        return;
    }

    while (
        fgets(line, sizeof(line), file)!=NULL &&
        student_count<MAX_STUDENTS
    )
    {
        Student student;
        char *fields[14];
        char extra[2];
        int field_count;
        int entrance_year;
        int index;

        line[strcspn(line, "\r\n")]='\0';
        trim(line);

        if (line[0]=='\0')
        {
            continue;
        }

        field_count=parse_csv_line(
            line,
            fields,
            14
        );

        if (
            field_count>0 &&
            (unsigned char)fields[0][0]==0xEF &&
            (unsigned char)fields[0][1]==0xBB &&
            (unsigned char)fields[0][2]==0xBF
        )
        {
            memmove(
                fields[0],
                fields[0]+3,
                strlen(fields[0]+3)+1
            );
        }

        if (
            field_count>=3 &&
            strings_equal_ignore_case(
                fields[0],
                "first_name"
            ) &&
            strings_equal_ignore_case(
                fields[1],
                "last_name"
            ) &&
            strings_equal_ignore_case(
                fields[2],
                "student_id"
            )
        )
        {
            continue;
        }

        if (field_count!=14)
        {
            invalid_count++;
            continue;
        }

        for (index=0; index<14; index++)
        {
            if (fields[index][0]=='\0')
            {
                break;
            }
        }

        if (index<14 ||
            sscanf(
                fields[5],
                "%d %1s",
                &entrance_year,
                extra
            )!=1)
        {
            invalid_count++;
            continue;
        }

        if (find_student_index(fields[2])!=-1 ||
            national_code_exists(fields[3]))
        {
            duplicate_count++;
            continue;
        }

        memset(
            &student,
            0,
            sizeof(student)
        );

        copy_str(
            student.first_name,
            fields[0],
            sizeof(student.first_name)
        );

        copy_str(
            student.last_name,
            fields[1],
            sizeof(student.last_name)
        );

        copy_str(
            student.student_id,
            fields[2],
            sizeof(student.student_id)
        );

        copy_str(
            student.national_code,
            fields[3],
            sizeof(student.national_code)
        );

        copy_str(
            student.field,
            fields[4],
            sizeof(student.field)
        );

        student.entrance_year=entrance_year;

        copy_str(
            student.section,
            fields[6],
            sizeof(student.section)
        );

        copy_str(
            student.mentor,
            fields[7],
            sizeof(student.mentor)
        );

        copy_str(
            student.department,
            fields[8],
            sizeof(student.department)
        );

        copy_str(
            student.answer_birth,
            fields[9],
            sizeof(student.answer_birth)
        );

        copy_str(
            student.answer_school,
            fields[10],
            sizeof(student.answer_school)
        );

        copy_str(
            student.answer_book,
            fields[11],
            sizeof(student.answer_book)
        );

        copy_str(
            student.answer_bike,
            fields[12],
            sizeof(student.answer_bike)
        );

        copy_str(
            student.password,
            fields[13],
            sizeof(student.password)
        );

        if (!student_record_is_valid(&student))
        {
            invalid_count++;
            continue;
        }

        students[student_count]=student;
        student_count++;
        imported++;
    }

    fclose(file);

    if (imported>0)
    {
        save_all();
    }

    printf(
        "Imported %d student(s).\n",
        imported
    );

    if (duplicate_count>0)
    {
        printf(
            "Skipped duplicate student IDs or national codes: %d\n",
            duplicate_count
        );
    }

    if (invalid_count>0)
    {
        printf(
            "Skipped invalid CSV lines: %d\n",
            invalid_count
        );
    }

    if (student_count>=MAX_STUDENTS)
    {
        printf("Student storage is full.\n");
    }
}

static void delete_student(void)
{
    char student_id[SMALL_SIZE];
    char confirmation[SMALL_SIZE];
    int student_index;
    int offering_index;
    int enrollment_index;
    int index;

    if (student_count==0)
    {
        printf("No students have been registered.\n");
        return;
    }

    printf("\n");
    printf("----------------------------------------\n");
    printf("Delete Student\n");
    printf("----------------------------------------\n");

    read_line(
        "Enter student ID: ",
        student_id,
        sizeof(student_id)
    );

    student_index=find_student_index(student_id);

    if (student_index==-1)
    {
        printf("Student ID not found.\n");
        return;
    }

    printf("\nStudent information:\n");

    printf(
        "Name: %s %s\n",
        students[student_index].first_name,
        students[student_index].last_name
    );

    printf(
        "Student ID: %s\n",
        students[student_index].student_id
    );

    read_line(
        "Are you sure you want to delete this student? "
        "(yes/no): ",
        confirmation,
        sizeof(confirmation)
    );

    if (!strings_equal_ignore_case(
            confirmation,
            "yes"
        ))
    {
        printf("Student deletion was cancelled.\n");
        return;
    }

    for (
        offering_index=0;
        offering_index<offering_count;
        offering_index++
    )
    {
        enrollment_index=offering_has_student(
            offering_index,
            student_id
        );

        if (enrollment_index==-1)
        {
            continue;
        }

        for (
            index=enrollment_index;
            index<
                offerings[offering_index].enrolled_count-1;
            index++
        )
        {
            offerings[offering_index]
                .enrollments[index]=
                offerings[offering_index]
                    .enrollments[index+1];
        }

        offerings[offering_index].enrolled_count--;

        memset(
            &offerings[offering_index].enrollments[
                offerings[offering_index].enrolled_count
            ],
            0,
            sizeof(
                offerings[offering_index].enrollments[
                    offerings[offering_index].enrolled_count
                ]
            )
        );
    }

    for (
        index=student_index;
        index<student_count-1;
        index++
    )
    {
        students[index]=students[index+1];
    }

    student_count--;

    memset(
        &students[student_count],
        0,
        sizeof(students[student_count])
    );

    save_all();

    printf("Student deleted successfully.\n");
}
static void admin_students_menu(void)
{
    int option;

    while (1)
    {
        printf("\n");
        printf("----------------------------------------\n");
        printf("Admin: Student Management\n");
        printf("----------------------------------------\n");

        printf("1. List students\n");
        printf("2. Search students\n");
        printf("3. Register one student\n");
        printf("4. Import students from CSV\n");
        printf("5. Delete a student\n");
        printf("6. Go back\n");

        option=read_int("Enter an option: ");

        if (option==1)
        {
            list_students();
        }
        else if (option==2)
        {
            search_students();
        }
        else if (option==3)
        {
            register_student();
        }
        else if (option==4)
        {
            import_students_file();
        }
        else if (option==5)
        {
            delete_student();
        }
        else if (option==6)
        {
            return;
        }
        else
        {
            printf(
                "Invalid option. Please try again.\n"
            );
        }
    }
}

static void list_faculty(void)
{
    int index;

    printf("\n");
    printf("----------------------------------------\n");
    printf("Faculty Members List\n");
    printf("----------------------------------------\n");

    if (faculty_count==0)
    {
        printf("No faculty members have been registered.\n");
        return;
    }

    for (index=0; index<faculty_count; index++)
    {
        printf("\nFaculty member number %d\n", index+1);

        printf(
            "Name: %s %s\n",
            faculty_members[index].first_name,
            faculty_members[index].last_name
        );

        printf(
            "Faculty ID: %s\n",
            faculty_members[index].faculty_id
        );

        printf(
            "National code: %s\n",
            faculty_members[index].national_code
        );

        printf(
            "Field: %s\n",
            faculty_members[index].field
        );

        printf(
            "Entrance year: %d\n",
            faculty_members[index].entrance_year
        );

        printf(
            "Degree: %s\n",
            faculty_members[index].degree
        );

        printf(
            "Department: %s\n",
            faculty_members[index].department
        );
    }

    printf("\nTotal faculty members: %d\n", faculty_count);
}

static void register_faculty(void)
{
    Faculty *faculty;
    char faculty_id[SMALL_SIZE];

    if (faculty_count>=MAX_FACULTY)
    {
        printf("Faculty storage is full.\n");
        return;
    }

    printf("\n");
    printf("----------------------------------------\n");
    printf("Register New Faculty Member\n");
    printf("----------------------------------------\n");

    read_line(
        "Faculty ID: ",
        faculty_id,
        sizeof(faculty_id)
    );

    if (faculty_id[0]=='\0')
    {
        printf("Faculty ID cannot be empty.\n");
        return;
    }

    if (find_faculty_index(faculty_id)!=-1)
    {
        printf("This faculty ID already exists.\n");
        return;
    }

    faculty=&faculty_members[faculty_count];

    memset(faculty, 0, sizeof(*faculty));

    copy_str(
        faculty->faculty_id,
        faculty_id,
        sizeof(faculty->faculty_id)
    );

    read_line(
        "First name: ",
        faculty->first_name,
        sizeof(faculty->first_name)
    );

    read_line(
        "Last name: ",
        faculty->last_name,
        sizeof(faculty->last_name)
    );

    read_line(
        "National code: ",
        faculty->national_code,
        sizeof(faculty->national_code)
    );

    read_line(
        "Field: ",
        faculty->field,
        sizeof(faculty->field)
    );

    faculty->entrance_year=
        read_int("Entrance year: ");

    read_line(
        "Degree: ",
        faculty->degree,
        sizeof(faculty->degree)
    );

    read_line(
        "Department: ",
        faculty->department,
        sizeof(faculty->department)
    );

    read_line(
        "Password: ",
        faculty->password,
        sizeof(faculty->password)
    );

    read_line(
        "Where were you born? ",
        faculty->answer_birth,
        sizeof(faculty->answer_birth)
    );

    read_line(
    "What was the name of your first school? ",
    faculty->answer_school,
    sizeof(faculty->answer_school)
    );

    read_line(
         "What was the title of the first book you read? ",
        faculty->answer_book,
        sizeof(faculty->answer_book)
    );

    read_line(
        "What was the color of your first bicycle? ",
        faculty->answer_bike,
        sizeof(faculty->answer_bike)
    );

    if (!faculty_record_is_valid(faculty))
    {
        printf("All faculty fields must be valid.\n");
        memset(faculty,0,sizeof(*faculty));
        return;
    }

    if (national_code_exists(faculty->national_code))
    {
        printf("This national code already exists.\n");
        memset(faculty,0,sizeof(*faculty));
        return;
    }

    faculty_count++;

    save_all();

    printf("\nFaculty member registered successfully.\n");
    printf("Faculty ID: %s\n", faculty->faculty_id);
}

static void import_faculty_file(void)
{
    char path[STR_SIZE];
    char line[LINE_SIZE];
    FILE *file;
    int imported=0;
    int duplicate_count=0;
    int invalid_count=0;

    read_line(
        "Enter faculty CSV file path: ",
        path,
        sizeof(path)
    );

    file=fopen(path, "r");

    if (file==NULL)
    {
        printf("Could not open the CSV file.\n");
        return;
    }

    while (
        fgets(line, sizeof(line), file)!=NULL &&
        faculty_count<MAX_FACULTY
    )
    {
        Faculty faculty;
        char *fields[13];
        char extra[2];
        int field_count;
        int entrance_year;
        int index;

        line[strcspn(line, "\r\n")]='\0';
        trim(line);

        if (line[0]=='\0')
        {
            continue;
        }

        field_count=parse_csv_line(
            line,
            fields,
            13
        );

        if (
            field_count>0 &&
            (unsigned char)fields[0][0]==0xEF &&
            (unsigned char)fields[0][1]==0xBB &&
            (unsigned char)fields[0][2]==0xBF
        )
        {
            memmove(
                fields[0],
                fields[0]+3,
                strlen(fields[0]+3)+1
            );
        }

        if (
            field_count>=3 &&
            strings_equal_ignore_case(
                fields[0],
                "first_name"
            ) &&
            strings_equal_ignore_case(
                fields[1],
                "last_name"
            ) &&
            strings_equal_ignore_case(
                fields[2],
                "faculty_id"
            )
        )
        {
            continue;
        }

        if (field_count!=13)
        {
            invalid_count++;
            continue;
        }

        for (index=0; index<13; index++)
        {
            if (fields[index][0]=='\0')
            {
                break;
            }
        }

        if (index<13 ||
            sscanf(
                fields[5],
                "%d %1s",
                &entrance_year,
                extra
            )!=1)
        {
            invalid_count++;
            continue;
        }

        if (find_faculty_index(fields[2])!=-1 ||
            national_code_exists(fields[3]))
        {
            duplicate_count++;
            continue;
        }

        memset(
            &faculty,
            0,
            sizeof(faculty)
        );

        copy_str(
            faculty.first_name,
            fields[0],
            sizeof(faculty.first_name)
        );

        copy_str(
            faculty.last_name,
            fields[1],
            sizeof(faculty.last_name)
        );

        copy_str(
            faculty.faculty_id,
            fields[2],
            sizeof(faculty.faculty_id)
        );

        copy_str(
            faculty.national_code,
            fields[3],
            sizeof(faculty.national_code)
        );

        copy_str(
            faculty.field,
            fields[4],
            sizeof(faculty.field)
        );

        faculty.entrance_year=entrance_year;

        copy_str(
            faculty.degree,
            fields[6],
            sizeof(faculty.degree)
        );

        copy_str(
            faculty.department,
            fields[7],
            sizeof(faculty.department)
        );

        copy_str(
            faculty.password,
            fields[8],
            sizeof(faculty.password)
        );

        copy_str(
            faculty.answer_birth,
            fields[9],
            sizeof(faculty.answer_birth)
        );

        copy_str(
            faculty.answer_school,
            fields[10],
            sizeof(faculty.answer_school)
        );

        copy_str(
            faculty.answer_book,
            fields[11],
            sizeof(faculty.answer_book)
        );

        copy_str(
            faculty.answer_bike,
            fields[12],
            sizeof(faculty.answer_bike)
        );

        if (!faculty_record_is_valid(&faculty))
        {
            invalid_count++;
            continue;
        }

        faculty_members[faculty_count]=faculty;
        faculty_count++;
        imported++;
    }

    fclose(file);

    if (imported>0)
    {
        save_all();
    }

    printf(
        "Imported %d faculty member(s).\n",
        imported
    );

    if (duplicate_count>0)
    {
        printf(
            "Skipped duplicate faculty IDs or national codes: %d\n",
            duplicate_count
        );
    }

    if (invalid_count>0)
    {
        printf(
            "Skipped invalid CSV lines: %d\n",
            invalid_count
        );
    }

    if (faculty_count>=MAX_FACULTY)
    {
        printf("Faculty storage is full.\n");
    }
}

static void delete_faculty(void)
{
    char faculty_id[SMALL_SIZE];
    char confirmation[SMALL_SIZE];
    int faculty_index;
    int index;

    if (faculty_count==0)
    {
        printf("No faculty members have been registered.\n");
        return;
    }

    printf("\n");
    printf("----------------------------------------\n");
    printf("Delete Faculty Member\n");
    printf("----------------------------------------\n");

    read_line(
        "Enter faculty ID: ",
        faculty_id,
        sizeof(faculty_id)
    );

    faculty_index=find_faculty_index(faculty_id);

    if (faculty_index==-1)
    {
        printf("Faculty ID not found.\n");
        return;
    }

        if (faculty_is_in_use(faculty_id))
        {
            printf(
            "This faculty member cannot be deleted because "
            "they have an offering or pending request.\n"
            );
            return;
        }

    printf("\nFaculty member information:\n");

    printf(
        "Name: %s %s\n",
        faculty_members[faculty_index].first_name,
        faculty_members[faculty_index].last_name
    );

    printf(
        "Faculty ID: %s\n",
        faculty_members[faculty_index].faculty_id
    );

    printf(
        "Department: %s\n",
        faculty_members[faculty_index].department
    );

    read_line(
        "Are you sure you want to delete this faculty member? "
        "(yes/no): ",
        confirmation,
        sizeof(confirmation)
    );

    if (strcmp(confirmation, "yes")!=0 &&
        strcmp(confirmation, "Yes")!=0 &&
        strcmp(confirmation, "YES")!=0)
    {
        printf("Faculty deletion was cancelled.\n");
        return;
    }

    for (
        index=faculty_index;
        index<faculty_count-1;
        index++
    )
    {
        faculty_members[index]=
            faculty_members[index+1];
    }

    faculty_count--;

    memset(
        &faculty_members[faculty_count],
        0,
        sizeof(faculty_members[faculty_count])
    );

    save_all();

    printf("Faculty member deleted successfully.\n");
}

static void admin_faculty_menu(void)
{
    int option;

    while (1)
    {
        printf("\n");
        printf("----------------------------------------\n");
        printf("Admin: Faculty Management\n");
        printf("----------------------------------------\n");

        printf("1. List faculty members\n");
        printf("2. Search faculty members\n");
        printf("3. Register one faculty member\n");
        printf("4. Import faculty from CSV\n");
        printf("5. Delete a faculty member\n");
        printf("6. Go back\n");

        option=read_int("Enter an option: ");

        if (option==1)
        {
            list_faculty();
        }
        else if (option==2)
        {
            search_faculty();
        }
        else if (option==3)
        {
            register_faculty();
        }
        else if (option==4)
        {
            import_faculty_file();
        }
        else if (option==5)
        {
            delete_faculty();
        }
        else if (option==6)
        {
            return;
        }
        else
        {
            printf(
                "Invalid option. Please try again.\n"
            );
        }
    }
}

static void list_courses(void)
{
    int index;

    printf("\n");
    printf("----------------------------------------\n");
    printf("Courses List\n");
    printf("----------------------------------------\n");

    if (course_count==0)
    {
        printf("No courses have been registered.\n");
        return;
    }

    for (index=0; index<course_count; index++)
    {
        printf("\nCourse number %d\n", index+1);
        printf("Course name: %s\n", courses[index].name);
        printf("Course ID: %s\n", courses[index].course_id);
        printf("Units: %d\n", courses[index].units);
        printf(
            "Prerequisites: %s\n",
            courses[index].prerequisites
        );
        printf("Section: %s\n", courses[index].section);
        printf("Field: %s\n", courses[index].field);
        printf(
            "Department: %s\n",
            courses[index].department
        );
    }

    printf("\nTotal courses: %d\n", course_count);
}

static void register_course(void)
{
    Course *course;
    char course_id[SMALL_SIZE];
    int units;

    if (course_count>=MAX_COURSES)
    {
        printf("Course storage is full.\n");
        return;
    }

    printf("\n");
    printf("----------------------------------------\n");
    printf("Register New Course\n");
    printf("----------------------------------------\n");

    read_line(
        "Course ID: ",
        course_id,
        sizeof(course_id)
    );

    if (course_id[0]=='\0')
    {
        printf("Course ID cannot be empty.\n");
        return;
    }

    if (find_course_index(course_id)!=-1)
    {
        printf("This course ID already exists.\n");
        return;
    }

    course=&courses[course_count];

    memset(course, 0, sizeof(*course));

    copy_str(
        course->course_id,
        course_id,
        sizeof(course->course_id)
    );

    read_line(
        "Course name: ",
        course->name,
        sizeof(course->name)
    );

    if (course->name[0]=='\0')
    {
        printf("Course name cannot be empty.\n");
        memset(course, 0, sizeof(*course));
        return;
    }

    units=read_int("Number of units: ");

    if (units<=0 || units>6)
    {
        printf(
            "The number of units must be between 1 and 6.\n"
        );

        memset(course, 0, sizeof(*course));
        return;
    }

    course->units=units;

    read_line(
        "Prerequisites: ",
        course->prerequisites,
        sizeof(course->prerequisites)
    );

    if (course->prerequisites[0]=='\0')
    {
        copy_str(
            course->prerequisites,
            "-",
            sizeof(course->prerequisites)
        );
    }

    if (!prerequisites_exist(
            course->prerequisites,
            course->course_id
        ))
    {
        printf(
            "Every prerequisite must be an existing "
            "course and cannot be the course itself.\n"
        );
        memset(course,0,sizeof(*course));
        return;
    }

    read_line(
        "Section: ",
        course->section,
        sizeof(course->section)
    );

    if (!is_valid_section(course->section))
    {
        printf("Section must be BSc, MSc or PhD.\n");
        memset(course,0,sizeof(*course));
        return;
    }

    read_line(
        "Field: ",
        course->field,
        sizeof(course->field)
    );

    read_line(
    "Department: ",
    course->department,
    sizeof(course->department)
);

    if (course->field[0]=='\0' ||
        course->department[0]=='\0')
    {
        printf("Field and department cannot be empty.\n");
        memset(course,0,sizeof(*course));
        return;
    }

    if (calendar_state.offering==PHASE_NOT_STARTED)
    {
        course->available_from_semester=
            calendar_state.current_semester;
    }
    else
    {
        course->available_from_semester=
            COURSE_AVAILABILITY_NEXT_SEMESTER;
    }

    course_count++;

    save_all();

    printf("\nCourse registered successfully.\n");
    printf("Course ID: %s\n", course->course_id);

    if (course->available_from_semester==
        COURSE_AVAILABILITY_NEXT_SEMESTER)
    {
        printf(
            "This course can be offered starting from "
            "the next semester.\n"
        );
    }
    else
    {
        printf(
            "Available from semester: %d\n",
            course->available_from_semester
        );
    }
}

static void delete_course(void)
{
    char course_id[SMALL_SIZE];
    char confirmation[SMALL_SIZE];
    int course_index;
    int index;

    if (course_count==0)
    {
        printf("No courses have been registered.\n");
        return;
    }

    printf("\n");
    printf("----------------------------------------\n");
    printf("Delete Course\n");
    printf("----------------------------------------\n");

    read_line(
        "Enter course ID: ",
        course_id,
        sizeof(course_id)
    );

    course_index=find_course_index(course_id);

    if (course_index==-1)
    {
        printf("Course ID not found.\n");
        return;
    }
 
         if (course_is_in_use(course_id))
        {
            printf(
            "This course cannot be deleted because it is used "
            "in an offering or pending request.\n"
           );
           return;
         }

    printf("\nCourse information:\n");
    printf(
        "Course name: %s\n",
        courses[course_index].name
    );
    printf(
        "Course ID: %s\n",
        courses[course_index].course_id
    );
    printf(
        "Units: %d\n",
        courses[course_index].units
    );

    read_line(
        "Are you sure you want to delete this course? "
        "(yes/no): ",
        confirmation,
        sizeof(confirmation)
    );

    if (strcmp(confirmation, "yes")!= 0&&
        strcmp(confirmation, "Yes")!=0&&
        strcmp(confirmation, "YES")!=0)
    {
        printf("Course deletion was cancelled.\n");
        return;
    }

    for (
        index=course_index;
        index<course_count-1;
        index++
    )
    {
        courses[index]=courses[index+1];
    }

    course_count--;

    memset(
        &courses[course_count],
        0,
        sizeof(courses[course_count])
    );

    save_all();

    printf("Course deleted successfully.\n");
}

static void admin_courses_menu(void)
{
    int option;

    while (1)
    {
        printf("\n");
        printf("----------------------------------------\n");
        printf("Admin: Course Management\n");
        printf("----------------------------------------\n");
        printf("1. List courses\n");
        printf("2. Search courses\n");
        printf("3. Register a course\n");
        printf("4. Delete a course\n");
        printf("5. Go back\n");

        option=read_int("Enter an option: ");

        if (option==1)
        {
            list_courses();
        }
        else if (option==2)
        {
            search_courses();
        }
        else if (option==3)
        {
            register_course();
        }
        else if (option==4)
        {
            delete_course();
        }
        else if (option==5)
        {
            return;
        }
        else
        {
            printf(
                "Invalid option. Please try again.\n"
            );
        }
    }
}

static const char *phase_status(PhaseState state)
{
    if (state==PHASE_NOT_STARTED)
    {
        return "not started";
    }

    if (state==PHASE_ACTIVE)
    {
        return "active";
    }

    return "finished";
}

static int transition_calendar_phase(
    PhaseState *phase,
    int can_start,
    int can_finish,
    const char *phase_name,
    const char *start_error,
    const char *finish_error
)
{
    if (*phase==PHASE_NOT_STARTED)
    {
        if (!can_start)
        {
            printf("%s\n", start_error);
            return 0;
        }

        *phase=PHASE_ACTIVE;

        printf(
            "%s started successfully.\n",
            phase_name
        );

        return 1;
    }

    if (*phase==PHASE_ACTIVE)
    {
        if (!can_finish)
        {
            printf("%s\n", finish_error);
            return 0;
        }

        *phase=PHASE_FINISHED;

        printf(
            "%s finished successfully.\n",
            phase_name
        );

        return 1;
    }

    printf(
        "%s has already finished for semester %d.\n",
        phase_name,
        calendar_state.current_semester
    );

    return 0;
}

static void start_next_semester(void)
{
    int next_semester;
    int index;
    int previous_semester;

    if (calendar_state.grade_recording!=
        PHASE_FINISHED)
    {
        printf(
            "The current semester cannot be closed "
            "before grade recording is finished.\n"
        );

        return;
    }

    next_semester=read_int(
        "Enter the next semester number: "
    );

    if (next_semester<=
        calendar_state.current_semester)
    {
        printf(
            "The next semester number must be "
            "greater than %d.\n",
            calendar_state.current_semester
        );
        return;
    }

    previous_semester=
    calendar_state.current_semester;

    for (index=0; index<course_count; index++)
    {
        if (courses[index].available_from_semester==
            COURSE_AVAILABILITY_NEXT_SEMESTER)
        {
            courses[index].available_from_semester=
                next_semester;
        }
    }

    for (index=0; index<request_count; index++)
    {
        if (requests[index].semester==
                previous_semester &&
            strcmp(
                requests[index].status,
                "pending"
            )==0)
        {
            copy_str(
                requests[index].status,
                "rejected",
                sizeof(requests[index].status)
            );
        }
    }

    calendar_state.current_semester=
        next_semester;

    calendar_state.offering=
        PHASE_NOT_STARTED;

    calendar_state.unit_selection=
        PHASE_NOT_STARTED;

    calendar_state.classes_exams=
        PHASE_NOT_STARTED;

    calendar_state.grade_recording=
        PHASE_NOT_STARTED;

    save_all();

    printf(
        "Semester %d initialized successfully.\n",
        calendar_state.current_semester
    );
}

static void admin_calendar_menu(void)
{
    int option;
    int changed;

    while (1)
    {
        printf("\n");
        printf("----------------------------------------\n");
        printf("Admin: Academic Calendar\n");
        printf("----------------------------------------\n");

        printf(
            "Current semester: %d\n",
            calendar_state.current_semester
        );

        printf(
            "1. Course offering: %s\n",
            phase_status(
                calendar_state.offering
            )
        );

        printf(
            "2. Unit selection: %s\n",
            phase_status(
                calendar_state.unit_selection
            )
        );

        printf(
            "3. Classes and exams: %s\n",
            phase_status(
                calendar_state.classes_exams
            )
        );

        printf(
            "4. Grade recording: %s\n",
            phase_status(
                calendar_state.grade_recording
            )
        );

        printf("5. Start next semester\n");
        printf("6. Go back\n");

        option=read_int("Enter an option: ");
        changed=0;

        if (option==1)
        {
            changed=transition_calendar_phase(
                &calendar_state.offering,
                1,
                1,
                "Course offering",
                "Course offering cannot start now.",
                "Course offering cannot finish now."
            );
        }
        else if (option==2)
        {
            changed=transition_calendar_phase(
                &calendar_state.unit_selection,
                calendar_state.offering!=
                    PHASE_NOT_STARTED,
                calendar_state.offering==
                    PHASE_FINISHED,
                "Unit selection",
                "Unit selection cannot start before "
                "course offering starts.",
                "Unit selection cannot finish before "
                "course offering finishes."
            );
        }
        else if (option==3)
        {
            changed=transition_calendar_phase(
                &calendar_state.classes_exams,
                calendar_state.unit_selection!=
                    PHASE_NOT_STARTED,
                calendar_state.unit_selection==
                    PHASE_FINISHED,
                "Classes and exams",
                "Classes and exams cannot start before "
                "unit selection starts.",
                "Classes and exams cannot finish before "
                "unit selection finishes."
            );
        }
        else if (option==4)
        {
            changed=transition_calendar_phase(
                &calendar_state.grade_recording,
                calendar_state.classes_exams!=
                    PHASE_NOT_STARTED,
                calendar_state.classes_exams==
                    PHASE_FINISHED,
                "Grade recording",
                "Grade recording cannot start before "
                "classes and exams start.",
                "Grade recording cannot finish before "
                "classes and exams finish."
            );
        }
        else if (option==5)
        {
            start_next_semester();
        }
        else if (option==6)
        {
            return;
        }
        else
        {
            printf(
                "Invalid option. Please try again.\n"
            );
        }

        if (changed)
        {
            save_all();
        }
    }
}

static void admin_dashboard(void)
{
    int option;

    while (1)
    {
        printf("\n");
        printf("----------------------------------------\n");
        printf("Admin Dashboard\n");
        printf("----------------------------------------\n");
        printf("Welcome %s\n", ADMIN_USERNAME);
        printf("\n");
        printf("1. Academic calendar\n");
        printf("2. Students\n");
        printf("3. Faculty members\n");
        printf("4. Requests\n");
        printf("5. Offerings\n");
        printf("6. Courses\n");
        printf("7. Log out\n");

        option=read_int("Enter an option: ");

        if (option==1)
        {
            admin_calendar_menu();
        }
        else if (option==2)
        {
            admin_students_menu();
        }
        else if (option==3)
        {
            admin_faculty_menu();
        }
        else if (option==4)
        {
            admin_requests_menu();
        }
        else if (option==5)
        {
            admin_offerings_menu();
        }
        else if (option==6)
        {
            admin_courses_menu();
        }
        else if (option==7)
        {
            printf("Admin logged out successfully.\n");
            return;
        }
        else
        {
            printf("Invalid option. Please try again.\n");
        }
    }
}

static void login_student(void)
{
    char username[SMALL_SIZE];
    char password[STR_SIZE];
    int index;

    read_line(
        "Enter student ID: ",
        username,
        sizeof(username)
    );

    index=find_student_index(username);

    if (index==-1)
    {
        printf("Student ID not found.\n");
        return;
    }

    read_line(
        "Enter password: ",
        password,
        sizeof(password)
    );

    if (strcmp(password, students[index].password)!=0)
    {
        printf("Incorrect password.\n");
        return;
    }

    printf("\nLogin successful.\n");
    student_dashboard(index);
}

static void login_faculty(void)
{
    char username[SMALL_SIZE];
    char password[STR_SIZE];
    int index;

    read_line(
        "Enter faculty ID: ",
        username,
        sizeof(username)
    );

    index=find_faculty_index(username);

    if (index==-1)
    {
        printf("Faculty ID not found.\n");
        return;
    }

    read_line(
        "Enter password: ",
        password,
        sizeof(password)
    );

    if (strcmp(password, faculty_members[index].password)!=0)
    {
        printf("Incorrect password.\n");
        return;
    }

    printf("\nLogin successful.\n");
    faculty_dashboard(index);
}

static void login_admin(void)
{
    char username[SMALL_SIZE];
    char password[STR_SIZE];

    read_line(
        "Enter admin username: ",
        username,
        sizeof(username)
    );

    if (strcmp(username, ADMIN_USERNAME)!=0)
    {
        printf("Admin username not found.\n");
        return;
    }

    read_line(
        "Enter password: ",
        password,
        sizeof(password)
    );

    if (strcmp(password, ADMIN_PASSWORD)!=0)
    {
        printf("Incorrect password.\n");
        return;
    }

    printf("\nLogin successful.\n");
    admin_dashboard();
}

int main(void)
{
    int option;

     if (load_all())
    {
        printf("Data loaded successfully from %s.\n",DATA_FILE);
    }
    else
    {
        printf("No valid data file was found. "
	"Creating sample data.\n");

        initialize_sample_data();

        if (!save_all())
        {
            printf("Warning: sample data could not be saved.\n");
        }
    }

    show_data_summary();

    while (1)
    {
        show_main_menu();
        option = read_int("Enter an option: ");

        if (option==1)
	{
    	    login_student();
        }
        else if (option==2)
        {
            login_faculty();
        }
        else if (option==3)
        {
            login_admin();
        }
        else if (option==4)
        {
            forgot_password_menu();
        }
        else if (option==5)
        {
	    save_all();
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

static int parse_csv_line(
    char *line,
    char *fields[],
    int max_fields
)
{
    char *source;
    char *destination;
    char delimiter;
    int field_count=0;

    if (line==NULL || fields==NULL || max_fields<=0)
    {
        return -1;
    }

    source=line;
    destination=line;

    while (1)
    {
        char *field_start;

        if (field_count>=max_fields)
        {
            return -1;
        }

        while (
            *source!='\0' &&
            isspace((unsigned char)*source)
        )
        {
            source++;
        }

        field_start=destination;
        fields[field_count++]=field_start;

        if (*source=='"')
        {
            int closing_quote_found=0;

            source++;

            while (*source!='\0')
            {
                if (*source=='"')
                {
                    if (source[1]=='"')
                    {
                        *destination++='"';
                        source+=2;
                    }
                    else
                    {
                        source++;
                        closing_quote_found=1;
                        break;
                    }
                }
                else
                {
                    *destination++=*source++;
                }
            }

            if (!closing_quote_found)
            {
                return -1;
            }

            while (
                *source!='\0' &&
                isspace((unsigned char)*source)
            )
            {
                source++;
            }

            delimiter=*source;

            if (delimiter!=',' && delimiter!='\0')
            {
                return -1;
            }
        }
        else
        {
            while (*source!=',' && *source!='\0')
            {
                *destination++=*source++;
            }

            while (
                destination>field_start &&
                isspace((unsigned char)destination[-1])
            )
            {
                destination--;
            }

            delimiter=*source;
        }

        *destination++='\0';

        if (delimiter=='\0')
        {
            break;
        }

        source++;

        if (*source=='\0')
        {
            if (field_count>=max_fields)
            {
                return -1;
            }

            fields[field_count++]=destination;
            *destination='\0';
            break;
        }
    }

    return field_count;
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