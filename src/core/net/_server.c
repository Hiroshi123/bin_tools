
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define MAX_LINE_LENGTH (1024)
#define change_line(X) (*(uint16_t*)X == 0x0a0d)

static uint32_t* GET = "GET";
static uint32_t* POST = "POST";

static uint32_t* METHOD;
static uint8_t* URI;
static uint8_t* PROT;

static uint32_t* r01 = "predict";
static uint32_t* r02 = "anal";

static char HTML_HEADER[] = "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>sample</title><style>a { color:#0000FF; text-decoration:none; } .code { position: absolute; left: 300px;} .codes { margin-left: 100px;}</style></head>";

static char TABLE_BODY1[] =
  "<!DOCTYPE html><html><head><meta charset=\"utf-8\"></head><body><form action=\"/predict\" method=\"post\"><div><label for=\"message\"></label><br><textarea name=\"feature\" id=\"textarea1\" rows=\"10\" cols=\"60\">0x00401000   mov rax,0x3</textarea></div><div class=\"button\"><button type=\"submit\">score</button></div></form><div>0x00401000   mov rax,0x3</div><div>0x00401003   mov rax,0x3</div></body></html>";

static char TABLE_BODY3[] =
  "<!DOCTYPE html><html><head><meta charset=\"utf-8\"></head><body>"\
  "<table>"					\
  "<table border>"\
  "<tr>"\
  "<th>path</th>"\
  "<th>hash</th>"\
  "<th>pkg name</th>"\
  "<th>file num</th>"\
  "<th>native</th>"\
  "<th>permission</th>"\
  "<th>intent</th>"\
  "<th>java module</th>"\
  "<th>native func</th>"\
  "<th>feature vec</th>"\
  "<th>score</th>"\
  "</tr>";
  /* "<tr>"						\ */
  /* "<td>a01.apk</td>""<td>19</td> <td>com.xxx</td></tr>"		\ */
  /* "<tr><td>a01.apk</td> <td>24</td> <td></td>com.xxx</tr>"\ */
  /* "<tr><td>a01.apk</td> <td>20</td> <td></td>com.xxx</tr>"\ */

static char TABLE_BODY2[] = "</table></body></html>";
static char* TABLE_BEGIN_P = 0;
static char* TABLE_RECORD_BEGIN_P = 0;
static char* __bodyp = 0;
static char* __bodylen = 0;

static void send_response(FILE *f, int address_found, char *ip_address) {
  fprintf(f, "HTTP/1.1 200 OK\r\n");
  fprintf(f, "Content-Type: application/json\r\n");
  fprintf(f, "\r\n");
  fprintf(f, "{\"address\": \"UNKNOWN\"}");
}

static void replace(char* sub, char* orig, uint8_t a, uint8_t b) {
  char* o = orig;
  char* s = sub;
  for (;;o++,s++) {
    if (*o < 0x21 || 0x7e < *o) break;
    // do not tell anyone<o..o>    
    if (*o == a) {
      if (b == 0) {
	s--;
	continue;
      }
      *s = b;
    }
    else *s = *o;
  }
}

static int extract_ip_address_from_header(char *line, char *address) {
  int found = 0;
  char *ptr;
  char *name;
  char *value;

  name = strndup(line, MAX_LINE_LENGTH);
  ptr = index(name, (int)':');
  if (ptr == NULL) {
    return 0;
  }
  // end the string at the colon
  *ptr = '\0';

  // get the value part of the header field
  ptr = index(line, (int) ':');
  value = strndup(ptr + 2, MAX_LINE_LENGTH);

  // most ugly way to remove \r\n from the end of the string
  value[strlen(value)-2] = '\0';

  if (strncmp(name, "X-Forwarded-For", MAX_LINE_LENGTH) == 0) {
    found = 1;
    strncpy(address, value, MAX_LINE_LENGTH);
  }

  free(name);
  free(value);

  return found;
}

static int open_connection(int port) {
  int sock;
  struct sockaddr_in addr_in;
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    printf("Failed to create socket (%d)\n", errno);
    exit(EXIT_FAILURE);
  }
  addr_in.sin_family = AF_INET;
  addr_in.sin_port = htons(port);
  addr_in.sin_addr.s_addr = INADDR_ANY;
  bind(sock, (struct sockaddr *) &addr_in, sizeof(struct sockaddr_in));
  if (listen(sock, 8) == -1) {
    printf("Failed to get socket to listen (%d)\n", errno);
    exit(EXIT_FAILURE);
  }
  return sock;
}

