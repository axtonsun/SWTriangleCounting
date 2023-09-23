#include<iostream>
#include<string>
#include<vector>
#include<map>
#include<unordered_map>
#include<assert.h>
#ifndef setting
#include "../../common/undirected-setting.h"
#define setting
#endif
#include "asy_sampletable.h"

using namespace std;

class asy_sample
{
public:
	asy_sampletable* ss;
	int window_size;
	int group_num;
	long long current_time;
	long long* land_mark;
	long long* last_mark;
	int edge_estimate;
	int sample_size;
	double sample_prob;
	int hashindex;
	int gswitch_iter; // use a iterator to mark the group with largest group ID and has switched slice. This iterator increase from 0 to group_num and return to 0;

    // g 是AG技术中的 group 的数量
	// size: 4万~12万 w: 3709万 g:10 hindex: 0~4
	asy_sample(int size, int w, int g, int hash_index)
	{
		ss = new asy_sampletable(size, g);
		hashindex = hash_index;// 0
		group_num = g;// 10
		window_size = w;// 3709万

		current_time = 0;
		land_mark = new long long[g];
		last_mark = new long long[g];
		for(int i=0;i<g;i++)
		{
			land_mark[i] = -(g-i)*(window_size/g);
			//-37090000
            //-33381000
            //-29672000
            //-25963000
            //-22254000
            //-18545000
            //-14836000
            //-11127000
            //-7418000
            //-3709000
//			std::cout<< land_mark[i]<<std::endl;
		}
		gswitch_iter = 0;
		sample_size = 0;
		edge_estimate = 0;
		sample_prob = 0;

	}

    ~asy_sample()
	{
		delete ss;
		delete []land_mark;
		delete []last_mark;
	}

	void proceed(unsigned int s, unsigned int d, long long time)
	{// s: 231370 d: 204123 time: 0
        // 两顶点值交换
		if (s < d)
		{
			unsigned int tmp = s;
			s = d;
			d = tmp;
		}
        // 无符号int型 转换成 string型
		// s_string: "231370"   s: 231370
		string s_string = my_to_string(s);
		string d_string = my_to_string(d);
        // 两个string型相连
        // eg. s="231370" d="204123" e="231370204123"
		string e = s_string + d_string;

		// 将字符串e通过哈希函数转换为一个大于0小于等于1的浮点数p
		// 通过hashindex从hfunc数组中获取了一个函数指针，然后调用这个哈希函数，对字符串e进行哈希处理
		// 具体来说，e.c_str()将字符串e转换为C风格字符串，然后通过(const unsigned char*)将其转换为无符号字符指针，最后用e.length()作为哈希函数的长度参数
		// 对哈希函数的结果进行了取模运算（% 1000000），然后加上1。取模运算的结果范围是0到999999，加1后的范围是1到1000000
		// 将上一步得到的结果强制转换为double类型，然后除以1000001。这一步的结果范围是大于0小于等于1的浮点数
		// hashindex: 0  -> MurmurHash
		// h: 2299451388 % 1000000 + 1 = 451389
		// p: 0.45138854861145139 = 451389.0 / 1000001
		double p = double((*hfunc[hashindex])((const unsigned char*)e.c_str(), e.length()) % 1000000 + 1) / 1000001;
		// hashindex: 0+2  group_num: 10  e.length(): 12
		// group: 1 = h: 1412841161 % 10
		int group  = ((*hfunc[hashindex+2])((unsigned char*)(e.c_str()), e.length()) % group_num);
		current_time = time;
        // time: 0 window_size: 37090000
		ss->update(time - window_size);

		while (time - land_mark[gswitch_iter] >= window_size)
		{// 0-landmark[0] >= 3709万
			assert(time - land_mark[gswitch_iter] < 2 * window_size);
			last_mark[gswitch_iter] = land_mark[gswitch_iter];
			land_mark[gswitch_iter] = land_mark[gswitch_iter] + window_size;
//			std::cout<< last_mark[gswitch_iter]<<std::endl;
//			std::cout<< land_mark[gswitch_iter]<<std::endl;

			//
			ss->slice_switch(last_mark[gswitch_iter], gswitch_iter);

			gswitch_iter++;
			if(gswitch_iter==group_num)
				gswitch_iter = 0;
		}
		ss->insert(s, d, group, p, time, land_mark[group], last_mark[group], hashindex);
	}

	void prepare()
	{
		sample_size = 0;
		for(int i=0;i<group_num;i++)
			sample_size += ss->valid_num[i];
		
		edge_estimate = 0;
		for(int i=0;i<group_num;i++)
		{
		int m = ss->group_size;
		double alpha = 0.7213 / (1 + (1.079 / m));
		int group_card = (double(alpha * m * m) / (ss->q_count[i]));
		if (group_card < 2.5*m)
			 group_card = -log(1 - double(ss->edge_count[i]) / m)*m;
		edge_estimate += group_card *(double(ss->valid_num[i]) / ss->edge_count[i]);
		}

		sample_prob = 	(double(sample_size) / edge_estimate) * (double(sample_size - 1) / (edge_estimate - 1)) * (double(sample_size - 2) / (edge_estimate - 2));

	}

    int count()
	{
		return (ss->trcount) / sample_prob;
	}

	int local_count(unsigned int v)
	{
		sample_node* tmp = ss->node_table->ID_to_pos(v);
		if(!tmp)
			return 0;
		else
			return (tmp->local_count) / sample_prob; 
	}
	
	void all_local(unordered_map<unsigned int, int>& cr)
	{
		for(int i=0;i<ss->node_table->length;i++)
		{
			int tr_count = (ss->node_table->table[i].local_count) / sample_prob;
			cr[ss->node_table->table[i].nodeID] = (ss->node_table->table[i].local_count) / sample_prob;
			sample_node* tmp = ss->node_table->table[i].next;
			while(tmp)
			{
				cr[tmp->nodeID] = (tmp->local_count) / sample_prob;
				tmp = tmp->next; 
			}
		}
	}

	int valid_count()
	{
		int total_count = 0;
		for(int i=0;i<group_num;i++)
			total_count += ss->valid_num[i];
		return total_count;
	}
};
