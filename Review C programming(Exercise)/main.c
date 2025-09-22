#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#define MAX_LINE 256

void start_menu();
void login_system();
void menu();
void schedule_menu();
void search_schedule_by_day();
void read_schedule_for_student_register();
void read_all_schedule();

int mssv_current;

typedef struct
{
    int mssv;
    char password[256];
} User;

typedef struct
{
    int mssv;
    int class_id;
} Student_Register;

typedef struct
{
    int id;
    char code[10];
    char name[50];
    char day[20];
    char week[50];
    char am_pm[10];
    char room[50];
    char period[50];
    char schedule[100];
} Schedule;
/*-----------------------------------------------------------Read User_Account-----------------------------------------------------------*/
int read_user(char *file_name, User users[], int max_users)
{
    FILE *file;
    file = fopen(file_name, "r");
    if (file == NULL)
    {
        printf("Không thể mở file %s", file_name);
        return 0;
    }
    int count = 0;
    while (count < max_users && fscanf(file, "%d %s", &users[count].mssv, users[count].password) == 2)
    {
        count++;
    }
    fclose(file);
    return count;
}

/*-----------------------------------------------------------Read Student_Register-----------------------------------------------------------*/
int read_register(char *file_name, Student_Register registers[], int max_registers)
{
    FILE *file;
    file = fopen(file_name, "r");
    if (file == NULL)
    {
        printf("Không thể mở file %s", file_name);
        return 0;
    }
    int count = 0;
    while (count < max_registers && fscanf(file, "%d %d", &registers[count].mssv, &registers[count].class_id) == 2)
    {
        count++;
    }
    fclose(file);
    return count;
}

/*-----------------------------------------------------------Read Schedule-----------------------------------------------------------*/
void trim(char *str)
{
    int len = strlen(str);
    while (len > 0 && (str[len - 1] == ' ' || str[len - 1] == '\t' || str[len - 1] == '\n'))
        str[--len] = '\0';

    char *start = str;
    while (*start == ' ' || *start == '\t')
        start++;

    if (start != str)
    {
        memmove(str, start, strlen(start) + 1);
    }
}

void parse_schedule(Schedule *s)
{
    char temp[200];
    strcpy(temp, s->schedule);

    temp[strcspn(temp, ";")] = '\0';

    char *token = strtok(temp, ",");
    char *slots[20];
    int count = 0;

    while (token != NULL)
    {
        slots[count++] = token;
        token = strtok(NULL, ",");
    }

    if (count < 3)
    {
        printf("Lịch học không hợp lệ!\n");
        return;
    }

    int day = slots[0][0] - '0';
    int session = slots[0][1] - '0';
    int start = slots[0][2] - '0';
    int end = slots[1][2] - '0';

    char *room = slots[count - 1];

    const char *thu[] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    if (day >= 2 && day <= 7)
        strcpy(s->day, thu[day - 2]);
    else
        strcpy(s->day, "Unknown");

    if (session == 1)
        strcpy(s->am_pm, "Morning");
    else if (session == 2)
        strcpy(s->am_pm, "Afternoon");
    else
        strcpy(s->am_pm, "Unknown");

    sprintf(s->period, "%d-%d", start, end);

    s->week[0] = '\0';
    for (int i = 2; i < count - 1; i++)
    {
        strcat(s->week, slots[i]);
        strcat(s->week, " ");
    }
    trim(s->week);

    strcpy(s->room, room);
}

