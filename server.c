#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>

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

char *ACCOUNT[3] = {"./userDB", "./agentDB", "./adminDB"};

void addUser(int sd){
    // printf("Inside add user\n");
    char username[100], password[100];
    int type, fd, valid=1;
    read(sd, &type, sizeof(int));

    if(type==4) return;

    read(sd, &username, sizeof(username));
	read(sd, &password, sizeof(password));

    // printf("Type: %d, Username: %s, Password: %s\n", type, username, password);
    // printf("%s\n", ACCOUNT[type-1]);
    if((fd = open(ACCOUNT[type-1], O_RDWR))==-1){
		printf("File Error\n");
        return;
	}

    struct account temp;
    
    struct flock lock;
	lock.l_type = F_WRLCK;
	lock.l_start = 0;
	lock.l_len = 0;
	lock.l_whence = SEEK_SET;
	lock.l_pid = getpid();

	fcntl(fd,F_SETLKW, &lock);

    while(read(fd, &temp, sizeof(temp))>0){
        if(temp.active==1 && strcmp(temp.username, username)==0){
            valid = 0;
            break;
        }
    }

    write(sd, &valid, sizeof(int));
    if(valid==0){
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
        close(fd);
        addUser(sd);
        return;
    }

	int fp = lseek(fd, 0, SEEK_END);

	if(fp==0){
		temp.id = 1;
	}
	else{
		fp = lseek(fd, -1 * sizeof(struct account), SEEK_CUR);
		read(fd, &temp, sizeof(temp));
        // printf("ID: %d\n", temp.id);
		temp.id++;
	}
	strcpy(temp.username, username);
	strcpy(temp.password, password);
    temp.active = 1;
	write(fd, &temp, sizeof(temp));

	write(sd, &temp.id, sizeof(temp.id));

	lock.l_type = F_UNLCK;
	fcntl(fd, F_SETLK, &lock);

	close(fd);
}

void deleteUser(int sd){
    // printf("Inside delete user\n");
    char username[100];
    int type, fd;
    read(sd, &type, sizeof(int));
    read(sd, &username, sizeof(username));
    // printf("Type: %d, Username: %s\n", type, username);
    // printf("%s\n", ACCOUNT[type-1]);
    if((fd = open(ACCOUNT[type-1], O_RDWR))==-1){
		printf("File Error\n");  
        return;
	}

    struct account temp;
    int id, valid=0;
    while(read(fd, &temp, sizeof(temp))>0){
        // printf("%s, %s, %d\n", temp.username, temp.password, temp.active);
        if(temp.active==1 && strcmp(temp.username, username)==0){
            id = temp.id;
            valid = 1;
        }
    }
    close(fd);

    if(valid==0){
        write(sd, &valid, sizeof(valid));
        deleteUser(sd);
        return;
    }

    write(sd, &valid, sizeof(valid));
    // printf("ID: %d\n", id);

    fd = open(ACCOUNT[type-1], O_RDWR);
    
    struct flock lock;
	lock.l_type = F_WRLCK;
	lock.l_start = (id-1)*sizeof(struct account);
	lock.l_len = sizeof(struct account);
	lock.l_whence = SEEK_SET;
	lock.l_pid = getpid();

	fcntl(fd,F_SETLKW, &lock);

	int fp = lseek(fd, (id-1) * sizeof(struct account), SEEK_SET);
    read(fd, &temp, sizeof(temp));
    temp.active = 0;
    
    lseek(fd, -1 * sizeof(struct account), SEEK_CUR);
    // printf("FP: %d, RD: %d\n", fp, rd);
	write(fd, &temp, sizeof(temp));

	lock.l_type = F_UNLCK;
	fcntl(fd, F_SETLK, &lock);

	close(fd);
}

