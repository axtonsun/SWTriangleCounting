#include<iostream>
#include<string>
#include<vector>
#include<map>
#include<unordered_map>
#ifndef setting 
#include "../../common/undirected-setting.h"
#define setting
#endif
#include "BPS-sampletable.h"

using namespace std;

// undirected & binary counting
class BPSsample
{
public:
	BPSSampleTable* st;
	int window_size;
	int current_time;
	int edge_estimate;
	double sample_prob;
	int hashindex;

    // 初始化函数，参数与 sample.h 的初始化函数相同
	BPSsample(int size, int w, int hi)
	{
		st = new BPSSampleTable(size);
		cout << "BPS sample size " << size << endl;
		window_size = w;
		current_time = 0;
		hashindex = hi;
	}
	~BPSsample()
	{
		delete st;
	}

    // 处理新边
	void proceed(unsigned long s, unsigned long d, long time)
	{
		if (s < d)
		{
			unsigned long tmp = s;
			s = d;
			d = tmp;
		}
		// 两个顶点转换并拼接成字符串
		string s_string = my_to_string(s);
		string d_string = my_to_string(d);
		string e = s_string + d_string;
		// 计算哈希值 也就是优先级
		double p = double((*hfunc[hashindex])((const unsigned char*)e.c_str(), e.length()) % 1000000 + 1) / 1000001;
		// 更新当前时间
		current_time = time;
		// 更新滑动窗口
		// time: 当前时间
		// time-window_size: 当前时间减去窗口大小
		// time-2*window_size: 当前时间减去两倍窗口大小
		st->update(time-window_size, time-2*window_size);
		// 插入新边
		// s: 顶点1
		// d: 顶点2
		// p: 概率
		// time: 当前时间
		// hashindex: 哈希索引
		st->insert(s, d, p, time, hashindex);
	}

	void prepare()
	{
		int m = st->size;
		double alpha = 0.7213 / (1 + (1.079 / m));
		int total_num = (double(alpha * m * m) / (st->q_count));
		int sample_num = st->valid_num;
		if (total_num < 2.5*m)
			total_num = -log(1 - double(st->edge_count) / m)*m;
		total_num = total_num *(double(st->valid_num) / st->edge_count);
		edge_estimate = total_num;
		//cout <<"BPS valid count "<< sample_num << "total edge" << total_num << "total sample " << st->edge_count << "q count" << st->q_count << endl;
		sample_prob = (double(sample_num) / total_num) * (double(sample_num - 1) / (total_num - 1)) * (double(sample_num - 2) / (total_num - 2));
	}

    // 估计全局三角形
	int count()
	{
		//cout <<"tc: "<<st->trcount<<' '<< p << endl;
		return (st->trcount) / sample_prob;

	}

    // 估计节点的局部三角形
	int local_count(unsigned int v)
	{
		sample_node* tmp = st->node_table->ID_to_pos(v);
		if (!tmp)
			return 0;
		else
			return (tmp->local_count) / sample_prob;
	}

    // 估计所有节点的局部三角形数量
    void all_local(unordered_map<unsigned int, int>& cr)
	{
		for(int i=0;i<st->node_table->length;i++)
		{
			cr[st->node_table->table[i].nodeID] = (st->node_table->table[i].local_count) / sample_prob;
			sample_node* tmp = st->node_table->table[i].next;
			while(tmp)
			{
				cr[tmp->nodeID] = (tmp->local_count) / sample_prob;
				tmp = tmp->next; 
			}
		}
	}
};

