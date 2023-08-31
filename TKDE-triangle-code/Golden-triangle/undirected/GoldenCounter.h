#include<iostream>
#include<string>
#include<vector> 
#include "Graph.h"
using namespace std;

struct timed_edge
{
	long long timestamp;
	unsigned int s; 
	unsigned int d;
	timed_edge* next;
	timed_edge(unsigned int s_, unsigned int d_, long long t)
	{
		s = s_;
		d = d_;
		timestamp = t;
		next = NULL;
	}
};

// 在滑动窗口中计算准确的三角形数量
class GoldenCounter
{
	private:
		int windowsize;
		long long current_time;
		timed_edge* tsl_head;
		timed_edge* tsl_tail;
		int duplicated_edgenum;
		Graph* graph;
	 
	public:
		GoldenCounter(int w)
		{
			windowsize = w;
			current_time = 0;
			graph = new Graph;
			duplicated_edgenum = 0;
			tsl_head = NULL;
			tsl_tail = NULL;
		}

        ~GoldenCounter()
		{
			timed_edge* cur = tsl_head;
			tsl_head = NULL;
			tsl_tail = NULL;
			timed_edge* next = cur;
			while (cur)
			{
				next = cur->next;
				delete cur;
				cur = next;
			}
			delete graph; 
		}

        int get_edgenum()
		{
			return graph->get_edgenum();
		}

        // 插入一个带有时间戳time的边(s,d)
        // 将当前时间T更新为参数time
        // 触发删除过期的边
		void insert_edge(unsigned int s, unsigned int d, long long time)
		{
		/*	if(s == d)
				return;*/
			duplicated_edgenum++;
			if(s < d)
			{
				unsigned int tmp = s;
				s = d;
				d = tmp;
			}
			///cout<<"inserting : "<<s<<' '<<d<<' ';
			current_time = time;
			timed_edge* e = new timed_edge(s, d, current_time);
			if (!tsl_head)
				tsl_head = e;
			if (tsl_tail)
				tsl_tail->next = e;
			tsl_tail = e;
			graph->insert_edge(s, d);
			
			timed_edge* cur = tsl_head;
			timed_edge* next;
			while (cur->timestamp < current_time - windowsize)
			{
				duplicated_edgenum--;
				next = cur->next;
				graph->delete_edge(cur->s, cur->d);
				delete cur;
				cur = next;
				tsl_head = next;
				if(!cur)
				    return;
			}
			//cout<<"insert finished"<<endl;
			return; 
		}

        // 计算滑动窗口中重复边的数量
		int duplicate_count()
		{
			return duplicated_edgenum;
		}

        // 计算滑动窗口中不同边的数量
        int edge_count()
		{
			return graph->get_edgenum();
		}

        // 在滑动窗口中二项计数全局三角形
		int triangle_count()
		{
			return graph->count_triangle();
		 }

         // 在滑动窗口中带权计数全局三角形
         unsigned long long weighted_count()
		 {
		 	return graph->weighted_count_triangle();
		 }

         void local_count(unordered_map<unsigned int, int> &cr)
		{
		  	graph->local_count(cr);	
		}

        void weighted_local(unordered_map<unsigned int, unsigned long long> &cr)
		{
			graph->weighted_local_count(cr);
		}
};