void updatePassword(int sd){
    // printf("Inside update password\n");
    char username[100], password[100];
    int type, fd;
    read(sd, &type, sizeof(int));
    read(sd, &username, sizeof(username));
    read(sd, &password, strlen(password));
    // printf("Type: %d, Username: %s\n", type, username);
    // printf("%s\n", ACCOUNT[type-1]);
    if((fd = open(ACCOUNT[type-1], O_RDWR))==-1){
		printf("File Error\n");  
        return;
	}

    struct account temp;
    int id, valid=0;
    while(read(fd, &temp, sizeof(temp))>0){
        // printf("%s, %s, %d\n", temp.username, temp.password, temp.active);
        if(temp.active==1 && strcmp(temp.username, username)==0){
            id = temp.id;
            valid = 1;
        }
    }
    close(fd);

    if(valid==0){
        write(sd, &valid, sizeof(valid));
        updatePassword(sd);
        return;
    }

    write(sd, &valid, sizeof(valid));
    // printf("ID: %d\n", id);

    fd = open(ACCOUNT[type-1], O_RDWR);
    
    struct flock lock;
	lock.l_type = F_WRLCK;
	lock.l_start = (id-1)*sizeof(struct account);
	lock.l_len = sizeof(struct account);
	lock.l_whence = SEEK_SET;
	lock.l_pid = getpid();

	fcntl(fd,F_SETLKW, &lock);

	int fp = lseek(fd, (id-1) * sizeof(struct account), SEEK_SET);
    read(fd, &temp, sizeof(temp));
    strcpy(temp.password, password);
    
    lseek(fd, -1 * sizeof(struct account), SEEK_CUR);
    // printf("FP: %d, RD: %d\n", fp, rd);
	write(fd, &temp, sizeof(temp));

	lock.l_type = F_UNLCK;
	fcntl(fd, F_SETLK, &lock);

	close(fd);
}

void displayUsers(int sd){
    // printf("Inside display users\n");
    int type;
    read(sd, &type, sizeof(int));

    int fd;
    if((fd = open(ACCOUNT[type-1], O_RDWR))==-1){
		printf("File Error\n");
        return;
	}
    close(fd);

    fd = open(ACCOUNT[type-1], O_RDWR);
    
    struct flock lock;
	lock.l_type = F_RDLCK;
	lock.l_start = 0;
	lock.l_len = 0;
	lock.l_whence = SEEK_SET;
	lock.l_pid = getpid();

	fcntl(fd,F_SETLKW, &lock);

    struct account temp;
    int i=0;

	while(read(fd, &temp, sizeof(temp))>0){
        if(temp.active==1){
            i=i+1;
        }
    }
    write(sd, &i, sizeof(int));
    lseek(fd, 0, SEEK_SET);
    while(i>0){
        read(fd, &temp, sizeof(temp));
        if(temp.active==1){
            write(sd, &temp, sizeof(temp));
            i--;
        }
    }

	lock.l_type = F_UNLCK;
	fcntl(fd, F_SETLK, &lock);

	close(fd);
}

void addTrain(int sd){
    // printf("Inside add train\n");
    struct train temp;
    char name[100];
    int seats, status=1, id=0;
    read(sd, &name, sizeof(name));
    read(sd, &seats, sizeof(int));

    int fd;
    if((fd=open("trainsDB", O_RDWR))==-1){
        printf("File Error\n");
        return;
    }

    struct flock lock;
    lock.l_len = 0;
    lock.l_start = 0;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_pid = getpid();

    fcntl(fd, F_SETLKW, &lock);

    while(read(fd, &temp, sizeof(temp))>0){
        id = temp.id;
        if(temp.active==1 && strcmp(temp.name, name)==0){
            status = -1;
            break;
        }
    }
    write(sd, &status, sizeof(int));

    if(status==-1){
        addTrain(sd);
        return;
    }

    strcpy(temp.name, name);
    temp.total_seats = seats;
    temp.avail_seats = seats;
    temp.id = id+1;
    temp.active = 1;

    write(fd, &temp, sizeof(temp));
    write(sd, &temp.id, sizeof(int));

    lock.l_type = F_UNLCK;
	fcntl(fd, F_SETLK, &lock);

	close(fd);
}