static uint64_t* H1 = "Content-Length:";

static uint8_t* check_request(uint8_t* p) {
  uint8_t r = 1;
  uint8_t* b = p;
  uint64_t* c = p;
  if (*H1 == *c && *(H1+1) == *(c+1)) {
    printf("matched......\n");
  }
  for (;*p != 0x20;p++);
  *p = 0;
  printf("..%s\n", b);
  printf("\n");
  return 0;
}

static int send_all(int socket, void *buffer, size_t length) {
  char *ptr = (char*) buffer;
  while (length > 0) {
    int i = send(socket, ptr, length, 0);
    printf("s:%d\n", i);      
    if (i < 1) return 0;
    ptr += i;
    length -= i;
  }
  return 1;
}

static void* get_param(uint8_t* p, char* name) {
  for (;*name;p++,name++) {
    if (*p == *name) {
      p++;
    }
  }
  return p;
}

static void convert_to_feature_txt(uint8_t* p, uint8_t* r, FILE* fd) {

  int i = 0;
  fwrite("1 ", 2, 1, fd);
  char digit[8] = {};
  for (;*p && p < r;p++,i++) {

    printf("%p,%p\n", p,*p);

    if (*p == 0x31) {
      printf("!%d\n", i);
      sprintf(digit, "%d", i);
      fwrite(digit, strlen(digit), 1, fd);
      fwrite(":1.0 ", 5, 1, fd);
    }
  }
}

static void read_body(uint8_t* q, uint32_t len) {
  uint8_t* r = q + len;
  printf("body:%d\n", len);
  uint8_t* p = get_param(q, "feature");
  FILE* fd = fopen("tmp01.txt", "w");
  convert_to_feature_txt(p, r, fd);
  fclose(fd);  
  printf("\n");
  fflush(stdout);
}

static uint8_t* get_request(uint8_t* p) {
  
  METHOD = p;
  /* if (*p == *GET) { */
  /*   printf("get\n"); */
  /* } else if (*p == *POST) { */
  /*   printf("post\n"); */
  /* } else { */
  /*   printf("else\n"); */
  /* } */
  for (;*p != 0x20;p++);
  *p = 0;
  p++;
  URI = p;
  for (;*p != 0x20;p++);
  *p = 0;
  p++;
  PROT = p;
  for (;*(uint16_t*)p != 0x0a0d;p++);
  *p = 0;
  p++;
  printf("METHOD:%s\n", METHOD);
  // *(URI + strlen(URI)) = 0;
  printf("URI:%s,%d\n", URI, strlen(URI));
  printf("PROTocol:%s\n", PROT);
  return p;
}

static void parse_request_header(uint8_t* p, int len) {
  uint8_t* last = p + len;
  uint8_t* q = 0;
  p = get_request(p);
  // q = f1(p, "Content-Length:");
  for (;p < last;p++) {
    // printf("%p,%p,%c\n", p, *p, *p);
    if (change_line(p)) {
      p+=2;
      if (change_line(p)) {
	// printf("end of header\n");
	if (len) {
	  __bodyp = p + 2;
	  __bodylen = len;
	  // read_body(p + 2, len);
	  printf("len:%d\n", len);
	}
	break;
      }
      q = check_request(p);
      // q = f1(p, "Content-Length:");
      if (q) {
	len = atoi(q);
	p = q;
      }
    }
  }
}

static void _send(int sock, void* d, int len) {
  
  char header[0x1000] = {};
  char length[30] = {};
  static char* hello = "HTTP/1.1 200 OK\r\n";
  static char* h1 = "Content-Type: text-html; charset=UTF-8\r\n";
  static char* h2 = "Connection: Keep-Alive\r\n";
  sprintf(&length, "Content-Length: %d\r\n", len);
  sprintf(&header, "%s%s%s%s\r\n", hello, h1, h2, &length);
  send_all(sock, &header, strlen(&header));
  send_all(sock, d, len);
  send_all(sock, "\r\n", strlen("\r\n"));
}

static void* wrap1(char* q, char* str, char* u) {
  q += sprintf(q, str, u);
  return q;
}

static char* get_pkg_name(char* fname) {
  FILE* f = fopen(fname, "r");
  char buf[MAX_LINE_LENGTH] = {};
  char* res;
  do {
    res = fgets(&buf[0], MAX_LINE_LENGTH, f);
    uint32_t* p = &buf[0];
  } while (res != NULL && strcmp(&buf[0], "\r\n") != 0);
  fclose(f);
}

