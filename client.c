#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <fcntl.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 8090

struct account{
    int id;
    char username[100];
    char password[100];
    int active;
};

struct train{
    int id;
    char name[100];
    int total_seats;
    int avail_seats;
    int active;
};

struct booking
{
	int id;
	int type;
	int acc_id;
	int train_id;
	int seats_booked;
	int active;
};


void addUser(int sd){
    int type, valid;
    printf("\nEnter the type of account :\n");
    printf("1) User\n2) Agent\n3) Admin\n4) Back\n");
    printf("Your choice: ");
    scanf("%d", &type);

    while(type > 4 || type < 1)
    {
        printf("Invalid Choice!\n");
        printf("Enter your choice again: ");
        scanf("%d", &type);
    }

    if(type == 4)
    {
        write(sd, &type, sizeof(type));
        return;
    }

    write(sd, &type, sizeof(type));

    char username[100], password[100];
    printf("Enter username: ");
    scanf("%s", username);
    strcpy(password,getpass("Enter password: "));

    write(sd, &username, sizeof(username));
    write(sd, &password, sizeof(password));

    read(sd, &valid, sizeof(int));
    if(valid==0){
        printf("\nUser with this name already exists! Try again\n");
        addUser(sd);
        return;
    }

    int id;
    read(sd, &id, sizeof(int));
    printf("\n-------Account successfully created and the Account ID is: %d.-------\n", id);
}

void deleteUser(int sd){
    int type;
    printf("\nEnter the type of account :\n");
    printf("1) User\n2) Agent\n3) Admin\n4) Back\n");
    printf("Your choice: ");
    scanf("%d", &type);

    while(type > 4 || type < 1)
    {
        printf("Invalid Choice!\n");
        printf("Enter your choice again : ");
        scanf("%d", &type);
    }

    if(type == 4)
    {
        write(sd, &type, sizeof(type));
        return;
    }

    write(sd, &type, sizeof(type));

    char username[100];
    printf("Enter username: ");
    scanf("%s", username);

    write(sd, &username, sizeof(username));
    int valid;
    read(sd, &valid, sizeof(int));
    if(valid==0){
        printf("\nNo account present with this username! Try again\n");
        deleteUser(sd);
        return;
    }

    printf("\n------Account successfully deleted------\n");
}

void updatePassword(int sd){
    int type;
    printf("\nEnter the type of account :\n");
    printf("1) User\n2) Agent\n3) Admin\n4) Back\n");
    printf("Your choice: ");
    scanf("%d", &type);

    while(type > 4 || type < 1)
    {
        printf("Invalid Choice!\n");
        printf("Enter your choice again : ");
        scanf("%d", &type);
    }

    if(type == 4)
    {
        write(sd, &type, sizeof(type));
        return;
    }

    write(sd, &type, sizeof(type));

    char username[100], password[100];
    printf("Enter username: ");
    scanf("%s", username);
    strcpy(password,getpass("Enter new password: "));

    write(sd, &username, sizeof(username));
    write(sd, &password, sizeof(password));

    int valid;
    read(sd, &valid, sizeof(int));
    if(valid==0){
        printf("\nNo account present with this username! Try again\n");
        updatePassword(sd);
        return;
    }

    printf("\n------Password changed succesfully------\n");
}

void displayUsers(int sd){
    int type;
    printf("\nEnter the type of account :\n");
    printf("1) User\n2) Agent\n3) Admin\n4) Back\n");
    printf("Your choice: ");
    scanf("%d", &type);

    while(type > 4 || type < 1)
    {
        printf("Invalid Choice!\n");
        printf("Enter your choice again : ");
        scanf("%d", &type);
    }

    if(type == 4)
    {
        write(sd, &type, sizeof(type));
        return;
    }
    write(sd, &type, sizeof(type));

    struct account temp;

    int len;
    read(sd, &len, sizeof(int));
    if(len==0){
        printf("\nNo users of this type are present!\n\n");
        return;
    }

    printf("----------------------------------\n");
    printf("ID\tUsername\tPassword\n");
    for(int i=0; i<len; i++){
        read(sd, &temp, sizeof(temp));
        printf("%d\t", temp.id);
        printf("%s\t\t", temp.username);
        printf("%s\n", temp.password);
    }
    printf("----------------------------------\n");
}

void addTrain(int sd){
    char name[100];
    int seats;
    printf("\nEnter train name: ");
    scanf("%s", name);
    printf("Total seats in the train: ");
    scanf("%d", &seats);

    write(sd, &name, sizeof(name));
    write(sd, &seats, sizeof(int));

    int status;
    read(sd, &status, sizeof(int));

    if(status==-1){
        printf("\nTrain with this name already exists! Try again\n");
        addTrain(sd);
        return;
    }
    int id;
    read(sd, &id, sizeof(int));
    printf("\n------Train added successfully with Train ID: %d------\n", id);
}

