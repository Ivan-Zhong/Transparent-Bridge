#include "fun.h"
bool check_addr(string port,string& port_smp,map<string,string>mac_tot) {
    /*
        if (port != "A" && port != "B" && port != "C" && port != "D" && port != "E" && port != "F")
                return false;
        else
                return true;
    */
    int ret=false;
    for(int i=0;i<='F'-'A';i++){
        string find="A";
        find[0]+=i;
        if(port==find||mac_tot[find]==port){
            port_smp=find;
            ret=true;
            break;
        }
    }
    return ret;
}
