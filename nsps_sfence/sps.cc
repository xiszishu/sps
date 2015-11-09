#include <stdlib.h>
#include <stdio.h>
#include <string.h> 
#include <sys/time.h>
#include <time.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <stdint.h>
#include <string>

#include "defines.h"

using namespace std;

typedef pair <int, string> Int_Pair;
double timer_begin,timer_end,sum;
double GetWallTime(void)
{
    struct timeval tp;
    static long start=0, startu;
    if (!start)
        {
            gettimeofday(&tp, NULL);
            start = tp.tv_sec;
            startu = tp.tv_usec;
            return(0.0);
        }
    gettimeofday(&tp, NULL);
    return( ((double) (tp.tv_sec - start)) + (tp.tv_usec-startu)/1000000.0 );
}
static inline void asm_clflush(volatile intptr_t *addr)
{
    __asm__ __volatile__ ("clflush %0 "::"m"(*addr));
}
static inline void asm_sfence(void)
{
    __asm__ __volatile__ ("sfence");
}
struct a1
{
    int src;
    string b[3];
};
typedef struct a1 pmem;

int build_array(vector<pmem>& a, int n)
{
    int i,j;
    char ch[256]={};
    srand(time(NULL));
    for (i = 0; i < n; i++)
    {
        for (j=0;j<255;j++)
            ch[j]='a'+rand()%26;
        a[i].src=0;
        a[i].b[0]=ch;
        a[i].b[2]=a[i].b[0];
        //cout<<a[i].b[0]<<endl;
    }
    return 0;
}


void array_swap(vector<pmem>& a, int n, int i)
{
  //mcsim_skip_instrs_begin();
  int temp, k1, k2;

  srand(time(NULL)+i*i);
  if (n%2)
  {
      k1 = rand() % (n/2);
      k2 = rand() % (n/2+1)+(n/2);
  }
  else
  {
      k1 = rand() % (n/2);
      k2 = rand() % (n/2)+(n/2);
  }

  //cout << "swaps a[" << k1 << "] and a[" << k2 << "]" << endl;  
  //mcsim_skip_instrs_end();
  
  // mcsim_log_begin();
  
  //undolog.insert(Int_Pair(k1, a[k1]));
  //undolog.insert(Int_Pair(k2, a[k2]));  
  //redolog.insert(Int_Pair(k1, a[k2]));
  //redolog.insert(Int_Pair(k2, a[k1]));
  
  //mcsim_log_end();
  //mcsim_mem_fence(); // clflush+fence
  /*
  temp  = a[k1];
  a[k1] = a[k2];
  a[k2] = temp;
  */
  a[k1].b[(a[k1].src+1)%3]=a[k2].b[a[k2].src];
  a[k2].b[(a[k2].src+1)%3]=a[k1].b[a[k1].src];

  a[k1].src++;
  a[k2].src++;
  a[k1].src%=3;
  a[k2].src%=3;


  asm_clflush((intptr_t *)&a[k1].src);
  asm_clflush((intptr_t *)&a[k1].b[a[k1].src][0]);
  asm_clflush((intptr_t *)(&a[k1].b[a[k1].src][0]+64));
  asm_clflush((intptr_t *)(&a[k1].b[a[k1].src][0]+128));
  asm_clflush((intptr_t *)(&a[k1].b[a[k1].src][0]+192));
  asm_clflush((intptr_t *)&a[k2].src);
  asm_clflush((intptr_t *)(&a[k2].b[a[k2].src][0]));
  asm_clflush((intptr_t *)(&a[k2].b[a[k2].src][0]+64));
  asm_clflush((intptr_t *)(&a[k2].b[a[k2].src][0]+128));
  asm_clflush((intptr_t *)(&a[k2].b[a[k2].src][0]+192));
  asm_sfence();
}

void print_array(vector<pmem>& a, int n, ofstream& file)
{
  int i;

  for (i = 0; i < n; i++)
      file << a[i].b[a[i].src]<<" ";
  file<<endl;
  file<<"************"<<endl;

}

int main(int argc, char **argv)
{
  //mcsim_skip_instrs_begin();
  if (argc == 1) {
    printf("\n=========== An Array Usage ============\n");
    printf("Build an array with random integers, randomly swaps between entries\n");
    printf("./sps --count <item_count> --swaps <num of swaps>\n");    
    printf("<item_count>, default 10^6\n\n");
    return 0;
  }

  int i;
  int item_count = ITEM_COUNT, swaps = 0;

  for (i = 1; i < argc; i++) {
    if (strncmp(argv[i], "--count", 7) == 0) {      
      item_count = atoi(argv[i+1]);
      ++i;      
    } else if (strncmp(argv[i], "--swaps", 7) == 0) {
      swaps = atoi(argv[i+1]);
      ++i;
    } else {
      printf("Invalid parameters: '%s'\n", argv[i]);
      return -1;
    }
  }   
  
  vector<pmem> array(item_count);
  //map<int, int> undolog, redolog;

  // Initialization: build an array with random intergers
  if (build_array(array, item_count)) {
    cerr << "Fails to build an array" << endl;
    return -1;
  }
  
  //mcsim_skip_instrs_end(); 

#ifdef SPS_DEBUG
  ofstream orig;
  orig.open("orig.debug");
  print_array(array, item_count, orig);
#endif
  //ofstream output("result.txt");
  sum=0;
  for (int k=1;k<=200;k++)
  {
  timer_begin=GetWallTime();
  // randomly swaps between entries
  for (i = 0; i < swaps; i++) {
    array_swap(array, item_count, i); // swap two entris at a time
    //print_array(array,item_count,output);
    //mcsim_mem_fence();  //clflush+fence
  }
  timer_end=GetWallTime();
  sum+=timer_end-timer_begin;

  //mcsim_skip_instrs_begin();
  }
  printf("%.15f\n",sum/200);
  cout << "done swaps " << i << endl;
  // make sure log structures are not dummy, will not discard by compile+O3
  //cout << "dummy: undolog.size= " << undolog.size() << endl;
  //cout << "dummy: redolog.size= " << redolog.size() << endl;
  
#ifdef SPS_DEBUG
  ofstream now;
  now.open("now.debug");
  print_array(array, item_count, now);
#endif
  
  return 0;
  //mcsim_skip_instrs_end();
}