void displayTrains(int sd){
    // printf("Inside display trains\n");
    int fd;
    if((fd = open("trainsDB", O_RDWR))==-1){
		printf("File Error\n"); 
        return;
	}
    
    struct flock lock;
	lock.l_type = F_RDLCK;
	lock.l_start = 0;
	lock.l_len = 0;
	lock.l_whence = SEEK_SET;
	lock.l_pid = getpid();

	fcntl(fd,F_SETLKW, &lock);

    struct train temp;
    int i=0;

	while(read(fd, &temp, sizeof(temp))>0){
        if(temp.active==1){
            i=i+1;
        }
    }
    write(sd, &i, sizeof(int));
    lseek(fd, 0, SEEK_SET);
    while(i>0){
        read(fd, &temp, sizeof(temp));
        if(temp.active==1){
            write(sd, &temp, sizeof(temp));
            i--;
        }
    }

	lock.l_type = F_UNLCK;
	fcntl(fd, F_SETLK, &lock);

	close(fd);
    return;
}

void deleteTrain(int sd){
    // printf("Inside delete train\n");
    int valid=0, fd;
    char name[100];
    read(sd, &name, sizeof(name));

    if((fd = open("trainsDB", O_RDWR))==-1){
		printf("File Error\n");
        return;
	}

    struct flock lock;
	lock.l_type = F_WRLCK;
	lock.l_start = 0;
	lock.l_len = 0;
	lock.l_whence = SEEK_SET;
	lock.l_pid = getpid();

	fcntl(fd,F_SETLKW, &lock);

    struct train temp;
    while(read(fd, &temp, sizeof(temp))>0){
        if(temp.active==1 && strcmp(temp.name, name)==0){
            valid = 1;
            break;
        }
    }
    write(sd, &valid, sizeof(int));
    if(valid==0){
        deleteTrain(sd);
        return;
    }

    lseek(fd, -1*sizeof(struct train), SEEK_CUR);
    temp.active = 0;
    write(fd, &temp, sizeof(temp));

    lock.l_type = F_UNLCK;
	fcntl(fd, F_SETLK, &lock);

	close(fd);
}

void updateTrain(int sd){
    // printf("Inside update train\n");
    int choice, fd, valid=0, id, seats;
    read(sd, &choice, sizeof(int));

    char name[100];
    read(sd, name, sizeof(name));

    if((fd = open("trainsDB", O_RDWR))==-1){
		printf("File Error\n"); 
        return;
	}
    struct train temp;
    while(read(fd, &temp, sizeof(temp))>0){
        if(temp.active==1 && strcmp(temp.name, name)==0){
            valid = 1;
            id = temp.id;
            break;
        }
    }
    write(sd, &valid, sizeof(valid));
    if(valid==0){
        updateTrain(sd);
        return;
    }

    struct flock lock;
	lock.l_type = F_WRLCK;
	lock.l_start = (id-1)*sizeof(struct train);
	lock.l_len = sizeof(struct train);
	lock.l_whence = SEEK_SET;
	lock.l_pid = getpid();

    fcntl(fd,F_SETLKW, &lock);

    lseek(fd, 0, SEEK_SET);
    if(choice==1){
        struct train temp2;
        while(1){
            valid = 1;
            read(sd, &name, sizeof(name));
            while(read(fd, &temp2, sizeof(temp2))>0){
                if(temp2.active==1 && strcmp(temp2.name, name)==0){
                    valid = 0;
                    break;
                }
            }
            write(sd, &valid, sizeof(int));
            if(valid==1){
                lseek(fd, (id-1)*sizeof(struct train), SEEK_SET);
                strcpy(temp.name, name);
                write(fd, &temp, sizeof(temp));
                break;
            }
        }
    }
    else{
        read(sd, &seats, sizeof(int));
        temp.total_seats = seats;
        if(seats < temp.avail_seats) temp.avail_seats = seats;
        lseek(fd, (id-1)*sizeof(struct train), SEEK_SET);
        write(fd, &temp, sizeof(temp));
    }

    lock.l_type = F_UNLCK;
	fcntl(fd, F_SETLK, &lock);

	close(fd);

    return;
}

int admin_menu(int sd, int id){
    // printf("Inside admin_menu\n");
    int choice;
    read(sd, &choice, sizeof(int));

    switch(choice){
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
            return -1;
        default:
            printf("Default\n");
            break;
    }
    return choice;
}

