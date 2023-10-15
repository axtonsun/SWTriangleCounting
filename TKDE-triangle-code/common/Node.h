#include<iostream>
#include<sstream>
#include<string> 
#define next_s 0
#define next_d 1
#define last_s 2
#define last_d 3
using namespace std;

// 辅助函数：接收一个无符号整数参数a，将将其转换为字符串并返回
string my_to_string(unsigned int a)
{
   	string str="";
	while(a>0)
	{
		str += (char)('0'+a%10);
		a=a/10;
	}
	string astr;
	astr.resize(str.length());
	for(int i=str.length()-1;i>=0;i--)
		astr[str.length()-1-i] = str[i]; 
    return astr ;
}

class sample_node
{
	public:
	unsigned int nodeID; // 节点ID(无符号整型)
	unsigned int local_count;  // not needed in global counting;
	unsigned int vision_count; // not needed in asy_sample and BPS sample
	
	int first_edge;
	
	sample_node* next;
	
	// 无参 构造函数
	sample_node()
	{
		nodeID = 0;
		local_count = 0;
		vision_count = 0;
		next = NULL;
		first_edge = -1;
	} 

    // 两个参数 构造函数
	sample_node(unsigned int s, int edge = -1)
	{
		nodeID = s;
		next = NULL;
		first_edge = edge;
		local_count = 0;
		vision_count = 0;
	 } 

	 // 初始化成员变量
     // s: 0
	 void init(unsigned int s, int edge = -1)
	 {
	 	nodeID = s;
		next = NULL;
		first_edge = edge;
		local_count = 0;
		vision_count = 0;
	 }

     // 设置第一条边
	 void set_first_edge(int s)
	 {
	 	first_edge = s;
	 }

     // 重置成员变量
	 void reset()
	 {
	 	nodeID = 0;
	 	first_edge = -1;
		local_count = 0;
		vision_count = 0;
	 }
};

// class for the candidate edge or test edge in each substream, used in both BPS and SWTC. 
// In BPS, it is the expired but not double-expired test edge. 
// In SWTC, it is the the not chosen edge in the 2 slices. 
// To be precise, in one case, it is the edge with largest priority in the last slice, but expires and servers as a test edge (case 2 in the paper).
// In the other case, it it the edge with largest priority in current slice, but not chosen (case 1 in the paper).
class candidate_unit
{
	public:
	unsigned int s, d; // nodes of the edge
	double priority; // can be computed according to s and d. saved only for convinience
	long long timestamp;
	int time_list_prev; // 在时间列表中前一个元素位置指针 suppose the size of the sample table is m. 
	// A pointer in value range 0 ~ m-1 means sample unit in the corresponding table pos. 
	// A pointer in value range m ~ 2m-1 means candidate unit in the corresponding table pos. -1 means an empty pointer.
	int time_list_next; // 在时间列表中后一个元素位置指针

	candidate_unit(unsigned int snum = 0, unsigned int dnum = 0, double p = -1, long long time = -1, int prev = -1, int next = 1)
	{
		s = snum;
		d = dnum;
		priority = p;
		timestamp = time;
		time_list_prev = prev;
		time_list_next = next;
	}

	void reset(unsigned int snum = 0, unsigned int dnum = 0, double p = -1, long long time = -1, int prev = -1, int next = -1)
	{
		s = snum;
		d = dnum;
		priority = p;
		timestamp = time;
		time_list_prev = prev;
		time_list_next = next;
	}
};
 
class sample_unit  // class for the sampled edge in each substream, used both in SWTC and BPS
{
	public:
	unsigned int s, d;
	double priority;
	long long timestamp;    
	int pointers[4]; // cross lists for the graph structure;
	int time_list_prev; // same as the pointers in candidate unit;
	int time_list_next;
	candidate_unit vice;

    // edgetable.h 中 EdgeTable 构造函数调用
    // snum =0
	sample_unit(unsigned int snum = 0, unsigned int dnum = 0, double p = -1, long long time = -1, int prev = -1, int next = -1)
	{
		s = snum;
		d = dnum;
		for(int i=0;i<4;i++)
			pointers[i] = -1;
		priority = p;
		timestamp = time;
		time_list_prev = prev;
		time_list_next = next;
		vice.reset();
	}

    void set_next_s(int s){pointers[next_s] = s;}
	void set_next_d(int d){pointers[next_d] = d;}
	void set_last_s(int s){pointers[last_s] = s;}
	void set_last_d(int d){pointers[last_d] = d;}

    // s_num: 4822 d_num: 2 p: 0.34361265638734362 time: 0 prev: -1 next:-1
    // s_num: 6437 d_num: 2 p: 0.045699954300045698 time: 10701850 prev: 34841 next:-1
	void reset(unsigned int snum = 0, unsigned int dnum = 0, double p = -1, long long time = -1, int prev = -1, int next = -1)
	{
		s = snum;
		d = dnum;
		for(int i=0;i<4;i++)
			pointers[i] = -1;
		priority = p; // priority: 0.34361265638734362
		timestamp = time; // 0
		time_list_prev = prev; // -1
		time_list_next = next; // -1
	}
}; 
