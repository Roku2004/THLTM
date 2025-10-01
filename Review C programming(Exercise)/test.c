#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#define MAXLINE 4096   /*max text line length*/
#define SERV_PORT 6868 /*port*/
#define LISTENQ 8      /*maximum number of client connections */

#define MAX_LINE 256

void login_system();
void search_schedule_by_day();
void read_schedule_for_student_register();
void read_all_schedule();


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

/*-------------------------------Handle Request ----------------------------------*/
void handle_request(char *buf, char *response){
    char action[50];
    int mssv_current;
    /*--------------------------------------------------Login-----------------------------------------------------*/
    int mssv;
    char password[256];
    if (sscanf(buf, "{\"action\": \"%[^\"]\", \"mssv\": %d, \"password\": \"%[^\"]\"}", action, &mssv, password) == 3)
    {
        login_system(action, mssv, password, response);
    }

    /*------------------------------------------------------------------------------------------------------------*/

    /*---------------------------------------------------Search Schedule By Day-----------------------------------*/
    char day[20];
    if (sscanf(buf, "{\"action\": \"%[^\"]\", \"day\": \"%[^\"]\", \"mssv_current\": %d}", action, day, &mssv_current) == 3)
    {
        search_schedule_by_day(action, day, response, mssv_current);
    }
    /*------------------------------------------------------------------------------------------------------------*/

    /*---------------------------------------------------Read all schedule student register-----------------------------------*/
    if (sscanf(buf, "{\"action\": \"%[^\"]\", \"mssv_current\": %d}", action, &mssv_current) == 2)
    {
        read_schedule_for_student_register(action, response, mssv_current);
    }
    /*------------------------------------------------------------------------------------------------------------*/

    /*----------------------------------------Read All Schedule--------------------------------------*/
    if (sscanf(buf, "{\"action\": \"%[^\"]\", \"mssv_current\": %d}", action, &mssv_current) == 2)
    {
        read_all_schedule(action, response, mssv_current);
    }
}

/*--------------------------------------------Login System-------------------------------------*/
void login_system(char action[256], int mssv, char password[256], char response[MAXLINE]){
    User users[1000];
    int user_count = read_user("User-account.txt", users, 1000);
    if (strcmp(action, "login") == 0)
    {
        int login_success = 0;
        for (int i = 0; i < user_count; i++)
        {
            if (users[i].mssv == mssv && strcmp(users[i].password, password) == 0)
            {
                login_success = 1;
                break;
            }
        }

        if (login_success)
        {
            printf("Login successful!\n");
            snprintf(response, MAXLINE, "1");
        }
        else
        {
            printf("Login failed!\n");
            snprintf(response, MAXLINE, "2");
        }
    }
    else
    {
        snprintf(response, MAXLINE, "Unknown action: %s", action);
    }
}

