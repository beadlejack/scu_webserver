/*
 * Jackson Beadle
 * COEN 317 - Programming Assignment 1
 *
 * Socket programming adapted in part from the
 * SimpleEchoSocket example posted on Camino.
*/


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/stat.h>
#include <mach/error.h>
#include <time.h>

#define BUFSIZE 1024

/* global variables */
char *document_root;
/* static strings for 400, 403, 404 responses */
static char* not_found_response =
  "HTTP/1.0 404 Not Found\r\n"
  "Content-type: text/html\r\n"
  "Content-length: 122\r\n"
  "Date: ";

static char* not_found_body =
  "<html><head><title>404: Not Found</title></head>\r\n"
  "<body><h1>Not Found</h1><p>Requested URL not found</p></body>\r\n"
  "</html>\r\n";

static char* forbidden_response =
  "HTTP/1.0 403 Forbidden\r\n"
  "Content-type: text/html\r\n"
  "Content-length: 151\r\n"
  "Date: ";

static char* forbidden_body =
  "<html><head><title>403: Forbidden</title></head>\r\n"
  "<body><h1>Forbidden</h1><p>You do not not have permission to view this resource</p>\r\n"
  "</body></html>\r\n";

static char* bad_request_response =
  "HTTP/1.0 400 Bad Request\r\n"
  "Content-type: text/html\r\n"
  "Content-length: 119\r\n"
  "Date: ";

static char* bad_request_body =
  "<html><head><title>400: Bad Request</title></head>\r\n"
  "<body><h1>Bad request</h1><p>Bad HTTP request</p></body>\r\n"
  "</html>\r\n";


/*
 * error - wrapper for perror
 */
void error(char *msg) {
    perror(msg);
    exit(0);
}

/*
 * respond(int socket): processing function once connection
 * has been accepted. Does all the parsing of the HTTP request,
 * creates the HTTP response, and writes to socket.
 */
void respond(int connfd) {

  /* variables */
  char buf[BUFSIZE];  /* buffer for reading */
  int n;
  char t1[10], t2[200], t3[10];  /* GET, resource, HTTP/1.0 */
  char *temp;
  int flag = 0;
  FILE *fp;
  char filename[200];
  struct stat st; /* for checking file status */
  int res;  /* for checking file status */
  char contenttype[15];  /* for Content-type header */
  long contentsize = 0; /* file size */

  /* read() HTTP request */
  bzero(buf, BUFSIZE);
  n = read(connfd, buf, BUFSIZE);
  if (n < 0)
    error("Error on read()");
  // printf("received %d bytes: %s", n, buf);

  /* generate Date field string for header */
  char timestr[200];
  time_t now = time(0);
  struct tm tm = *gmtime(&now);
  strftime(timestr, 200, "%a, %d %b %Y %H:%M:%S %Z", &tm);
  //printf("Current time: %s\n", timestr);

  /* t1 = "GET", t2 = resource, t3 = http type */
  sscanf(buf, "%s %s %s", t1, t2, t3);

  /* if first line is formed correctly */
  if ((strncmp(t1, "GET", 3) == 0) &&
      ((strncmp(t3, "HTTP/1.0", 8) == 0) ||(strncmp(t3, "HTTP/1.1", 8)== 0))) {


    //printf("resource = %s\n", t2);

    /* correct "/" to "/index.html" */
    if ((strcmp(t2, "/") == 0)) {
      bzero(t2, 200);
      sprintf(t2, "/index.html");
      flag = 1;
    }



  } else {
    /* malformed HTTP, send 400 */
    // error("Error: malformed HTTP");
    bzero(buf, BUFSIZE);
    sprintf(buf, "%s%s\r\n%s", bad_request_response, timestr, bad_request_body);
    write(connfd, buf, strlen(buf));
    return;
  }

  /* check if document_root ends in '/' */
  if (document_root[strlen(document_root) - 1] == '/')
    document_root[strlen(document_root) - 1] = '\0';
  /* form file path */
  sprintf(filename, "%s%s", document_root, t2);
  //printf("Filename: %s\n", filename);


  /* printf() to find length of error content */
  // printf("404: %lu\n", strlen(not_found_body));
  // printf("403: %lu\n", strlen(forbidden_body));
  // printf("400: %lu\n", strlen(bad_request_body));

  /* check for files that AREN'T html, txt, gif, jpg */
  /* t2 holds resource */
  temp = strrchr(t2, '.');
  // printf("resource file ext: %s\n", temp);
  if (strncmp(temp, ".html", 5) == 0)
    sprintf(contenttype, "text/html");
  else if (strncmp(temp, ".txt", 4) == 0)
    sprintf(contenttype, "text/txt");
  else if (strncmp(temp, ".js", 3) == 0)
    sprintf(contenttype, "text/js");
  else if (strncmp(temp, ".css", 4) == 0)
    sprintf(contenttype, "text/css");
  else if (strncmp(temp, ".jpg", 4) == 0)
    sprintf(contenttype, "image/jpg");
  else if (strncmp(temp, ".gif", 4) == 0)
    sprintf(contenttype, "image/gif");
  else if (strncmp(temp, ".svg", 4) == 0)
    sprintf(contenttype, "image/svg");
  else if (strncmp(temp, ".png", 4) == 0)
    sprintf(contenttype, "image/png");
  else {
    /* invalid file type, send 400 */
    // error("Error: malformed HTTP");
    bzero(buf, BUFSIZE);
    sprintf(buf, "%s%s\r\n\r\n%s", bad_request_response, timestr, bad_request_body);
    write(connfd, buf, strlen(buf));
    return;
  }

  /* check file status */
  res = stat(filename, &st);
  if (res != 0) {
    if (errno == ENOENT) {
      /* file not found, send 404 */
      // error("Error: file not found");
      bzero(buf, BUFSIZE);
      sprintf(buf, "%s%s\r\n\r\n%s", not_found_response, timestr, not_found_body);
      write(connfd, buf, strlen(buf));
      return;
    } else if (errno == EACCES) {
      /* no access, send 403 */
      // error("Error: no access");
      bzero(buf, BUFSIZE);
      sprintf(buf, "%s%s\r\n\r\n%s", forbidden_response, timestr, forbidden_body);
      write(connfd, buf, strlen(buf));
      return;
    }
  } else { /* file found and ready to open */
    fp = fopen(filename, "r");
    if (fp == NULL) {
      error("Error opening file");
    }
  }

  /* file is open, form HTTP headers, then read and write() file */

  /* find file size */
  fseek(fp, 0L, SEEK_END);
  contentsize = ftell(fp);
  rewind(fp);
  // printf("File size: %ld\n", contentsize);

  /* write header */
  bzero(buf, BUFSIZE);
  sprintf(buf, "HTTP/1.0 200 Ok\r\nContent-type: %s\r\nContent-length: %ld\r\nDate: %s\r\n\r\n",
      contenttype, contentsize, timestr);
  write(connfd, buf, strlen(buf));

  /* read file, write() to socket */
  do {
    bzero(buf, BUFSIZE);
    n = fread(buf, 1, BUFSIZE, fp);
    if (n > 0)
      write(connfd, buf, n);
  } while (n > 0);

}