static void disp_file(int sock, char* u) {

  char buf[MAX_LINE_LENGTH];
  char cmd[MAX_LINE_LENGTH];
  char nas_path[MAX_LINE_LENGTH];
  char static_path[MAX_LINE_LENGTH];
  
  replace(&nas_path[0], u, '-', '/');
  sprintf(&cmd[0],
	  "./do02.sh /%s %s %s",
	  &nas_path[0],
	  "15",
	  "./pathout.txt");
  system(&cmd[0]);
  
  /* printf("%d,%d,%d\n", strlen(&nas_path[0]), strlen(pa), strlen(u)); */
  /* replace(buf3, path, '/', '_'); */
  FILE* f = fopen("./pathout.txt", "r");
  char* res;
  char* q = TABLE_RECORD_BEGIN_P;
  do {
    res = fgets(&buf[0], MAX_LINE_LENGTH, f);
    buf[strlen(&buf[0])-1] = 0;
    q += sprintf(q, "<tr><td>%s</td>", &buf[0]);
    q = wrap1(q, "<td>%s</td>", "get");
    /* q = wrap1(q, "<td>%s</td>", u); */
    // q += sprintf(q, "<td>%s</td></tr>", "abc");
    
    // get_pkg_name(local_path);
    // ./%s/%s/misc.log;
    
    q += sprintf(q, "<td><a href=\"./%s/%s/axml.log\">_</td>", u, &buf[0]);
    q += sprintf(q, "<td><a href=\"./%s/%s/misc.log\">_</td>", u, &buf[0]);
    q += sprintf(q, "<td><a href=\"./%s/%s/misc.log\">_</td>", u, &buf[0]);
    q += sprintf(q, "<td><a href=\"./%s/%s/native.log\">_</td>", u, &buf[0]);
    q += sprintf(q, "<td><a href=\"./%s/%s/axml.log\">_</td>", u, &buf[0]);
    q += sprintf(q, "<td><a href=\"./%s/%s/axml.log\">_</td>", u, &buf[0]);
    q += sprintf(q, "<td><a href=\"./%s/%s/dex.log\">_</td></tr>", u, &buf[0]);
    // printf("%s\n", buf2);
  } while (res != NULL && strcmp(&buf[0], "\r\n") != 0);
  fclose(f);
  printf("size:%p,%p!!!!!!!!!!!!!!!!!!\n",
	 q - (char*)TABLE_RECORD_BEGIN_P,
	 q - (char*)TABLE_BEGIN_P
	 );
  /* read(); */
  // char data[0x10] = "ok!\n";
  char* x = &TABLE_BODY2[0];
  for (;*x;x++,q++) *q = *x;  
  _send(sock, TABLE_BEGIN_P, strlen(TABLE_BEGIN_P)); 
}

static char* convert() {
  char path01[] =
    "/mnt/mal05/malware/android/purchased/VirusSign/20173Q/android/Android_170928001045/ST21/apk";
  
}

static char* strfind(char* u, char q) {
  for (;*u;u++) {
    if (*u < 0x21 || 0x7e < *u) break;
    if (*u == q) return u;
  }
  return 0;
}

static void collect_feature(int sock, char* u) {

  char buf2[MAX_LINE_LENGTH] = {};
  char path01[MAX_LINE_LENGTH] = {};
  replace(&path01[0], u, '-', '/');
  printf("ok\n");
  sprintf(&buf2[0], "mkdir -p ./static/%s", u);
  system(&buf2[0]);
  
  sprintf(&buf2[0],
	  "./feature_collector /%s ./static/%s -lev 2",
	  &path01[0],
	  u
	  );
  system(&buf2[0]);
  
  char* buf = "ok";
  _send(sock, buf, strlen(buf));
}

static int check_file(char* u) {
  char buf[0x100] = {};
  sprintf(&buf[0], "./static/%s", u);
  int fd = open(buf, O_RDONLY);
  if (fd < 0) {
    dprintf(1, "open error:%s\n", &buf[0]);
    return 0;
  }
  return fd;
}

static void view_file(int sock, char* u) {

  int fd = check_file(u);
  if (fd == 0) {
    collect_feature(sock, u);
    return;
  }
  struct stat stbuf = {};
  if (fstat(fd, &stbuf) == -1) {
    close(fd);
    exit(1);
  }
  size_t map_size = (stbuf.st_size + 0x1000) & 0xfffff000;  
  char* p = mmap
    (NULL, map_size, PROT_READ,
     MAP_PRIVATE, fd, 0);
  if (p == -1) {
    printf("map error\n");
  }
  _send(sock, p, stbuf.st_size);
  munmap(p, map_size);
  close(fd);
}

