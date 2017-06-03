#include "stdafx.h"
#include "Router.h"

Router::Router()
{
}

Router::~Router()
{
}

int Router::add_name(const std::string& name)
{
    assert(names.find(name) == names.end());
    adj.push_back(std::unordered_map<int, int>{});
    auto id = (int)names.size();
    names[name] = id;
    namesList.push_back(name);
    return id;
}

void Router::connect(int from, int to, int cost)
{
    assert(from >= 0 && from < (int)adj.size());
    assert(to >= 0 && to < (int)adj.size());
    adj[from][to] = cost;
    adj[to][from] = cost;
}

void Router::connect(const std::string& from, const std::string& to, int cost)
{
    adj[get_id_by_name(from)][get_id_by_name(to)] = cost;
    adj[get_id_by_name(to)][get_id_by_name(from)] = cost;
}

bool Router::is_direct(int from, int to) const
{
    return adj[from].find(to) != adj[from].end();
}

int Router::get_id_by_name(const std::string& name) const
{
    auto id = names.find(name);
    assert(id != names.end());
    return id->second;
}

const char* Router::get_name_by_id(int id) const
{
    assert(id >= 0 && id < (int)names.size());
    return namesList[id].c_str();
}

void Router::run()
{
    auto count = (int)names.size();
    recv.clear();
    tasks.clear();
    data.clear();
    delivery.clear();
    msgQueue.clear();
    flags.clear();
    times.clear();
    recv.resize(names.size());
    tasks.resize(names.size());
    data.resize(names.size());
    delivery.resize(names.size());
    flags.resize(names.size(), true);
    times.resize(names.size());
    task _task;
    _task.type = task_start;
    _task.value = nullptr;
    running_tasks = count;
    for (auto& task : tasks)
    {
        task.push(_task);
    }
    for (; running_tasks; ++time)
    {
        auto iter = msgQueue.begin();
        while (iter != msgQueue.end())
        {
            if (--iter->delay < 0)
            {
                delivery[iter->router].push_back(*iter);
                iter = msgQueue.erase(iter);
            }
            else
            {
                ++iter;
            }
        }
        for (auto i = 0; i < count; ++i)
        {
            if (flags[i])
                handle(i);
        }
    }
}

void Router::new_task(task_t type, int id, void* value)
{
    tasks[id].push(task{ type, value });
    times[id].push_back(time);
}

void Router::pop_task(int id)
{
    tasks[id].pop();
    times[id].pop_back();
}

void Router::send_msg(msg_t type, int from, int to, int ttl, unsigned long data)
{
    printf("[%04d] Message: type=%d, %s -> %s\n", time, type, get_name_by_id(from), get_name_by_id(to));
    auto router = get_router(from, to);
    auto dist = router == -1 ? adj[from][to] : adj[from][router];
    msgQueue.push_back(msg{ type, from, to, from, ttl, dist, time, data });
}

void Router::deliver_msg(int id)
{
    for (auto& m : delivery[id])
    {
        if (m.ttl == 0)
            continue;
        if (m.to == id)
        {
            recv[id].push(m);
        }
        else
        {
            m.ttl--;
            // router works
            auto router = get_router(id, m.to);
            if (router != -1)
            {
                m.router = router;
                m.delay = adj[router][m.to];
                msgQueue.push_back(m);
                printf("[%04d] Transmit: type=%d, router=%s, %s -> %s, time=%d, delay=%d\n", time, m.type,
                    get_name_by_id(router), get_name_by_id(m.from), get_name_by_id(m.to), m.time, m.delay);
            }
            else if (is_direct(m.from, m.to))
            {
                delivery[m.to].push_back(m);
            }
        }
    }
    delivery[id].clear();
}

int Router::get_router(int from, int to)
{
    auto distance = (task::map_int_long *)data[from][data_router];
    if (distance)
    {
        auto d = distance->find(to);
        if (d != distance->end())
        {
            return d->second;
        }
    }
    return -1;
}

void Router::handle(int id)
{
    switch (tasks[id].top().type)
    {
    case task_start:
        handle_start(id);
        printf("[%04d] Start: %s\n", time, get_name_by_id(id));
        break;
    case task_sleep:
        handle_sleep(id);
        break;
    case task_init:
        handle_init(id);
        break;
    case task_init_resp:
        handle_init_resp(id);
        break;
    case task_init_run:
        handle_init_run(id);
        break;
    case task_init_run_resp:
        handle_init_run_resp(id);
        break;
    case task_running:
        handle_running(id);
        break;
    case task_exit:
        handle_exit(id);
    default:
        break;
    }
    deliver_msg(id);
}

void Router::handle_start(int id)
{
    auto& current_task = tasks[id].top();
    auto unreq_list = new task::set_int{};
    current_task.type = task_init;
    current_task.value = unreq_list;
    for (auto& unreq : adj[id])
        unreq_list->insert(unreq.first);
    data[id][data_init_dist] = new task::map_int_long{};
}

void Router::handle_sleep(int id)
{
    auto& current_task = tasks[id].top();
    auto& t = (int&)current_task.value;
    if (t-- < 0)
    {
        pop_task(id);
    }
}

