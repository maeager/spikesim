// BBServer.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef BBSERVER_H
#define BBSERVER_H


#include <map>
#include "BBS.h"

#define debug 0
#define nil 0

#define QUIT 0
#define POST 1
#define LOOK 2
#define LOOK_TAKE 3
#define TAKE 4
#define LOOK_YES 6
#define LOOK_NO 7
#define LOOK_TAKE_YES 8
#define LOOK_TAKE_NO 9
#define HELLO 10
#define POST_TODO 11
#define POST_RESULT 12
#define LOOK_TAKE_TODO 13
#define LOOK_TAKE_RESULT 14
#define TAKE_TODO 15
#define CONTEXT 16
#define CRAY_POST 17
#define CRAY_POST_TODO 18
#define CRAY_POST_RESULT 19
#define FIRSTID 20

#define MessageList MpiMessageList
#define WorkItem MpiWorkItem
#define WorkList MpiWorkList
#define ReadyList MpiReadyList
#define ResultList MpiResultList
#define PendingList MpiPendingList
#define LookingToDoList MpiLookingToDoList

extern int bbs_poll_;
#define BBSPOLL if (--bbs_poll_ == 0) { bbs_handle(); }



class MpiMessageList;
class MpiPendingList;
class MpiWorkList;
class MpiReadyList;
class MpiLookingToDoList;
class MpiResultList;
struct bbsmpibuf;
class KeepArgs;

void bbs_context_wait();
void bbs_handle();

//! Abstract BBS server class
/*! methods remain virtual so that these commands are done by BBSDirectServer, 
 * adding layer to send and recv buffers 
 * 
 */
class BBSDirect : public BBSImpl
{
public:
    BBSDirect();
    virtual ~BBSDirect();

    virtual bool look(const char*);

    virtual void take(const char*); /* blocks til something to take */
    virtual bool look_take(const char*); /* returns false if nothing to take */
    // after taking use these
    virtual int upkint();
    virtual double upkdouble();
    virtual void upkvec(int, double*);
    virtual char* upkstr(); // delete [] char* when finished

    // before posting use these
    virtual void pkbegin();
    virtual void pkint(int);
    virtual void pkdouble(double);
    virtual void pkvec(int, double*);
    virtual void pkstr(const char*);
    virtual void post(const char*);

    virtual void post_todo(int parentid);
    virtual void post_result(int id);
    virtual int look_take_result(int pid); // returns id, or 0 if nothing
    virtual int look_take_todo(); // returns id, or 0 if nothing
    virtual int take_todo(); // returns id
    virtual void save_args(int);
    virtual void return_args(int);

    virtual void context();

    virtual void start();
    virtual void done();

    virtual void perror(const char*);
    static void check_pvm();
private:
    KeepArgs* keepargs_;

    bbsmpibuf* sendbuf_, *recvbuf_;

};

//! BBSDirectServer - server side of bulletin board
/*! Performs the task of collecting queues of messages and controlling operations of the BBS.
 */
class BBSDirectServer
{
public:
    BBSDirectServer();
    virtual ~BBSDirectServer();

    void post(const char* key, bbsmpibuf*);
    bool look(const char* key, bbsmpibuf**);
    bool look_take(const char* key, bbsmpibuf**);
    bool take_pending(const char* key, int* cid);
    void put_pending(const char* key, int cid);
    static BBSDirectServer* server_;
    static void handle(); // all remote requests
    static void handle1(int size, int tag, int source);
    void start();
    void done();

    void post_todo(int parentid, int cid, bbsmpibuf*);
    void context(bbsmpibuf*);
    bool send_context(int cid); // sends if not sent already
    void post_result(int id, bbsmpibuf*);
    int look_take_todo(bbsmpibuf**);
    int look_take_result(int parentid, bbsmpibuf**);
    void context_wait();
private:
    void add_looking_todo(int cid);
private:
    MpiMessageList* messages_;
    MpiPendingList* pending_;
    MpiWorkList* work_;
    MpiLookingToDoList* looking_todo_;
    MpiReadyList* todo_;
    MpiResultList* results_;
    MpiLookingToDoList* send_context_;
    int next_id_;
    bbsmpibuf* context_buf_;
    int remaining_context_cnt_;
};



/** Client side of BBS
 *
 */
class BBSClient : public BBSImpl
{
public:
    BBSClient();
    virtual ~BBSClient();

    virtual bool look(const char*);

    virtual void take(const char*); /* blocks til something to take */
    virtual bool look_take(const char*); /* returns false if nothing to take */
    // after taking use these
    virtual int upkint();
    virtual double upkdouble();
    virtual void upkvec(int, double*);
    virtual char* upkstr(); // delete [] char* when finished

    // before posting use these
    virtual void pkbegin();
    virtual void pkint(int);
    virtual void pkdouble(double);
    virtual void pkvec(int, double*);
    virtual void pkstr(const char*);
    virtual void post(const char*);

    virtual void post_todo(int parentid);
    virtual void post_result(int id);
    virtual int look_take_result(int pid); // returns id, or 0 if nothing
    virtual int look_take_todo(); // returns id, or 0 if nothing
    virtual int take_todo(); // returns id
    virtual void save_args(int);
    virtual void return_args(int);

    virtual void start();
    virtual void done();

    virtual void perror(const char*);
private:
    int get(const char* key, int type); // return type
    int get(int key, int type); // return type
    int get(int type); // return type
private:
    static int sid_;
    KeepArgs* keepargs_;

    void upkbegin();
    char* getkey();
    int getid();
    bbsmpibuf* sendbuf_, *recvbuf_, *request_;


};



#endif
