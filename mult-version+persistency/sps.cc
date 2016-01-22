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
#include <pthread.h>
#include "defines.h"

using namespace std;
volatile int logging=0,threadend=0;
typedef pair <int, string> Int_Pair;
pthread_t *log_id=0;
double timer_begin,timer_end,sum;
int pn;
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
    return( ((double) (tp.tv_sec - start)) + (tp.tv_usec-startu)/1000000.0);
}
static inline void asm_clflush(volatile intptr_t *addr)
{
    __asm__ __volatile__ ("clflush %0 "::"m"(*addr));
}
static inline void asm_mfence(void)
{
    __asm__ __volatile__ ("mfence");
}
struct a1
{
    bool p;
    int st,dst,src;
    string b[3];
    volatile bool changing,flushing,flushed;
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
        a[i].p=1;
        a[i].src=0;
        a[i].dst=1;
        a[i].st=2;
        a[i].changing=a[i].flushing=a[i].flushed=0;
        //a[i].st1=3;
        //cout<<a[i].b[0]<<endl;
    }
    pn=0;
    //cout<<"^^^^^^^^^^^^^"<<endl;
    return 0;
}


void array_swap(vector<pmem>& a, int n, int i)
{
  //mcsim_skip_instrs_begin();
  int tmp, k1, k2;

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
  if (a[k1].p)
  {
      a[k1].p=0;
      pn++;
  }
  //a[k1].p=0;
  if (a[k2].p)
  {
      a[k2].p=0;
      pn++;
  }
  //a[k2].p=0;
  if (logging)
  {
      while ((logging)&&(a[k1].flushing||a[k2].flushing||(!a[k1].flushed)||(!a[k2].flushed)));
      //while (a[k1].flushing||a[k2].flushing||(a[k1].flushed^a[k2].flushed));
      //while (a[k1].p^a[k2].p);
      a[k1].changing=a[k2].changing=1;
      //a[k1].p=a[k2].p=0;
      a[k1].b[a[k1].dst]=a[k2].b[a[k2].src];
      a[k2].b[a[k2].dst]=a[k1].b[a[k1].src];
      tmp=a[k1].dst;
      a[k1].dst=a[k1].src;
      a[k1].src=tmp;
      tmp=a[k2].dst;
      a[k2].dst=a[k2].src;
      a[k2].src=tmp;
      a[k1].changing=a[k2].changing=0;
  }
  else
  {
      a[k1].b[a[k1].dst]=a[k2].b[a[k2].src];
      a[k2].b[a[k2].dst]=a[k1].b[a[k1].src];
      tmp=a[k1].dst;
      a[k1].dst=a[k1].src;
      a[k1].src=tmp;
      tmp=a[k2].dst;
      a[k2].dst=a[k2].src;
      a[k2].src=tmp;
  }

  //cout<<a[k1].b[a[k1].dst]<<" "<<a[k2].b[a[k2].dst]<<endl;
  //cout<<pn<<endl;
  if ((!logging)&&((double)pn/(double)n>=0.8))
  {
      //a[k1]
      logging=1;
      //cout<<pn<<endl;
      //pn=0;
  }

  //a[k1].src++;
  //a[k2].src++;
  //a[k1].src%=3;
  //a[k2].src%=3;

  /*
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
  asm_mfence();
  */

}

void print_array(vector<pmem>& a, int n, ofstream& file)
{
  int i;

  for (i = 0; i < n; i++)
      file << a[i].b[a[i].src]<<" ";
  file<<endl;
  file<<"************"<<endl;

}
void *log_all_data(void *x)
{
    int i,j,tmp;
    i=0;
    //int n=a.size();
    vector<pmem> *a=(static_cast<vector<pmem>*>(x));
    //cout<<a->size()<<endl;
    vector<pmem>::iterator it;
    //it=a->end();
    //puts(it->b[0]);
    it=a->begin();
    //cout<<a->src<<endl;
    //cout<<a->size()<<endl;
    while (1)
    {
        while (!logging) if (threadend) return NULL;
        //cout<<"flush"<<endl;
        i=0;
        for (it=a->begin();it!=a->end();it++)
        {
            if (!it->p)
            {
                while (it->changing);
                i++;
                it->flushing=1;
                //cout<<it->b[it->src]<<" "<<i<<" "<<pn<<endl;
                asm_clflush((intptr_t *)&it->b[it->src][0]);
                asm_clflush((intptr_t *)(&it->b[it->src][0]+64));
                asm_clflush((intptr_t *)(&it->b[it->src][0]+128));
                asm_clflush((intptr_t *)(&it->b[it->src][0]+192));
                asm_mfence();
                tmp=it->src;
                it->src=it->st;
                it->st=tmp;
                it->p=1;
                //it->flushed=1;
                //cout<<it->b[it->src]<<endl;
                it->flushing=0;
                pn--;
            }
            it->flushed=1;
        }

        //cout<<"********"<<endl;
        logging=0;
        for (it=a->begin();it!=a->end();it++) it->flushed=0;
        //pn=0;
        if (threadend) break;
    }
    return NULL;
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
  //memset(array,0,sizeof(array));
  //map<int, int> undolog, redolog;

  // Initialization: build an array with random intergers
  if (build_array(array, item_count)) {
    cerr << "Fails to build an array" << endl;
    return -1;
  }
  //T_skip_instrs_end();

#ifdef SPS_DEBUG
  ofstream orig;
  orig.open("orig.debug");
  print_array(array, item_count, orig);
#endif
  //ofstream output("result.txt");
  sum=0;
  logging=0;
  log_id = (pthread_t*) malloc(sizeof(pthread_t));
  pthread_create(log_id,NULL,log_all_data,&array);
  for (int k=1;k<=200;k++)
  {
  timer_begin=GetWallTime();
  // randomly swaps between entries
  for (i = 0; i < swaps; i++) {
    array_swap(array, item_count, i); // swap two entris at a time
    //pthread_join();
    //print_array(array,item_count,output);
    //mcsim_mem_fence();  //clflush+fence
  }
  
  timer_end=GetWallTime();
  sum+=timer_end-timer_begin;

  //mcsim_skip_instrs_begin();
  }
  threadend=1;
  pthread_join(*log_id,NULL);
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
