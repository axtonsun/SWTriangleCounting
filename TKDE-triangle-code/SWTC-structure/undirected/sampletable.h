#include<iostream>
#include<vector>
#include<assert.h>
#include<math.h>
using namespace std;

struct count_result
{
	int trcount;
	int illusion;
};

// 维护样本图并且是 sample 类的主要组成部分
class SampleTable
 {
 	public:
 	EdgeTable* edge_table; // common/edgetable.h
	NodeTable* node_table; // common/nodetable.h
 	int size;
	int edge_count;
	int node_count;
	int valid_num;
	int illusion_valid;
	int trcount;
	int illusion;
	double q_count;
	double illusion_q;

    // 接受一个整型参数s，然后根据s初始化类中的一些变量
    // 并且创建一个EdgeTable类型和一个NodeTable类型的对象
    // 4万~12万
 	SampleTable(int s)
 	{
 		size = s; //4万~12万
		edge_count = 0;
		node_count = 0;
		valid_num = 0;
		trcount = 0;
		illusion = 0;
		edge_table = new EdgeTable(s); // 4万~12万
		node_table = new NodeTable(4, 2*s);// 4 , 8万~24万
		q_count = size; // 4万~12万
		illusion_q = q_count; // 4万~12万
		illusion_valid = 0;

	 }

    ~SampleTable ()
	 {
		delete edge_table;
		delete node_table;
	 }

	void modify_triangle(sample_node* pos_s, sample_node* pos_d, long long last_mark, int op) // op = 1 means add, op = -1 means minus
	{
		vector<unsigned int> v1;
		vector<unsigned int> v2;
		unsigned int s_num = pos_s->nodeID;
		unsigned int d_num = pos_d->nodeID;
		int edge_s = pos_s->first_edge;
		int edge_d = pos_d->first_edge;
		while (edge_s >= 0)
		{
			unsigned int tmp;
			int next_index;
			if (edge_table->table[edge_s].s == s_num)
			{
				tmp = edge_table->table[edge_s].d;
				next_index = edge_table->table[edge_s].pointers[last_s];
			}
			else if (edge_table->table[edge_s].d == s_num)
			{
				tmp = edge_table->table[edge_s].s;
				next_index = edge_table->table[edge_s].pointers[last_d];
			}

			if (edge_table->table[edge_s].vice.timestamp<last_mark || edge_table->table[edge_s].vice.timestamp>edge_table->table[edge_s].timestamp)  // only count the valid edge
				v1.push_back(tmp);
			edge_s = next_index;
		}

		while (edge_d >= 0)
		{
			unsigned int tmp;
			int next_index;
			if (edge_table->table[edge_d].d == d_num)
			{
				tmp = edge_table->table[edge_d].s;
				next_index = edge_table->table[edge_d].pointers[last_d];
			}
			else if (edge_table->table[edge_d].s == d_num)
			{
				tmp = edge_table->table[edge_d].d;
				next_index = edge_table->table[edge_d].pointers[last_s];
			}

			if (edge_table->table[edge_d].vice.timestamp<last_mark || edge_table->table[edge_d].vice.timestamp>edge_table->table[edge_d].timestamp)  // only count the valid edge
				v2.push_back(tmp);
			edge_d = next_index;
		}
		vector<unsigned int> cn;
		count_join(v1, v2, cn);//
		for(int i=0;i<cn.size();i++)
		{
			int u = cn[i];
			(node_table->ID_to_pos(u))->local_count += op;
			pos_s->local_count += op;
			pos_d->local_count += op;
			trcount += op; //
		}
		cn.clear();
		vector<unsigned int>().swap(cn);
		v1.clear();
		vector<unsigned int>().swap(v1);
		v2.clear();	
		vector<unsigned int>().swap(v2);
	}

	void modify_illusion(sample_node* pos_s, sample_node* pos_d, long long landmark, int op)
	{
		vector<unsigned int> v1;
		vector<unsigned int> v2;
		unsigned int s_num = pos_s->nodeID;
		unsigned int d_num = pos_d->nodeID;
		int edge_s = pos_s->first_edge;
		int edge_d = pos_d->first_edge;
		while (edge_s >= 0)
		{
			unsigned int tmp;
			int next_index;
			if (edge_table->table[edge_s].s == s_num)
			{
				tmp = edge_table->table[edge_s].d;
				next_index = edge_table->table[edge_s].pointers[last_s];
			}
			else if (edge_table->table[edge_s].d == s_num)
			{
				tmp = edge_table->table[edge_s].s;
				next_index = edge_table->table[edge_s].pointers[last_d];
			}
			if(edge_table->table[edge_s].timestamp>=landmark) // only count edges in current slice in vision counting.
				v1.push_back(tmp);
			edge_s = next_index;
		}

		while (edge_d >= 0)
		{
			unsigned int tmp;
			int next_index;
			if (edge_table->table[edge_d].d == d_num)
			{
				tmp = edge_table->table[edge_d].s;
				next_index = edge_table->table[edge_d].pointers[last_d];
			}
			else if (edge_table->table[edge_d].s == d_num)
			{
				tmp = edge_table->table[edge_d].d;
				next_index = edge_table->table[edge_d].pointers[last_s];
			}
			if(edge_table->table[edge_d].timestamp>=landmark)	// only count edges in current slice in vision counting.
				v2.push_back(tmp);
			edge_d = next_index;
		}
		vector<unsigned int> cn;
		count_join(v1, v2, cn);
		for(int i=0;i<cn.size();i++)
		{
			int u = cn[i];
			(node_table->ID_to_pos(u))->vision_count += op;
			pos_s->vision_count += op;
			pos_d->vision_count += op;
			illusion += op;
		}
		cn.clear();
		v1.clear();
		v2.clear();	
	}
	// SWTC
    void modify_both(sample_node* pos_s, sample_node* pos_d, long long last_mark, long long land_mark, int op)
	{
		vector<unsigned int> v1;
		vector<unsigned int> v2;
		vector<unsigned int> vi1;
		vector<unsigned int> vi2;
		unsigned int s_num = pos_s->nodeID;
		unsigned int d_num = pos_d->nodeID;
		int edge_s = pos_s->first_edge; // 34841
		int edge_d = pos_d->first_edge; // 34841
		while (edge_s >= 0)
		{
			unsigned int tmp;
			int next_index;
			//cout << "the first edge is " <<s_num << endl;
			if (edge_table->table[edge_s].s == s_num) // 4088 == 4088
			{
				tmp = edge_table->table[edge_s].d; // 2
				next_index = edge_table->table[edge_s].pointers[last_s]; // -1
			}
			else if (edge_table->table[edge_s].d == s_num)
			{
				tmp = edge_table->table[edge_s].s;
				next_index = edge_table->table[edge_s].pointers[last_d];
			}
			else
			{
				cout << "wrong!!!" << endl;
			//	system("pause");
			}

			// 侯选边的时间戳 < last_mark or 侯选边的时间戳 > 采样边的时间戳
			if (edge_table->table[edge_s].vice.timestamp < last_mark || edge_table->table[edge_s].vice.timestamp > edge_table->table[edge_s].timestamp)  // only count the valid edge in triangle count
				v1.push_back(tmp);
			if(edge_table->table[edge_s].timestamp >= land_mark)
				vi1.push_back(tmp); // only count edges in current slice, as at next landmark only these edges will be active.
			edge_s = next_index; // -1                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  			edge_s = next_index;
		}

		while (edge_d >= 0) // 34841
		{
			unsigned int tmp; //2
			int next_index; // 0
			if (edge_table->table[edge_d].d == d_num)
			{
				tmp = edge_table->table[edge_d].s;
				next_index = edge_table->table[edge_d].pointers[last_d];
			}
			else if (edge_table->table[edge_d].s == d_num)
			{
				tmp = edge_table->table[edge_d].d;
				next_index = edge_table->table[edge_d].pointers[last_s];
			}

			if (edge_table->table[edge_d].vice.timestamp < last_mark || edge_table->table[edge_d].vice.timestamp > edge_table->table[edge_d].timestamp)  // only count the valid edge in triangle count
				v2.push_back(tmp);
			if(edge_table->table[edge_d].timestamp>=land_mark)
				vi2.push_back(tmp);
			edge_d = next_index; // -1
		}

		vector<unsigned int> cn;
		vector<unsigned int> vcn;

		count_join(v1, v2, cn);// v1:2 v2:4822
		count_join(vi1, vi2, vcn);// vi1:2 vi2:4822

		for(int i=0;i<cn.size();i++) // cn.size(): 0
		{
			int u = cn[i];
			(node_table->ID_to_pos(u))->local_count += op;
			pos_s->local_count += op;
			pos_d->local_count += op;
			trcount += op;
		}

        for(int i=0;i<vcn.size();i++) // vcn.size(): 0
		{
			int u = vcn[i];
			(node_table->ID_to_pos(u))->vision_count += op;
			pos_s->vision_count += op;
			pos_d->vision_count += op;
			illusion += op;
		}

		v1.clear();
		vector<unsigned int>().swap(v1);
		v2.clear();
		vector<unsigned int>().swap(v2);
		vi1.clear();
		vector<unsigned int>().swap(vi1);
		vi2.clear();
		vector<unsigned int>().swap(vi2);
		cn.clear();
		vector<unsigned int>().swap(cn);
		vcn.clear();
		vector<unsigned int>().swap(vcn);
		return;
	}

    void link_list(sample_node* pos_s, sample_node* pos_d, int pos, unsigned int s_num, unsigned int d_num)
	{

		edge_table->table[pos].set_last_s(pos_s->first_edge);
		edge_table->table[pos].set_last_d(pos_d->first_edge);

		if (pos_s->first_edge>=0)
		{
			if (edge_table->table[pos_s->first_edge].s == s_num)
				edge_table->table[pos_s->first_edge].set_next_s(pos);
			else
				edge_table->table[pos_s->first_edge].set_next_d(pos);
		}

		if (pos_d->first_edge>=0)
		{
			if (edge_table->table[pos_d->first_edge].s == d_num)
				edge_table->table[pos_d->first_edge].set_next_s(pos);
			else
				edge_table->table[pos_d->first_edge].set_next_d(pos);
		}
		pos_s->set_first_edge(pos);
		pos_d->set_first_edge(pos);			// set the cross list;

	}
	
    void dismiss(sample_node* pos_s, sample_node* pos_d, int pos)
	{
		unsigned int s = pos_s->nodeID;
		unsigned int d = pos_d->nodeID;
		
		int last_edge_s = edge_table->table[pos].pointers[last_s];  // isolate this edge from the list
		int last_edge_d = edge_table->table[pos].pointers[last_d];
		int next_edge_s = edge_table->table[pos].pointers[next_s];
		int next_edge_d = edge_table->table[pos].pointers[next_d];

		if (pos_s->first_edge == pos)
		{
			//cout<<"delete source node"<<endl;
			if (last_edge_s < 0) // there are no edges left for this node
			{
				pos_s = NULL;
				node_table->delete_via_ID(s);
				node_count--;
			}
			else
				pos_s->first_edge = last_edge_s;
		}
		if(!pos_d||pos_d->nodeID!=d)
			pos_d = node_table->ID_to_pos(d);
		
		if (pos_d->first_edge == pos)
		{
			//cout<<"delete destinition node"<<endl;
			if (last_edge_d < 0)
			{
				pos_d = NULL;
				node_table->delete_via_ID(d);
				node_count--;
			}
			else
				pos_d->first_edge = last_edge_d;
		}


		if (last_edge_s>=0)
		{
		//	if (last_edge_s == 14042)
		//		cout << "pause" << endl;
			if (edge_table->table[last_edge_s].s == s)
				edge_table->table[last_edge_s].set_next_s(next_edge_s);
			else
				edge_table->table[last_edge_s].set_next_d(next_edge_s);
		}

		if (next_edge_s>=0)
		{
			if (edge_table->table[next_edge_s].s == s)
				edge_table->table[next_edge_s].set_last_s(last_edge_s);
			else
				edge_table->table[next_edge_s].set_last_d(last_edge_s);
		}

		if (last_edge_d>=0)
		{
		//	if (last_edge_d == 14042)
		//		cout << "pause" << endl;
			if (edge_table->table[last_edge_d].d == d)
				edge_table->table[last_edge_d].set_next_d(next_edge_d);
			else
				edge_table->table[last_edge_d].set_next_s(next_edge_d);
		}

		if (next_edge_d>=0)
		{
			//if (pos == 14042)
		//		cout << "pause" << endl;
			if (edge_table->table[next_edge_d].d == d)
				edge_table->table[next_edge_d].set_last_d(last_edge_d);
			else
				edge_table->table[next_edge_d].set_last_s(last_edge_d);
		}
	}

	void dismiss(unsigned int s, unsigned int d, int pos)
	{

		int last_edge_s = edge_table->table[pos].pointers[last_s];  // isolate this edge from the list
		int last_edge_d = edge_table->table[pos].pointers[last_d];
		int next_edge_s = edge_table->table[pos].pointers[next_s];
		int next_edge_d = edge_table->table[pos].pointers[next_d];
		
		sample_node* pos_s = node_table->ID_to_pos(s);

		if (pos_s->first_edge == pos)
		{
			//cout<<"delete source node"<<endl;
			if (last_edge_s < 0) // there are no edges left for this node
			{
				pos_s = NULL;
				node_table->delete_via_ID(s);
				node_count--;
			}
			else
				pos_s->first_edge = last_edge_s;
		}

		sample_node* pos_d = node_table->ID_to_pos(d);
		if (pos_d->first_edge == pos)
		{
			//cout<<"delete destinition node"<<endl;
			if (last_edge_d < 0)
			{
				pos_d = NULL;
				node_table->delete_via_ID(d);
				node_count--;
			}
			else
				pos_d->first_edge = last_edge_d;
		}


		if (last_edge_s>=0)
		{
		//	if (last_edge_s == 14042)
		//		cout << "pause" << endl;
			if (edge_table->table[last_edge_s].s == s)
				edge_table->table[last_edge_s].set_next_s(next_edge_s);
			else
				edge_table->table[last_edge_s].set_next_d(next_edge_s);
		}

		if (next_edge_s>=0)
		{
			if (edge_table->table[next_edge_s].s == s)
				edge_table->table[next_edge_s].set_last_s(last_edge_s);
			else
				edge_table->table[next_edge_s].set_last_d(last_edge_s);
		}

		if (last_edge_d>=0)
		{
		//	if (last_edge_d == 14042)
		//		cout << "pause" << endl;
			if (edge_table->table[last_edge_d].d == d)
				edge_table->table[last_edge_d].set_next_d(next_edge_d);
			else
				edge_table->table[last_edge_d].set_next_s(next_edge_d);
		}

		if (next_edge_d>=0)
		{
			//if (pos == 14042)
		//		cout << "pause" << endl;
			if (edge_table->table[next_edge_d].d == d)
				edge_table->table[next_edge_d].set_last_d(last_edge_d);
			else
				edge_table->table[next_edge_d].set_last_s(last_edge_d);
		}
	}

	// 4822->2
	// p: 0.34361265638734362
	// time:0 land_mark:0 last_mark:0 hashindex: 0
    void insert(unsigned int s_num, unsigned int d_num, double p, long long time, long long land_mark,long long last_mark, int hashindex)
	 {
		 string s = my_to_string(s_num);
		 string d = my_to_string(d_num);
		 string edge = s + d;

		 // pos:34841 hashindex:0 edge:"48222" size:4万
		 // 3596994841 % 40000 求余得 34841
         unsigned int pos = (*hfunc[hashindex+1])((unsigned char*)(edge.c_str()), edge.length())%size;

		 // if the vice edge is elder than the last_mark, then it is a left test edge and need to be cleaned.
		 // 如果vice边比上一个标记要晚，那么它是一个左测试边，并且需要清理
		 // -1 < 0 && -1 >=0
		 if (edge_table->table[pos].vice.timestamp < last_mark && edge_table->table[pos].vice.timestamp >= 0)
			 edge_table->table[pos].vice.reset();

		 // 第pos位置的边中的 s 和 d 均为0，则进入条件
		 if (edge_table->table[pos].s == 0 && edge_table->table[pos].d == 0)
		 {
			 illusion_valid++;

			 // whatever the case, in the illusion a new edge is inserted.
			 // -(log(1 - p) / log(2))：先对1 - p取自然对数，然后除以log(2)，即转换为以2为底的对数，再取相反数。
			 // int(-(log(1 - p) / log(2)))：将上一步得到的结果转换为int类型，也就是向下取整。
			 // pow(2, int(-(log(1 - p) / log(2)))+1)：pow是一个求幂的函数，这里是求2的int(-(log(1 - p) / log(2))) + 1次方。
			 // 1/pow(2, int(-(log(1 - p) / log(2)))+1)：计算上一步得到的结果的倒数。
			 // illusion_q = illusion_q - 1 + 1/pow(2, int(-(log(1 - p) / log(2)))+1)：将原来的illusion_q减去1，然后加上上一步计算得到的结果，然后赋值给illusion_q。
			 // illusion_q = 由 40000 变成 39999.5
			 illusion_q = illusion_q - 1 + 1/pow(2, int(-(log(1 - p) / log(2)))+1);

			 sample_node* pos_s = node_table->ID_to_pos(s_num);
			 sample_node* pos_d = node_table->ID_to_pos(d_num);
			 // pos_s: NULL pos_d: NULL
             if (!pos_s)
			 {
				 pos_s = node_table->insert(s_num);// nodeID:4022 local_count:0 vision_count: 0 first_edge: -1 next: NULL
				 node_count++; // 1
			 }
			 if (!pos_d)
			 {
				 pos_d = node_table->insert(d_num);
				 node_count++; // 2
			 }					// if the node is not in the table ,insert it
             // edge_table->table[pos].vice.timestamp: -1 < 0
			 if (edge_table->table[pos].vice.timestamp >= 0)// there may be this case: the substream has not received a new item for a long time, and the old sample become test item, but no sample;
			 {
				 assert(edge_table->table[pos].vice.timestamp < land_mark && edge_table->table[pos].vice.timestamp >= last_mark);

				 if (p >= edge_table->table[pos].vice.priority)
				 {
					 valid_num++;
					 edge_table->replace_sample(s_num, d_num, p, time, pos);
					 link_list(pos_s, pos_d, pos, s_num, d_num);
					 modify_both(pos_s, pos_d, last_mark, land_mark, 1);
					 q_count = q_count - 1/pow(2, int(-(log(1 - edge_table->table[pos].vice.priority) / log(2)))+1) + 1/pow(2, int(-(log(1 - p) / log(2)))+1);
					 edge_table->table[pos].vice.reset();
				 }
				 else
				 {
					 edge_table->replace_sample(s_num, d_num, p, time, pos);
					 link_list(pos_s, pos_d, pos, s_num, d_num);
					 modify_illusion(pos_s, pos_d, land_mark, 1);
				 }

			 }
			 else{
				 edge_count++; // 1
				 valid_num++; // 1
				 edge_table->replace_sample(s_num, d_num, p, time, pos); // 4022 2 0.3436126 0 34841 if there is no sampled edge in this substream, this is the first one

                 link_list(pos_s, pos_d, pos, s_num, d_num);
				 modify_both(pos_s, pos_d, last_mark, land_mark, 1);

				 q_count = q_count - 1 + 1/pow(2, int(-(log(1 - p) / log(2)))+1); // q_count: 39999.5
				// cout <<p<<' '<< int((log(1 - p) / log(2)))<< endl;
			 }
			return;
		}

		 // if the inserted edge has already shown up and is sampled.
		 if (edge_table->table[pos].s == s_num && edge_table->table[pos].d == d_num)
		 {
		 	if(edge_table->table[pos].timestamp<land_mark){
		 	illusion_q = illusion_q- 1 + 1/pow(2, int(-(log(1 - edge_table->table[pos].priority) / log(2)))+1);
			 illusion_valid++;
			 sample_node* pos_s = node_table->ID_to_pos(s_num);
			 sample_node* pos_d = node_table->ID_to_pos(d_num);
			 modify_illusion(pos_s, pos_d, land_mark, 1);
			}
			 edge_table->update_sample(pos, time);
			 if (edge_table->table[pos].vice.priority < edge_table->table[pos].priority && edge_table->table[pos].vice.timestamp <= edge_table->table[pos].timestamp)
				 edge_table->delete_vice(pos);
			 return;
		 }

		 // else if the sampled edge is in last slice
		 if (edge_table->table[pos].timestamp<land_mark)
		{
			if (p >= edge_table->table[pos].priority)// if larger than the sampled p, replace it;
			{
				assert(edge_table->table[pos].vice.timestamp >= land_mark|| edge_table->table[pos].vice.timestamp <0);
				// in this case, the vice edge is not needed anyway;
				edge_table->delete_vice(pos);
				// reset the vice edge

				// replace the sample edge
				sample_node* old_s = node_table->ID_to_pos(edge_table->table[pos].s);
				sample_node* old_d = node_table->ID_to_pos(edge_table->table[pos].d);
				q_count += -1/pow(2, int(-(log(1 - edge_table->table[pos].priority) / log(2)))+1) + 1/pow(2, int(-(log(1 - p) / log(2)))+1);
				illusion_q += -1 + 1/pow(2, int(-(log(1 - p) / log(2)))+1);
				illusion_valid++;
				modify_triangle(old_s, old_d, last_mark, -1);
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
				}					// if the node is not in the table ,insert it
				link_list(pos_s, pos_d, pos, s_num, d_num);
				modify_both(pos_s, pos_d, last_mark, land_mark, 1);

			}
			else   // if smaller than the sampled p, check the vice edge;
			{
				if (p >= edge_table->table[pos].vice.priority) // can replace it;
				{
					edge_table->replace_vice(s_num, d_num, p, time, pos);
					//node_table->insert(s_num);
				//	node_table->insert(d_num);
				}
			}
		}
		 else  //the sample edge is larger than the landmark
		 {
				 if (p >= edge_table->table[pos].priority)// if larger than the sampled p, replace it;
				 {
					 sample_node* old_s = node_table->ID_to_pos(edge_table->table[pos].s);
					 sample_node* old_d = node_table->ID_to_pos(edge_table->table[pos].d);
					 // in this case, we need to check the vice edge,

					 if (edge_table->table[pos].vice.timestamp < land_mark &&edge_table->table[pos].vice.timestamp >= 0) // then this is a test edge
					  {

						  assert(edge_table->table[pos].vice.timestamp >= last_mark);
 						  if (edge_table->table[pos].vice.priority <= edge_table->table[pos].priority)
						  {
							  cout << "pause" << endl;
						  }
						 if (p >= edge_table->table[pos].vice.priority) // the new edge can replace the test edge
						 {
							 q_count = q_count - 1/pow(2, int(-(log(1 - edge_table->table[pos].vice.priority) / log(2)))+1) + 1/pow(2, int(-(log(1 - p) / log(2)))+1);
							 illusion_q = illusion_q - 1/pow(2, int(-(log(1 - edge_table->table[pos].priority) / log(2)))+1) + 1/pow(2, int(-(log(1 - p) / log(2)))+1);

							 edge_table->delete_vice(pos); // the test edge is not needed any more;
							 modify_illusion(old_s, old_d, land_mark, -1); // in this case the sampled edge is only an illusion

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
							 }					// if the node is not in the table ,insert it

							 link_list(pos_s, pos_d, pos, s_num, d_num);
							 modify_both(pos_s, pos_d, last_mark, land_mark, 1);
							 valid_num++;
						 }
						 else
						 {
							 illusion_q = illusion_q - 1/pow(2, int(-(log(1 - edge_table->table[pos].priority) / log(2)))+1) + 1/pow(2, int(-(log(1 - p) / log(2)))+1);

							 modify_illusion(old_s, old_d, land_mark, -1); // in this case the sampled edge is only an illusion
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
							 }					// if the node is not in the table ,insert it
							 link_list(pos_s, pos_d, pos,s_num, d_num);
							 modify_illusion(pos_s, pos_d, land_mark, 1); // the new edge is still an illusion

						 }
					 }
					 else //else there should be no vice edge, we replace the sampled edge.
					 {
						 if(edge_table->table[pos].vice.timestamp >= 0)
						  {
							cout<<edge_table->table[pos].timestamp<<' '<<time<<' '<<edge_table->table[pos].vice.timestamp<<' '<<land_mark<<' '<< last_mark<<endl;
							assert(false);
						  }
						 q_count = q_count - 1/pow(2, int(-(log(1 - edge_table->table[pos].priority) / log(2)))+1) + 1/pow(2, int(-(log(1 - p) / log(2)))+1);
						 illusion_q = illusion_q - 1/pow(2, int(-(log(1 - edge_table->table[pos].priority) / log(2)))+1) + 1/pow(2, int(-(log(1 - p) / log(2)))+1);

						 modify_both(old_s, old_d, last_mark, land_mark, -1);
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
						 }					// if the node is not in the table ,insert it
						 link_list(pos_s, pos_d, pos, s_num, d_num);
						 modify_both(pos_s, pos_d, last_mark, land_mark, 1);
					 }
					 // reset the vice edge

					 // replace the sample edge
					/* sample_node* old_s = node_table->ID_to_pos(edge_table->table[pos].s);
					 sample_node* old_d = node_table->ID_to_pos(edge_table->table[pos].d);
					 trcount -= count_triangle(old_s, old_d);
					 dismiss(old_s, old_d, pos);
					 edge_table->replace_sample(s_num, d_num, p, time, pos);
					 link_list(pos_s, pos_d, pos);*/
				}

			}

		//	if (time == 40597)
		//		cout << "pause 1" << endl;
		 return;
	 }

    // when the sampled edge expires, delete it and move the candidate edge one rank upper.
    // Before this function the cross lists including this pos should be changed,
    // and after this function the new sampled edge (valid or not) should be added into the curresponding cross lists;
    void update(long long time, long long land_mark, long long last_mark)
	{
		// time(这里的time是当前时间-滑动窗口大小): time - window-size
		int tsl_pos = edge_table->tsl_head;
		// -1 < 0 return
		if(tsl_pos < 0)
			return;
		int pos = tsl_pos % size;

		// 只要边表位置pos处的时间戳 小于 time(当前时间(T)-滑动窗口(W))，就一直循环
		while (edge_table->table[pos].timestamp < time)
		{
			tsl_pos = edge_table->table[pos].time_list_next;

			// if the vice edge is elder than the last_mark, then it is a left test edge and need to be cleaned.
			// 如果 vice边 比上一个标记要 晚，那么它是一个左测试边，并且需要清理
			if (edge_table->table[pos].vice.timestamp < last_mark && edge_table->table[pos].vice.timestamp >= 0)
				edge_table->table[pos].vice.reset();
			
			// vice edge 是 old->new 之间的边(case 2: 过期并作为测试(候选)边) / 当前切片中具有最高优先级的边
			if(edge_table->table[pos].vice.timestamp >= time)
			{
				illusion_q = illusion_q - 1 + 1/pow(2, int(-(log(1 - edge_table->table[pos].vice.priority) / log(2)))+1);

				sample_node* old_s = node_table->ID_to_pos(edge_table->table[pos].s);
				sample_node* old_d = node_table->ID_to_pos(edge_table->table[pos].d);
				modify_triangle(old_s, old_d, last_mark, -1); // 
				dismiss(old_s, old_d, pos);
				sample_unit tmp = edge_table->table[pos];
	
				edge_table->delete_sample(pos); // delete the expired sample;

				valid_num--;  // the valid num decreases, but the illusion_valid do not decrease, as after the land mark this bucket will still be valid;

				edge_table->table[pos].reset(tmp.vice.s, tmp.vice.d, tmp.vice.priority, tmp.vice.timestamp, tmp.vice.time_list_prev, tmp.vice.time_list_next); // the vice edge is an invalid sample now
				
				edge_table->set_time_list(tmp.vice.time_list_prev, 1, pos);
				edge_table->set_time_list(tmp.vice.time_list_next, 0, pos); // the pointer used to be pos + total_size (candidate unit), now updated to pos (sample unit); 
				if(edge_table->tsl_tail == pos + size)
				edge_table->tsl_tail = pos;
				if(edge_table->tsl_head == pos + size)
				edge_table->tsl_head = pos; 
				
				sample_node* pos_s = node_table->ID_to_pos(tmp.vice.s);
				sample_node* pos_d = node_table->ID_to_pos(tmp.vice.d);
				if (!pos_s)
				{
					pos_s = node_table->insert(tmp.vice.s);
					node_count++;
				}
				if (!pos_d)
				{
					pos_d = node_table->insert(tmp.vice.d);
					node_count++;
				}	// if the node is not in the table ,insert it

				link_list(pos_s, pos_d, pos, tmp.vice.s, tmp.vice.d); // link the cross list;
				modify_illusion(pos_s, pos_d, land_mark, 1); // the inserted is invalid, therefore only the illusion is increased;
				illusion_valid++;
			
				edge_table->table[pos].vice.reset(tmp.s, tmp.d, tmp.priority, tmp.timestamp);
			}
			else  // if there is no vice edge
			{
				sample_node* old_s = node_table->ID_to_pos(edge_table->table[pos].s);
				sample_node* old_d = node_table->ID_to_pos(edge_table->table[pos].d);
				modify_triangle(old_s, old_d, last_mark, -1); // 
				dismiss(old_s, old_d, pos);
				valid_num--;

				edge_table->table[pos].vice.reset(edge_table->table[pos].s, edge_table->table[pos].d, edge_table->table[pos].priority, edge_table->table[pos].timestamp);
				
				edge_table->delete_sample(pos);

			}

			if(tsl_pos<0)
				break;
			pos = tsl_pos % size;	
		}
	}

	void ective()
	 {
		 trcount = illusion;
		 valid_num = illusion_valid;
		 q_count = illusion_q;
		 edge_count = illusion_valid; 

		 illusion_q = size;
		 illusion_valid = 0;
		 illusion = 0;
		 
		 node_table->active();
	 }

 };
 
 
