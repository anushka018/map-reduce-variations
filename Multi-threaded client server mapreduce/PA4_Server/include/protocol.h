#ifndef SERVER_CLIENT_PROTOCOL_H
#define SERVER_CLIENT_PROTOCOL_H

#define WORD_LENGTH_RANGE           20
#define REQUEST_MSG_SIZE            23
#define RESPONSE_MSG_SIZE           3
#define LONG_RESPONSE_MSG_SIZE      22
#define MAX_NUM_CLIENTS             20
#define MAX_CONCURRENT_CLIENTS      50

//Request Structure Starting Index
#define RQS_RQS_CODE_NUM            0
#define RQS_CLIENT_ID               1
#define RQS_DATA                    2
#define RQS_PERSISTENT_FLAG         22

//Request Codes
#define UPDATE_WSTAT                1
#define GET_MY_UPDATES              2
#define GET_ALL_UPDATES             3
#define GET_WSTAT                   4

//Response Structure Index
#define RSP_RQS_CODE_NUM            0
#define RSP_RSP_CODE_NUM            1
#define RSP_DATA                    2

//Response Codes
#define RSP_NOK                     0
#define RSP_OK                      1

int resultHistogram[20]; // final ds
int clientStatus[20];

void updateWSTAT(int request[]);

int getAllUpdates();

void *parseRequest(void *args);

#endif //SERVER_CLIENT_PROTOCOL_H
