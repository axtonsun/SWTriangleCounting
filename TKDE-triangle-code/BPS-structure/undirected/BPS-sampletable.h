#include<iostream>
#include<signal.h>
#include<stdlib.h>
#include<unistd.h>
#include<vector>
#include<assert.h>
#include<math.h>
using namespace std;

 class BPSSampleTable
 {
 	public:
 	EdgeTable* edge_table;
	NodeTable* node_table;
 	int size;
	int edge_count;
	int node_count;
	int valid_num;
	int trcount; // 样本图的三角形个数
	double q_count;

 	BPSSampleTable(int s)
 	{
 		size = s; // 4万~12万
		edge_count = 0;
		node_count = 0;
		valid_num = 0;
		trcount = 0;
		edge_table = new EdgeTable(s); // 4万~12万
		node_table = new NodeTable(4, s*2); // 4 8万~24万
		q_count = size;

	 }
	~BPSSampleTable ()
	 {
		delete edge_table;
		delete node_table;
	 }
			
	void modify_triangle(sample_node* pos_s, sample_node* pos_d, int op)
	{
		vector<unsigned int> v1;
		vector<unsigned int> v2;
		
		unsigned int s_num = pos_s->nodeID;
		unsigned int d_num = pos_d->nodeID;
		
		// edge_s: 34841 edge_d: 34841
		int edge_s = pos_s->first_edge;
		int edge_d = pos_d->first_edge;

		bool print = false;

		while (edge_s >= 0)
		{
			unsigned int tmp;
			int next_index;
			// edge_table->table[edge_s].s: 4822 s_num: 4822
			if (edge_table->table[edge_s].s == s_num)
			{
				tmp = edge_table->table[edge_s].d; // tmp: 2
				next_index = edge_table->table[edge_s].pointers[last_s]; // next_index: -1
			}
			else if (edge_table->table[edge_s].d == s_num)
			{
				tmp = edge_table->table[edge_s].s;
				next_index = edge_table->table[edge_s].pointers[last_d];
			}

			if (edge_table->table[edge_s].vice.timestamp < 0)  // only count the valid edge
			{
				v1.push_back(tmp); // v1: 2
			}	
			edge_s = next_index; // edge_s: -1
		}
				
		while (edge_d >= 0)
		{
			unsigned int tmp;
			int next_index;
			if(print) cout<<edge_d<<' '<<edge_table->table[edge_d].s<<' '<<edge_table->table[edge_d].d<<endl;			// edge_table->table[edge_d].d: 2 d_num: 2
			if (edge_table->table[edge_d].d == d_num)
			{
				tmp = edge_table->table[edge_d].s; // tmp: 4822 / 6437->4822
				next_index = edge_table->table[edge_d].pointers[last_d]; // next_index: -1
			}
			else if (edge_table->table[edge_d].s == d_num)
			{
				tmp = edge_table->table[edge_d].d;
				next_index = edge_table->table[edge_d].pointers[last_s];
			}
			
			if (edge_table->table[edge_d].vice.timestamp<0)  // only count the valid edge
			{
				v2.push_back(tmp); // v2: 4822 / 6437&4822
			}
			edge_d = next_index;
		}

		vector<unsigned int> cn;
		count_join(v1, v2, cn);
		
		for(int i=0;i<cn.size();i++)
		{
			pos_s->local_count += op;
			pos_d->local_count += op;
			trcount += op;
			(node_table->ID_to_pos(cn[i]))->local_count += op;
		}
		
		// clear()可以清空所有元素
		// 即使clear()，vector所占用的内存空间依然如故，无法保证内存的回收。
		// swap()可以释放内存
		// swap()是交换函数，使vector离开其自身的作用域，从而强制释放vector所占的内存空间
		// 释放vector内存最简单的方法是 vector.swap(nums)
		v1.clear();
		vector<unsigned int>().swap(v1);
		v2.clear();
		vector<unsigned int>().swap(v2);
		cn.clear();
		vector<unsigned int>().swap(cn);
		return;
	}

	// 通过更新边表来连接图中的两个节点(两个sample_node类型的指针pos_s和pos_d)
	void link_list(sample_node* pos_s, sample_node* pos_d, int pos)
	{
		// 提取两个输入节点的节点ID，并将其存储在 s_num 和 d_num 中
		unsigned int s_num = pos_s->nodeID;
		unsigned int d_num = pos_d->nodeID;
		
		// pos: 8627
		edge_table->table[pos].set_last_s(pos_s->first_edge);
		edge_table->table[pos].set_last_d(pos_d->first_edge);

		// 检查源节点是否已经有边连接
		// -1 >= 0 / -1 >= 0
		if (pos_s->first_edge >= 0)
		{
			// 如果有，则检查源节点的第一条边是否具有与 s_num 相同的源节点ID
			if (edge_table->table[pos_s->first_edge].s == s_num)
				edge_table->table[pos_s->first_edge].set_next_s(pos); // 如果是，则将源节点的下一条边设置为 pos
			else
				edge_table->table[pos_s->first_edge].set_next_d(pos); // 否则，将目标节点的下一条边设置为 pos
		}

		// 检查目标节点是否已经有边连接
		// -1 >= 0 / 34841 >= 0
		if (pos_d->first_edge >= 0)
		{
			if (edge_table->table[pos_d->first_edge].s == d_num) // edge_table->table[pos_d->first_edge].s: 4822 d_num: 2
				edge_table->table[pos_d->first_edge].set_next_s(pos);
			else
				edge_table->table[pos_d->first_edge].set_next_d(pos); // edge_table->table[pos_d->first_edge].pointers[next_d(1)]: 8627 pos: 8627
		}

		// 将s节点和d节点都设置为第一条边，从而有效地连接这两个节点
		// pos_s:(nodeID:4822 local_count:0 vision_count:0 first_edge:34841 / 8627 next:0x0)
		// pos_d:(nodeID:2 local_count:0 vision_count:0 first_edge:34841 / 8627 next:0x0)
		pos_s->set_first_edge(pos);
		pos_d->set_first_edge(pos);			// set the cross list;

	}

	// 两个指向sample_node类型对象的指针，用于存储节点表
	// 一个整数 pos
	// 该函数用于通过更新 边表 和 节点表 来从图中移除一条边
	void dismiss(sample_node* pos_s, sample_node* pos_d, int pos)
	{
		// 提取连接边的两个节点的节点ID
		unsigned int old_s = pos_s->nodeID;
		unsigned int old_d = pos_d->nodeID;

		// 更新列表中前一个和后一个边的索引(指针)，将该边从边表中隔离出来
		int last_edge_s = edge_table->table[pos].pointers[last_s];  // isolate this edge from the list
		int last_edge_d = edge_table->table[pos].pointers[last_d];
		int next_edge_s = edge_table->table[pos].pointers[next_s];
		int next_edge_d = edge_table->table[pos].pointers[next_d];

		if (pos_s->first_edge == pos)
		{
			if (last_edge_s < 0) // there are no edges left for this node
			{
				node_table->delete_via_ID(pos_s->nodeID);
				node_count--;
			}
			else
				pos_s->first_edge = last_edge_s;
		}	
		
		if(!pos_d || pos_d->nodeID != old_d)
			pos_d = node_table->ID_to_pos(old_d); 
		
		if (pos_d->first_edge == pos)
		{
			if (last_edge_d < 0)
			{
				node_table->delete_via_ID(pos_d->nodeID);
				node_count--;
			}
			else
				pos_d->first_edge = last_edge_d;
		}

		if (last_edge_s>=0)
		{
			if (edge_table->table[last_edge_s].s == old_s)
				edge_table->table[last_edge_s].set_next_s(next_edge_s);
			else
				edge_table->table[last_edge_s].set_next_d(next_edge_s);
		}

		if (next_edge_s>=0)
		{
			if (edge_table->table[next_edge_s].s == old_s)
				edge_table->table[next_edge_s].set_last_s(last_edge_s);
			else
				edge_table->table[next_edge_s].set_last_d(last_edge_s);
		}

		if (last_edge_d>=0)
		{
			if (edge_table->table[last_edge_d].d == old_d)
				edge_table->table[last_edge_d].set_next_d(next_edge_d);
			else
				edge_table->table[last_edge_d].set_next_s(next_edge_d);
		}

		if (next_edge_d>=0)
		{
			if (edge_table->table[next_edge_d].d == old_d)
				edge_table->table[next_edge_d].set_last_d(last_edge_d);
			else
				edge_table->table[next_edge_d].set_last_s(last_edge_d);
		}
	}
	
	void insert(unsigned int s_num, unsigned int d_num, double p, long long time, int hashindex)
	{
		string s = my_to_string(s_num);
		string d = my_to_string(d_num);
		string edge = s + d;

		// edge.c_str(), edge.length(): 字符串转换为字符数组，长度
		//  (*hfunc[hashindex+1]): 哈希函数的下一个的值
		//  (*hfunc[hashindex+1])((unsigned char*)(edge.c_str()), edge.length())%size: 哈希函数的下一个的值对size取余
		//  pos: 哈希索引对应的位置

		// 在40000中找到存的位置，例 s:4822 d:2 edge:48222, pos: 34841 / 8627(第二条数据)
		// 即 40000 个子流中应该存放在哪一个子流当中
		// 针对第一条数据而言是存放在第34841个子流中
		unsigned int pos = (*hfunc[hashindex+1])((unsigned char*)(edge.c_str()), edge.length())%size;

		// 【采样边】的源节点和目标节点都为 0 (即无采样边(bucket为空 / 只有测试项))
		// sample_unit: edge_table->table[pos]
		// candidate_unit: edge_table->table[pos].vice
		if (edge_table->table[pos].s == 0 && edge_table->table[pos].d == 0) // no sample edge, then 2 cases: the bucket is empty, or only has test item;
		{
			// 用一个 新的边 替换一个 样本边
			edge_table->replace_sample(s_num, d_num, p, time, pos);
			// 在一个哈希表中查找与给定节点ID相匹配的节点，并返回其位置
			// 如果在哈希表中找不到匹配的节点，就返回NULL
			sample_node* pos_s = node_table->ID_to_pos(s_num);
			sample_node* pos_d = node_table->ID_to_pos(d_num);
			// 返回NULL 进入条件语句
			if (!pos_s)
			{
				pos_s = node_table->insert(s_num); // 插入点
				node_count++; // 点数+1
			}
			if (!pos_d)
			{
				 pos_d = node_table->insert(d_num); // 插入点
				 node_count++; // 点数+1
			}
			// node_count:2 +1(s:4822) +1(d:2)

			// pos_s:(nodeID:4822 local_count:0 vision_count:0 first_edge:-1 next:0x0)
			// pos_d:(nodeID:2 local_count:0 vision_count:0 first_edge:-1 next:0x0)
			// pos:34841 / 8627
			link_list(pos_s, pos_d, pos);
			
			// vice 是 过期但是没有双重过期的测试边
			// 如果vice边的时间戳小于 0 (bucket未使用，q_count初始化是 1)
			if (edge_table->table[pos].vice.timestamp < 0) // if this bucket is unused, the q_count is 1 initially;
			{
				// 39999 = 40000 -1
				q_count = q_count - 1; 
				edge_count++;
				valid_num++;
			}
			else if (p >= edge_table->table[pos].vice.priority) // if p is larger than the test priority, the test priority can be deleted now;
			{
				valid_num++;	
				q_count -= 1/pow(2, int(-(log(1 - edge_table->table[pos].vice.priority) / log(2)))+1); // it used to be the largest priority
				edge_table->delete_vice(pos);
			}
			// vice 边的时间戳 小于 0
			// -1 < 0
			if (edge_table->table[pos].vice.timestamp < 0)
			{
				// 修改三角形
				// pos_s:(nodeID:4822 local_count:0 vision_count:0 first_edge:34841 next:0x0)
				// pos_d:(nodeID:2 local_count:0 vision_count:0 first_edge:34841 next:0x0)
				modify_triangle(pos_s, pos_d, 1);
				q_count += 1/pow(2, int(-(log(1 - p) / log(2)))+1); // 39999->39999.5->39998.5->39999
			}

			return;
		}

		// if the inserted edge has already shown up and is sampled.
		if (edge_table->table[pos].s == s_num && edge_table->table[pos].d == d_num)
		{
			edge_table->update_sample(pos, time);
			return;
		}

		// else if the sampled edge is in last slice
		// if larger than the sampled p, replace it;
		if (p >= edge_table->table[pos].priority)
		{
			// replace the sample edge
			sample_node* old_s = node_table->ID_to_pos(edge_table->table[pos].s);
			sample_node* old_d = node_table->ID_to_pos(edge_table->table[pos].d);

			if (edge_table->table[pos].vice.timestamp < 0) // if there is no test item;
			{
				q_count = q_count-1/pow(2, int(-(log(1 - edge_table->table[pos].priority) / log(2)))+1) + 1/pow(2, int(-(log(1 - p) / log(2)))+1);
				
				modify_triangle(old_s, old_d, -1);
				dismiss(old_s, old_d, pos);
				
				edge_table->replace_sample(s_num, d_num, p, time, pos);
				
				sample_node* pos_s = node_table->ID_to_pos(s_num);
				sample_node* pos_d = node_table->ID_to_pos(d_num);
				
				if (!pos_s)
				{
					pos_s = node_table->insert(s_num);
					node_count++;
				}
				if (!pos_d)
				{
					pos_d = node_table->insert(d_num);
					node_count++;
				}
				
				link_list(pos_s, pos_d, pos);
				modify_triangle(pos_s, pos_d, 1);
			}
			else if (p >= edge_table->table[pos].vice.priority)
			{
				q_count = q_count - 1/pow(2, int(-(log(1 - edge_table->table[pos].vice.priority) / log(2)))+1) + 1/pow(2, int(-(log(1 - p) / log(2)))+1);
				valid_num++;

				dismiss(old_s, old_d, pos);
				
				edge_table->delete_vice(pos);
				edge_table->replace_sample(s_num, d_num, p, time, pos);
				
				sample_node* pos_s = node_table->ID_to_pos(s_num);
				sample_node* pos_d = node_table->ID_to_pos(d_num);
				
				if (!pos_s)
				{
					pos_s = node_table->insert(s_num);
					node_count++;
				}
				if (!pos_d)
				{
					pos_d = node_table->insert(d_num);
					node_count++;
				}
				
				link_list(pos_s, pos_d, pos);
				modify_triangle(pos_s, pos_d, 1);
			}
			else
			{
				dismiss(old_s, old_d, pos);
				
				edge_table->replace_sample(s_num, d_num, p, time, pos);
				
				sample_node* pos_s = node_table->ID_to_pos(s_num);
				sample_node* pos_d = node_table->ID_to_pos(d_num);
				
				if (!pos_s)
				{
					pos_s = node_table->insert(s_num);
					node_count++;
				}
				if (!pos_d)
				{
					pos_d = node_table->insert(d_num);
					node_count++;
				}
				
				link_list(pos_s, pos_d, pos);
			}
		}
		// if smaller than the sampled p nothing need to be done;
		return;
	 }
	
	// when the sampled edge expires, delete it and move the candidate edge one rank upper. 
	// Before this function the cross lists including this pos should be changed, （该位置的交叉列表应进行更改）
	// and after this function the new sampled edge (valid or not) should be added into the curresponding cross lists;（新采样的边（有效或无效）应添加到相应的交叉列表中）
	void update(long long ex_time, long long de_time)  
	{
		// ex_time: 当前数据的时间戳 - 窗口大小
		// de_time: 当前数据的时间戳 - 2 * 窗口大小
		
		// expiration 过期但没有双重过期的边 / 未过期的边
		// -1 34841 9270
		if (edge_table->expiration == -1)
			return;
		int tsl_pos = edge_table->expiration;
		int pos = tsl_pos % size; // 34841 % 40000 = 34841

		// 采样边(sample edge)的时间戳 小于 当前时间-窗口大小
		// 该边过期被纳入到测试边(test edge)中
		// 第 pos 个子流中边的时间戳 是否小于 当前时间 - 窗口大小
		while (edge_table->table[pos].timestamp < ex_time) // the expired edge mush be a sampled edge rather than a test edge
		{
			// 如果小于
			// 取出过期边的下一个边
			cout << "prev: " << edge_table->table[pos].time_list_prev << " " << ex_time << endl;
			cout << "next: " << edge_table->table[pos].time_list_next << " " << ex_time << endl;
		 	
			tsl_pos = edge_table->table[pos].time_list_next;
		 	
			sample_node* pos_s = node_table->ID_to_pos(edge_table->table[pos].s);
			sample_node* pos_d = node_table->ID_to_pos(edge_table->table[pos].d);

			// if the expired edge has a test item, then the item must be double expired. 
			// In this case the sampled edge is never valid, we do not need to delete triangle, 
			// this case is very rare, only when the stream has not been updated for a long time
			if (edge_table->table[pos].vice.timestamp >= 0) 
			{
				q_count -= 1/pow(2, int(-(log(1 - edge_table->table[pos].vice.priority) / log(2)))+1);
				q_count += 1/pow(2, int(-(log(1 - edge_table->table[pos].priority) / log(2)))+1);  
				edge_table->delete_vice(pos);
			}
			else // otherwise 
			{
				modify_triangle(pos_s, pos_d, -1);
				valid_num--;
			}

			dismiss(pos_s, pos_d, pos);

			edge_table->table[pos].vice.reset(edge_table->table[pos].s, edge_table->table[pos].d, edge_table->table[pos].priority, edge_table->table[pos].timestamp, edge_table->table[pos].time_list_prev, edge_table->table[pos].time_list_next);
			edge_table->set_time_list(edge_table->table[pos].time_list_prev, 1, pos+size);
			edge_table->set_time_list(edge_table->table[pos].time_list_next, 0, pos+size);
			
			if(edge_table->tsl_head == pos)
			 	edge_table->tsl_head = pos + size;
			if(edge_table->tsl_tail == pos)
			 	edge_table->tsl_tail = pos + size;	 

			edge_table->table[pos].reset();
			 
			edge_table->expiration = tsl_pos; // move expiration to next edge in the time sequence list;
			if(tsl_pos<0)
			 	break;
			else
				pos = tsl_pos % size;
		}
		 
		tsl_pos = edge_table->tsl_head;
		if(tsl_pos<0)
			return;
		pos = tsl_pos % size; // ? % 40000
		
		// 过期但没有双重过期的边(test edge)的时间戳 小于 当前时间-2*窗口大小
		// 该边双重过期
		while (edge_table->table[pos].vice.timestamp < de_time)
		{
			tsl_pos = edge_table->table[pos].vice.time_list_next; // the double expired edge mush be a test edge;
			
			q_count -= 1/pow(2, int(-(log(1 - edge_table->table[pos].vice.priority) / log(2)))+1);
			edge_table->delete_vice(pos); // the tsl_head and the list_unit is changed in this function

			if (edge_table->table[pos].timestamp >= 0)
			{
				q_count += 1/pow(2, int(-(log(1 - edge_table->table[pos].priority) / log(2)))+1);
				sample_node* pos_s = node_table->ID_to_pos(edge_table->table[pos].s);
				sample_node* pos_d = node_table->ID_to_pos(edge_table->table[pos].d);
				modify_triangle(pos_s, pos_d, 1);
				valid_num++;
			}
			else
			{
				q_count += 1; // if the substream has no new item since the test item expires, then the bucket is empty;
				edge_count--;
			}
			if(tsl_pos<0)
				break;
			else
				pos = tsl_pos % size;
		}
	}
	 
 };
 
 
