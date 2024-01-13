#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
# include <string.h>
# define PORT 3000
 
 
typedef struct  sockaddr  SOCKADDR;
typedef struct  sockaddr_in SOCKADDR_IN;
typedef int SOCKET;
char response[1000];
 
int main(int argc, char **argv) {
    SOCKET sock;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock!= -1) {
        char lien[10];
        int i = 0;
        for (int i = 0; i < 10; i++)
            lien[i] = argv[1][i];
        struct hostent *host = gethostbyname(lien);
        SOCKADDR_IN  sin;
        bcopy(host->h_addr, &(sin.sin_addr), host->h_length);
    
        sin.sin_family= AF_INET;
        sin.sin_port= htons(PORT);
    
        if(connect(sock, (SOCKADDR*)&sin, sizeof sin)!= -1) {
            char buffer[1024];
            FILE *fptr ;
            fptr = fopen("entete.txt", "r");
            char c;
            int i = 0;
            while ((c = fgetc(fptr)) != EOF) {
                if (c == '\n') {
                    buffer[i] ='\r';
                    i++;
                    buffer[i] = '\n';
                    i++;
                }
                buffer[i] = c;
                i++;
            }
            buffer[i] = '\r';
            buffer[i + 1] = '\n';
            buffer[i + 2] = '\r';
            buffer[i + 3] = '\n';
            i += 4;
            int n = send(sock, buffer, i + 2, 0);

            if(n!=-1) {
                recv(sock, response, 1000,0);
                int write = 0;
                for (i = 0; i < 1000; i++) {
                    if (response[i] == '<')
                            write = 1;
                    if (write)
                        printf("%c", response[i]);
                }
            }
            
        }
    }
}