int read_schedule(Schedule schedules[], int max, const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Không thể mở file %s!\n", filename);
        return 0;
    }

    int count = 0;
    char line[MAX_LINE];

    while (fgets(line, sizeof(line), file) && count < max)
    {

        sscanf(line, "%d %s", &schedules[count].id, schedules[count].code);

        char *p = strstr(line, schedules[count].code) + strlen(schedules[count].code);
        while (*p == ' ')
            p++;

        char *q = strpbrk(p, "0123456789");
        if (q)
        {
            strncpy(schedules[count].name, p, q - p);
            schedules[count].name[q - p] = '\0';
            trim(schedules[count].name);
            strcpy(schedules[count].schedule, q);
        }
        else
        {
            strcpy(schedules[count].name, p);
            trim(schedules[count].name);
            strcpy(schedules[count].schedule, "");
        }

        schedules[count].schedule[strcspn(schedules[count].schedule, "\n")] = '\0';
        parse_schedule(&schedules[count]);

        count++;
    }

    fclose(file);
    return count;
}
/*-----------------------------------------------------Start Menu------------------------------------------*/
void start_menu()
{
    int choice;
    do
    {
        printf("1. Login\n");
        printf("2. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        printf("-----------------------------------------------------------------------------\n");
        switch (choice)
        {
        case 1:
            login_system();
            break;
        case 2:
            printf("Exiting the program.\n");
            break;
        default:
            printf("Invalid choice. Please try again.\n");
            break;
        }
    } while (choice >= 1);
}

/*-----------------------------------------------------Main Menu------------------------------------------*/
void menu()
{
    int choice;
    printf("Main Menu:\n");
    do
    {
        printf("1. Schedule\n");
        printf("2. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        printf("-----------------------------------------------------------------------------\n");
        switch (choice)
        {
        case 1:
            schedule_menu();
            break;
        case 2:
            printf("Exiting the program.\n");
            start_menu();
            break;
        default:
            printf("Invalid choice. Please try again.\n");
            break;
        }
    } while (choice >= 1);
}

/*-------------------------------LOGIN SYSTEM ----------------------------------*/
void login_system()
{
    User users[1000];
    int user_count = read_user("User-account.txt", users, 1000);
    int mssv_input;
    char password_input[256];
    int count = 0;
    while (count < 3)
    {
        for (int i = 0; i < user_count; i++)
        {
            printf("Enter MSSV: ");
            scanf("%d", &mssv_input);
            printf("Enter Password: ");
            scanf("%s", password_input);
            if (users[i].mssv == mssv_input && strcmp(users[i].password, password_input) == 0)
            {
                mssv_current = mssv_input;
                printf("Login successful!\n");
                menu();
            }
            else
            {
                printf("Login failed!\n");
                count++;
                if (count == 3)
                {
                    printf("Too many failed attempts. Exiting.\n");
                    menu();
                }
            }
        }
        break;
    }
    printf("-----------------------------------------------------------------------------\n");
}

/*-----------------------------------------------------------ROLE_STUDENT-----------------------------------------------------------*/
/*-------------------------------Search Schedule By Day-------------------------------*/
void search_schedule_by_day()
{
    printf("Search Schedule By Day:\n");
    Schedule schedules[1000];
    int schedule_count = read_schedule(schedules, 1000, "course_schedule.txt");
    printf("Nhập ngày: ");
    char day_search[20];
    scanf("%s", day_search);
    int found = 0;
    printf("Class.ID  |  Code  |  Course  |  Day  |  Week  |  Period  |  AM/PM  |  Room  \n");
    for (int i = 0; i < schedule_count; i++)
    {
        if (strcasecmp(schedules[i].day, day_search) == 0)
        {
            printf("%d  |  %s  |  %s  |  %s  |  %s  |  %s  |  %s  |  %s  \n",
                   schedules[i].id, schedules[i].code, schedules[i].name,
                   schedules[i].day, schedules[i].week, schedules[i].period,
                   schedules[i].am_pm, schedules[i].room);
            found = 1;
        }
    }
    if (!found)
    {
        printf("Không tìm thấy lớp có thứ %s\n", day_search);
    }
    printf("-----------------------------------------------------------------------------\n");
}

/*-------------------------------Read Schedule For Student Register-------------------------------*/
void read_schedule_for_student_register()
{
    printf("Schedule For Student:\n");
    Student_Register registers[1000];
    Schedule schedules[1000];
    int register_count = read_register("student_registration.txt", registers, 1000);
    int schedule_count = read_schedule(schedules, 1000, "course_schedule.txt");
    int found = 0;
    printf("Class.ID  |  Code  |  Course  |  Day  |  Week  |  Period  |  AM/PM  |  Room  \n");
    int x = 20191121; // Nhập MSSV của sinh viên hiện tại
    for (int i = 0; i < schedule_count; i++)
    {
        for (int j = 0; j < register_count; j++)
        {
            if (schedules[i].id == registers[j].class_id && registers[j].mssv == mssv_current)
            {
                printf("%d  |  %s  |  %s  |  %s  |  %s  |  %s  |  %s  |  %s  \n",
                       schedules[i].id, schedules[i].code, schedules[i].name,
                       schedules[i].day, schedules[i].week, schedules[i].period,
                       schedules[i].am_pm, schedules[i].room);
                found = 1;
            }
        }
    }
    if (!found)
    {
        printf("Không tìm thấy lớp \n");
    }
    printf("-----------------------------------------------------------------------------\n");
}

/*-------------------------------Read All Schedule-------------------------------*/
void read_all_schedule()
{
    char schedule[13][6][50];
    printf("All Schedule:\n");
    Schedule schedules[1000];
    int schedule_count = read_schedule(schedules, 1000, "course_schedule.txt");
    for (int i = 0; i < 13; i++)
        for (int j = 0; j < 6; j++)
            schedule[i][j][0] = '\0';

    for (int k = 0; k < schedule_count; k++)
    {
        int day_index = -1;
        if (strcmp(schedules[k].day, "Monday") == 0)
            day_index = 0;
        else if (strcmp(schedules[k].day, "Tuesday") == 0)
            day_index = 1;
        else if (strcmp(schedules[k].day, "Wednesday") == 0)
            day_index = 2;
        else if (strcmp(schedules[k].day, "Thursday") == 0)
            day_index = 3;
        else if (strcmp(schedules[k].day, "Friday") == 0)
            day_index = 4;
        else if (strcmp(schedules[k].day, "Saturday") == 0)
            day_index = 5;

        if (day_index == -1)
            continue;

        int start_period = atoi(schedules[k].period);
        int end_period = start_period;

        char *dash = strchr(schedules[k].period, '-');
        if (dash != NULL)
        {
            end_period = atoi(dash + 1);
        }

        for (int period = start_period; period <= end_period; period++)
        {
            int display_period = period;

            if (strcmp(schedules[k].am_pm, "Afternoon") == 0)
            {
                display_period = period + 6;
            }

            if (display_period >= 1 && display_period <= 12)
            {
                snprintf(schedule[display_period][day_index], 50, "%s", schedules[k].room);
            }
        }
    }

    printf("|Monday    |Tuesday   |Wednesday |Thursday  |Friday    |Saturday  |\n");
    for (int i = 1; i <= 12; i++)
    {
        printf("%d|", i);
        for (int d = 0; d < 6; d++)
        {
            if (strlen(schedule[i][d]) > 0)
            {
                printf("%-9s|", schedule[i][d]);
            }
            else
            {
                printf("         |");
            }
        }
        printf("\n");
    }
    printf("-----------------------------------------------------------------------------\n");
}
void schedule_menu()
{
    int choice;
    printf("Schedule Menu:\n");
    do
    {
        printf("1. Search Schedule By Day\n");
        printf("2. View Schedule For Today\n");
        printf("3. All\n");
        printf("4. Return to Main Menu\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        printf("-----------------------------------------------------------------------------\n");
        switch (choice)
        {
        case 1:
            search_schedule_by_day();
            break;
        case 2:
            read_schedule_for_student_register();
            break;
        case 3:
            read_all_schedule();
            break;
        case 4:
            menu();
            break;
        case 5:
            printf("Exiting the program.\n");
            start_menu();
            break;
        default:
            printf("Invalid choice. Please try again.\n");
            break;
        }
    } while (choice >= 1);
}

int main()
{
    User users[1000];
    Student_Register registers[1000];
    Schedule schedules[1000];

    int user_count = read_user("User-account.txt", users, 1000);
    for (int i = 0; i < user_count; i++)
    {
        printf("MSSV: %d, Password: %s\n", users[i].mssv, users[i].password);
    }
    printf("--------------------\n");

    int register_count = read_register("student_registration.txt", registers, 1000);
    for (int i = 0; i < register_count; i++)
    {
        printf("MSSV: %d, Code: %d \n", registers[i].mssv, registers[i].class_id);
    }
    printf("--------------------\n");

    int schedule_count = read_schedule(schedules, 1000, "course_schedule.txt");
    for (int i = 0; i < schedule_count; i++)
    {
        printf("Class.ID  |  Code  |  Course  |  Day  |  Week  |  Period  |  AM/PM  |  Room  \n");
        printf("%d  |  %s  |  %s  |  %s  |  %s  |  %s  |  %s  |  %s  \n",
               schedules[i].id, schedules[i].code, schedules[i].name,
               schedules[i].day, schedules[i].week, schedules[i].period,
               schedules[i].am_pm, schedules[i].room);
    }
    printf("--------------------\n");

    printf("Welcome to the Course Management System\n");
    start_menu();
    return 0;
}