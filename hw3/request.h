#ifndef __REQUEST_H__

struct Satistics    //struct for statistics
{
    int id;
    int total_requests;
    int static_requests;
    int dynamic_requests;
};

void requestHandle(int fd, struct timeval* creation, struct timeval* working, struct Satistics* stats_p);

#endif
