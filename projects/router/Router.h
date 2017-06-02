#ifndef _ROUTER_H
#define _ROUTER_H

#include <cassert>
#include <string>
#include <vector>
#include <queue>
#include <stack>
#include <unordered_map>
#include <unordered_set>

class Router
{
private:
    enum msg_t
    {
        msg_init_req = 10,
        msg_init_resp,
    };

    struct msg
    {
        msg_t type;
        int from, to;
        int value;
        int delay;
    };

    enum task_t
    {
        task_exit = -1,
        task_start = 0,
        task_init = msg_init_req,
        task_init_resp = msg_init_resp,
        task_init_run,
    };

    struct task
    {
        task_t type;
        void* value;

        typedef std::unordered_map<int, long> map_int_long;
        typedef std::unordered_set<int> set_int;
    };

    enum data_t
    {
        data_init_dist = task_init,
        data_router,
    };

    std::vector<std::unordered_map<int, int>> adj;
    std::unordered_map<std::string, int> names;
    std::vector<std::string> namesList;
    std::vector<msg> msgQueue;
    std::vector<std::vector<msg>> delivery;
    std::vector<std::queue<msg>> recv;
    std::vector<std::stack<task>> tasks;
    std::vector<std::unordered_map<int, void *>> data;
    std::vector<bool> flags;
    std::vector<std::vector<long>> times;
    long time{ 0 };
    int ttl{ 10 };
	int timed_out{ 120 };
    int running_tasks{ 0 };

public:
    Router();

    ~Router();

    int add_name(const std::string& name);
    void connect(int from, int to, int cost);
    void connect(const std::string& from, const std::string& to, int cost);
    bool is_direct(int from, int to) const;
    int get_id_by_name(const std::string& name) const;
    const char* get_name_by_id(int id) const;
    void run();

private:
    void new_task(task_t type, int id, void* value);
    void send_msg(msg_t type, int from, int to, int value);
    void deliver_msg(int id);
    void handle(int id);
    void handle_start(int id);
    void handle_init(int id);
    void handle_init_resp(int id);
};


#endif
