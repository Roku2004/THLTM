#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAXLINE 4096
#define SERV_PORT 6868

void start_menu();
void login_system();
void menu();
void schedule_menu();
void search_schedule_by_day();
void read_schedule_for_student_register();
void read_all_schedule();

int mssv_current;

/*-----------------------------Connect Server----------------------------------------*/
void connect_server(int argc, char **argv, char sendline[MAXLINE], char recvline[MAXLINE]) {
    int sockfd;
    struct sockaddr_in servaddr;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <IP address of the server>\n", argv[0]);
        exit(1);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Problem in creating the socket");
        exit(2);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    servaddr.sin_port = htons(SERV_PORT);

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Problem in connecting to the server");
        exit(3);
    }

    // Gửi JSON (mssv + password)
    send(sockfd, sendline, strlen(sendline), 0);

    // Nhận phản hồi từ server
    int n = recv(sockfd, recvline, MAXLINE, 0);
    if (n <= 0) {
        perror("The server terminated prematurely");
        exit(4);
    }
    recvline[n] = '\0'; // Kết thúc chuỗi

    close(sockfd);
}
/*----------------------------------------Start Menu-------------------------------------*/
void start_menu(int argc, char **argv)
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
            login_system(argc, argv);
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

/*-------------------------------LOGIN SYSTEM ----------------------------------*/
void login_system(int argc, char **argv)
{
    int mssv_input;
    char password_input[256];
    int count = 0;
    int login_success = 0;
    char sendline[MAXLINE];
    char recvline[MAXLINE];
    while (count < 3 && !login_success)
    {
        printf("Enter MSSV: ");
        scanf("%d", &mssv_input);
        printf("Enter Password: ");
        scanf("%s", password_input);
        snprintf(sendline, MAXLINE, "{\"action\": \"login\", \"mssv\": %d, \"password\": \"%s\"}",mssv_input, password_input);
        connect_server(argc, argv, sendline, recvline);
        if(strcmp(recvline,"1") == 0){
            login_success = 1;
            mssv_current = mssv_input;
            printf("Login successful\n");
            menu(argc, argv);
            return;
        } else if (strcmp(recvline,"2") == 0){
            count++;
            printf("Login failed!\n");
            if (count == 3)
            {
                printf("Too many failed attempts. Returning to start menu.\n");
            }
        } else if(strcmp(recvline,"3") == 0){
            count++;
            printf("Invalid JSON format!\n");
            if (count == 3)
            {
                printf("Too many failed attempts. Returning to start menu.\n");
            }
        }
    }
    printf("-----------------------------------------------------------------------------\n");
}
/*------------------------------------------------------------------------------------------------*/


/*-----------------------------------------------------Main Menu------------------------------------------*/
void menu(int argc, char **argv)
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
            schedule_menu(argc, argv);
            break;
        case 2:
            printf("Exiting the program.\n");
            start_menu(argc, argv);
            break;
        default:
            printf("Invalid choice. Please try again.\n");
            break;
        }
    } while (choice >= 1);
}
/*---------------------------------------------------------------------------------------------------------------*/

/*------------------------------------------Schedule menu--------------------------------------------------------*/
void schedule_menu(int argc, char **argv)
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
            search_schedule_by_day(argc, argv);
            break;
        case 2:
            read_schedule_for_student_register(argc, argv);
            break;
        case 3:
            read_all_schedule(argc, argv);
            break;
        case 4:
            menu(argc, argv);
            break;
        case 5:
            printf("Exiting the program.\n");
            start_menu(argc, argv);
            break;
        default:
            printf("Invalid choice. Please try again.\n");
            break;
        }
    } while (choice >= 1);
}
/*-------------------------------------------------------------------------------------*/

/*-------------------------------Search Schedule By Day-------------------------------*/
void search_schedule_by_day(int argc, char **argv){
    char sendline[MAXLINE];
    char recvline[MAXLINE];
    printf("Search Schedule By Day:\n");
    printf("Nhập ngày: ");
    char day_search[20];
    scanf("%s", day_search);
    
    snprintf(sendline, MAXLINE, "{\"action\": \"searchScheduleByDay\", \"day\": \"%s\", \"mssv_current\": %d}", 
             day_search, mssv_current);
    
    connect_server(argc, argv, sendline, recvline);
    
    printf("| %-8s | %-10s | %-25s | %-10s | %-15s | %-8s | %-10s | %-10s |\n", 
           "Class.ID", "Code", "Course", "Day", "Week", "Period", "AM/PM", "Room");
    printf("|----------|------------|---------------------------|------------|-----------------|----------|------------|------------|\n");
    
    // Hiển thị tất cả các lớp học tìm thấy
    char *current = recvline;
    int class_count = 0;
    
    while ((current = strstr(current, "\"id\":")) != NULL) {
        int id;
        char code[50], name[100], day[50], week[100], am_pm[20], room[50], period[50];
        
        if (sscanf(current, 
                  "\"id\": %d, \"code\": \"%49[^\"]\", \"name\": \"%99[^\"]\", "
                  "\"day\": \"%49[^\"]\", \"week\": \"%99[^\"]\", \"am_pm\": \"%19[^\"]\", "
                  "\"room\": \"%49[^\"]\", \"period\": \"%49[^\"]\"",
                  &id, code, name, day, week, am_pm, room, period) == 8) {
            
            printf("| %-8d | %-10s | %-25s | %-10s | %-15s | %-8s | %-10s | %-10s |\n",
                   id, code, name, day, week, period, am_pm, room);
            class_count++;
        }
        current++;
    }
    
    if (class_count > 0) {
        printf("=> Tìm thấy %d lớp học cho thứ %s\n", class_count, day_search);
    } else {
        printf("Không tìm thấy lớp nào cho thứ %s\n", day_search);
    }
    
    printf("-----------------------------------------------------------------------------\n");
}
/*----------------------------------------------------------------------------------------------*/

