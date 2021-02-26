#include <set>
#include <vector>
#include <iostream>
#include <algorithm>
#include <map>
#include <memory.h>
#include <fstream>
#include <sstream>
#include<typeinfo>

using namespace std;

/*
bucket结构，包含：
index:由g(x)确定，一个bucket有独立的index（0 <= index < r）
size:当前bucket中item的个数，
	即原始x中，有size个x的g(x)都相同，因此被放入同一个bucket
keys:size个原始x的值*/
typedef struct bucket{
	//第i个bucket的index为
	uint64_t index; 
	int size;
	//keys中所有元素的g(key)结果相同，均为index
	vector<uint64_t> keys;
}bucket;

/*
g()逻辑：生成bucket的index
// jenkins hash g(x)逻辑*/
uint64_t g(uint64_t key) {
    key = (~key) + (key << 21); // key = (key << 21) - key - 1;
    key = key ^ (key >> 24);
    key = (key + (key << 3)) + (key << 8); // key * 265
    key = key ^ (key >> 14);
    key = (key + (key << 2)) + (key << 4); // key * 21
    key = key ^ (key >> 28);
    key = key + (key << 31);
    return key;
}

/*
f1()逻辑*/
uint64_t f1(uint64_t key) {
	return key % 4;
}

/*
f2()逻辑*/
uint64_t f2(uint64_t key) {
	return key % 3;
}

/*insert_bucket
将当前y=g(x)插入bucket vector中*/
bool insert_bucket(vector<bucket> *B, uint64_t g_x, uint64_t input_key) {
	int B_size = B->size();
    for (int i = 0; i < B_size; i++)
    {
		bucket tmp_keys = B->at(i);
		//若当前bucket vector中已有当前y的bucket，更新其属性
		if (tmp_keys.index == g_x) {
			tmp_keys.size += 1;
			tmp_keys.keys.push_back(input_key);
			B->at(i) = tmp_keys;
			return false;
		}
    }
	//若B长度为0，或B中没有当前g(x)的bucket，添加一个新的bucket
	vector<uint64_t> key_vector_tmp = {input_key};
	bucket tmp = {g_x, 1, key_vector_tmp};
	B->push_back(tmp);
	return true;
}

/*b_compare
对bucket，按size排序*/
bool b_compare(const bucket& b1, const bucket& b2)
{
	return (b1.size > b2.size);
}

/*set_function
生成（设置）完美哈希函数y = phf(x) = (f1(key) + d0 * f2(key) + d1) % m;
对每个bucket，将其对应的完美哈希函数的参数d0, d1保存至f_map中*/
bool set_function(uint64_t *input_k, int n, int m, map<uint64_t, uint64_t> &f_map) {
	//第一步，定义bucket vector，将哈希函数g()用于input_key，结果保存在bucket vector B 中
	vector<bucket> *B;
	vector<bucket> Bucket;
	B = &Bucket;
	cout << "original keys are:" << endl;
	for (int i=0; i < n; i++) {
		cout << input_k[i] << " ";
	}
	cout << endl << "(keys , bucket indices) are:" << endl;
	for (int i=0; i < n; i++) {
		uint64_t g_x = g(input_k[i]);
		//将当前g(x),x的值插入bucket vector
		bool new_bucket = insert_bucket(B, g_x, input_k[i]);
		cout << "(" << int(input_k[i]) << ",";
		cout << int(g_x) << ") ";
	}
	int r = B->size();
	cout << endl << "Total number of buckets is: " << r << endl;
	cout << "-----------------------------" << endl;
	//对bucket vector按每个bucket的size由高到低排序
	sort(B->begin(), B->end(), b_compare);
	
	//构建 T[m]，辅助算法判断有哪些x已经生成对应的hash函数值y(哪些bucket已经处理好）
	int T[m] = {0};
	//对每个bucket进行hash函数的确定
	for (int i=0; i < r; i++) {
		cout << "Bucket " << i << ":" <<endl;
		//找到当前bucket，并对当前生成专有的hash func参数(d0, d1)
		bucket b = B->at(i);
		int size = b.size;
		vector<uint64_t> keys = b.keys;
		int d0, d1;
		for (d0=0; d0 < m; d0++) {
			for (d1=0; d1 < m; d1++) {
				//y_set确保当前bucket中，每个x的hash值y不冲突
				set<uint64_t> y_set = {};
				int T_tmp[m] = {0};
				bool flag = true;
				for (int j=0; j < size; j++) {
					uint64_t y = (f1(keys[j]) + d0 * f2(keys[j]) + d1) % m;
					/*  debug
					cout << "============================" << endl;
					cout << "cur original key is:" << keys[j] << endl;
					cout << "cur g(x) key is:" << int(b.index) << endl;
					cout << "cur d0 is:" << d0 << endl;
					cout << "cur d1 is:" << d1 << endl;
					cout << "cur y is:   " << y << endl;
					cout << "T is:  ";
					for (int u=0; u < sizeof(T)/sizeof(int);u++) {
						cout << T[u] << "  ";
					}
					cout << endl;
					cout << "y_set ok? " << boolalpha << !(y_set.find(y)!=y_set.end()) << endl;
					cout << "T[y] is: " << T[y] << endl; */
					if (T[y] == 1 || y_set.find(y)!=y_set.end()) {
						//运行至这里说明当前的组合hash函数，得到的y有重复
						flag = false;
						break;
					} else {
						y_set.insert(y);
						T_tmp[y] = 1;
					}
				}
				// cout << "flag is:" << boolalpha << flag << endl;
				// cout << "================================================" << endl;
				if (flag) {
					/*flag为true说明
					1,这个d0,d1组合可以确保当前bucket中，每个x的hash值y不冲突
					2,所有y都不在当前的T[]中。
					将临时的y值(共size个)导入最终的T数组中,本bucket结束*/
					for (int j=0; j < m; j++) {
						if (T_tmp[j] == 1) {
							T[j] = 1; //更新T
						}
					}
					goto bucket_success;
				} // else {continue;}
			}
		}
		cout << "PHF fails for Current bucket " << int(b.index) << endl;
		throw "hash function g, f1, f2 don't work! Please change those functions.";
		bucket_success:
		/*
		将当前bucket生成的h(x)所用的(d0,d1)，和对应的g(x)进行配对记录：
		用数组存储：key为g(x)的值，value为(d0 << 32) + d1，便于存储 （或构建查找表，空间换时间）
		查找时，先求g(x)，再查得d0,d1
		最后通过 y = (f1(x) + d0 * f2(x) + d1) % m 查找最终的hash函数值(1到m)
		*/
		cout << "success, d0: " << d0 << ", d1: " << d1 << endl;
		/* debug
		cout << "T is:  ";
		for (int u=0; u < sizeof(T)/sizeof(int);u++) {
			cout << T[u] << "  ";
		}
		cout << endl; */
		//添加当前bucket的哈希函数参数d0, d1
		const uint64_t insertValue = (uint64_t(d0) << 32) + uint64_t(d1);
		const uint64_t ind = b.index;
		//为每一个bucket构建一个配对记录
		f_map.insert(map<uint64_t, uint64_t>::value_type (ind, insertValue));
	}
	return true;
}

