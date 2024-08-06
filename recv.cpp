#include <iostream>
#include <string>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define BUF_LEN 1024 /* バッファのサイズ */
#define TWAIT 100000 // 再接続の待ち時間（マイクロ秒

#define recDATA 0

using namespace std;

int remove_response_header(char *buf,int buf_size);

void HTTP_Recv(int YOUR_ID, int RECEIVE_FROM_ID, int TAG, void *p, int LEN, const char *TYPE) {
    int data_length = 0;
    string body, filename, flag_status;

    char buf[BUF_LEN + 2];
    int read_size = 0;
    char *recv_DATA_P = (char *)p;
    bool ok;

    const char *httppath[] = { "/VC/send_recv/MPI_Recv_DATA.php" };

    unsigned short port = 80;
    char httphost[20] = "127.0.0.1";

    filename = to_string(TAG) + "_SendID_" + to_string(RECEIVE_FROM_ID) + "_" + to_string(YOUR_ID) + ".txt";
    body = "YOUR_ID" + to_string(YOUR_ID) + "&TAG=" + to_string(TAG) + "&RECEIVE_FROM_ID" + to_string(RECEIVE_FROM_ID) + "&DATA_TITTLE" + filename;

    ok = false;

    while (1) {
        int dstSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (dstSocket < 0) printf("ソケット生成エラー\n");

        struct sockaddr_in dstAddr;

        memset(&dstAddr, 0, sizeof(dstAddr));
        dstAddr.sin_port = htons(port);
	    dstAddr.sin_family = AF_INET;
	    dstAddr.sin_addr.s_addr = inet_addr(httphost);

        /* 接続 */
        int result = connect(dstSocket, (struct sockaddr *)&(dstAddr), sizeof(dstAddr));
        if (result < 0) {
            printf("バインドエラー\n");
            exit(1);
        }

        /* headerの送信 (normal_header) */
        char *text = (char *)malloc(sizeof(char)*512);

        // http header
        sprintf(text, "POST %s?ID_%d HTTP/1.1\r\n", httppath[recDATA], YOUR_ID);
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

        int array_position = 0; // 読み取った値を格納する配列のインデックス
        int data_id = 0; // データを送ったワーカのid
        int recv_cnt = 0; // 指定されたワーカから受け取ったデータのカウンタ

        /* _RECV_FILE */
        bool first_recv = true; // 1回目のrecvのときtrue
        bool separated = false; // 値が分断されているときtrue
        char buf_read[25]; // 値1つ分の文字列
        int buf_read_p = 0; // buf_readのインデックス
        int content_len = 0; // Content-Lengthの値

        while (1) {
            memset(buf, 0, sizeof(buf));
            read_size = recv(dstSocket, buf, BUF_LEN, 0); // レスポンスをbufに読む 成功 受信バイト数, 失敗 -1
            if (read_size == 0) {
                if (first_recv == false && content_len <= 0) break; // データをすべて受け取ったら
                else continue;
            }
            int buf_p = 0;
            while(buf[buf_p] !='\0') buf_p++;
            buf[buf_p] = '\n';
            buf[buf_p + 1] = '\0';
            buf_p = 0;

            if (first_recv) { // 1回目のみHTTPヘッダが削除される
                first_recv = false;
                content_len = remove_response_header(buf, sizeof(buf));
                if (!strncmp(buf, "OK\n", 3)) { // bufの最初がokだったら？
                    content_len = strlen(buf) - 1;
                    while ( buf[buf_p] != '\n') buf_p++;
                    buf_p++;
                    ok = true;
                } else {
                    break;
                }
            }
            else content_len -= read_size;

            while (buf[buf_p] != '\0') {
                /* 値が分断されていなければ初期化 */
                if (separated == false) {
                    memset(buf_read, 0, sizeof(buf_read));
                    buf_read_p = 0;
                } else {
                    separated = false;
                }

                while (buf[buf_p] != '\n') {
                    buf_read[buf_read_p] = buf[buf_p];
                    buf_p++;
                    buf_read_p++;
                }
                buf_p++;

                if (strncmp("\0", &buf[buf_p], 1) == 0) separated = true; // \n\0で終われば、値が分断されている
                else {
                    // 1つの値を適当な型に変換し配列に代入する操作
                    if (!strcmp(TYPE, "MPI_SHORT")) {
                        short v = (short)atoi(buf_read);
                        short *p = (short *)recv_DATA_P + array_position;
                        *p = v;
                    } else if (!strcmp(TYPE, "MPI_INT")) {
                        int v = (int)atoi(buf_read);
                        int *p = (int *)recv_DATA_P + array_position;
                        *p = v;
                    } else if (!strcmp(TYPE, "MPI_LONG")) {
                        long v = (long)atol(buf_read);
                        long *p = (long *)recv_DATA_P + array_position;
                        *p = v;
                    } else if (!strcmp(TYPE, "MPI_FLOAT")) {
                        float v = (float)atof(buf_read);
                        float *p = (float *)recv_DATA_P + array_position;
                        *p = v;
                    } else if (!strcmp(TYPE, "MPI_DOUBLE")) {
                        double v = (double)atof(buf_read);
                        double *p = (double *)recv_DATA_P + array_position;
                        *p = v;
                    }

                    array_position++;
                }
            }
        }

        /* _RECV_FILE終わり */

        /* socket close */
        close(dstSocket);

        if (ok) break;

        usleep(TWAIT);
    }
}

int remove_response_header(char *buf,int buf_size){
	int i=0;
	int content_len=0;
	char str[20];
	int str_p;
	while (1){
		// Content-Lengthを取得
		if(buf[i]=='C' && strncmp(&buf[i],"Content-Length:",strlen("Content-Length:"))==0){
			i+=strlen("Content-Length:");
			while(buf[i]==' ') i++;
			str_p=0;
			while('0'<=buf[i] && buf[i]<='9' && str_p<sizeof(str)-1){
				str[str_p++]=buf[i++];
			}
			str[str_p]='\0';
			continue;
		}
		// ヘッダの終了 (\r\n\r\n) を探す
		if (buf[i] == '\r' && buf[i + 1] == '\n' && buf[i + 2] == '\r' && buf[i + 3] == '\n'){
			i += 4;
			break;
		}
		i++;
	}

	for (int j = 0; j < buf_size; j++){
		if (i < buf_size){
			buf[j] = buf[i];
		}else{
			buf[j] = '\0';
		}
		i++;
	}
	return atoi(str);
}