/*---------------------------------------------------Search Schedule By Day-----------------------------------*/
void search_schedule_by_day(char action[256], char Byday[20], char response[MAXLINE], int mssv){
    Schedule schedules[1000];
    int schedule_count = read_schedule(schedules, 1000, "course_schedule.txt");
    Student_Register registers[1000];
    int register_count = read_register("student_registration.txt", registers, 1000);

    printf("Searching for classes on: %s\n", Byday);
    printf("Class.ID  |  Code  |  Course  |  Day  |  Week  |  Period  |  AM/PM  |  Room  \n");

    int found = 0;
    char temp_response[MAXLINE] = ""; // Buffer tạm để build response
    char class_buffer[500];           // Buffer cho mỗi lớp học

    if (strcmp(action, "searchScheduleByDay") == 0)
    {
        // Bắt đầu JSON array
        strcat(temp_response, "{\"found\": true, \"classes\": [");

        for (int i = 0; i < schedule_count; i++)
        {
            if (strcmp(schedules[i].day, Byday) == 0)
            {
                // Kiểm tra sinh viên có đăng ký lớp này không
                for (int j = 0; j < register_count; j++)
                {
                    if (schedules[i].id == registers[j].class_id && registers[j].mssv == mssv)
                    {
                        found = 1;

                        // In ra màn hình server
                        printf("%d  |  %s  |  %s  |  %s  |  %s  |  %s  |  %s  |  %s  \n",
                               schedules[i].id, schedules[i].code, schedules[i].name,
                               schedules[i].day, schedules[i].week, schedules[i].period,
                               schedules[i].am_pm, schedules[i].room);

                        // Thêm vào JSON array (thêm dấu phẩy nếu không phải phần tử đầu)
                        if (strlen(temp_response) > 25)
                        { // Nếu không phải phần tử đầu
                            strcat(temp_response, ", ");
                        }

                        // Tạo JSON object cho mỗi lớp
                        snprintf(class_buffer, sizeof(class_buffer),
                                 "{\"id\": %d, \"code\": \"%s\", \"name\": \"%s\", "
                                 "\"day\": \"%s\", \"week\": \"%s\", \"am_pm\": \"%s\", "
                                 "\"room\": \"%s\", \"period\": \"%s\"}",
                                 schedules[i].id,
                                 schedules[i].code,
                                 schedules[i].name,
                                 schedules[i].day,
                                 schedules[i].week,
                                 schedules[i].am_pm,
                                 schedules[i].room,
                                 schedules[i].period);

                        strcat(temp_response, class_buffer);
                        break; // Thoát vòng lặp j khi đã tìm thấy đăng ký
                    }
                }
            }
        }

        // Kết thúc JSON array
        strcat(temp_response, "]}");

        if (found)
        {
            strcpy(response, temp_response);
            printf("Found %d class(es) on %s\n", found, Byday);
        }
        else
        {
            printf("Không tìm thấy lớp có thứ %s\n", Byday);
            snprintf(response, MAXLINE, "{\"found\": false, \"message\": \"No classes found for %s\"}", Byday);
        }
    }
}
/*------------------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------Read all schedule student register----------------------------------------------*/
void read_schedule_for_student_register(char action[256], char response[MAXLINE], int mssv){
    Schedule schedules[1000];
    int schedule_count = read_schedule(schedules, 1000, "course_schedule.txt");
    Student_Register registers[1000];
    int register_count = read_register("student_registration.txt", registers, 1000);
    printf("Schedule For Student:\n");
    printf("Class.ID  |  Code  |  Course  |  Day  |  Week  |  Period  |  AM/PM  |  Room  \n");

    int found = 0;
    char temp_response[MAXLINE] = ""; // Buffer tạm để build response
    char class_buffer[500];           // Buffer cho mỗi lớp học

    if (strcmp(action, "searchScheduleAllStudentRegister") == 0)
    {
        // Bắt đầu JSON array
        strcat(temp_response, "{\"found\": true, \"classes\": [");

        for (int i = 0; i < schedule_count; i++)
        {
            for (int j = 0; j < register_count; j++)
            {
                if (schedules[i].id == registers[j].class_id && registers[j].mssv == mssv)
                {
                    found = 1;

                    // In ra màn hình server
                    printf("%d  |  %s  |  %s  |  %s  |  %s  |  %s  |  %s  |  %s  \n",
                           schedules[i].id, schedules[i].code, schedules[i].name,
                           schedules[i].day, schedules[i].week, schedules[i].period,
                           schedules[i].am_pm, schedules[i].room);

                    // Thêm vào JSON array (thêm dấu phẩy nếu không phải phần tử đầu)
                    if (strlen(temp_response) > 25)
                    { // Nếu không phải phần tử đầu
                        strcat(temp_response, ", ");
                    }

                    // Tạo JSON object cho mỗi lớp
                    snprintf(class_buffer, sizeof(class_buffer),
                             "{\"id\": %d, \"code\": \"%s\", \"name\": \"%s\", "
                             "\"day\": \"%s\", \"week\": \"%s\", \"am_pm\": \"%s\", "
                             "\"room\": \"%s\", \"period\": \"%s\"}",
                             schedules[i].id,
                             schedules[i].code,
                             schedules[i].name,
                             schedules[i].day,
                             schedules[i].week,
                             schedules[i].am_pm,
                             schedules[i].room,
                             schedules[i].period);

                    strcat(temp_response, class_buffer);
                    break; // Thoát vòng lặp j khi đã tìm thấy đăng ký
                }
            }
        }

        // Kết thúc JSON array
        strcat(temp_response, "]}");

        if (found)
        {
            strcpy(response, temp_response);
            printf("Found schedule");
        }
        else
        {
            printf("Không có lịch học\n");
            snprintf(response, MAXLINE, "{\"found\": false, \"message\": \"No schedule\"}");
        }
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------Read All Schedule-----------------------------------------------------*/
void read_all_schedule(char action[256], char response[MAXLINE], int mssv){
    char schedule[13][6][50];
    Schedule schedules[1000];
    Student_Register registers[1000];
    int register_count = read_register("student_registration.txt", registers, 1000);
    int schedule_count = read_schedule(schedules, 1000, "course_schedule.txt");
    
    if(strcmp(action, "searchScheduleAll") == 0){
            // Khởi tạo schedule
        for (int i = 0; i < 13; i++)
            for (int j = 0; j < 6; j++)
                schedule[i][j][0] = '\0';

        for (int k = 0; k < schedule_count; k++)
        {
            for (int l = 0; l < register_count; l++)
            {
                if (schedules[k].id == registers[l].class_id && registers[l].mssv == mssv)
                {
                    int day_index = -1;
                    if (strcmp(schedules[k].day, "Monday") == 0) day_index = 0;
                    else if (strcmp(schedules[k].day, "Tuesday") == 0) day_index = 1;
                    else if (strcmp(schedules[k].day, "Wednesday") == 0) day_index = 2;
                    else if (strcmp(schedules[k].day, "Thursday") == 0) day_index = 3;
                    else if (strcmp(schedules[k].day, "Friday") == 0) day_index = 4;
                    else if (strcmp(schedules[k].day, "Saturday") == 0) day_index = 5;

                    if (day_index == -1) continue;

                    int start_period = atoi(schedules[k].period);
                    int end_period = start_period;

                    char *dash = strchr(schedules[k].period, '-');
                    if (dash != NULL) {
                        end_period = atoi(dash + 1);
                    }

                    for (int period = start_period; period <= end_period; period++)
                    {
                        int display_period = period;
                        if (strcmp(schedules[k].am_pm, "Afternoon") == 0) {
                            display_period = period + 6;
                        }

                        if (display_period >= 1 && display_period <= 12) {
                            snprintf(schedule[display_period][day_index], 50, "%s", schedules[k].room);
                        }
                    }
                }
            }
        }

        char temp_response[MAXLINE] = "{\"schedule\":[";
        int first = 1;
        
        for (int i = 1; i <= 12; i++) {
            for (int j = 0; j < 6; j++) {
                if (!first) {
                    strcat(temp_response, ",");
                }
                
                char cell[100];
                snprintf(cell, sizeof(cell), "{\"period\":%d,\"day\":%d,\"room\":\"%s\"}", 
                        i, j, schedule[i][j]);
                strcat(temp_response, cell);
                first = 0;
            }
        }
        
        strcat(temp_response, "]}");
        strcpy(response, temp_response);
    }
}
/*-----------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------Connect Client--------------------------------------------*/
int main()
{
    int listenfd, connfd, n;
    socklen_t clilen;
    struct sockaddr_in servaddr, cliaddr;
    char buf[MAXLINE], response[MAXLINE];

    // Tạo socket
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    listen(listenfd, LISTENQ);

    printf("Server running... waiting for connections.\n");

    while (1)
    {
        clilen = sizeof(cliaddr);
        connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
        if (connfd < 0)
        {
            perror("Accept error");
            continue;
        }

        if (fork() == 0)
        { // Child process
            close(listenfd);

            char ipClient[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(cliaddr.sin_addr), ipClient, INET_ADDRSTRLEN);
            int portClient = ntohs(cliaddr.sin_port);
            printf("Client connected: %s:%d\n", ipClient, portClient);

            while ((n = recv(connfd, buf, MAXLINE, 0)) > 0)
            {
                buf[n] = '\0';
                printf("Received: %s\n", buf);

                handle_request(buf, response);

                send(connfd, response, strlen(response), 0);
            }

            printf("Client disconnected: %s:%d\n", ipClient, portClient);
            close(connfd);
            exit(0);
        }

        close(connfd); // cha đóng socket client
    }

    close(listenfd);
    return 0;
}