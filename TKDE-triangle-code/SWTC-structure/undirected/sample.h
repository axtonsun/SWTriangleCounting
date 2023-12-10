#include<iostream>
#include<string>
#include<vector>
#include<unordered_map>
#include<map>
#include<assert.h>
#ifndef setting 
#include "../../common/undirected-setting.h"
#define setting
#endif
#include "sampletable.h"

using namespace std;

// undirected & binary counting
class sample
{
public:
	SampleTable* st; // 指向 SampleTable 类型的指针 | SWTC-structure/undirected/sampletable.h	
	int window_size;
	long long current_time;
	long long land_mark;
	long long last_mark;
	int edge_estimate;
	double sample_prob; //样本概率
	int hashindex; // 哈希索引

    // 采样器的初始化
    // size 是子流的数量，k
    // w 是窗口长度(N 乘以时间单位(这是数据集的平均时间跨度))
    // hi 是哈希索引(将 hi 从0改变到10将使 SWTC 能够用不同的哈希函数实现)
    // 4万~12万 3709万 0~4
	sample(int size, int w, int hi)
	{
		st = new SampleTable(size); //4万~12万
		window_size = w; // 3709万
		current_time = 0;
		land_mark = 0;
		last_mark = 0;
		hashindex = hi; // 0~4
	}

    ~sample()
	{
		delete st;
	}

    // 处理一个无向边(s,d)其时间戳为time
    // 当前时间T将更新为参数time
    void proceed(unsigned int s, unsigned int d, long long time)
	{
        // 源id < 目的id 交换位置
        // s:2 d:4822
		if (s < d)
		{
			unsigned int tmp = s;
			s = d;
			d = tmp;
		}
        // s:"4822" d:"2" e:"48222"
		string s_string = my_to_string(s); // wiki: 4822=4821+1
		string d_string = my_to_string(d); // wiki: 2=1+1
		string e = s_string + d_string;

		// p: 0.34361265638734362
		// 计算边的优先级
		double p = double((*hfunc[hashindex])((const unsigned char*)e.c_str(), e.length()) % 1000000+1) / 1000001;

		// time:0
		current_time = time;

        // -3709万 = time-window_size land_mark:0 last_mark:0
		// 时间-窗口大小，路标，上一个路标
		st->update(time-window_size, land_mark, last_mark);

		// 路标切换
		// 时间 - 路标 大于等于 窗口大小
		if (time - land_mark >= window_size)
		{
			// 时间 - 路标 小于 2倍窗口大小 才不会出错
			assert(time - land_mark < 2*window_size);
			last_mark = land_mark;
			land_mark = land_mark + window_size;
			st->ective();
			// cout << time << ' '<<land_mark<<" effected " << window_size << endl;
		}

		// 4822->2
		// p: 0.34361265638734362
		// time:0 land_mark:0 last_mark:0 hashindex: 0
		st->insert(s, d, p, time, land_mark, last_mark, hashindex);
	}

    // 为查询做准备，需要调用以下函数之前被调用
	void prepare()
	{
		int m = st->size; // 40000
		double alpha = 0.7213 / (1 + (1.079 / m)); // alpha: 0.72128054345734038
		// st->q_count: 34834.332702636719
        int total_num = (double(alpha * m * m) / (st->q_count));// 33129
		int sample_num = st->valid_num; // 7588

        if (total_num < 2.5*m)
			total_num = -log(1 - double(st->edge_count) / m)*m; // 8414
		total_num = total_num *(double(st->valid_num) / st->edge_count); // 8414 * (7588/7588)

        edge_estimate = total_num; // 8414

        //cout << q << ' ' << total_num << endl;
		//cout <<"st count "<< sample_num << ' ' << total_num << ' '<<st->edge_count<<' '<<st->q_count<<endl;

        // sample_num 为样本图中的边数 m
        // total_num 为估算出的滑动窗口内互异边数量 n
		// sample_prob: 0.73342816564049085
		sample_prob = (double(sample_num) / total_num) * (double(sample_num - 1) / (total_num - 1)) * (double(sample_num - 2) / (total_num - 2));

	}

    // 返回全局三角形的估计值
    // 样本图中全局三角形个数 / 三角形三条边均被加入样本的概率
    int count()
	{
		//cout <<"tc: "<<st->trcount<<' '<< p << endl;
		return (st->trcount) / sample_prob;

	}

    // 返回节点 v 的局部计数
    int local_count(unsigned int v)
	{
		sample_node* tmp = st->node_table->ID_to_pos(v);
		if (!tmp)
			return 0;
		else
			return (tmp->local_count) / sample_prob;
	}

    // 将所有采样节点的局部计数以<node ID,local count>对的形式存储在 cr 中
    // 不在采样图中的节点不会被报告 这些节点的局部计数估计值为 0
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

