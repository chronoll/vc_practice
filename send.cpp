#include <iostream>
#include <string>

#include<sys/socket.h>

using namespace std;

void HTTP_Send(int YOUR_ID, int SEND_TO_ID, int TAG, void *p, int LEN, const char *TYPE)
{
    int data_length = 0;
    string body, filename, flag_status;
    
    while (1) {
        int dstSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (dstSocket < 0) printf("ソケット生成エラー\n");

        int result = connect(dstSocket, (struct sockaddr *)&(this->dstAddr), sizeof(this->dstAddr));

    }

}