/*
查找时，先求g(key)，再查得d0,d1
最后计算phf(key) = ( f1(key) + d0 * f2(key) + d1 ) % m*/
int lookup(uint64_t key, map<uint64_t, uint64_t> &f_map, int m) {
	cout << "Key: " << key << " => " ;
	uint64_t j1 = g(key); //将g(x)值cast为uint64_t类型，用于查找d0及d1
	uint64_t func = (f_map.at(j1));
	// cout << "g(x) is:" << j1 << endl;
	uint64_t d0 = func >> 32;
	uint64_t d1 = (func << 32) >> 32;
	// cout << "hash function d0 is:" << int(d0) << endl;
	// cout << "hash function d1 is:" << int(d1) << endl;
	uint64_t result = (f1(key) + d0 * f2(key) + d1) % m;
	cout << result;
	cout << "	d0: "<< int(d0) << "	d1: " << int(d1) << endl;
	return result;
}

// test chd algorithm
int main () {
	// import from input file
	int datalen = 0;
	uint64_t input_keys[4000]; //这里保证数组个数与输入文件行数相等
	ifstream input("./input_file/input.txt", ios::in);
	string line;
	while (getline(input, line)) {
		stringstream ss;
		ss << line;
		if (!ss.eof()) {
			ss >> input_keys[datalen++];
		}
	}
	// int input_keys[] = {6,9,2,3,4,5};
	// uint64_t input_keys[] = {24,12,2,3,4,5,6,7,8,9,13,14,15,16,19};
	// uint64_t input_keys[] = {24,12,2,3,4,5,6,7,8,9,13,14,15,16,19,375,321,532,842,25,68,71,29};
	int n = sizeof(input_keys) / sizeof(input_keys[0]);
	int m = n;
	cout << "number of keys: m = n = " << n << endl;
	uint64_t input_k[n];
	memset(input_k,0,n*sizeof(uint64_t));
	memcpy(input_k,input_keys,n*sizeof(uint64_t));

	// copy(begin(input_keys),end(input_keys),begin(input_k));
	//构建map func_map，存g(x)与d0,d1的配对记录 pair(g(x), d0<<32 + d1)
	map<uint64_t, uint64_t> function_map;
	map<uint64_t, uint64_t> &func_map = function_map;
	set_function(input_keys, n, m, func_map);
	uint64_t searchKey;
	/* 单个key查询
	cout << "enter a key to search: ";
	cin >> searchKey;
	cout << "Key is: " << searchKey << endl;
	bool keyValid = false;
	for(int i=0; i<n;i++) {
		if(input_keys[i] == searchKey) {
			keyValid = true;
			break;
		}
	}
	if (!keyValid) {
		throw "Please enter a valid search key.";
	}
	int value = lookup(searchKey, func_map, m);
	cout << "The hashed value is: " << value << endl;
	*/
	// 对minimum完美哈希函数的结果进行冲突检查
	set<uint64_t> value_set = {};

	cout << "set function complete-----------------------------" << endl << endl;
	cout << "phf(key) = ( f1(key) + d0 * f2(key) + d1 ) % m" << endl;
	cout << "m = n = " << m << endl << endl;
	for (int u=0; u<n; u++) {
		searchKey = input_k[u];
		int value = lookup(searchKey, func_map, m);
		if (value_set.find(value)!=value_set.end()) {
			// 说明哈希结果有冲突存在，失败
			throw "Error, Hash function has conflict!";
		} else if (value >= m || value < 0) {
			// 说明哈希结果存在大于m或小于0的值，不“完美”
			throw "Error, Hash function has conflict!";
		}
		value_set.insert(value);
	}
	cout << "Perfect Hash Function generated successfully." << endl;
	return 0;
}