void bookTicket(int sd, int user_id, int type){
    // printf("Inside book ticket\n");
    int fd;
    if((fd = open("trainsDB", O_RDWR))==-1){
		printf("File Error\n");  
        return;
	}
    
    struct flock lock;
	lock.l_type = F_WRLCK;
	lock.l_start = 0;
	lock.l_len = 0;
	lock.l_whence = SEEK_SET;
	lock.l_pid = getpid();

    fcntl(fd,F_SETLKW, &lock);

    int trains=0;
    struct train temp;
    while(read(fd, &temp, sizeof(temp))>0){
        if(temp.active==1){
            trains++;
        }
    }
    write(sd, &trains, sizeof(int));

    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);

    if(trains==0) return;

    displayTrains(sd);
    int train_id, booked_seats, valid=0;
    read(sd, &train_id, sizeof(int));
    read(sd, &booked_seats, sizeof(int));

    lock.l_type = F_WRLCK;
    fcntl(fd, F_SETLK, &lock);

    lseek(fd, 0, SEEK_SET);
    while(read(fd, &temp, sizeof(temp))>0){
        if(temp.active==1 && temp.avail_seats>=booked_seats && temp.id==train_id && booked_seats>0){
            valid=1;
            break;
        }
    }
    write(sd, &valid, sizeof(int));

    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);

    if(valid==0){
        bookTicket(sd, user_id, type);
        return;
    }

    close(fd);

    if((fd = open("bookingsDB", O_RDWR))==-1){
		printf("File Error\n");
        return;
	}

    lock.l_type = F_WRLCK;
    fcntl(fd, F_SETLK, &lock);

    struct booking temp2;
    int booking_id = 0;
    int fp = lseek(fd, 0, SEEK_END);

	if(fp==0){
		temp2.id = 1;
	}
	else{
		fp = lseek(fd, -1 * sizeof(struct booking), SEEK_CUR);
		read(fd, &temp2, sizeof(temp2));
		temp2.id++;
	}

    temp2.acc_id = user_id;
    temp2.active = 1;
    temp2.seats_booked = booked_seats;
    temp2.train_id = train_id;
    temp2.type = type;

    write(fd, &temp2, sizeof(temp2));

    write(sd, &temp2.id, sizeof(int));

    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
	close(fd);

    if((fd = open("trainsDB", O_RDWR))==-1){
		printf("File Error\n"); 
        return;
	}

    lock.l_type = F_WRLCK;
    fcntl(fd, F_SETLK, &lock);

    lseek(fd, (train_id-1)*sizeof(struct train), SEEK_SET);
    read(fd, &temp, sizeof(temp));
    temp.avail_seats = temp.avail_seats - booked_seats;
    lseek(fd, -1*sizeof(struct train), SEEK_CUR);
    write(fd, &temp, sizeof(temp));

    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
	close(fd);
}

void viewBookings(int sd, int id, int type){
    // printf("Inside view bookings\n");
    struct booking temp;
    int fd, bookings=0;
    if((fd = open("bookingsDB", O_RDWR))==-1){
		printf("File Error\n"); 
        return;
	}

    struct flock lock;
	lock.l_type = F_RDLCK;
	lock.l_start = 0;
	lock.l_len = 0;
	lock.l_whence = SEEK_SET;
	lock.l_pid = getpid();

    fcntl(fd,F_SETLKW, &lock);

    while(read(fd, &temp, sizeof(temp))>0){
        if(temp.active==1 && temp.acc_id==id && temp.type==type){
            bookings++;
        }
    }
    write(sd, &bookings, sizeof(int));
    if(bookings==0){
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
        close(fd);
        return;
    }

    lseek(fd, 0, SEEK_SET);
    while(read(fd, &temp, sizeof(temp))>0){
        if(temp.active==1 && temp.acc_id==id && temp.type==type){
            write(sd, &temp, sizeof(temp));
        }
    }

    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
	close(fd);
    return;
}

