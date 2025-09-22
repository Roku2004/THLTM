#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define MAX_LINE 256

typedef struct {
    int mssv;
    char password[256];
} User;

typedef struct {
    int mssv;
    int class_id;
}Student_Register;

typedef struct {
    char id[10];    
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
int read_user(char *file_name, User users[], int max_users) {
    FILE *file;
    file = fopen(file_name, "r");
    if (file == NULL) {
        printf("Không thể mở file %s", file_name);
        return 0;
    }
    int count = 0;
    while(count<max_users && fscanf(file,"%d %s", &users[count].mssv, users[count].password) == 2) {
        count++;
    }
    fclose(file);
    return count;
}

/*-----------------------------------------------------------Read Student_Register-----------------------------------------------------------*/
int read_register(char *file_name, Student_Register registers[], int max_registers) {
    FILE *file;
    file = fopen(file_name, "r");
    if (file == NULL) {
        printf("Không thể mở file %s", file_name);
        return 0;
    }
    int count = 0;
    while(count<max_registers && fscanf(file,"%d %d", &registers[count].mssv, &registers[count].class_id) == 2) {
        count++;
    }
    fclose(file);
    return count;
}

/*-----------------------------------------------------------Read Schedule-----------------------------------------------------------*/
void trim(char *str) {
    int len = strlen(str);
    while (len > 0 && (str[len-1] == ' ' || str[len-1] == '\t' || str[len-1] == '\n'))
        str[--len] = '\0';

    char *start = str;
    while (*start == ' ' || *start == '\t') start++;

    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}

void parse_schedule(Schedule *s) {
    char temp[200];
    strcpy(temp, s->schedule);

    temp[strcspn(temp, ";")] = '\0';

    char *token = strtok(temp, ",");
    char *slots[20];
    int count = 0;

    while (token != NULL) {
        slots[count++] = token;
        token = strtok(NULL, ",");
    }

    if (count < 3) {
        printf("Lịch học không hợp lệ!\n");
        return;
    }

    int day = slots[0][0] - '0';       
    int session = slots[0][1] - '0';
    int start = slots[0][2] - '0';  
    int end   = slots[1][2] - '0'; 

    char *room = slots[count-1]; 

    const char *thu[] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    if (day >= 2 && day <= 7)
        strcpy(s->day, thu[day-2]);
    else
        strcpy(s->day, "Unknown");

    if (session == 1) strcpy(s->am_pm, "Morning");
    else if (session == 2) strcpy(s->am_pm, "Afternoon");
    else strcpy(s->am_pm, "Unknown");

    sprintf(s->period, "%d-%d", start, end);

    s->week[0] = '\0';
    for (int i = 2; i < count-1; i++) {
        strcat(s->week, slots[i]);
        strcat(s->week, " ");
    }
    trim(s->week);

    strcpy(s->room, room);

    // printf("Week Day: %s\n", s->day);
    // printf("AM/PM: %s\n", s->am_pm);
    // printf("Period: %s\n", s->period);
    // printf("Week: %s\n", s->week);
    // printf("Room: %s\n", s->room);
}

int read_schedule(Schedule schedules[], int max, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Không thể mở file %s!\n", filename);
        return 0;
    }

    int count = 0;
    char line[MAX_LINE];

    while (fgets(line, sizeof(line), file) && count < max) {
       
        sscanf(line, "%s %s", schedules[count].id, schedules[count].code);

        
        char *p = strstr(line, schedules[count].code) + strlen(schedules[count].code);
        while (*p == ' ') p++; 

        
        char *q = strpbrk(p, "0123456789");
        if (q) {
            strncpy(schedules[count].name, p, q - p);
            schedules[count].name[q - p] = '\0';
            trim(schedules[count].name); 
            strcpy(schedules[count].schedule, q);
        } else {
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

/*-------------------------------LOGIN SYSTEM ----------------------------------*/
void login_system() {
    User users[1000];
    int user_count = read_user("User-account.txt", users, 1000);
    int mssv_input;
    char password_input[256];
    int count = 0;
    for(int i=0; i<user_count; i++){
        printf("Enter MSSV: ");
        scanf("%d", &mssv_input);
        printf("Enter Password: ");
        scanf("%s", password_input);
        if(users[i].mssv == mssv_input && strcmp(users[i].password, password_input) == 0){
            printf("Login successful!\n");
            break;
        }
        else 
        {
            printf("Login failed!\n");
            count++;
            if(count == 3){
                printf("Too many failed attempts. Exiting.\n");
                break;  
            }
        }
    }
}

/*-------------------------------Search Schedule By Day-------------------------------*/
void search_schedule_by_day(){
    Schedule schedules[1000];
    int schedule_count = read_schedule(schedules, 1000, "course_schedule.txt");
    printf("Nhập ngày: ");
    char day_search[20];
    scanf("%s",day_search);
    int found = 0;
    for (int i = 0; i < schedule_count; i++) {
        if(strcasecmp(schedules[i].day, day_search) == 0){
            printf("Class.ID: %s, Code: %s, Course: %s, Day: %s, Week: %s, Period: %s, AM/PM: %s, Room: %s\n",
               schedules[i].id, schedules[i].code, schedules[i].name,
               schedules[i].day, schedules[i].week, schedules[i].period,
               schedules[i].am_pm, schedules[i].room);
            found = 1;
        }
    }
    if (!found) {
        printf("Không tìm thấy lớp có thứ %s\n", day_search);
    }
}

int main(void) {
    User users[1000];
    Student_Register registers[1000];
    Schedule schedules[1000];

    int user_count = read_user("User-account.txt", users, 1000);
    for(int i=0; i<user_count;i++){
        printf("MSSV: %d, Password: %s\n", users[i].mssv, users[i].password);
    }
    printf("--------------------\n");

    int register_count = read_register("student_registration.txt", registers, 1000);
    for(int i=0;i<register_count;i++){
        printf("MSSV: %d, Code: %d \n",registers[i].mssv, registers[i].class_id);
    }
    printf("--------------------\n");

    int schedule_count = read_schedule(schedules, 1000, "course_schedule.txt");    
    for (int i = 0; i < schedule_count; i++) {
        printf("ID: %s, Code: %s, Name: %s, Day: %s, Week: %s, Period: %s, AM/PM: %s, Room: %s\n",
               schedules[i].id, schedules[i].code, schedules[i].name,
               schedules[i].day, schedules[i].week, schedules[i].period,
               schedules[i].am_pm, schedules[i].room);
    }
    printf("--------------------\n");

    int choice;
    do
    {
        printf("1. Login\n");
        printf("2. Search Schedule By Day\n");
        printf("3. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        switch (choice)
        {
        case 1:
            login_system();
            break;
        case 2:
            search_schedule_by_day();
            break;
        case 3:
            printf("Exiting the program.\n");
            break;
        default:
            printf("Invalid choice. Please try again.\n");
            break;
        }
    } while (choice>=1);
   
    return 0;
}