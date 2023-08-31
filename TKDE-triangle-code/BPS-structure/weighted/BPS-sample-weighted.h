#include<iostream>
#include<string>
#include<vector>
#include<map>
#include<unordered_map>
#ifndef setting 
#include "../../common/undirected-setting.h"
#define setting
#endif
#include "BPS-sampletable-weighted.h"

using namespace std;

// undirected & weighted counting
// BPS-sample.h 和 BPS-sampletable.h 的 加权版本
// 与相应的 binary 版本具有相同的接口
// 除了 count() 返回值为 无符号长整型
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

	void proceed(unsigned long s, unsigned long d, long time)
	{
		if (s < d)
		{
			unsigned long tmp = s;
			s = d;
			d = tmp;
		}
		double p = (rand()+1)/double(RAND_MAX+2);
		current_time = time;
		st->update(time-window_size, time-2*window_size);
		//if(time>44000000) cout<<" BPS update finished"<<endl;
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
	unsigned long long count()
	{

		//cout <<"tc: "<<st->trcount<<' '<< p << endl;
		return (st->trcount) / sample_prob;

	}
	unsigned int local_count(unsigned int v)
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

