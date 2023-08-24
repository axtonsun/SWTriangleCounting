#include<iostream>
#include<string>
#include<vector>
#include<map>

class NodeTable
{
public:
		sample_node* table; // 一个指向sample_node类型对象的指针，用于存储节点表
		int k;
		int length;
		int slice_length;
		// 构造函数，接收两个整数参数，并初始化类成员变量，同时创建新的sample_node数组并初始化
		NodeTable(int k_num, int l_num)
		{
			k = k_num;
			length = l_num;
			slice_length = length / k;
			table = new sample_node[length];
			for (int i = 0; i<length; i++)
				table[i].init(0);
		}
		// 删除所有在堆上动态分配的内存
		~NodeTable()
		{
			for (int i = 0; i<length; i++)
			{
				if (table[i].next)
				{
					sample_node* tmp = table[i].next;
					sample_node* next = tmp;
					while (tmp)
					{
						next = tmp->next;
						delete tmp;
						tmp = next;
					}
				}
			}
			delete[]table;
		}

		// 插入新节点到表中
		sample_node* insert(unsigned long node, int edge = -1);

		// 设置特定节点的边缘值
		sample_node* set_edge(unsigned long node, int edge);

		// 获取特定节点的边缘值
		int get_edge(unsigned long node);

		// 将给定ID转换为对应位置(如果存在)
		sample_node* ID_to_pos(unsigned long node);

		// 删除具有给定ID的元素(如果存在)
		void delete_via_ID(unsigned long node);

		// 将vision_count复制到local_count，并重置vision_count
		void active();

};


sample_node* NodeTable::insert(unsigned long node, int edge)
{
	int min_length = 0x7FFFFFFF;
	int min_pos = -1;
	int empty_pos = -1;

	string node_s = my_to_string(node);

	for (int i = 0; i<k; i++)
	{
		unsigned int pos = (*hfunc[i])((unsigned char*)(node_s.c_str()), node_s.length()) % slice_length;
		int address = i*slice_length + pos;


		if (table[address].nodeID == 0)
		{
			if (empty_pos<0)
				empty_pos = address;
		}

		if (table[address].nodeID == node)
		{
			if (edge>0)
				table[address].set_first_edge(edge);
			return &(table[address]);
		}
		else
		{
			int l = 0;
			sample_node* tmp = table[address].next;
			while (tmp)
			{
				l++;
				if (tmp->nodeID == node)
				{
					if (edge>0)
						tmp->set_first_edge(edge);
					return tmp;
				}
				tmp = tmp->next;
			}
			if (l<min_length)
			{
				min_length = l;
				min_pos = address;
			}
		}
	}

	if (empty_pos>=0)
	{
		table[empty_pos].nodeID = node;
		table[empty_pos].set_first_edge(edge);
		return &(table[empty_pos]);
	}
	sample_node* tmp = table[min_pos].next;
	if (!tmp)
	{
		tmp = new sample_node(node, edge);
		table[min_pos].next = tmp;
	}
	else
	{
		sample_node* last = tmp;
		while (tmp)
		{
			last = tmp;
			tmp = tmp->next;
		}
		tmp = new sample_node(node, edge);
		last->next = tmp;
	}
	return tmp;
}

sample_node* NodeTable::set_edge(unsigned long node, int edge)
{
	string node_s = my_to_string(node);
	for (int i = 0; i<k; i++)
	{
		unsigned int pos = (*hfunc[i])((unsigned char*)(node_s.c_str()), node_s.length()) % slice_length;
		int address = i*slice_length + pos;

		if (table[address].nodeID == node)
		{
			table[address].set_first_edge(edge);
			return &(table[address]);
		}

		else
		{
			sample_node* tmp = table[address].next;
			while (tmp)
			{
				if (tmp->nodeID == node)
				{
					tmp->set_first_edge(edge);
					return tmp;
				}
				tmp = tmp->next;
			}
		}
	}

	return NULL;
}

int NodeTable::get_edge(unsigned long node)
{
	string node_s = my_to_string(node);
	for (int i = 0; i<k; i++)
	{
		unsigned int pos = (*hfunc[i])((unsigned char*)(node_s.c_str()), node_s.length()) % slice_length;
		int address = i*slice_length + pos;

		if (table[address].nodeID == node)
			return table[address].first_edge;
		else
		{
			sample_node* tmp = table[address].next;
			while (tmp)
			{
				if (tmp->nodeID == node)
					return tmp->first_edge;
				tmp = tmp->next;
			}
		}
	}

	return -1;
}

sample_node* NodeTable::ID_to_pos(unsigned long node)
{
	string node_s = my_to_string(node);
	for (int i = 0; i<k; i++)
	{
		unsigned int pos = (*hfunc[i])((unsigned char*)(node_s.c_str()), node_s.length()) % slice_length;
		int address = i*slice_length + pos;
		if (table[address].nodeID == node)
			return &(table[address]);
		else
		{
			sample_node* tmp = table[address].next;
			while (tmp)
			{
				if (tmp->nodeID == node)
					return tmp;
				tmp = tmp->next;
			}
		}
	}
	return NULL;
}

void NodeTable::delete_via_ID(unsigned long node)
{
	string node_s = my_to_string(node);
	for (int i = 0; i<k; i++)
	{
		unsigned int pos = (*hfunc[i])((unsigned char*)(node_s.c_str()), node_s.length()) % slice_length;
		int address = i*slice_length + pos;

		if (table[address].nodeID == node)
		{
			if (table[address].next)
			{
				sample_node* cur = table[address].next;
				table[address].first_edge = cur->first_edge;
				table[address].next = cur->next;
				table[address].nodeID = cur->nodeID;
				delete cur;
			}
			else
				table[address].reset();
			return;
		}

		else
		{
			sample_node* tmp = table[address].next;
			sample_node* last = tmp;
			while (tmp)
			{
				if (tmp->nodeID == node)
				{
					if (last == tmp)
					{
						table[address].next = tmp->next;
						delete tmp;
					}
					else
					{
						last->next = tmp->next;
						delete tmp;
					}
					return;
				}
				last = tmp;
				tmp = tmp->next;
			}
		}
	}
	return;
}

void NodeTable::active()  // only used in SWTC with vision counting, and in local counting semantics
{
	for (int i = 0; i < length; i++)
	{
		table[i].local_count = table[i].vision_count;
		table[i].vision_count = 0;
		sample_node* tmp = table[i].next;
		while (tmp)
		{
			tmp->local_count = tmp->vision_count;
			tmp->vision_count = 0;
			tmp = tmp->next;
		}
	}
}