/*-------------------------------------Read all schedule student register------------------------------------------*/
void read_schedule_for_student_register(int argc, char **argv){
    char sendline[MAXLINE];
    char recvline[MAXLINE];
    printf("Schedule For Student:\n");

    snprintf(sendline, MAXLINE, "{\"action\": \"searchScheduleAllStudentRegister\", \"mssv_current\": %d}", mssv_current);
    
    connect_server(argc, argv, sendline, recvline);
    
    printf("| %-8s | %-10s | %-25s | %-10s | %-15s | %-8s | %-10s | %-10s |\n", 
           "Class.ID", "Code", "Course", "Day", "Week", "Period", "AM/PM", "Room");
    printf("|----------|------------|---------------------------|------------|-----------------|----------|------------|------------|\n");
    
    // Hiển thị tất cả các lớp học tìm thấy
    char *current = recvline;
    int class_count = 0;
    
    while ((current = strstr(current, "\"id\":")) != NULL) {
        int id;
        char code[50], name[100], day[50], week[100], am_pm[20], room[50], period[50];
        
        if (sscanf(current, 
                  "\"id\": %d, \"code\": \"%49[^\"]\", \"name\": \"%99[^\"]\", "
                  "\"day\": \"%49[^\"]\", \"week\": \"%99[^\"]\", \"am_pm\": \"%19[^\"]\", "
                  "\"room\": \"%49[^\"]\", \"period\": \"%49[^\"]\"",
                  &id, code, name, day, week, am_pm, room, period) == 8) {
            
            printf("| %-8d | %-10s | %-25s | %-10s | %-15s | %-8s | %-10s | %-10s |\n",
                   id, code, name, day, week, period, am_pm, room);
            class_count++;
        }
        current++;
    }
    
    if (class_count > 0) {
        printf("=> Tìm thấy %d lớp học\n", class_count);
    } else {
        printf("Không tìm thấy lịch học\n");
    }
    
    printf("-----------------------------------------------------------------------------\n");
}
/*----------------------------------------------------------------------------------------------*/

/*---------------------------------------Read All Schedule---------------------------------------------------------*/
void read_all_schedule(int argc, char **argv){
    char sendline[MAXLINE];
    char recvline[MAXLINE];
    
    printf("All Schedule:\n");
    snprintf(sendline, MAXLINE, "{\"action\": \"searchScheduleAll\", \"mssv_current\": %d}", mssv_current);
    connect_server(argc, argv, sendline, recvline);
    
    // Parse dữ liệu từ server
    char schedule[13][6][50] = {0};
    
    // Tìm và parse dữ liệu schedule từ JSON
    char *current = recvline;
    while ((current = strstr(current, "{\"period\":")) != NULL) {
        int period, day;
        char room[50] = "";
        
        if (sscanf(current, 
                  "{\"period\":%d,\"day\":%d,\"room\":\"%49[^\"]\"}",
                  &period, &day, room) == 3) {
            
            if (period >= 1 && period <= 12 && day >= 0 && day <= 5) {
                strcpy(schedule[period][day], room);
            }
        }
        current++;
    }
    
    // Hiển thị thời khóa biểu
    printf("| %-2s | %-12s | %-12s | %-12s | %-12s | %-12s | %-12s |\n", 
           "TT", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday");
    printf("|----|--------------|--------------|--------------|--------------|--------------|--------------|\n");
    
    for (int i = 1; i <= 12; i++)
    {
        printf("| %-2d |", i);
        for (int d = 0; d < 6; d++)
        {
            if (strlen(schedule[i][d]) > 0)
            {
                printf(" %-12s |", schedule[i][d]);
            }
            else
            {
                printf(" %-12s |", "");
            }
        }
        printf("\n");
    }
    printf("-----------------------------------------------------------------------------\n");
}
/*-----------------------------------------------------------------------------------------------------------------*/

int main(int argc, char **argv) {
    printf("Welcome to the Course Management System\n");
    start_menu(argc, argv);
    return 0;
}