int contains(char* u, char q) {  
  for (;;u++) {
    if (*u == 0x20 || *u == 0) break;
    if (*u == q) return 1;
  }
  return 0;
}

static void handle_get(int sock) {

  uint8_t* p = URI;
  uint32_t v = *(uint32_t*)(URI+1);
  if (*URI == '/' && *(URI+1) == 0x00) {
    printf("root,%p,%p\n", *(p + 1),*(p + 2));
    _send(sock, TABLE_RECORD_BEGIN_P, strlen(TABLE_RECORD_BEGIN_P));
  }
  if (*URI == '/' && *(URI+1) == 0x61 && *(URI+2) == '/') {
    if (contains(URI + 3, '/')) {
      view_file(sock, URI+3);
    } else {
      disp_file(sock, URI+3);
    }
  }
  // /e/ = > execute
  if (*URI == '/' && *(URI+1) == 0x62 && *(URI+2) == '/') {
    if (contains(URI + 3, '/')) {
      collect_feature(sock, URI+3);
    }
  }
  if (*URI == '/' && *(URI+1) == 0x63 && *(URI+2) == '/') {    
    view_file(sock, URI+3);
  }
  printf("r01:%p,%p\n", *(uint32_t*)(URI+1), *r01);  
}

static void convert_to_html() {
  FILE* out = fopen("./static/instruction.html", "w");
  FILE* in = fopen("./instruction.log", "r");

  struct stat statBuf = {};
  if (stat("./instruction.log", &statBuf) == 0) {
    printf("e\n");
  }
  fwrite(&HTML_HEADER[0], 1, strlen(HTML_HEADER), out);  
  uint8_t* buf = malloc(100);
  uint8_t* head = buf;
  for (;;) {
    if (fgets(buf, 100, in) == 0)
      break;
    int s = fwrite("<div>", 1, strlen("<div>"), out);
    int first = 0;
    for (;*buf != '\n';buf++) {
      if (*buf != '\t') {
	fwrite(buf, 1, 1, out);
      } else {
	if (first) break;
	first = 1;
	fwrite(buf, 1, 1, out);
      }
    }
    buf++;
    fwrite("<a class=\"code\">", 1, strlen("<a class=\"code\">"), out);
    fwrite(buf, 1, strlen(buf), out);
    fwrite("</a></div>\n", 1, strlen("</a></div>\n"), out);
  }
  
  // int s = fwrite("/<div>", 1, strlen("/<div>"), out);
  
}

static void handle_post(int sock) {

  uint8_t* p = URI;
  uint32_t v = *(uint32_t*)(URI+1);
  if (v == *r02) {
    // int rc = execv("./ls", argv);
    char* argv[6] = {};
    argv[0] = "./__z__loader.so";
    argv[1] = "./zob";
    argv[2] = "./a02.exe";
    /* argv[3] = "./res01.txt"; */
    int rc = execv("__z__loader.so", argv);
    
    
    // __os__open();
    
  }
  if (v == *r01) {
    printf("predict ok\n");
    char* argv[6] = {};
    if (__bodyp) {
      read_body(__bodyp, __bodylen);
      argv[0] = "./ls";
      /* argv[1] = "./tmp01.txt"; */
      /* argv[2] = "./model"; */
      /* argv[3] = "./res01.txt"; */
      int rc = execv("./ls", argv);
      printf("next:%d\n", rc);
    }
  }
}

static void route(int sock) {

  if (*METHOD == *GET) {
    printf("get\n");
    handle_get(sock);
  }
  if (*METHOD == *POST) {
    printf("post\n");
    handle_post(sock);
  }
  
  /* for (;p<;p++) { */
  /*   printf("%p,%p\n", p, p); */
  /* } */
  /* if () { */    
  /* } */
  
}

