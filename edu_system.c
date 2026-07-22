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

static CalendarState calendar_state={0, 0, 0, 0, 0};

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

static int verify_security_answers(
    const char *expected_birth,
    const char *expected_book,
    const char *expected_bike
);

static void recover_student_password(void);
static void recover_faculty_password(void);
static void forgot_password_menu(void);

static void copy_str(char *destination, const char *source, size_t size);

static int find_student_index(const char *student_id);
static int find_faculty_index(const char *faculty_id);
static int find_course_index(const char *course_id);
static int find_request_index(int request_id);

static void print_offering(const Offering *offering, int number);
static void list_offerings(void);
static void list_offerings_by_semester(int semester);
static void list_faculty_offerings(int faculty_index);

static void search_students(void);
static void search_faculty(void);
static void search_courses(void);
static void search_offerings(void);

static void course_catalog_menu(void);
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

static void faculty_manage_offering(int faculty_index);

static void list_requests(void);
static void approve_request(void);
static void reject_request(void);
static void admin_requests_menu(void);

static const char *calendar_status(int enabled);
static void admin_calendar_menu(void);
static void student_course_survey(int student_index);

static int course_is_in_use(const char *course_id);
static int faculty_is_in_use(const char *faculty_id);

static void initialize_sample_data(void);
static void show_data_summary(void);

static void list_students(void);
static void register_student(void);
static void delete_student(void);
static void admin_students_menu(void);

static void list_faculty(void);
static void register_faculty(void);
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

static void list_student_enrollments(int student_index);
static void student_enroll_course(int student_index);
static void student_withdraw_course(int student_index);
static void student_offerings_menu(int student_index);

static void login_student(void);
static void login_faculty(void);
static void login_admin(void);

static void student_dashboard(int student_index);
static void faculty_dashboard(int faculty_index);
static void admin_dashboard(void);

static void trim(char *text);
static void read_line(const char *prompt, char *output, size_t size);
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

static int verify_security_answers(
    const char *expected_birth,
    const char *expected_book,
    const char *expected_bike)
{
    char birth[STR_SIZE];
    char book[STR_SIZE];
    char bike[STR_SIZE];

    if (expected_birth[0]=='\0' ||
        expected_book[0]=='\0' ||
        expected_bike[0]=='\0')
    {
        printf(
            "Security answers have not been configured "
            "for this account.\n");

        return 0;
    }

    read_line("Where were you born? ", birth, sizeof(birth));

    read_line("What was the first book you read? ", book,sizeof(book));

    read_line("What was the color of your first bicycle? ", bike,sizeof(bike));

    if (!strings_equal_ignore_case(birth,expected_birth) ||
        !strings_equal_ignore_case(book,expected_book) ||
        !strings_equal_ignore_case(bike,expected_bike))
    {
        printf("One or more security answers are incorrect.\n");
        return 0;
    }
    return 1;
}

static void recover_student_password(void)
{
    Student *student;
    char student_id[SMALL_SIZE];
    char new_password[STR_SIZE];
    char confirmation[STR_SIZE];
    int student_index;

    printf("\n");
    printf("----------------------------------------\n");
    printf("Student Password Recovery\n");
    printf("----------------------------------------\n");

    read_line("Enter student ID: ",student_id,sizeof(student_id));

    student_index=find_student_index(student_id);

    if (student_index==-1)
    {
        printf("Student ID not found.\n");
        return;
    }

    student=&students[student_index];

    if (!verify_security_answers(student->answer_birth, student->answer_book, student->answer_bike))
    {
        return;
    }

    read_line("Enter new password: ",new_password,sizeof(new_password));

    if (new_password[0]=='\0')
    {
        printf("Password cannot be empty.\n");
        return;
    }

    read_line("Confirm new password: ",confirmation,sizeof(confirmation));

    if (strcmp(new_password, confirmation)!=0)
    {
        printf("Passwords do not match.\n");
        return;
    }

    copy_str(student->password,new_password,sizeof(student->password));

    save_all();

    printf("Student password changed successfully.\n");
}