void displayTrains(int sd){
    struct train temp;

    int len;
    read(sd, &len, sizeof(int));
    if(len==0){
        printf("\nNo trains are present!\n\n");
        return;
    }

    printf("---------------------------------------------------\n");
    printf("ID\tTot_seats\tAvail_seats\tTrain_name\n");
    for(int i=0; i<len; i++){
        read(sd, &temp, sizeof(temp));
        // printf("ID: %d\n", temp.id);
        // printf("Total seats in the train: %d\n", temp.total_seats);
        // printf("Seats available: %d\n", temp.avail_seats);
        // printf("Name: %s\n", temp.name);
        // printf("-----------------\n");
        printf("%d\t", temp.id);
        printf("%d\t\t", temp.total_seats);
        printf("%d\t\t", temp.avail_seats);
        printf("%s\n", temp.name);
    }
    printf("---------------------------------------------------\n");
    return;
}

void deleteTrain(int sd){
    char name[100];
    printf("\nEnter train name: ");
    scanf("%s", name);

    write(sd, &name, sizeof(name));
    int valid;
    read(sd, &valid, sizeof(int));
    if(valid==0){
        printf("\nNo train present with this name! Try again\n");
        deleteTrain(sd);
        return;
    }

    printf("\n------Train successfully deleted------\n");
    return;
}

void updateTrain(int sd){
    printf("\nWhich value you want to update?\n1) Train name\n2) Seats\n");
    int choice, seats;
    printf("Your choice: ");
    scanf("%d", &choice);
    write(sd, &choice, sizeof(int));

    char name[100];
    printf("Enter train name: ");
    scanf("%s", name);
    write(sd, name, sizeof(name));

    int valid;
    read(sd, &valid, sizeof(int));
    if(valid==0){
        printf("\nNo train present with this name! Try again\n");
        updateTrain(sd);
        return;
    }

    if(choice==1){
        while(1){
            valid = 1;
            printf("\nEnter new train name: ");
            scanf("%s", name);
            write(sd, &name, sizeof(name));

            read(sd, &valid, sizeof(int));
            if(valid==1){
                break;
            }
            else{
                printf("\nTrain of this name already exists! Try again\n");
            }
        }
    }
    else{
        printf("\nEnter new value of total seats: ");
        scanf("%d", &seats);
        write(sd, &seats, sizeof(int));
    }

    printf("\n------Train details updated successfully------\n");
}

int admin_menu(int sd){
    printf("1) Add user\n2) Delete user\n3) Change user account password\n4) Display users\n5) Add train\n6) Delete train\n7) Modify train\n8) Display trains\n9) Logout\n");
    int choice;
    printf("Your choice: ");
    scanf("%d", &choice);
    write(sd, &choice, sizeof(int));

    while(choice>9 || choice<1){
        printf("Invalid Choice!\n");
		printf("Enter choice again\n");
		scanf("%d", &choice);
    }

    switch (choice)
    {
        case 1:
            addUser(sd);
            break;
        case 2:
            deleteUser(sd);
            break;
        case 3:
            updatePassword(sd);
            break;
        case 4:
            displayUsers(sd);
            break;
        case 5:
            addTrain(sd);
            break;
        case 6:
            deleteTrain(sd);
            break;
        case 7:
            updateTrain(sd);
            break;
        case 8:
            displayTrains(sd);
            break;
        case 9:
            printf("\n-------------------------------------------------------\n");
            printf("\tLogged out successfully! Please visit again\n");
            printf("-------------------------------------------------------\n");
            return -1;
        default:
            printf("Default\n");
            break;
    }  
    return choice; 
}

void bookTicket(int sd){
    int trains;
    read(sd, &trains, sizeof(int));
    if(trains==0){
        printf("\nNo trains available at the moment\n\n");
        return;
    }
    printf("\n\tPlease select the train ID of the train you want to book from the following:\n");
    displayTrains(sd);
    int id;
    printf("\nEnter train id to book: ");
    scanf("%d", &id);
    int seats;
    printf("\nHow many seats you want to book?: ");
    scanf("%d", &seats);

    write(sd, &id, sizeof(int));
    write(sd, &seats, sizeof(int));

    int valid;
    read(sd, &valid, sizeof(int));
    if(valid==0){
        printf("\nFailed to book ticket! Try again\n");
        bookTicket(sd);
        return;
    }
    int booking_id;
    read(sd, &booking_id, sizeof(int));
    printf("\n------Your booking ID is: %d------\n", booking_id);
}

