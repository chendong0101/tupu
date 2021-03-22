
#include <sstream>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFSIZE 2048
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 16555

enum MSG_ID {
    HELLO = 0, RET_HELLO,
    SET, RET_SET,
    GET, RET_GET,
    QUERY,
};

typedef struct {
    MSG_ID msg_id;
    size_t length;
} MessageHead;

class MStream {
    public:
        MStream() :len(0), size(100), cursor(0) {
            msg = new char[size];
        }
        MStream(char *_msg, size_t _length)
            : size(_length), len(_length), cursor(0)
        {
            msg = std::move(_msg);
        }
        virtual ~MStream() {
            delete msg;
        }
        MStream& operator<<(const char *s) {
            size_t slen = strlen(s);
            if (sizeof(slen) > size - len) {
                resize();
            }
            memcpy(msg + len, &slen, sizeof(slen));
            len += sizeof(slen);
            if (slen > size - len) {
                resize();
            }
            memcpy(msg + len, s, slen);
            len += slen;
            return *this;
        }
        MStream &operator<<(const long &i)
        {
            if (sizeof(i) > size - len)
            {
                resize();
            }
            memcpy(msg + len, &i, sizeof(i));
            len += sizeof(i);
            return *this;
        }
        MStream &operator<<(const int &i)
        {
            if (sizeof(i) > size - len)
            {
                resize();
            }
            memcpy(msg + len, &i, sizeof(i));
            len += sizeof(i);
            return *this;
        }
        MStream& operator>>(long &l)
        {
            if (sizeof(l) + cursor > len)
            {
                return *this;
            }
            memcpy(&l, msg + cursor, sizeof(l));
            cursor += sizeof(l);
            return *this;
        }
        MStream& operator>>(std::string &s) {
            size_t slen;
            if (sizeof(slen) + cursor > len) {
                return *this;
            }
            memcpy(&slen, msg + cursor, sizeof(slen));
            cursor += sizeof(slen);
            if (slen + cursor > len) {
                return *this;
            }
            char ss[slen];
            memcpy(ss, msg + cursor, slen);
            cursor += slen;
            s.assign(ss, slen);
            return *this;
        }
        const char* c_str() const {
            return msg;
        }
        size_t length() const {
            return len;
        } 

    private:
        char *msg;
        size_t size, len, cursor;
        void resize() {
            size *= 2;
            char *msg_tmp = new char[size];
            memcpy(msg_tmp, msg, len);
            delete msg;
            msg = msg_tmp;
        }
};

void setDoc(int sockfd, long id, std::string value) {
    MStream ss;
    ss << id << value.c_str();
    size_t bufflen = ss.length();
    MessageHead head;
    head.msg_id = SET;
    head.length = bufflen;
    send(sockfd, &head, sizeof(head), 0);
    send(sockfd, ss.c_str(), bufflen, 0);
    printf("Send: %s, len = %ld\n", ss.c_str(), bufflen);
}

void getDoc(int sockfd, long id) {
    MStream ss;
    ss << id;
    size_t bufflen = ss.length();
    MessageHead head;
    head.msg_id = GET;
    head.length = bufflen;
    send(sockfd, &head, sizeof(head), 0);
    send(sockfd, ss.c_str(), bufflen, 0);
    // printf("Send: %s, len = %ld\n", ss.c_str(), bufflen);


    MessageHead mh;
    int recv_size = recv(sockfd, &mh, sizeof(mh), 0);
    char *value = new char[mh.length];
    recv_size = recv(sockfd, value, mh.length, 0);
    MStream rss(value, mh.length);
    long doc_id;
    std::string doc_value;
    rss >> doc_id >> doc_value;

    // printf("Recv: msg_id %d, lenght %ld, value %s\n", mh.msg_id, mh.length, doc_value.c_str());
}

int setTest() {
    struct sockaddr_in servaddr;
    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == sockfd)
    {
        printf("Create socket error(%d): %s\n", errno, strerror(errno));
        return -1;
    }
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVER_IP, &servaddr.sin_addr);
    servaddr.sin_port = htons(SERVER_PORT);
    if (-1 == connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)))
    {
        printf("Connect error(%d): %s\n", errno, strerror(errno));
        return -1;
    }

    setDoc(sockfd, 1l, "SELECT 配偶 WHERE NAME = 陈毅");
    setDoc(sockfd, 2l, "SELECT 配偶 WHERE NAME = 刘德华");
    setDoc(sockfd, 3l, "UPDATE 配偶 WHERE NAME = 刘德华");
    setDoc(sockfd, 4l, "UPDATE 配偶 WHERE NAME = 陈毅");

    ::close(sockfd);
    return 0;

}

int clientTest()
{
    struct sockaddr_in servaddr;
    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == sockfd)
    {
        printf("Create socket error(%d): %s\n", errno, strerror(errno));
        return -1;
    }
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVER_IP, &servaddr.sin_addr);
    servaddr.sin_port = htons(SERVER_PORT);
    if (-1 == connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)))
    {
        printf("Connect error(%d): %s\n", errno, strerror(errno));
        return -1;
    }
    int i;
    for (i = 0; i < 1000; i++) {
        getDoc(sockfd, 1l);
        getDoc(sockfd, 2l);
        getDoc(sockfd, 4l);
        getDoc(sockfd, 3l);
    }

    printf("%d\n", i);

    ::close(sockfd);
    return 0;
}
