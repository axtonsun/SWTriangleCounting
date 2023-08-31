#include<iostream>
#include<string>
#include<vector>
#include<map>
#include<unordered_map>
#ifndef directedsetting 
#include "../../common/directed-setting.h"
#define directedsetting
#endif
#include "BPS-sampletable-direct.h"

using namespace std;

// directed & binary counting
// BPS-sample.h 和 BPS-sampletable.h 的 有向版本
// 与相应的无向版本具有相同的接口
class BPSsample
{
public:
	BPSSampleTable* st;
	int window_size;
	int current_time;
	int edge_estimate;
	double sample_prob;
	int hashindex;

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

	void proceed(unsigned int s, unsigned int d, long long time)
	{
		string s_string = my_to_string(s);
		string d_string = my_to_string(d);
		string e = s_string + d_string;
		double p = double((*hfunc[hashindex])((const unsigned char*)e.c_str(), e.length()) % 1000000 + 1) / 1000001;
		current_time = time;
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
		sample_prob = (double(sample_num) / total_num) * (double(sample_num - 1) / (total_num - 1)) * (double(sample_num - 2) / (total_num - 2));

	}
	int count()
	{
		return (st->trcount) / sample_prob;
	}
	int local_count(unsigned int v)
	{
		sample_node* tmp = st->node_table->ID_to_pos(v);
		if (!tmp)
			return 0;
		else
			return (tmp->local_count) / sample_prob;
	}
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

