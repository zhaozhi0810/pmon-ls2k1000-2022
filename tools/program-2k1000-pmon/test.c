#include<stdio.h>
#include<unistd.h>
int main(){
#define NUM 100
  char buffer[NUM+1] = {0};//存储进度条字符
  char arr[5] = {"-/|\\"};//存储基本的变化字幕
  for(int i = 0;i<NUM ;++i){
    buffer[i] = '#';
    printf("[%-100s][%d%%][%c]\r",buffer,i+1,arr[i%4]);
    fflush(stdout);
    usleep(60000);
  }
  printf("\n");
  return 0;
}
