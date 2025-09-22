#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 256

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

    printf("Week Day: %s\n", s->day);
    printf("AM/PM: %s\n", s->am_pm);
    printf("Period: %s\n", s->period);
    printf("Week: %s\n", s->week);
    printf("Room: %s\n", s->room);
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

        printf("ID: %s\n", schedules[count].id);
        printf("Code: %s\n", schedules[count].code);
        printf("Name: %s\n", schedules[count].name);
        parse_schedule(&schedules[count]);
        printf("----------------------------\n");

        count++;
    }

    fclose(file);
    return count;
}

int main() {
    Schedule schedules[1000];
    int schedule_count = read_schedule(schedules, 1000, "course_schedule.txt");

    printf("\n== Tổng hợp dữ liệu ==\n");
    for (int i = 0; i < schedule_count; i++) {
        printf("ID: %s, Code: %s, Name: %s, Day: %s, Week: %s, Period: %s, AM/PM: %s, Room: %s\n",
               schedules[i].id, schedules[i].code, schedules[i].name,
               schedules[i].day, schedules[i].week, schedules[i].period,
               schedules[i].am_pm, schedules[i].room);
    }

    printf("Tổng số dòng đọc được: %d\n", schedule_count);
    return 0;
}