static void f2(int sock) {
  struct sockaddr_in client_addr = {};
  socklen_t clientaddr_len;
  int client_sock = accept(sock, (struct sockaddr *)&client_addr, &clientaddr_len);
  if (client_sock == -1) {
    printf("Failed to accept connection (%d)\n", errno);
    return;
  }
  /* fprintf(f, "HTTP/1.1 200 OK\r\n"); */
  /* fprintf(f, "Content-Type: application/json\r\n"); */
  /* fprintf(f, "\r\n"); */
  /* fprintf(f, "{\"address\": \"UNKNOWN\"}"); */
  
  printf("aaaa\n");
  uint8_t* buf = malloc(65535);
  int rcvd = recv(client_sock, buf, 65535, 0);
  uint8_t* q = 0;
  uint8_t* r = 0;
  int pid = 0;
  if (rcvd) {
    if ((pid = fork())==0) {
      parse_request_header(buf, rcvd);      
      route(client_sock);      
    } else {
      printf("parent:%d\n", pid);
      int status = 0;
      int ret = waitpid(pid, &status, 0);
      printf("child finished:%d\n", ret);
      free(buf);
      return 0;
    }
  } else {
    // recv == -1 or 0
    printf("recv:%d\n", rcvd);
    char* hello = "HTTP/1.1 200 OK\r\n\r\n";
    send_all(client_sock, hello, strlen(hello));
    free(buf);
    return;
  }
  // write(sock, a, strlen(a));
  // close(client_sock);

  printf("done\n");
  // write(sock, );
  close(client_sock);
  fflush(stdout);
  free(buf);
  
  return;
}

static void accept_client(int sock) {
  int address_found = 0;
  struct sockaddr_in client_addr;
  socklen_t clientaddr_len;
  char header_line[MAX_LINE_LENGTH];
  char *res;
  FILE *f;
  FILE *fw;
  char ip_address[MAX_LINE_LENGTH];
  int client_sock = accept(sock, (struct sockaddr *)&client_addr, &clientaddr_len);
  if (client_sock == -1) {
    printf("Failed to accept connection (%d)\n", errno);
    exit(EXIT_FAILURE);
  }
  
  f = fdopen(client_sock, "r+");
  fw = fdopen(client_sock, "w+");
  
  int i = 0;
  uint32_t len;
  do {
    res = fgets(header_line, MAX_LINE_LENGTH, f);
    if (res != NULL) {
      printf("%d;%s", i, res);
      uint8_t* re = check_request(res);//, "Content-Length:");
      if (re) {
	printf("this is content type:%d,%c\n", atoi(re), *re);
	len = atoi(re);
      }
    }
    i++;
  } while (res != NULL && strcmp(header_line, "\r\n") != 0);
  
  if (res && len) {
    // read header
    int s = fread(header_line, 1, len, f);
    // uint8_t* last = res + len;
    // uint8_t* p = res;
    printf("hh::%c,%c\n", header_line[0], header_line[1]);
    /* for (;p < last;p++) { */
    /*   printf("res:%p,%c\n", p, *p); */
    /* } */
  }
  printf("send,%d,%d\n", feof(f), feof(fw));
  send_response(f, "122.333.333.333", ip_address);

  fclose(f);
  fclose(fw);
  
  // stdout needs to be flushed in order for heroku to read the logs
  fflush(stdout);
}

static void prepare_html01() {
  char* p = mmap
    (NULL, 0x8000, PROT_READ | PROT_WRITE,
     MAP_SHARED | MAP_ANON, -1, 0);
  TABLE_BEGIN_P = p;
  char* e = p + strlen(TABLE_BODY1);
  char* b = &TABLE_BODY1[0];
  for (;*b;p++,b++) *p = *b;
  TABLE_RECORD_BEGIN_P = p;
}

int main(int argc, char **argv) {

  convert_to_html();
  exit(1);
  
  FILE* f = fopen("./static/index.html", "r");
  struct stat statBuf = {};
  if (stat("./static/index.html", &statBuf) == 0) {
    printf("e\n");
  }
  void* a1 = malloc(statBuf.st_size);
  TABLE_RECORD_BEGIN_P = a1;
  //  = a1;
  fread(a1, sizeof(int), statBuf.st_size, f);
  printf("%s\n", a1);
  fclose(f);
  printf("%d\n", statBuf.st_size);
  
  // prepare_html01();
  int _port;
  /* if (argc != 2) { */
  /*   printf("%s [port-number]\n", argv[0]); */
  /*   exit(EXIT_FAILURE); */
  /* } */
  char* port;
  if ((port = getenv("PORT")) != NULL) {
    printf("PORT:%s\n", port);
    _port = atoi(port);
  } else {
    _port = 3000;
  }
  printf("a\n");
  int sock = open_connection(_port);
  
  while (1) {
    // accept_client(sock);
    f2(sock);
  }

  close(sock);
  exit(EXIT_SUCCESS);
}