void Router::handle_init(int id)
{
    auto& current_task = tasks[id].top();
    auto distance = (task::map_int_long *)data[id][data_init_dist];
    auto unreq_list = (task::set_int *)current_task.value;
    if (unreq_list->empty())
    {
        delete unreq_list;
        current_task.type = task_init_run;
        data[id][data_router] = new task::map_int_long;
        new_task(task_sleep, id, (void*)100);
        return;
    }
    for (auto& unreq : *unreq_list)
    {
        send_msg(msg_init_req, id, unreq, ttl, time);
        (*distance)[unreq] = time;
    }
    new_task(task_init_resp, id, (void *)unreq_list);
}

void Router::handle_init_resp(int id)
{
    auto& current_task = tasks[id].top();
    auto unreq_list = (task::set_int *)current_task.value;
    if (recv[id].empty())
    {
        auto span = time - times[id].back();
        if (span % timed_out == 0)
        {
            for (auto& unreq : *unreq_list)
                send_msg(msg_init_req, id, unreq, ttl, time);
        }
        return;
    }
    auto distance = (task::map_int_long *)data[id][data_init_dist];
    auto recv_msg = recv[id].front();
    recv[id].pop();
    switch (recv_msg.type)
    {
    case msg_init_req:
        if (recv_msg.to == id)
            send_msg(msg_init_resp, id, recv_msg.from, ttl, time);
        break;
    case msg_init_resp:
        (*distance)[recv_msg.from] = time - (*distance)[recv_msg.from];
        printf("[%04d] Distance: dist=%d, %s -> %s\n", time, (*distance)[recv_msg.from], get_name_by_id(id), get_name_by_id(recv_msg.from));
        unreq_list->erase(recv_msg.from);
        if (unreq_list->empty())
            pop_task(id);
        break;
    default:
        break;
    }
}

void Router::handle_init_run(int id)
{
    auto& current_task = tasks[id].top();
    auto distance = (task::map_int_long *)data[id][data_init_dist];
    for (auto& dist : *distance)
        send_msg(msg_init_router_req, id, dist.first, ttl, (unsigned long)distance);
    new_task(task_init_run_resp, id, distance);
}

void Router::handle_init_run_resp(int id)
{
    auto& current_task = tasks[id].top();
    auto distance = (task::map_int_long *)data[id][data_init_dist];
    auto router = (task::map_int_long *)data[id][data_router];
    if (recv[id].empty())
    {
        if (time - times[id].front() > 600)
        {
            printf("---- Routing Table @ %s ----\n", get_name_by_id(id));
            for (auto& d : *distance)
            {
                if (d.first == id)
                    continue;
                auto r = router->find(d.first);
                if (r != router->end())
                    printf("%s, jump=%s, dist=%d\n", get_name_by_id(d.first), get_name_by_id(r->second), d.second);
                else
                    printf("%s, DIRECT, dist=%d\n", get_name_by_id(d.first), d.second);
            }
            printf("---- Routing Table @ %s ----\n", get_name_by_id(id));
            current_task.type = task_running;
            times[id].front() = time;
        }
        return;
    }
    auto recv_msg = recv[id].front();
    recv[id].pop();
    auto distance2 = (task::map_int_long *)recv_msg.data;
    for (auto& dist : *distance2)
    {
        if (dist.first == id)
            continue;
        auto d = distance->find(dist.first);
        if (d == distance->end())
        {
            (*distance)[dist.first] = dist.second;
            (*router)[dist.first] = recv_msg.from;
        }
        else
        {
            auto& old_dist = (*distance)[dist.first];
            auto new_dist = dist.second + (*distance)[recv_msg.from];
            printf("[%04d] Distance: old=%4d, %s -> %s | new=%4d, %s -> %s -> %s\n", time,
                old_dist, get_name_by_id(id), get_name_by_id(dist.first),
                new_dist, get_name_by_id(id), get_name_by_id(recv_msg.from), get_name_by_id(dist.first));
            if (new_dist < old_dist)
            {
                (*router)[dist.first] = recv_msg.from;
                old_dist = new_dist;
            }
        }
    }
}

void Router::handle_running(int id)
{
    auto& current_task = tasks[id].top();
    auto span = time - times[id].front();
    if (span > 220)
        current_task.type = task_exit;
    //printf("[%04d] Running: %s\n", time, get_name_by_id(id));
    if (span < 2)
    {
        for (size_t i = 0; i < names.size(); ++i)
        {
            if (i == id)
                continue;
            send_msg(msg_ping, id, i, ttl, time);
        }
    }
    if (!recv[id].empty())
    {
        auto recv_msg = recv[id].front();
        recv[id].pop();
        switch (recv_msg.type)
        {
        case msg_ping:
            printf("[%04d] Received ping: time=%d %s -> %s\n", time, time - (int)recv_msg.data, get_name_by_id(recv_msg.from), get_name_by_id(recv_msg.to));
            break;
        default:
            break;
        }
    }
}

void Router::handle_exit(int id)
{
    delete (task::map_int_long *)data[id][data_init_dist];
    flags[id] = false;
    running_tasks--;
    printf("[%04d] Exit: %s\n", time, get_name_by_id(id));
}