/*
 * main: creates, binds socket and starts listening for connections.
 * Every accepted connection forks off for multiple request processing.
 */
int main(int argc, char **argv) {

  /* variables */
  int listenfd; /* listening socket */
  int connfd; /* connection socket */
  int portno; /* port number */
  uint clientlen;
  struct sockaddr_in serveraddr; /* server addr */
  struct sockaddr_in clientaddr; /* client addr */
  struct hostent *hostp; /*client host info */

  char *hostaddrp;
  int optval;


  int pid;  /* process ID */

  /* check command line arguments */
  if (argc != 5) {
    fprintf(stderr, "usage: %s -document_root \"path/to/folder\" -port <portno>", argv[0]);
    exit(1);
  }

  /* read document_root path */
  if (strcmp("-document_root", argv[1]) == 0) {
    document_root = argv[2];
  } else {
    fprintf(stderr, "usage: %s -document_root \"path/to/folder\" -port <portno>", argv[0]);
    exit(1);
  }

  /* read portno */
  if (strcmp("-port", argv[3]) == 0) {
    portno = atoi(argv[4]);
  } else {
    fprintf(stderr, "usage: %s -document_root \"path/to/folder\" -port <portno>", argv[0]);
    exit(1);
  }


  /* printfs to make sure we're reading parameters correctly */
  // printf("document_root: %s ", document_root);
  // printf("portno: %d\n", portno);


  /* create listening socket */
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd < 0) {
    error("Error opening listening socket");
  }

  /* setsockopt: Handy debugging trick that lets
  * us rerun the server immediately after we kill it;
  * otherwise we have to wait about 20 secs.
  * Eliminates "ERROR on binding: Address already in use" error.
  * Take from SimpleEchoServer example.
  */
  optval = 1;
  setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));


  /* build server's internet address */
  bzero((char *) &serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET; /* we are using the Internet */
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); /* accept reqs to any IP addr */
  serveraddr.sin_port = htons((unsigned short)portno); /* port to listen on */

  /* bind() */
  if (bind(listenfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0)
    error("Error on bind()");

  /* listen() */
  if (listen(listenfd, 5) < 0)
    error("Error on listen()");

  clientlen = sizeof(clientaddr);
  while (1) {

    /* accept () */
    connfd = accept(listenfd, (struct sockaddr *) &clientaddr, &clientlen);
    if (connfd < 0)
      error("Error on accept()");

    /* gethostbyaddr to ID client */
    hostp = gethostbyaddr((const char *) &clientaddr.sin_addr.s_addr,
        sizeof(clientaddr.sin_addr.s_addr), AF_INET);
    if (hostp == NULL)
      error("Error on gethostbyaddr() (w/ client)");
    hostaddrp = inet_ntoa(clientaddr.sin_addr);
    if (hostaddrp == NULL)
      error("Error on inet_ntoa\n");
    // printf("Server established with %s (IP: %s)\n", hostp->h_name, hostaddrp);


    pid = fork();

    if (pid < 0) {
      error("Error on fork()");
    }

    if (pid == 0) {
      /* client process */
      close(listenfd);
      respond(connfd);
      exit(0);
    } else {
      close (connfd);
    }
  } /* end of while */

}
