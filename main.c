#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include "ssu_score.h"

#define SECOND_TO_MICRO 1000000

void ssu_runtime(struct timeval *begin_t, struct timeval *end_t);

int main(int argc, char *argv[])
{
	struct timeval begin_t, end_t;
	gettimeofday(&begin_t, NULL);//프로그램 시작시간 begin_t에 저장
	ssu_score(argc, argv);//채점하는 함수

	gettimeofday(&end_t, NULL);//프로그램 종료시간 end_t에 저장
	ssu_runtime(&begin_t, &end_t);//프로그램 수행시간 측정 함수

	exit(0);
}

void ssu_runtime(struct timeval *begin_t, struct timeval *end_t)
{//프로그램 수행시간 측정 함수
	end_t->tv_sec -= begin_t->tv_sec;//수행시간측정(초단위)

	if(end_t->tv_usec < begin_t->tv_usec){//마이크로단위에서 종료시간이 시작시간보다 작을때
		end_t->tv_sec--;//초단위를 하나 빼줌
		end_t->tv_usec += SECOND_TO_MICRO;//1초를 마이크로 단위로 바꾸고 tv_usec에 더해줌
	}

	end_t->tv_usec -= begin_t->tv_usec;//수행시간(마이크로단위)
	printf("Runtime: %ld:%06ld(sec:usec)\n", end_t->tv_sec, end_t->tv_usec);
}
