#include<iostream>
#include<string>
#include<vector>
#include<map>
#include<assert.h>
#include "../../common/undirected-setting.h"
#include "simple_sample.h"

using namespace std;

class  nv_sample
{
public:
	Simple_Sample* ss;
	int window_size;
	long long current_time;
	long long land_mark;
	long long last_mark;
	int edge_estimate;
	int hashindex;

    // 初始化函数
    // 参数与 sample.h 中相同
	nv_sample(int size, int w, int hash_index)
	{
		ss = new Simple_Sample(size);
		window_size = w;
		current_time = 0;
		land_mark = 0;
		last_mark = 0;
		hashindex = hash_index;
	}
	~nv_sample()
	{
		delete ss;
	}

    // 与 sample.h 中的 proceed() 接口相同
	void proceed(unsigned int  s, unsigned int  d, long long time)
	{
		if (s < d)
		{
			unsigned int  tmp = s;
			s = d;
			d = tmp;
		}
		string s_string = to_string(s);
		string d_string = to_string(d);
		string e = s_string + d_string;
		double p = double((*hfunc[hashindex])((const unsigned char*)e.c_str(), e.length()) % 1000000+1) / 1000001;
		current_time = time;
		ss->update(time-window_size);
		if (time - land_mark >= window_size)
		{
			assert(time - land_mark < 2*window_size);
			last_mark = land_mark;
			land_mark = land_mark + window_size;
			ss->slice_switch(last_mark);
		//	cout << time << ' '<<land_mark<<" effected" << endl;
		}
		ss->insert(s, d, p, time, land_mark, last_mark, hashindex);
	}

    // 估算滑动窗口中全局三角形的数量
    // sample.h 和 asy_sample.h 中使用了 prepare 函数，因为它们同时支持 局部和全局计数，并且这些查询共享相同的准备步骤
    // 只提供 全局计数 因此 prepare 的步骤都包含在 count() 里
	int count()
	{
		int m = ss->size;
		double alpha = 0.7213 / (1 + (1.079 / m));
		int total_num = (double(alpha * m * m) / (ss->q_count));
		int sample_num = ss->valid_num;
		if (total_num < 2.5*m)
			total_num = -log(1 - double(ss->edge_count) / m)*m;
		total_num = total_num *(double(ss->valid_num) / ss->edge_count);
		edge_estimate = total_num;
		double p = (double(sample_num) / total_num) * (double(sample_num-1) / (total_num-1)) * (double(sample_num-2) / (total_num-2));
	//	cout <<"nc: "<<ss->trcount<<' '<< p << endl;
		return (ss->trcount) / p;
	}
};
