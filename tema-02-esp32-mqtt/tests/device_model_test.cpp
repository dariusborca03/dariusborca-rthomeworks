// Darius Borca - teste ale parserului folosit efectiv de placa.
#include <cassert>
#include <iostream>
const uint32_t PERIOD_MS_INITIAL=2000;
const float THRESHOLD_INITIAL=45;
#include "../start/esp32_device/device_model.h"
int main() {
 uint32_t period=PERIOD_MS_INITIAL; float threshold=THRESHOLD_INITIAL; char reply[128];
 const char* invalid[]={"", "UNKNOWN", "SET_INTERVAL", "SET_INTERVAL 0", "SET_INTERVAL -1", "SET_INTERVAL 49", "SET_INTERVAL 3600001", "SET_INTERVAL 1e3", "SET_INTERVAL 999999999999999999999999999", "SET_INTERVAL 100 extra", "SET_THRESHOLD NaN", "SET_THRESHOLD inf", "SET_THRESHOLD 1x", "SET_THRESHOLD 1e100", "RESET x", "STATUS x"};
 for(auto text:invalid) {
    execute_command(text,period,threshold,reply,sizeof(reply));
    assert(strncmp(reply,"ERROR",5)==0); assert(period==2000 && threshold==45);
 }
 execute_command("SET_INTERVAL 50",period,threshold,reply,sizeof(reply)); assert(period==50 && strncmp(reply,"OK",2)==0);
 execute_command("SET_INTERVAL 3600000",period,threshold,reply,sizeof(reply)); assert(period==3600000);
 execute_command("SET_THRESHOLD -2.5",period,threshold,reply,sizeof(reply)); assert(threshold==-2.5f);
 execute_command("STATUS",period,threshold,reply,sizeof(reply)); assert(strstr(reply,"interval=3600000") && strstr(reply,"threshold=-2.50"));
 execute_command("RESET",period,threshold,reply,sizeof(reply)); assert(period==2000 && threshold==45 && strcmp(reply,"OK RESET")==0);
 std::cout<<"Device command model: all assertions passed\n";
}