static void recover_faculty_password(void)
{
    Faculty *faculty;
    char faculty_id[SMALL_SIZE];
    char new_password[STR_SIZE];
    char confirmation[STR_SIZE];
    int faculty_index;

    printf("\n");
    printf("----------------------------------------\n");
    printf("Faculty Password Recovery\n");
    printf("----------------------------------------\n");

    read_line("Enter faculty ID: ",faculty_id,sizeof(faculty_id));

    faculty_index=find_faculty_index(faculty_id);

    if (faculty_index==-1)
    {
        printf("Faculty ID not found.\n");
        return;
    }

    faculty=&faculty_members[faculty_index];

    if (!verify_security_answers(faculty->answer_birth,faculty->answer_book,faculty->answer_bike))
    {
        return;
    }

    read_line("Enter new password: ",new_password,sizeof(new_password));

    if (new_password[0]=='\0')
    {
        printf("Password cannot be empty.\n");
        return;
    }

    read_line("Confirm new password: ",confirmation,sizeof(confirmation));

    if (strcmp(new_password, confirmation)!=0)
    {
        printf("Passwords do not match.\n");
        return;
    }

    copy_str(faculty->password,new_password,sizeof(faculty->password));

    save_all();

    printf("Faculty password changed successfully.\n");
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
        "\"offering\":%d,"
        "\"unit_selection\":%d,"
        "\"classes_exams\":%d,"
        "\"grade_recording\":%d,"
        "\"course_survey\":%d}",
        calendar_state.offering,
        calendar_state.unit_selection,
        calendar_state.classes_exams,
        calendar_state.grade_recording,
        calendar_state.course_survey
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
            if (next_json_int(&cursor, &calendar_state.offering) &&
                next_json_int(&cursor,&calendar_state.unit_selection) &&
                next_json_int(&cursor,&calendar_state.classes_exams) &&
                next_json_int(&cursor,&calendar_state.grade_recording) &&
                next_json_int(&cursor,&calendar_state.course_survey))
            {
                loaded_any=1;
            }
            else
            {
                invalid_lines++;
            }
        }
        else if (strcmp(record_type, "student")==0 && student_count < MAX_STUDENTS)
        {
            Student student;

            memset(&student, 0, sizeof(student));

            if (
                next_json_string(&cursor,student.first_name,sizeof(student.first_name)) &&
                next_json_string(&cursor,student.last_name,sizeof(student.last_name)) &&
                next_json_string(&cursor,student.student_id,sizeof(student.student_id)) &&
                next_json_string(&cursor,student.national_code,sizeof(student.national_code)) &&
                next_json_string(&cursor,student.field,sizeof(student.field)) &&
                next_json_int(&cursor,&student.entrance_year) &&
                next_json_string(&cursor,student.section,sizeof(student.section)) &&
                next_json_string(&cursor,student.mentor,sizeof(student.mentor)) &&
                next_json_string(&cursor,student.department,sizeof(student.department)) &&
                next_json_string(&cursor,student.answer_birth,sizeof(student.answer_birth)) &&
                next_json_string(&cursor,student.answer_book,sizeof(student.answer_book)) &&
                next_json_string(&cursor,student.answer_bike,sizeof(student.answer_bike)) &&
                next_json_string(&cursor,student.password,sizeof(student.password)))
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
        else if (
            strcmp(record_type, "faculty")==0 &&
            faculty_count < MAX_FACULTY)
        {
            Faculty faculty;

            memset(&faculty, 0, sizeof(faculty));

            if (next_json_string(&cursor,faculty.first_name,sizeof(faculty.first_name)) &&
                next_json_string(&cursor,faculty.last_name,sizeof(faculty.last_name)) &&
                next_json_string(&cursor,faculty.faculty_id,sizeof(faculty.faculty_id)) &&
                next_json_string(&cursor,faculty.national_code,sizeof(faculty.national_code)) &&
                next_json_string(&cursor,faculty.field,sizeof(faculty.field)) &&
                next_json_int(&cursor,&faculty.entrance_year) &&
                next_json_string(&cursor,faculty.degree,sizeof(faculty.degree)) &&
                next_json_string(&cursor,faculty.department,sizeof(faculty.department)) &&
                next_json_string(&cursor,faculty.password,sizeof(faculty.password)) &&
                next_json_string(&cursor,faculty.answer_birth,sizeof(faculty.answer_birth)) &&
                next_json_string(&cursor,faculty.answer_book,sizeof(faculty.answer_book)) &&
                next_json_string(&cursor,faculty.answer_bike,sizeof(faculty.answer_bike)))
            
            {
                faculty_members[faculty_count]=faculty;
                faculty_count++;
                loaded_any=1;
            }
            else
            {
                invalid_lines++;
            }
        }
        else if (strcmp(record_type, "course")==0 &&
            course_count<MAX_COURSES)
        {
            Course course;

            memset(&course, 0, sizeof(course));

            if (next_json_string(&cursor,course.name,sizeof(course.name)) &&
                next_json_string(&cursor,course.course_id,sizeof(course.course_id)) &&
                next_json_int(&cursor,&course.units) &&
                next_json_string(&cursor,course.prerequisites,sizeof(course.prerequisites)) &&
                next_json_string(&cursor, course.section,sizeof(course.section)) &&
                next_json_string(&cursor,course.field,sizeof(course.field)) &&
                next_json_string(&cursor,course.department,sizeof(course.department)))
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

	copy_str(student->answer_birth,"Tehran",sizeof(student->answer_birth));

	copy_str(student->answer_book,"Shahnameh",sizeof(student->answer_book));

	copy_str(student->answer_bike,"Blue",sizeof(student->answer_bike));

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

	copy_str(faculty->answer_birth,"Shiraz",sizeof(faculty->answer_birth));

	copy_str(faculty->answer_book,"Golestan",sizeof(faculty->answer_book));

	copy_str(faculty->answer_bike,"Red",sizeof(faculty->answer_bike));

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

static void list_offerings(void)
{
    int index;

    printf("\n");
    printf("----------------------------------------\n");
    printf("Course Offerings\n");
    printf("----------------------------------------\n");

    if (offering_count==0)
    {
        printf("No course offerings are available.\n");
        return;
    }

    for (index=0; index<offering_count; index++)
    {
        print_offering(
            &offerings[index],
            index+1
        );
    }

    printf(
        "\nTotal offerings: %d\n",
        offering_count
    );
}

static void list_offerings_by_semester(int semester)
{
    int index;
    int found=0;

    printf("\n");
    printf("----------------------------------------\n");
    printf("Course Offerings in Semester %d\n", semester);
    printf("----------------------------------------\n");

    if (semester<=0)
    {
        printf("Semester number must be greater than zero.\n");
        return;
    }

    for (index=0; index<offering_count; index++)
    {
        if (offerings[index].semester==semester)
        {
            print_offering(&offerings[index],index+1);
            found=1;
        }
    }

    if (!found)
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

static void search_offerings(void)
{
    char key[STR_SIZE];
    char faculty_name[STR_SIZE*2+2];
    int option;
    int index;
    int course_index;
    int faculty_index;
    int found=0;
    int matches;

    printf("\n");
    printf("----------------------------------------\n");
    printf("Search Course Offerings\n");
    printf("----------------------------------------\n");
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
            print_offering(&offerings[index],index+1);

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

static void admin_offerings_menu(void)
{
    int option;
    int semester;

    while (1)
    {
        printf("\n");
        printf("----------------------------------------\n");
        printf("Admin: Offering Management\n");
        printf("----------------------------------------\n");
        printf("1. List all offerings\n");
        printf("2. List offerings by semester\n");
        printf("3. Search offerings\n");
        printf("4. Go back\n");

        option=read_int("Enter an option: ");

        if (option==1)
        {
            list_offerings();
        }
        else if (option==2)
        {
            semester=read_int("Enter semester number: ");

            list_offerings_by_semester(semester);
        }
        else if (option==3)
        {
            search_offerings();
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

static void list_faculty_offerings(int faculty_index)
{
    int index;
    int found=0;
    Faculty *faculty=&faculty_members[faculty_index];

    printf("\n");
    printf("----------------------------------------\n");
    printf("My Course Offerings\n");
    printf("----------------------------------------\n");

    for (index=0; index<offering_count; index++)
    {
        if (strcmp(offerings[index].faculty_id,faculty->faculty_id)==0)
        {
            print_offering(&offerings[index],index+1);

            found=1;
        }
    }

    if (!found)
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
    int semester;
    int index;

    faculty=&faculty_members[faculty_index];

        if (!calendar_state.offering)
       {
        printf("Course offering time is disabled.\n");
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

    semester=read_int("Enter semester number: ");

    if (semester<=0)
    {
        printf("Semester number must be greater than zero.\n");
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
            requests[index].semester == semester &&
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
            offerings[index].semester==semester)
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

    request->semester=semester;
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

    if (!calendar_state.offering)
    {
        printf(
            "Removing an offering is allowed only while "
            "the offering period is enabled.\n"
        );
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

static void list_student_enrollments(int student_index)
{
    Student *student;
    int offering_index;
    int enrollment_index;
    int found=0;

    student=&students[student_index];

    printf("\n");
    printf("----------------------------------------\n");
    printf("My Enrolled Courses\n");
    printf("----------------------------------------\n");

    for (
        offering_index=0;
        offering_index<offering_count;
        offering_index++
    )
    {
        enrollment_index=offering_has_student(
            offering_index,
            student->student_id
        );

        if (enrollment_index!=-1)
        {
            Enrollment *enrollment=
                &offerings[offering_index]
                    .enrollments[enrollment_index];

            print_offering(
                &offerings[offering_index],
                offering_index+1
            );

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

            found=1;
        }
    }

    if (!found)
    {
        printf(
            "You have not enrolled in any course offerings.\n"
        );
    }
}

static void student_enroll_course(int student_index)
{
    Student *student;
    Course *course;
    Offering *offering;
    Enrollment *enrollment;
    int offering_number;
    int offering_index;
    int course_index;

    student=&students[student_index];

        if (!calendar_state.unit_selection)
        {
         printf(
        "Unit selection is disabled. "
        "You cannot enroll now.\n"
        );
        return;
        }

    if (offering_count==0)
    {
        printf("No course offerings are available.\n");
        return;
    }

    list_offerings();

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

static void student_withdraw_course(int student_index)
{
    Student *student;
    Offering *offering;
    int offering_number;
    int offering_index;
    int enrollment_index;
    int index;

    student=&students[student_index];

        if (!calendar_state.unit_selection)
        {
        printf(
        "Unit selection is disabled. "
        "You cannot withdraw now.\n"
        );
        return;
        }
 
    if (!student_is_enrolled(student->student_id))
    {
        printf(
            "You have not enrolled in any course offerings.\n"
        );

        return;
    }

    list_student_enrollments(student_index);

    offering_number=
        read_int("Enter offering number to withdraw: ");

    offering_index=offering_number-1;

    if (offering_index<0 ||
        offering_index>=offering_count)
    {
        printf("Offering not found.\n");
        return;
    }

    offering=&offerings[offering_index];

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
    int option;
    int semester;

    while (1)
    {
        printf("\n");
        printf("----------------------------------------\n");
        printf("Student: Course Enrollment\n");
        printf("----------------------------------------\n");
        printf("1. List all offerings\n");
        printf("2. List offerings by semester\n");
        printf("3. Search offerings\n");
        printf("4. Enroll in an offering\n");
        printf("5. List my enrolled courses\n");
        printf("6. Withdraw from an offering\n");
        printf("7. Go back\n");

        option=read_int("Enter an option: ");

        if (option==1)
        {
            list_offerings();
        }
        else if (option==2)
        {
            semester=
                read_int("Enter semester number: ");

            list_offerings_by_semester(semester);
        }
        else if (option==3)
        {
            search_offerings();
        }
        else if (option==4)
        {
            student_enroll_course(student_index);
        }
        else if (option==5)
        {
            list_student_enrollments(student_index);
        }
        else if (option==6)
        {
            student_withdraw_course(student_index);
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

    if (!calendar_state.course_survey)
    {
        printf("Course survey time is disabled.\n");
        return;
    }

    printf("\n");
    printf("----------------------------------------\n");
    printf("Student: Course Survey\n");
    printf("----------------------------------------\n");

    semester=
        read_int("Enter semester number: ");

    if (semester<=0)
    {
        printf(
            "Semester number must be greater than zero.\n"
        );
        return;
    }

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
        printf("1. Course enrollment\n");
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

    if (!calendar_state.grade_recording)
    {
        printf(
            "Grade recording time is disabled.\n"
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

    list_offering_students(offering_index);

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
    int offering_number;
    int offering_index;
    int option;

    faculty=&faculty_members[faculty_index];

    list_faculty_offerings(faculty_index);

    offering_number=
        read_int("Enter offering number: ");

    offering_index=offering_number-1;

    if (offering_index<0 ||
        offering_index>=offering_count ||
        strcmp(
            offerings[offering_index].faculty_id,
            faculty->faculty_id
        )!=0)
    {
        printf("Offering not found.\n");
        return;
    }

    while (1)
    {
        print_offering(
            &offerings[offering_index],
            offering_index+1
        );

        printf("\n");
        printf("1. Add capacity\n");
        printf("2. Record a grade\n");
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
            faculty_record_grade_for_offering(
                faculty_index,
                offering_index
            );
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
                "Homework publishing is a bonus LMS "
                "feature. Placeholder created.\n"
            );
        }
        else if (option==5)
        {
            printf(
                "Exam publishing is a bonus LMS "
                "feature. Placeholder created.\n"
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

static void student_report_card(int student_index)
{
    Student *student;
    Offering *offering;
    Enrollment *enrollment;
    int offering_index;
    int enrollment_index;
    int course_index;
    int units;
    int total_units=0;
    int found=0;
    double weighted_sum=0.0;

    student=&students[student_index];

    printf("\n");
    printf("----------------------------------------\n");
    printf("Student Report Card\n");
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

    for (
        offering_index=0;
        offering_index<offering_count;
        offering_index++
    )
    {
        enrollment_index=offering_has_student(
            offering_index,
            student->student_id
        );

        if (enrollment_index==-1)
        {
            continue;
        }

        found=1;

        offering=&offerings[offering_index];

        enrollment=
            &offering->enrollments[enrollment_index];

        course_index=
            find_course_index(offering->course_id);

        printf("\n");
        printf("Course ID: %s\n", offering->course_id);
        printf("Semester: %d\n", offering->semester);

        units=0;

        if (course_index!=-1)
        {
            units=courses[course_index].units;

            printf(
                "Course name: %s\n",
                courses[course_index].name
            );

            printf("Units: %d\n", units);
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

            if (units>0)
            {
                weighted_sum+=
                    enrollment->grade*units;

                total_units+=units;
            }
        }
    }

    if (!found)
    {
        printf(
            "\nNo courses are available "
            "in your report card.\n"
        );
        return;
    }

    if (total_units>0)
    {
        printf("\n");
        printf(
            "Recorded units: %d\n",
            total_units
        );

        printf(
            "Average: %.2f\n",
            weighted_sum / total_units
        );
    }
    else
    {
        printf(
            "\nNo grades have been recorded yet.\n"
        );
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
        printf("2. Offerings in a semester\n");
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
                search_offerings();
            }
            else if (submenu_option!=3)
            {
                printf("Invalid option.\n");
            }
        }
        else if (option==2)
        {
            semester=
                read_int("Enter semester number: ");

            list_offerings_by_semester(semester);
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

    if (student->password[0]=='\0')
    {
        copy_str(
            student->password,
            "123456",
            sizeof(student->password)
        );
    }

    read_line(
        "Where were you born? ",
        student->answer_birth,
        sizeof(student->answer_birth)
    );

    read_line(
        "What was the first book you read? ",
        student->answer_book,
        sizeof(student->answer_book)
    );

    read_line(
        "What was the color of your first bicycle? ",
        student->answer_bike,
        sizeof(student->answer_bike)
    );

    student_count++;

        save_all();

    printf("\nStudent registered successfully.\n");
    printf("Student ID: %s\n", student->student_id);
}

static void delete_student(void)
{
    char student_id[SMALL_SIZE];
    char confirmation[SMALL_SIZE];
    int student_index;
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

    if (student_is_enrolled(student_id))
    {
        printf(
            "This student cannot be deleted because "
            "they are enrolled in a course offering.\n"
        );

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
        "Are you sure you want to delete this student? (yes/no): ",
        confirmation,
        sizeof(confirmation)
    );

    if (strcmp(confirmation, "yes")!=0 &&
        strcmp(confirmation, "YES")!=0 &&
        strcmp(confirmation, "Yes")!=0)
    {
        printf("Student deletion was cancelled.\n");
        return;
    }

    for (index=student_index; index<student_count-1; index++)
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
        printf("3. Register a student\n");
        printf("4. Delete a student\n");
        printf("5. Go back\n");

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
            delete_student();
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

    if (faculty->password[0]=='\0')
    {
        copy_str(
            faculty->password,
            "123456",
            sizeof(faculty->password)
        );
    }

    read_line(
        "Where were you born? ",
        faculty->answer_birth,
        sizeof(faculty->answer_birth)
    );

    read_line(
        "What was the first book you read? ",
        faculty->answer_book,
        sizeof(faculty->answer_book)
    );

    read_line(
        "What was the color of your first bicycle? ",
        faculty->answer_bike,
        sizeof(faculty->answer_bike)
    );

    faculty_count++;

    save_all();

    printf("\nFaculty member registered successfully.\n");
    printf("Faculty ID: %s\n", faculty->faculty_id);
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
        printf("3. Register a faculty member\n");
        printf("4. Delete a faculty member\n");
        printf("5. Go back\n");

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
            delete_faculty();
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

    if (units<=0)
    {
        printf(
            "The number of units must be greater than zero.\n"
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

    read_line(
        "Section: ",
        course->section,
        sizeof(course->section)
    );

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

    course_count++;

    save_all();

    printf("\nCourse registered successfully.\n");
    printf("Course ID: %s\n", course->course_id);
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

static const char *calendar_status(int enabled)
{
    if (enabled)
    {
        return "enabled";
    }

    return "disabled";
}

static void admin_calendar_menu(void)
{
    int option;

    while (1)
    {
        printf("\n");
        printf("----------------------------------------\n");
        printf("Admin: Academic Calendar\n");
        printf("----------------------------------------\n");

        printf(
            "1. Course offering: %s\n",
            calendar_status(calendar_state.offering)
        );

        printf(
            "2. Unit selection: %s\n",
            calendar_status(calendar_state.unit_selection)
        );

        printf(
            "3. Classes and exams: %s\n",
            calendar_status(calendar_state.classes_exams)
        );

        printf(
            "4. Grade recording: %s\n",
            calendar_status(calendar_state.grade_recording)
        );

        printf(
            "5. Course survey: %s\n",
            calendar_status(calendar_state.course_survey)
        );

        printf("6. Go back\n");

        option=read_int("Enter an option to toggle: ");

        if (option==1)
        {
            calendar_state.offering=
                !calendar_state.offering;

            save_all();

            printf(
                "Course offering is now %s.\n",
                calendar_status(calendar_state.offering)
            );
        }
        else if (option==2)
        {
            calendar_state.unit_selection=
                !calendar_state.unit_selection;

            save_all();

            printf(
                "Unit selection is now %s.\n",
                calendar_status(
                    calendar_state.unit_selection
                )
            );
        }
        else if (option==3)
        {
            calendar_state.classes_exams=
                !calendar_state.classes_exams;

            save_all();

            printf(
                "Classes and exams are now %s.\n",
                calendar_status(
                    calendar_state.classes_exams
                )
            );
        }
        else if (option==4)
        {
            calendar_state.grade_recording=
                !calendar_state.grade_recording;

            save_all();

            printf(
                "Grade recording is now %s.\n",
                calendar_status(
                    calendar_state.grade_recording
                )
            );
        }
        else if (option==5)
        {
            calendar_state.course_survey=
                !calendar_state.course_survey;

            save_all();

            printf(
                "Course survey is now %s.\n",
                calendar_status(
                    calendar_state.course_survey
                )
            );
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