void cancelBooking(int sd, int id, int type){
    // printf("Inside cancel booking\n");
    int booking_id;
    read(sd, &booking_id, sizeof(int));
    int fd1, fd2, valid=0;
    if((fd1 = open("bookingsDB", O_RDWR))==-1){
		printf("File Error\n");
        return;
	}
    if((fd2 = open("trainsDB", O_RDWR))==-1){
		printf("File Error\n");
        return;
	}
    struct flock lock;
	lock.l_type = F_WRLCK;
	lock.l_start = 0;
	lock.l_len = 0;
	lock.l_whence = SEEK_SET;
	lock.l_pid = getpid();

    fcntl(fd1,F_SETLKW, &lock);
    fcntl(fd2,F_SETLKW, &lock);

    struct booking temp1;
    struct train temp2;

    while(read(fd1, &temp1, sizeof(temp1))>0){
        if(temp1.active==1 && temp1.id==booking_id && temp1.acc_id==id && temp1.type==type){
            valid = 1;
            while(read(fd2, &temp2, sizeof(temp2))>0){
                if(temp2.active==1 && temp2.id==temp1.train_id){
                    temp2.avail_seats = temp2.avail_seats + temp1.seats_booked;
                    break;
                }
            }
        }
    }
    write(sd, &valid, sizeof(int));

    if(valid==0){
        lock.l_type = F_UNLCK;
        fcntl(fd1, F_SETLK, &lock);
        fcntl(fd2, F_SETLK, &lock);
        close(fd1);
        close(fd2);
        cancelBooking(sd, id, type);
        return;
    }

    lseek(fd2, (-1)*sizeof(struct train), SEEK_CUR);
    write(fd2, &temp2, sizeof(temp2));

    lseek(fd1, (booking_id-1)*sizeof(struct booking), SEEK_SET);
    temp1.active = 0;
    write(fd1, &temp1, sizeof(temp1));

    lock.l_type = F_UNLCK;
    fcntl(fd1, F_SETLK, &lock);
    fcntl(fd2, F_SETLK, &lock);
    close(fd1);
    close(fd2);
}

void updateBooking(int sd, int id, int type){
    // printf("Inside update booking\n");
    int booking_id, value, bookings=0, fd1, fd2, valid=0;

    struct booking temp1;
    struct train temp2;

    if((fd1 = open("bookingsDB", O_RDWR))==-1){
		printf("File Error\n");
        return;
	}

    while(read(fd1, &temp1, sizeof(temp1))>0){
        if(temp1.active==1 && temp1.acc_id==id && temp1.type==type){
            bookings++;
        }
    }
    close(fd1);
    write(sd, &bookings, sizeof(int));
    if(bookings==0) return;

    viewBookings(sd, id, type);

    read(sd, &booking_id, sizeof(int));
    read(sd, &value, sizeof(int));

    if((fd1 = open("bookingsDB", O_RDWR))==-1){
		printf("File Error\n"); 
        return;
	}
    if((fd2 = open("trainsDB", O_RDWR))==-1){
		printf("File Error\n");
        return;
	}

    struct flock lock;
	lock.l_type = F_WRLCK;
	lock.l_start = 0;
	lock.l_len = 0;
	lock.l_whence = SEEK_SET;
	lock.l_pid = getpid();

    fcntl(fd1,F_SETLKW, &lock);
    fcntl(fd2,F_SETLKW, &lock);
    int train_avail = 0;

    while(read(fd1, &temp1, sizeof(temp1))>0){
        if(temp1.active==1 && temp1.id==booking_id && temp1.acc_id==id && temp1.type==type){
            valid = 1;
            while(read(fd2, &temp2, sizeof(temp2))>0){
                if(temp2.active==1 && temp2.id==temp1.train_id && temp2.avail_seats>=value){
                    train_avail = 1;
                    break;
                }
            }
            break;
        }
    }
    write(sd, &train_avail, sizeof(int));
    if(train_avail==0){
        lock.l_type = F_UNLCK;
        fcntl(fd1, F_SETLK, &lock);
        fcntl(fd2, F_SETLK, &lock);
        close(fd1);
        close(fd2);
        return;
    }
    write(sd, &valid, sizeof(int));

    if(valid==0){
        lock.l_type = F_UNLCK;
        fcntl(fd1, F_SETLK, &lock);
        fcntl(fd2, F_SETLK, &lock);
        close(fd1);
        close(fd2);
        updateBooking(sd, id, type);
        return;
    }

    lseek(fd2, (temp2.id-1)*sizeof(struct train), SEEK_SET);
    temp2.avail_seats = temp2.avail_seats + temp1.seats_booked - value;
    write(fd2, &temp2, sizeof(temp2));

    lseek(fd1, (booking_id-1)*sizeof(struct booking), SEEK_SET);
    temp1.seats_booked = value;
    write(fd1, &temp1, sizeof(temp1));

    lock.l_type = F_UNLCK;
    fcntl(fd1, F_SETLK, &lock);
    fcntl(fd2, F_SETLK, &lock);
    close(fd1);
    close(fd2);
}

