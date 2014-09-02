#include <time.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <strings.h>
#include <pthread.h>
#include <time.h>
#include <assert.h>
#include <getopt.h>


/********************************************************************
 *
 * Code below (to END mod_unique_id.c) from mod_unique_id.c
 *
 *******************************************************************/

#define UNIQUE_ID_REC_MAX 5

static unsigned short unique_id_rec_offset[UNIQUE_ID_REC_MAX],
                      unique_id_rec_size[UNIQUE_ID_REC_MAX],
                      unique_id_rec_total_size,
                      unique_id_rec_size_uu;

typedef struct {
    unsigned int stamp;
    unsigned int in_addr;
    unsigned int pid;
    unsigned short counter;
    unsigned int thread_index;
} unique_id_rec;

/* NOTE: This is *NOT* the same encoding used by base64encode ... the last two
 * characters should be + and /.  But those two characters have very special
 * meanings in URLs, and we want to make it easy to use identifiers in
 * URLs.  So we replace them with @ and -.
 */
static const char uuencoder[64] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '@', '-',
};

/********************************************************************
 *
 * END mod_unique_id.c
 *
 *******************************************************************/


void unique_id_global_init();
static const unique_id_rec make_unique_id_rec();
void print_unique_id(const unique_id_rec);
static const unique_id_rec unique_id_uuencode(char *, const unique_id_rec, unsigned int);
static const unique_id_rec unique_id_uudecode(const char *);
void roundtrip_test();


static struct option longopts[] = {
  { "test", no_argument,       NULL, 't' },
  { "id",   required_argument, NULL, 'i' },
  { NULL,   0,                 NULL, 0 }
};

int main(int argc, char** argv)
{
  int opt;

  unique_id_global_init();

  while ((opt = getopt_long(argc, argv, "ti:", longopts, NULL)) != -1)
  {
    switch (opt)
    {
      case 't':
        roundtrip_test();
        exit(0);
      case 'i':
        print_unique_id(unique_id_uudecode(optarg));
        exit(0);
      default:
        printf("don't know what to do\n");
    }
  }
}

void roundtrip_test()
{
  char *s_unique_id = (char *)malloc(unique_id_rec_size_uu + 1);
  unique_id_rec unique_id_a, unique_id_b;
  unique_id_a = make_unique_id_rec();
  unique_id_a = unique_id_uuencode(s_unique_id, unique_id_a, 1234);
  unique_id_b = unique_id_uudecode(s_unique_id);
  free(s_unique_id);
  assert(unique_id_a.stamp == unique_id_b.stamp);
  assert(unique_id_a.in_addr == unique_id_b.in_addr);
  assert(unique_id_a.pid == unique_id_b.pid);
  assert(unique_id_a.counter == unique_id_b.counter);
  assert(unique_id_a.thread_index == unique_id_b.thread_index);
}

static const unique_id_rec make_unique_id_rec()
{
  unique_id_rec unique_id = {0};
  inet_pton(AF_INET, "10.180.248.75", &(unique_id.in_addr));
  unique_id.pid = htonl(getpid());
  unique_id.counter = htons(1);
  unique_id.thread_index = htonl(1234);
  return unique_id;
}

void print_unique_id(const unique_id_rec unique_id)
{
  char ipstr[INET_ADDRSTRLEN] = {0};
  time_t tstamp = ntohl(unique_id.stamp);
  printf("unique_id.stamp = %s", ctime(&tstamp));
  inet_ntop(AF_INET, &(unique_id.in_addr), ipstr, INET_ADDRSTRLEN);
  printf("unique_id.in_addr = %s\n", ipstr);
  printf("unique_id.pid = %d\n", ntohl(unique_id.pid));
  printf("unique_id.counter = %d\n", ntohs(unique_id.counter));
  printf("unique_id.thread_index = %d\n", ntohl(unique_id.thread_index));
}