void viewBookings(int sd){
    struct booking temp;
    int bookings;
    read(sd, &bookings, sizeof(int));
    if(bookings==0){
        printf("\nYou have not made any bookings yet!\n\n");
        return;
    }
    printf("-----------------------------------------------\n");
    printf("Booking_ID\tTrain_ID\tSeats_Booked\n");
    for(int i=0; i<bookings; i++){
        read(sd, &temp, sizeof(temp));
        printf("%d\t\t", temp.id);
        printf("%d\t\t", temp.train_id);
        printf("%d\n", temp.seats_booked);
    }
    printf("-----------------------------------------------\n");
}

void cancelBooking(int sd){
    printf("\nEnter booking ID to delete: ");
    int booking_id, valid;
    scanf("%d", &booking_id);
    write(sd, &booking_id, sizeof(int));
    read(sd, &valid, sizeof(int));

    if(valid==0){
        printf("\nNo booking found with this ID! Try again\n");
        cancelBooking(sd);
        return;
    }

    printf("\n------Booking deleted successfully------\n");
}

void updateBooking(int sd){
    int bookings;
    read(sd, &bookings, sizeof(int));
    if(bookings==0){
        printf("\nYou have not made any bookings yet!\n\n");
        return;
    }

    printf("\n\tPlease select the booking ID of the ticket you want to update from the following:\n");
    viewBookings(sd);
    printf("\nEnter booking ID to be updated: ");
    int booking_id, valid, value, train_avail;
    scanf("%d", &booking_id);
    printf("\nWhat is the new value of seats you want to book?: ");
    scanf("%d", &value);

    write(sd, &booking_id, sizeof(int));
    write(sd, &value, sizeof(int));

    read(sd, &train_avail, sizeof(int));
    if(train_avail==0){
        printf("\nThis booking is already expired because the train is cancelled!\n");
        return;
    }
    read(sd, &valid, sizeof(int));
    if(valid==0){
        printf("\nFailed to update booking! Try again\n");
        return;
    }

    printf("\n------Booking updated successfully------\n");
}

int user_menu(int sd){
    printf("1) Book ticket\n2) View bookings\n3) Update booking\n4) Cancel booking\n5) Logout\n");
    int choice;
    printf("Your choice: ");
    scanf("%d", &choice);
    write(sd, &choice, sizeof(int));

    while(choice>5 || choice<1){
        printf("\nInvalid Choice!\n");
		printf("Enter choice again\n");
		scanf("%d", &choice);
    }

    switch (choice)
    {
        case 1:
            bookTicket(sd);
            break;
        case 2:
            viewBookings(sd);
            break;
        case 3:
            updateBooking(sd);
            break;
        case 4:
            cancelBooking(sd);
            break;
        case 5:
            printf("\n-------------------------------------------------------\n");
            printf("\tLogged out successfully! Please visit again\n");
            printf("-------------------------------------------------------\n");
            return -1;
        default:
            printf("Default\n");
            break;
    }  
    return choice;
}

int entry_point(int sd){
    int choice;
    printf("Login as:\n1) User\n2) Agent\n3) Admin\n4) Exit\n");
    printf("Your choice: ");
    scanf("%d", &choice);

    while(choice > 4 || choice < 1)
	{
		printf("Invalid Choice!\n");
		printf("Enter choice again\n");
		scanf("%d", &choice);
	}

    write(sd, &choice, sizeof(choice));

    if(choice==4) return 4;

    char username[100], password[100];
    printf("Enter username: ");
    scanf("%s", username);
    strcpy(password,getpass("Enter password: "));

    write(sd, &username, sizeof(username));
    write(sd, &password, sizeof(password));

    int valid_login;
	read(sd, &valid_login, sizeof(int));

    // printf("Valid: %d\n", valid_login);
    
    if(valid_login==0){
        printf("\n\tInvalid details. Please try again\n\n");
        return 1;
    }

    printf("\n-------------------------------------------------------\n");
    printf("\tWelcome %s. Please select your option\n", username);
    printf("-------------------------------------------------------\n");

    if(choice==1 || choice==2){
        while (user_menu(sd)!=-1);
        // system("clear");
        return 1;
    }
    else{
        while(admin_menu(sd)!=-1);
        // system("clear");
        return 1;
    }
}

void main(){
    int cli_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(cli_fd == -1){
		printf("socket creation failed\n");
		exit(0);
	}
	struct sockaddr_in ca;
	ca.sin_family=AF_INET;
	ca.sin_port= htons(PORT);
	ca.sin_addr.s_addr = inet_addr("127.0.0.1");
	if(connect(cli_fd, (struct sockaddr *)&ca, sizeof(ca))==-1){
		printf("connect failed\n");
		exit(0);
	}
	// printf("connection established\n");
	printf("********** Welcome to Railway Ticket Booking System **********\n");
	while(entry_point(cli_fd)!=4);
    printf("\n********** Come visit again! Bye **********\n\n");
	close(cli_fd);
}