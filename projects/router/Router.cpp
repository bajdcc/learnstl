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
            if (--iter->delay == 0)
            {
                delivery[iter->from].push_back(*iter);
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
}

void Router::send_msg(msg_t type, int from, int to, int value)
{
    printf("Message: type=%d, %s -> %s\n", type, get_name_by_id(from), get_name_by_id(to));
    msgQueue.push_back(msg{ type, from, to, value, adj[from][to] });
}

void Router::deliver_msg(int id)
{
    for (auto& m : delivery[id])
    {
        if (m.value == 0)
            continue;
        if (m.to == id)
        {
            recv[id].push(m);
        }
        else if (is_direct(m.from, m.to))
        {
            m.value--;
            delivery[m.to].push_back(m);
        }
        else
        {
            m.value--;
            // router works
        }
    }
    delivery[id].clear();
}

void Router::handle(int id)
{
    switch (tasks[id].top().type)
    {
    case task_start:
        handle_start(id);
        printf("Start: %s\n", get_name_by_id(id));
        break;
    case task_init:
        handle_init(id);
        break;
    case task_init_resp:
        handle_init_resp(id);
        break;
    case task_exit:
        flags[id] = false;
        running_tasks--;
        printf("Exit: %s\n", get_name_by_id(id));
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

void Router::handle_init(int id)
{
    auto& current_task = tasks[id].top();
    auto unreq_list = (task::set_int *)current_task.value;
    if (unreq_list->empty())
    {
        delete unreq_list;
        current_task.type = task_init_run;
        return;
    }
    auto distance = (task::map_int_long *)data[id][data_init_dist];
    for (auto& unreq : *unreq_list)
    {
        send_msg(msg_init_req, id, unreq, ttl);
        (*distance)[unreq] = time;
    }
    new_task(task_init_resp, id, (void *)unreq_list);
    times[id].push_back(time);
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
				send_msg(msg_init_req, id, unreq, ttl);
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
            send_msg(msg_init_resp, id, recv_msg.from, ttl);
        break;
    case msg_init_resp:
        (*distance)[recv_msg.from] = time - (*distance)[recv_msg.from];
        printf("Distance: dist=%d, %s -> %s\n", (*distance)[recv_msg.from], get_name_by_id(id), get_name_by_id(recv_msg.from));
        unreq_list->erase(recv_msg.from);
		if (unreq_list->empty())
		{
			current_task.type = task_exit;
			times[id].pop_back();
		}
        break;
    default:
        break;
    }
}
