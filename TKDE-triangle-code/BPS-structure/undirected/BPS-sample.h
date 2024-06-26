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

    // 函数初始化
	// size: 子流大小 w: 滑动窗口大小 hi: 哈希索引
	BPSsample(int size, int w, int hi)
	{
		st = new BPSSampleTable(size);
		window_size = w;
		current_time = 0;
		hashindex = hi;
	}
	~BPSsample()
	{
		delete st;
	}

    // 处理新边
	// s: 源节点 d: 目标节点 time: 时间戳(当前数据的时间戳 - 第一条数据的时间戳)
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
		// 计算边的优先级
		// % 1000000 + 1: 取模
		// / 1000001: 归一化
		// p: 0.34361265638734362 / 0.045699954300045698
		double p = double((*hfunc[hashindex])((const unsigned char*)e.c_str(), e.length()) % 1000000 + 1) / 1000001;
		// 更新当前时间
		current_time = time;
		// 更新滑动窗口
		st->update(time-window_size, time-2*window_size);
		
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

    // 估计全局三角形: 样本图三角形个数/采样概率
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

