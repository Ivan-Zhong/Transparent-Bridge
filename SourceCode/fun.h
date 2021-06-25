#ifndef FUN_H
#define FUN_H
#include <string>
#include <map>
using namespace std;
// 如果合法 则返回单字母版mac地址到port_smp
// 否则返回false
bool check_addr(string port,string& port_smp,map<string,string>mac_tot);
#endif // FUN_H
