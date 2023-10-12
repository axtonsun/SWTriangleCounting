#include<iostream>
#include<fstream>
#include<vector>
#include<time.h>
#define triangle_type 1	// triangle type is defined for directed counting, 0 means type A, 1 means type B. Details of the types can be found in our paper.
#include "../SWTC-structure/undirected/sample.h"	// undirected, binary counting, include different files for different semantics.
#include "../SWTC-structure/undirected/asy_sample.h"
#include "../BPS-structure/undirected/BPS-sample.h"
#include "../Golden-triangle/undirected/GoldenCounter.h"
using namespace std;

int main()
{
	 	unsigned int s, d, w, w_;
        long long t;
        string t1, t2, p;

        double time_unit = 18.545;	// the unit of the window length, we use average time span as unit, namely (maximum timestamp - minimum timestamp)/number_of_edges.
        // 时间单位定义为平均时间跨度 time_unit = (最大时间戳 - 最小时间戳)/边的数量
        // Stackoverflow: (1457273428 - 1217567877) / 63497050 = 3.775
        // WikiTalk: (1449042928 - 985765204) / 24981163 = 18.545
        // Actor: (2521428675 - 1693552096) / 33115812 = 24.999

        int gap = 2000000; // the length of the sliding window

        for(int hindex =0;hindex<5; hindex++)	// use different hash functions to carry out multiple groups of 
        {//使用不同哈希函数执行多组操作 hindex从0到4
            // 对于每个子流数目，它使用不同的哈希函数进行五次实验

        	for(int sample_size = 40000;sample_size<=120000;sample_size+=20000)
        	{ // 样本大小从 4万 增加到 12万，2万的间隔大小
        	long wsize = gap*time_unit; // wsize = 2百万*平均时间跨度 Wiki:3709万 Stack: 755万
            long count = 0;

            ifstream fin("../data/WikiTalk.txt"); //读取数据集
            string index = "../result/undirect/Wiki/EstimateResult_wiki_undirect_2M_h";//输出结果
            // EstimateResult_wiki_undirect_2M_h?_xx
            // ？是哈希索引，从0变化到4
            // xx是 (子流数量)/2万
            index += char(hindex+'0');
            index += '_';
            index += my_to_string(sample_size/20000);
            ofstream fout(index.c_str());

            // sample_size: 4万~12万 wsize: 时间型滑动窗口 3709万 / 755万 hindex: 0~4
            // asy_sample* ac = new asy_sample(sample_size, wsize,10, hindex);
            BPSsample* bc = new BPSsample(sample_size, wsize, hindex);
            sample* sc = new sample(sample_size, wsize, hindex);
            GoldenCounter* gc = new GoldenCounter(wsize);

            long long t0 = -1;
            long long tmp_point = 0;
            // 在每次实验中，当滑动窗口其长度的 1/10 时(前两个窗口没有checkpoint)，它会设定一个checkpoint
            // 并在每个 checkpoint 计算 SWTC、Baseline、Golden 的三角形估计
            int checkpoint = wsize/10;
            int num = 0;

            // 从输入流中读取数据到变量s,d,t
            while(fin>>s>>d>>t)
            {
                // t0 初始化为 -1，如果t0小于0，t0=t；然后两顶点值各自+1
                // t0 就是数据集的第一个数据的时间戳
                if(t0<0) t0 = t;
                s = s + 1;
                d = d + 1;
                // 两顶点数值不同 则 count=0
                // 否则 继续循环
                // t - t0 就是得出当前t时刻的数据距离第一个数据相差多少时间，也就是两个时间戳进行相减
                if (s != d)     count = t - t0;	// use the timestamp as the time unit
                else continue;
                num++;

//            	ac->proceed(s, d, count);
                bc->proceed(s, d, count);
                sc->proceed(s, d, count);
                gc->insert_edge(s,d,count);
                
                // 当时间戳相差大于等于两倍的时间滑动窗口时并且
                if (count >= 2*wsize && int(count/checkpoint) > tmp_point)	// whenever the window
                {
                    srand((int)time(0));
                    // tmp_point 从20开始递增+1
                    // 时间戳相差/checkpoint
                    tmp_point = count/checkpoint; // 35574条边 tmp_point: 20 count:74180246 checkpoint: 3709000
//                    ac->prepare();
                    bc->prepare();
                    sc->prepare();
                    // 写入数据
                    // 第1个参数: 有效数量
                    // 第2个参数: 边估计值(为估算出的滑动窗口内互异边数量 n)
                    // 第3个参数: 样本图中的全局三角形个数
                    // 第4个参数: 流图中的全局三角形个数
//                    fout<<"asyn triangle "<< ac->valid_count() <<' '<<ac->edge_estimate<<' '<<ac->ss->trcount<<' '<<ac->count()<<endl;
                    fout<<"BPS triangle "<<bc->st->valid_num<<' '<<bc->edge_estimate<<' '<<bc->st->trcount<<' '<<bc->count()<<endl;
                    fout<<"SWTC triangle "<<sc->st->valid_num<<' '<<sc->edge_estimate<<' '<<sc->st->trcount<<' '<<sc->count()<<endl;
                    fout<<"Golden triangle "<<gc->duplicate_count()<<' '<<gc->edge_count()<<' '<<gc->triangle_count()<<endl;
                    fout<<endl;
                    //
                    cout<<sample_size<<" check point " << tmp_point << endl;
                }
            }
            fin.close();
            fout.close();

//        	delete ac;
            delete sc;
            delete bc;
            delete gc;
        	}
        }
        return 0;
}
                                                                      
