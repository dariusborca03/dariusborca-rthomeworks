// Darius Borca: tranzitii, limite temporale, cereri repetate si overflow millis.
#include <cassert>
#include <cstdint>
#include <iostream>
const uint32_t CAR_GREEN_MIN_MS=10000, CAR_YELLOW_MS=3000, ALL_RED_MS=2000, PED_GREEN_MS=8000;
#include "../start/esp32_semafor/traffic_model.h"
int main() {
 TrafficState s{CAR_GREEN, 0, false};
 advance(s,50000); assert(s.phase==CAR_GREEN); // fara cereri, verde nelimitat
 s={CAR_GREEN,0,true}; advance(s,9999); assert(s.phase==CAR_GREEN);
 advance(s,10000); assert(s.phase==CAR_YELLOW && s.requested);
 advance(s,12999); assert(s.phase==CAR_YELLOW);
 advance(s,13000); assert(s.phase==ALL_RED);
 advance(s,14999); assert(s.phase==ALL_RED);
 advance(s,15000); assert(s.phase==PED_GREEN && !s.requested);
 assert(seconds_left(s,15000)==8); assert(seconds_left(s,15800)==8);
 assert(seconds_left(s,22700)==1); assert(seconds_left(s,23000)==0);
 s.requested=true; // apasare in timpul traversarii: urmatorul ciclu
 advance(s,23000); assert(s.phase==CAR_GREEN && s.requested);
 advance(s,32999); assert(s.phase==CAR_GREEN);
 advance(s,33000); assert(s.phase==CAR_YELLOW);
 s={CAR_GREEN,UINT32_MAX-4999,true}; advance(s,4999); assert(s.phase==CAR_GREEN);
 advance(s,5000); assert(s.phase==CAR_YELLOW);
 // Simulare 2 minute: fara sarit peste galben si fara verde simultan.
 s={CAR_GREEN,0,true}; Phase old=s.phase;
 for(uint32_t t=0;t<120000;t+=100) {
    if(t%17000==0) s.requested=true;
    advance(s,t);
    assert(!(old==CAR_GREEN && (s.phase==ALL_RED || s.phase==PED_GREEN)));
    bool carGreen=s.phase==CAR_GREEN, pedGreen=s.phase==PED_GREEN;
    assert(!(carGreen && pedGreen)); old=s.phase;
 }
 std::cout<<"Traffic model: all assertions passed\n";
}
