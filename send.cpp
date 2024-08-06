#include <iostream>
#include <string>

#include <sys/socket.h>
#include <netinet/in.h>
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
    int dstSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (dstSocket < 0) printf("ソケット生成エラー\n");

    struct sockaddr_in dstAddr;

    /* 接続 */
    int result = connect(dstSocket, (struct sockaddr *)&(dstAddr), sizeof(dstAddr));
    if (result < 0) {
        printf("バインドエラー\n");
        exit(1);
    }

    /* headerの送信 (normal_header) */
    char *text = (char *)malloc(sizeof(char)*512);

    // http header
    sprintf(text, "POST %s?ID_%d HTTP/1.1\r\n", httppath[ID], YOUR_ID);
    send(dstSocket, text, strlen(text), 0);

    // set host
    sprintf(text, "Host: %s:%d\r\n", httphost, port);
    send(dstSocket, text, strlen(text), 0);

    // コンテンツタイプの指定
    sprintf(text, "Content-Type: application/x-www-form-urlencoded\r\n");
    send(dstSocket, text, strlen(text), 0);

    // コンテンツの長さ(バイト)を指定する
    sprintf(text, "Content-Length: %d\r\n", (int)body.length());
    send(dstSocket, text, strlen(text), 0);

    sprintf(text, "Connection: Close\r\n");
    send(dstSocket, text, strlen(text), 0);

    sprintf(text, "\r\n");
    send(dstSocket, text, strlen(text), 0);

    /* bodyの送信 (sock.mysend) */
    send(dstSocket, body.c_str(), (int)body.length(), 0);

    /* socket close */
    close(dstSocket);

    /*************************************************************************/

    /* ソケット生成 */
    int dstSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (dstSocket < 0) printf("ソケット生成エラー\n");

    /* 接続 */
    int result = connect(dstSocket, (struct sockaddr *)&(dstAddr), sizeof(dstAddr));
    if (result < 0) {
        printf("バインドエラー\n");
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
    char* text=(char*)malloc(sizeof(char)*512);

    sprintf(text, "POST %s?ID_%d HTTP/1.1\r\n", httppath[DATA], YOUR_ID);
    send(dstSocket, text, strlen(text), 0);

    sprintf(text, "Host: %s:%d\r\n", httphost, port);
    send(dstSocket, text, strlen(text), 0);

    sprintf(text, "Content-Length: %d\r\n", data_length);
    send(dstSocket, text, strlen(text), 0);

    sprintf(text, "Connection: Close\r\n");
	send(dstSocket, text, strlen(text), 0);

    sprintf(text, "Content-Type: multipart/form-data; boundary=xYzZY\r\n");
    send(dstSocket, text, strlen(text), 0);

    sprintf(text, "\r\n"); 
    send(dstSocket, text, strlen(text), 0);

    sprintf(text, "--xYzZY\r\n");
    send(dstSocket, text, strlen(text), 0);

    sprintf(text, "Content-Disposition: form-data; name=\"file\"; filename=%s\r\n", filename.c_str());
    send(dstSocket, text, strlen(text), 0);

    sprintf(text, "Content-Type: text/plain\r\n");
    send(dstSocket, text, strlen(text), 0);

    sprintf(text, "\r\n");
    send(dstSocket, text, strlen(text), 0);

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
		send(dstSocket, toSendText, strlen(toSendText), 0);
    }

    // text_end
    char* text=(char*)malloc(sizeof(char)*512);

    sprintf(text, "\r\n");
    send(dstSocket, text, strlen(text), 0);

    sprintf("text", "--xYzZY--\r\n");
    send(dstSocket, text, strlen(text), 0);

    sprintf(text, "\r\n");
    send(dstSocket, text, strlen(text), 0);

    /* socket close */
    close(dstSocket);

    /*************************************************************************/

    body = "DATA_TITTLE=" + filename + "&TAG=" + to_string(TAG);
    ok = false;

    while (1) {
        /* ソケット生成 */
        int dstSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (dstSocket < 0) printf("ソケット生成エラー\n");

        /* 接続 */
        int result = connect(dstSocket, (struct sockaddr *)&(dstAddr), sizeof(dstAddr));
        if (result < 0) {
            printf("バインドエラー\n");
            exit(1);
        }

        /* headerの送信 (normal_header) */
        char *text = (char *)malloc(sizeof(char)*512);

        // http header
        sprintf(text, "POST %s?ID_%d HTTP/1.1\r\n", httppath[FLAG], YOUR_ID);
        send(dstSocket, text, strlen(text), 0);

        // set host
        sprintf(text, "Host: %s:%d\r\n", httphost, port);
        send(dstSocket, text, strlen(text), 0);

        // コンテンツタイプの指定
        sprintf(text, "Content-Type: application/x-www-form-urlencoded\r\n");
        send(dstSocket, text, strlen(text), 0);

        // コンテンツの長さ(バイト)を指定する
        sprintf(text, "Content-Length: %d\r\n", (int)body.length());
        send(dstSocket, text, strlen(text), 0);

        sprintf(text, "Connection: Close\r\n");
        send(dstSocket, text, strlen(text), 0);

        sprintf(text, "\r\n");
        send(dstSocket, text, strlen(text), 0);

        /* bodyの送信 (sock.mysend) */
        send(dstSocket, body.c_str(), (int)body.length(), 0);

        /* socket close */
        close(dstSocket);

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