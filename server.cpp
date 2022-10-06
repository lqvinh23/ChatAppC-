#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <cstdio>
#include <thread>

using namespace std;

#define MSGSZ 256
#define SHMSZ 256

#define SERVER_ID 10
#define SERVER_NAME "LG"

// message queue
struct message {
	long    mtype; //sender
	char    content[MSGSZ]; //content
    int     contentType;
    int     imgId;
    long    receiverId; //receiver
} message_buf;

int oldStatus[3];
int *status;
key_t key_server, key1, key2, key3;
int msqid_server, msqid1, msqid2, msqid3, shmid_server;
string client[3] = {"Vinh", "Thu", "Nam"};

int checkStatusChanged() {
    for(int i=0; i<3; i++) {
        if (oldStatus[i] != status[i]) {
            return 1;
        }
    }
    return 0;
}

void intermediary() {
    // pselect, epool
    while (1) {
        msgrcv(msqid_server, &message_buf, sizeof(message_buf), 0, 0); 
        if (message_buf.receiverId == 1) {
            msgsnd(msqid1, &message_buf, sizeof(message_buf), 0);
            cout <<"\nSent message to Vinh"<<endl;
        }
        if (message_buf.receiverId == 2) {
            msgsnd(msqid2, &message_buf, sizeof(message_buf), 0);
            cout <<"\nSent message to Thu"<<endl;
        }
        if (message_buf.receiverId == 3) {
            msgsnd(msqid3, &message_buf, sizeof(message_buf), 0);
            cout <<"\nSent message to Nam"<<endl;
        }
    }
}

void manageStatus() {
    while (1) {
        if (checkStatusChanged()) {
            cout << endl;
            for (int i=0; i<3; i++) {
                cout << client[i] << ": " << ((status[i] == 0) ? "Offline" : "Online") << endl;
                oldStatus[i] = status[i];
            }
        }
    }
}

int main() {
    cout << "Server started" << endl;

    // ftok to generate unique key
    key_server = ftok("vinh.txt", SERVER_ID);
    key1 = ftok("vinh.txt", 1);
    key2 = ftok("vinh.txt", 2);
    key3 = ftok("vinh.txt", 3);

    // key_server = 10;
    // key1 = 11;
    // key2 = 12;
    // key3 = 13;

    // cout << key_server << endl << key1 << key2 << key3 << endl;

    // msgget creates a message queue and returns identifier
    msqid_server = msgget(key_server,  0666 | IPC_CREAT );
    msqid1 = msgget(key1, 0666 | IPC_CREAT);
    msqid2 = msgget(key2, 0666 | IPC_CREAT);
    msqid3 = msgget(key3, 0666 | IPC_CREAT);

    // shmget returns an identifier in shmid
    shmid_server = shmget(key_server, 1024, 0666 | IPC_CREAT);

    // shmat to attach to shared memory
    status = (int*) shmat(shmid_server,(void*)0,0);

    for(int i=0; i<3; i++) {
        status[i] = 0;
    }

    for(int i=0; i<3; i++) {
        cout << client[i] << ": " << ((status[i] == 0) ? "Offline" : "Online") << endl;
        oldStatus[i] = status[i];
    }

    thread t1(intermediary);
    thread t2(manageStatus);

    t1.join();
    t2.join();

    return 0;
}