void unique_id_global_init()
{
    unique_id_rec cur_unique_id = {0};

    /*
     * Calculate the sizes and offsets in cur_unique_id.
     */
    unique_id_rec_offset[0] = offsetof(unique_id_rec, stamp);
    unique_id_rec_size[0] = sizeof(cur_unique_id.stamp);
    unique_id_rec_offset[1] = offsetof(unique_id_rec, in_addr);
    unique_id_rec_size[1] = sizeof(cur_unique_id.in_addr);
    unique_id_rec_offset[2] = offsetof(unique_id_rec, pid);
    unique_id_rec_size[2] = sizeof(cur_unique_id.pid);
    unique_id_rec_offset[3] = offsetof(unique_id_rec, counter);
    unique_id_rec_size[3] = sizeof(cur_unique_id.counter);
    unique_id_rec_offset[4] = offsetof(unique_id_rec, thread_index);
    unique_id_rec_size[4] = sizeof(cur_unique_id.thread_index);
    unique_id_rec_total_size = unique_id_rec_size[0] + unique_id_rec_size[1] +
                               unique_id_rec_size[2] + unique_id_rec_size[3] +
                               unique_id_rec_size[4];

    /*
     * Calculate the size of the structure when encoded.
     */
    unique_id_rec_size_uu = (unique_id_rec_total_size*8+5)/6;
}


#define uudecoder(c) (int)(strchr(uuencoder,c)-uuencoder)

static const unique_id_rec unique_id_uudecode(const char *str)
{
    struct {
        unique_id_rec foo;
        unsigned char pad[2];
    } paddedbuf;
    unsigned char *x, *y;
    char index[4] = {0};
    int k = 0;
    unique_id_rec unique_id = {0};

    y = (unsigned char *) &paddedbuf;

    for (int i = 0; i < unique_id_rec_total_size; i += 3) {
      index[0] = uudecoder(str[k++]);
      index[1] = uudecoder(str[k++]);
      /* first 6 bits + next 2 bits */
      y[i] = ((index[0] << 2) & 0xfc) | ((index[1] >> 4) & 0x03);

      if (k == unique_id_rec_size_uu) break;
      index[2] = uudecoder(str[k++]);
      /* remaining 4 bits + next 4 bits */
      y[i+1] = ((index[1] << 4) & 0xf0) | ((index[2] >> 2) & 0x0f);

      if (k == unique_id_rec_size_uu) break;
      index[3] = uudecoder(str[k++]);
      /* remaining 2 bits + next 6 bits (& 0x3f probably not necessary) */
      y[i+2] = ((index[2] << 6) & 0xc0) | (index[3] & 0x3f);
    }

    /* unwind the struct from the byte array */
    k = 0;
    for (int i = 0; i < UNIQUE_ID_REC_MAX; i++) {
        x = ((unsigned char *) &unique_id) + unique_id_rec_offset[i];
        for (int j = 0; j < unique_id_rec_size[i]; j++, k++) {
            x[j] = y[k];
        }
    }

    return unique_id;
}

static const unique_id_rec unique_id_uuencode(char *str, const unique_id_rec cur_unique_id, unsigned int connection_id)
{
    /*
     * Buffer padded with two final bytes, used to copy the unique_id_red
     * structure without the internal paddings that it could have.
     */
    unique_id_rec new_unique_id;
    struct {
        unique_id_rec foo;
        unsigned char pad[2];
    } paddedbuf;
    unsigned char *x,*y;
    unsigned short counter;
    int i,j,k;

    new_unique_id.in_addr = cur_unique_id.in_addr;
    new_unique_id.pid = cur_unique_id.pid;
    new_unique_id.counter = cur_unique_id.counter;

    new_unique_id.stamp = htonl(time(NULL));
    new_unique_id.thread_index = htonl(connection_id);

    /* we'll use a temporal buffer to avoid uuencoding the possible internal
     * paddings of the original structure */
    x = (unsigned char *) &paddedbuf;
    k = 0;
    for (i = 0; i < UNIQUE_ID_REC_MAX; i++) {
        y = ((unsigned char *) &new_unique_id) + unique_id_rec_offset[i];
        for (j = 0; j < unique_id_rec_size[i]; j++, k++) {
            x[k] = y[j];
        }
    }
    /*
     * We reset two more bytes just in case padding is needed for the uuencoding.
     */
    x[k++] = '\0';
    x[k++] = '\0';

    /* alloc str and do the uuencoding */
    k = 0;
    for (i = 0; i < unique_id_rec_total_size; i += 3) {
        y = x + i;
        str[k++] = uuencoder[y[0] >> 2];
        str[k++] = uuencoder[((y[0] & 0x03) << 4) | ((y[1] & 0xf0) >> 4)];
        if (k == unique_id_rec_size_uu) break;
        str[k++] = uuencoder[((y[1] & 0x0f) << 2) | ((y[2] & 0xc0) >> 6)];
        if (k == unique_id_rec_size_uu) break;
        str[k++] = uuencoder[y[2] & 0x3f];
    }
    str[k++] = '\0';

    return new_unique_id;
}
