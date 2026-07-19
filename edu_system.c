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
#define ADMIN_USERNAME "admin"
#define ADMIN_PASSWORD "admin123"

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

static void list_students(void);
static void register_student(void);
static void delete_student(void);
static void admin_students_menu(void);

static void list_faculty(void);
static void register_faculty(void);
static void delete_faculty(void);
static void admin_faculty_menu(void);

static void login_student(void);
static void login_faculty(void);
static void login_admin(void);

static void student_dashboard(int student_index);
static void faculty_dashboard(int faculty_index);
static void admin_dashboard(void);

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
            printf("Student offerings will be added later.\n");
        }
        else if (option==2)
        {
            printf("Course list will be added later.\n");
        }
        else if (option==3)
        {
            printf("Report card will be added later.\n");
        }
        else if (option==4)
        {
            printf("Course survey will be added later.\n");
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

static void faculty_dashboard(int faculty_index)
{
    int option;
    Faculty *faculty=&faculty_members[faculty_index];

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
        printf("Faculty ID: %s\n", faculty->faculty_id);
        printf("\n");
        printf("1. My offerings\n");
        printf("2. Offerings in a semester\n");
        printf("3. Courses\n");
        printf("4. Offer a course\n");
        printf("5. Log out\n");

        option=read_int("Enter an option: ");

        if (option==1)
        {
            printf("Faculty offerings will be added later.\n");
        }
        else if (option==2)
        {
            printf("Semester offerings will be added later.\n");
        }
        else if (option==3)
        {
            printf("Course list will be added later.\n");
        }
        else if (option==4)
        {
            printf("Course offering request will be added later.\n");
        }
        else if (option==5)
        {
            printf("Faculty member logged out successfully.\n");
            return;
        }
        else
        {
            printf("Invalid option. Please try again.\n");
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
        printf("2. Register a student\n");
	printf("3. Delete a student\n");
        printf("4. Go back\n");

        option=read_int("Enter an option: ");

        if (option==1)
        {
            list_students();
        }
        else if (option==2)
        {
            register_student();
        }
        else if (option==3)
        {
            delete_student();
        }
	else if (option==4)
	{
    	    return;
	}
        else
        {
            printf("Invalid option. Please try again.\n");
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
	printf("2. Register a faculty member\n");
	printf("3. Delete a faculty member\n");
	printf("4. Go back\n");
        option=read_int("Enter an option: ");

        if (option==1)
        {
            list_faculty();
        }
        else if (option==2)
        {
            register_faculty();
        }
        else if (option==3)
        {
            delete_faculty(); 
        }
	else if (option==4)
	{
  	    return;
	}
        else
        {
            printf("Invalid option. Please try again.\n");
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
            printf("Academic calendar will be added later.\n");
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
            printf("Request management will be added later.\n");
        }
        else if (option==5)
        {
            printf("Offering management will be added later.\n");
        }
        else if (option==6)
        {
            printf("Course management will be added later.\n");
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

    initialize_sample_data();
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