int user_menu(int sd, int id, int type){
    // printf("Inside user_menu\n");
    int choice;
    read(sd, &choice, sizeof(int));

    switch (choice)
    {
        case 1:
            bookTicket(sd, id, type);
            break;
        case 2:
            viewBookings(sd, id, type);
            break;
        case 3:
            updateBooking(sd, id, type);
            break;
        case 4:
            cancelBooking(sd, id, type);
            break;
        case 5:
            printf("Logged out successfully!\n");
            return -1;
        default:
            printf("Default\n");
            break;
    }
    return choice;
}

void authenticate(int sd, int choice){
    // printf("Inside authentication");
    char username[100], password[100];
    read(sd, &username, sizeof(username));
	read(sd, &password, sizeof(password));
    int fd;
    if((fd = open(ACCOUNT[choice-1], O_RDWR))==-1){
        printf("File Error\n");
        return;
    }

    struct account temp;
    int id, valid=0;
    // printf("%s, %s\n", username, password);

    while(read(fd, &temp, sizeof(temp))>0){
        // printf("%s, %s, %d\n", temp.username, temp.password, temp.active);
        if(temp.active==1 && strcmp(temp.username, username)==0 && strcmp(temp.password, password)==0){
            id = temp.id;
            valid = 1;
            break;
        }
    }
    close(fd);

    // printf("Valid: %d\n", valid);
    if(valid==0){
        write(sd, &valid, sizeof(valid));
        return;
    }

    fd = open(ACCOUNT[choice-1], O_RDWR);

    struct flock lock;
	
	lock.l_start = (id-1)*sizeof(struct account);
	lock.l_len = sizeof(struct account);
	lock.l_whence = SEEK_SET;
	lock.l_pid = getpid();

    if(choice==1){
        lock.l_type = F_WRLCK;
        fcntl(fd,F_SETLKW, &lock);
        write(sd, &valid, sizeof(valid));
        while(user_menu(sd, id, choice)!=-1);
    }
    else if(choice==2){
        lock.l_type = F_RDLCK;
        fcntl(fd,F_SETLKW, &lock);
        write(sd, &valid, sizeof(valid));
        while(user_menu(sd, id, choice)!=-1);
    }
    else{
        lock.l_type = F_WRLCK;
        fcntl(fd,F_SETLKW, &lock);
        write(sd, &valid, sizeof(valid));
        while(admin_menu(sd, id)!=-1);
    }

    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
}

void service_cli(int sd){
    int choice;
	printf("Client connected\n");
	while(1){		
		read(sd, &choice, sizeof(int));
        if(choice==1 || choice==2 || choice==3) authenticate(sd, choice);
		else { break;}
	}
	close(sd);
	printf("Client disconnected\n");
}

void main(){
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd==-1) {
		printf("socket creation failed\n");
	}
	int optval = 1;
	int optlen = sizeof(optval);
	/*
	to close socket automatically while terminating process
	SOL_SOCKET : To  manipulate options  at  the  sockets API level
    SO_REUSEADDR: Bind will permit reuse of local addresses for this socket
	*/
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &optval, optlen)==-1){
		printf("set socket options failed\n");
	}
	struct sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	sa.sin_port = htons(PORT);

	printf("Binding socket...\n");
	if(bind(sockfd, (struct sockaddr *)&sa, sizeof(sa))==-1){
		printf("binding port failed\n");
	}

	if(listen(sockfd, 5)==-1){
		printf("listen failed\n");
	}
	printf("Listening...\n");
	while(1){ 
		int connectedfd;
		if((connectedfd = accept(sockfd, (struct sockaddr *)NULL, NULL))==-1){
			printf("connection error\n");
		}

		if(fork()==0){
			service_cli(connectedfd);
			exit(1);
		}
	}
	close(sockfd);
	printf("Connection closed!\n");
}