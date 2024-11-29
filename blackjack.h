#include <inttypes.h>   /* stdint: int8_t, int16_t, uint16_t inttypes: PRId8 */
#include <sys/socket.h> /* sockaddr_in, socket, AF_INET */
#include <arpa/inet.h>  /* htons */
#include <stdlib.h>     /* atoi */
#include <stdio.h>      /* printf, puts */

#include <string.h>     /* memset */

/* --- Card --- */

struct card {
  int8_t rank;
  int8_t suit;
};

void card_print(struct card c);

/* --- Deck --- */

struct deck {
  int16_t head;
  struct card cards[52];
};

/* Nao eh preguica, eh loop unrolling
 * obs: talvez seja preguica :p 
 */

void deck_init(struct deck *d);

void deck_shuffle(struct deck *d);

struct card deck_draw(struct deck *d);

/* --- Player --- */


struct player {
  int index;
  struct card hand[21];
  float bet;
};

void player_init(struct player *p);

void player_card_store(struct player *p, struct card c);

void player_hand_print(struct player p);

void player_hand_clean(struct player *p);

int8_t player_hand_value(struct player *p);

void player_bet_store(struct player *p, float b);

void table_print(struct player *players, int8_t n_players);

void table_print_dealer(struct player *players, int8_t n_players);

/* --- Ring --- */

#define SYNC 'S'
#define READY 'R'
#define BROADCAST 'B'
#define ASK 'A'
#define START 'T'

#define HIT '1'
#define STAND '0'
#define DOUBLE_DOWN '2'
#define SPLIT '3'

#define PORT_READ   20241
#define PORT_WRITE  PORT_READ

#define BIND 0
#define CONNECT 1

struct ring_pkg { 
  int8_t type;
  int8_t addr;
  struct card card;
  int8_t bet;
};

struct ring_socket {
  int fd;
  struct sockaddr_in addr;
};

int ring_socket_init(struct ring_socket *r_socket, uint16_t port, char *ipv4_addr, int type);
