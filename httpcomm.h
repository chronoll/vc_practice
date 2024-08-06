#ifndef MYMPI_H
#define MYMPI_H

void HTTP_Send(int YOUR_ID, int SEND_TO_ID, int TAG, void *p, int LEN, const char *TYPE);
void HTTP_Recv(int YOUR_ID, int RECEIVE_FROM_ID, int TAG, void *p, int LEN, const char *TYPE);

#endif