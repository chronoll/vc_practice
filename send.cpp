#include <iostream>
#include <string>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

// phpファイルの指定に使用
#define ID 0
#define DATA 1
#define recDATA 2
#define FLAG 3

#define BUF_LEN 1024 /* バッファのサイズ */
#define TWAIT 100000 // 再接続の待ち時間（マイクロ秒）

using namespace std;

int text_end_len(const char *filename);

void HTTP_Send(int YOUR_ID, int SEND_TO_ID, int TAG, void *p, int LEN, const char *TYPE)
{
    int data_length = 0;
    string body, filename, flag_status;
    char toSendText[BUF_LEN];
    char *send_DATA_P = (char *)p;
    bool ok;

    const char *httppath[] = {
		"/VC/send_recv/MPI_Send_ID.php",
		"/VC/send_recv/MPI_Send_DATA.php",
		"",
		"/VC/send_recv/MPI_Send_FLAG.php"
    };

    unsigned short port = 80;
    char httphost[20] = "127.0.0.1";

    body = "YOUR_ID=" + to_string(YOUR_ID) + "&SEND_TO_ID=" + to_string(SEND_TO_ID) + "&TAG=" + to_string(TAG);
    filename = to_string(TAG) + "_SendID_" + to_string(YOUR_ID) + "_" + to_string(SEND_TO_ID) + ".txt";

    /*************************************************************************/
    
    /* ソケット生成 */
    int dstSocket_ID = socket(AF_INET, SOCK_STREAM, 0);
    if (dstSocket_ID < 0) printf("ID:ソケット生成エラー\n");

    struct sockaddr_in dstAddr_ID;

    memset(&dstAddr_ID, 0, sizeof(dstAddr_ID));
	dstAddr_ID.sin_port = htons(port);
	dstAddr_ID.sin_family = AF_INET;
	dstAddr_ID.sin_addr.s_addr = inet_addr(httphost);

    /* 接続 */
    int result_ID = connect(dstSocket_ID, (struct sockaddr *)&(dstAddr_ID), sizeof(dstAddr_ID));
    if (result_ID < 0) {
        printf("ID:バインドエラー\n");
        exit(1);
    }

    /* headerの送信 (normal_header) */
    char *text_ID = (char *)malloc(sizeof(char)*512);

    // http header
    sprintf(text_ID, "POST %s?ID_%d HTTP/1.1\r\n", httppath[ID], YOUR_ID);
    send(dstSocket_ID, text_ID, strlen(text_ID), 0);

    // set host
    sprintf(text_ID, "Host: %s:%d\r\n", httphost, port);
    send(dstSocket_ID, text_ID, strlen(text_ID), 0);

    // コンテンツタイプの指定
    sprintf(text_ID, "Content-Type: application/x-www-form-urlencoded\r\n");
    send(dstSocket_ID, text_ID, strlen(text_ID), 0);

    // コンテンツの長さ(バイト)を指定する
    sprintf(text_ID, "Content-Length: %d\r\n", (int)body.length());
    send(dstSocket_ID, text_ID, strlen(text_ID), 0);

    sprintf(text_ID, "Connection: Close\r\n");
    send(dstSocket_ID, text_ID, strlen(text_ID), 0);

    sprintf(text_ID, "\r\n");
    send(dstSocket_ID, text_ID, strlen(text_ID), 0);

    /* bodyの送信 (sock.mysend) */
    send(dstSocket_ID, body.c_str(), (int)body.length(), 0);

    /* socket close */
    close(dstSocket_ID);

    /*************************************************************************/

    /* ソケット生成 */
    int dstSocket_DATA = socket(AF_INET, SOCK_STREAM, 0);
    if (dstSocket_DATA < 0) printf("DATA:ソケット生成エラー\n");

    struct sockaddr_in dstAddr_DATA;

    memset(&dstAddr_DATA, 0, sizeof(dstAddr_DATA));
	dstAddr_DATA.sin_port = htons(port);
	dstAddr_DATA.sin_family = AF_INET;
	dstAddr_DATA.sin_addr.s_addr = inet_addr(httphost);

    /* 接続 */
    int result_DATA = connect(dstSocket_DATA, (struct sockaddr *)&(dstAddr_DATA), sizeof(dstAddr_DATA));
    if (result_DATA < 0) {
        printf("DATA:バインドエラー\n");
        exit(1);
    }

    /* HTTP Body部の文字数を計算 */
    for (int i = 0; i < LEN; i++) {
        memset(toSendText, 0, sizeof(toSendText));
        if (!strcmp(TYPE, "MPI_CHAR"))
			sprintf(toSendText, "%c\n", send_DATA_P[i]);
		else if (!strcmp(TYPE, "MPI_SHORT"))
			sprintf(toSendText, "%d\n", ((short*)send_DATA_P)[i]);
		else if (!strcmp(TYPE, "MPI_INT"))
			sprintf(toSendText, "%d\n", ((int*)send_DATA_P)[i]);
		else if (!strcmp(TYPE, "MPI_LONG"))
			sprintf(toSendText, "%ld\n", ((long*)send_DATA_P)[i]);
		else if (!strcmp(TYPE, "MPI_FLOAT"))
			sprintf(toSendText, "%f\n", ((float*)send_DATA_P)[i]);
		else if (!strcmp(TYPE, "MPI_DOUBLE"))
			sprintf(toSendText, "%lf\n", ((double*)send_DATA_P)[i]);
        data_length += strlen(toSendText);
    }

    data_length += text_end_len(filename.c_str());

    /* headerの送信(file_header) */
    char* text_DATA=(char*)malloc(sizeof(char)*512);

    sprintf(text_DATA, "POST %s?ID_%d HTTP/1.1\r\n", httppath[DATA], YOUR_ID);
    send(dstSocket_DATA, text_DATA, strlen(text_DATA), 0);

    sprintf(text_DATA, "Host: %s:%d\r\n", httphost, port);
    send(dstSocket_DATA, text_DATA, strlen(text_DATA), 0);

    sprintf(text_DATA, "Content-Length: %d\r\n", data_length);
    send(dstSocket_DATA, text_DATA, strlen(text_DATA), 0);

    sprintf(text_DATA, "Connection: Close\r\n");
	send(dstSocket_DATA, text_DATA, strlen(text_DATA), 0);

    sprintf(text_DATA, "Content-Type: multipart/form-data; boundary=xYzZY\r\n");
    send(dstSocket_DATA, text_DATA, strlen(text_DATA), 0);

    sprintf(text_DATA, "\r\n"); 
    send(dstSocket_DATA, text_DATA, strlen(text_DATA), 0);

    sprintf(text_DATA, "--xYzZY\r\n");
    send(dstSocket_DATA, text_DATA, strlen(text_DATA), 0);

    sprintf(text_DATA, "Content-Disposition: form-data; name=\"file\"; filename=%s\r\n", filename.c_str());
    send(dstSocket_DATA, text_DATA, strlen(text_DATA), 0);

    sprintf(text_DATA, "Content-Type: text/plain\r\n");
    send(dstSocket_DATA, text_DATA, strlen(text_DATA), 0);

    sprintf(text_DATA, "\r\n");
    send(dstSocket_DATA, text_DATA, strlen(text_DATA), 0);

    /* Bodyの送信 */
    for (int i = 0; i < LEN; i++) {
        memset(toSendText, 0, sizeof(toSendText));
		if (!strcmp(TYPE, "MPI_CHAR"))
			sprintf(toSendText, "%c\n", send_DATA_P[i]);
		else if (!strcmp(TYPE, "MPI_SHORT"))
			sprintf(toSendText, "%d\n", ((short*)send_DATA_P)[i]);
		else if (!strcmp(TYPE, "MPI_INT"))
			sprintf(toSendText, "%d\n", ((int*)send_DATA_P)[i]);
		else if (!strcmp(TYPE, "MPI_LONG"))
			sprintf(toSendText, "%ld\n", ((long*)send_DATA_P)[i]);
		else if (!strcmp(TYPE, "MPI_FLOAT"))
			sprintf(toSendText, "%f\n", ((float*)send_DATA_P)[i]);
		else if (!strcmp(TYPE, "MPI_DOUBLE"))
			sprintf(toSendText, "%lf\n", ((double*)send_DATA_P)[i]);
		send(dstSocket_DATA, toSendText, strlen(toSendText), 0);
    }

    // text_end
    char* text_end=(char*)malloc(sizeof(char)*512);

    sprintf(text_end, "\r\n");
    send(dstSocket_DATA, text_end, strlen(text_end), 0);

    sprintf(text_end, "--xYzZY--\r\n");
    send(dstSocket_DATA, text_end, strlen(text_end), 0);

    sprintf(text_end, "\r\n");
    send(dstSocket_DATA, text_end, strlen(text_end), 0);

    /* socket close */
    close(dstSocket_DATA);

    /*************************************************************************/

    body = "DATA_TITTLE=" + filename + "&TAG=" + to_string(TAG);
    ok = false;

    while (1) {
        /* ソケット生成 */
        int dstSocket_FLAG = socket(AF_INET, SOCK_STREAM, 0);
        if (dstSocket_FLAG < 0) printf("FLAG:ソケット生成エラー\n");

        struct sockaddr_in dstAddr_FLAG;

        memset(&dstAddr_FLAG, 0, sizeof(dstAddr_FLAG));
        dstAddr_FLAG.sin_port = htons(port);
	    dstAddr_FLAG.sin_family = AF_INET;
	    dstAddr_FLAG.sin_addr.s_addr = inet_addr(httphost);

        /* 接続 */
        int result_FLAG = connect(dstSocket_FLAG, (struct sockaddr *)&(dstAddr_FLAG), sizeof(dstAddr_FLAG));
        if (result_FLAG < 0) {
            printf("FLAG:バインドエラー\n");
            exit(1);
        }

        /* headerの送信 (normal_header) */
        char *text_FLAG = (char *)malloc(sizeof(char)*512);

        // http header
        sprintf(text_FLAG, "POST %s?ID_%d HTTP/1.1\r\n", httppath[FLAG], YOUR_ID);
        send(dstSocket_FLAG, text_FLAG, strlen(text_FLAG), 0);

        // set host
        sprintf(text_FLAG, "Host: %s:%d\r\n", httphost, port);
        send(dstSocket_FLAG, text_FLAG, strlen(text_FLAG), 0);

        // コンテンツタイプの指定
        sprintf(text_FLAG, "Content-Type: application/x-www-form-urlencoded\r\n");
        send(dstSocket_FLAG, text_FLAG, strlen(text_FLAG), 0);

        // コンテンツの長さ(バイト)を指定する
        sprintf(text_FLAG, "Content-Length: %d\r\n", (int)body.length());
        send(dstSocket_FLAG, text_FLAG, strlen(text_FLAG), 0);

        sprintf(text_FLAG, "Connection: Close\r\n");
        send(dstSocket_FLAG, text_FLAG, strlen(text_FLAG), 0);

        sprintf(text_FLAG, "\r\n");
        send(dstSocket_FLAG, text_FLAG, strlen(text_FLAG), 0);

        /* bodyの送信 (sock.mysend) */
        send(dstSocket_FLAG, body.c_str(), (int)body.length(), 0);

        /* socket close */
        close(dstSocket_FLAG);

        if (ok) {
            break;
        }

        usleep(TWAIT);
    }
}

int text_end_len(const char *filename) {
	char* text=(char*)malloc(sizeof(char)*512);
	int len=0;
	sprintf(text, "--xYzZY\r\n");
	len+=strlen(text);

	sprintf(text, "Content-Disposition: form-data; name=\"file\"; filename=%s\r\n",filename);
	len+=strlen(text);

	sprintf(text, "Content-Type: text/plain\r\n");
	len+=strlen(text);

	sprintf(text, "\r\n");
	len+=strlen(text);

	sprintf(text, "\r\n");
	len+=strlen(text);

	sprintf(text, "--xYzZY--\r\n");
	len+=strlen(text);

	sprintf(text, "\r\n");
	len+=strlen(text);
	return len;
}