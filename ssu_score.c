#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "ssu_score.h"
#include "blank.h"

extern struct ssu_scoreTable score_table[QNUM];
extern char id_table[SNUM][10];

struct ssu_scoreTable score_table[QNUM];
char id_table[SNUM][10];

char stuDir[BUFLEN];
char ansDir[BUFLEN];
char errorDir[BUFLEN];
char threadFiles[ARGNUM][FILELEN];
char iIDs[ARGNUM][FILELEN];

int eOption = false;//-e 옵션
int tOption = false;//-t 옵션
int mOption = false;//-m 옵션
int iOption = false;//-i 옵션

void ssu_score(int argc, char *argv[])
{
	char saved_path[BUFLEN];
	int i;

	for(i = 0; i < argc; i++){//옵션 여러개 사용가능.
		if(!strcmp(argv[i], "-h")){//단, -h를 만난경우 
			print_usage();//사용법 출력
			return;//종료
		}
	}

	memset(saved_path, 0, BUFLEN);//saved_path를 0으로 BUFLEN만큼 채운다
	if(argc >= 3 && strcmp(argv[1], "-i") != 0){//2개 이상의 인자를 입력받고 첫번째 인자로 -i를 받지 않아야함
		strcpy(stuDir, argv[1]);//argv[1]을 stuDir로 설정
		strcpy(ansDir, argv[2]);//argv[2]를 ansDir로 설정
	}

	if(!check_option(argc, argv))//옵션체크하기.안됐을경우
		exit(1);

	if(!eOption && !tOption && !mOption && iOption){//e,t,m옵션 아니고, i옵션일 경우
		do_iOption(iIDs);//iOption 들어가야함
		return;
	}
	
	if(mOption)
		do_mOption();//-m 옵션에 대한 처리

	getcwd(saved_path, BUFLEN);//현재 작업디렉토리는 saved_path

	if(chdir(stuDir) < 0){//현재 작업디렉토리 stuDir로 변경
		fprintf(stderr, "%s doesn't exist\n", stuDir);
		return;
	}
	getcwd(stuDir, BUFLEN);//현재 작업디렉토리 stuDir 저장

	chdir(saved_path);//현재 작업 디렉토리 saved_path로 변경
	if(chdir(ansDir) < 0){//현재 작업 디렉토리 ansDir 변경
		fprintf(stderr, "%s doesn't exist\n", ansDir);
		return;
	}
	getcwd(ansDir, BUFLEN);//현재 작업토리 ansDir에 저장

	chdir(saved_path);//현재 작업토리 saved_path 변경

	set_scoreTable(ansDir);//ansDir를 이용해 정답/문제번호/점수를 score_table 저장하는 함수
	set_idTable(stuDir);//id_table을 정렬하고 저장하는 함수

	printf("grading student's test papers..\n");
	score_students();//학생들의 점수 측정

	if(do_iOption)
		do_iOption(iIDs);//-i 옵션에 대한 처리

	return;
}

int check_option(int argc, char *argv[])//옵션 체크 함수
{
	int i, j;
	int c;

	while((c = getopt(argc, argv, "e:thim")) != -1)//옵션값으로 e/t/h/i/m을 받는다
	{
		switch(c){
			case 'e'://-e옵션
				eOption = true;
				strcpy(errorDir, optarg);

				if(access(errorDir, F_OK) < 0)//errorDir 없는경우
					mkdir(errorDir, 0755);//errorDir 생성
				else{
					rmdirs(errorDir);//원래있던 errorDir 삭제
					mkdir(errorDir, 0755);//새로 errorDir 생성
				}
				break;
			case 't'://-t옵션
				tOption = true;
				i = optind;//getopt()와 연관된 전역변수 optind.몇번째 인자를 처리할 것인지 표시
				j = 0;

				while(i < argc && argv[i][0] != '-'){//'-'로 시작하지 않는 인자. 옵션이 아닌 인자

					if(j >= ARGNUM)//최대로 받을 수 있는 가변인자의 개수를 초과했을때
						printf("Maximum Number of Argument Exceeded.  :: %s\n", argv[i]);
					else
						strcpy(threadFiles[j], argv[i]);//i번째 인자를 threadFiles의 j번째에 넣기
					i++; 
					j++;
				}
				break;
			case 'i'://-i 옵션
				iOption=true;
				i=optind;

				j=0;

				while(i<argc && argv[i][0]!='-'){//"-i" 부분이 아닌경우(학번)
					if(j>=ARGNUM)//최대로 받을 수 있는 가변인자의 개수를 초과했을때
						printf("Maximum Number of Argument Exceeded. :: %s\n",argv[i]);
					else
						strcpy(iIDs[j],argv[i]);//학번 복사
					i++;
					j++;
				}
				break;
			case 'm'://-m 옵션
				mOption = true;
				break;
			case '?'://그 외의 옵션
				printf("Unkown option %c\n", optopt);
				return false;
		}
	}

	return true;
}
void do_mOption()//-m 옵션 처리하는 함수
{
	FILE *fp;
	char tmp[BUFLEN];
	char num[FILELEN];
	char copy[FILELEN];
	char score[FILELEN];
	char *buf;
	char *fname;
	int size=sizeof(score_table)/sizeof(score_table[0]);
	int i,j;

	while(1)
	{
		sprintf(tmp,"%s","score_table.csv");
		read_scoreTable(tmp);//score_table.csv를 읽어서 score_table 구조체에 저장

		if((fp=fopen(tmp,"r+"))==NULL)	{//읽기쓰기 모드로 파일 오픈
		fprintf(stderr,"file open error\n");
		exit(1);	}

		printf("Input question's number to modify >>");
		scanf("%s",num);
		if(!strcmp(num,"no")) break;
		
		for(i=0;i<size;i++)
		{
			buf=score_table[i].qname;
			strncpy(copy,buf,strlen(buf));
			fname=strtok(copy,".");
			if(!strcmp(fname,num))//입력받은 문제번호와 score_table에 있는 문제번호 비교해서 같은지 확인
			{
				printf("Current score :");
				printf("%.2f\n",score_table[i].score);//현재 점수 출력
				printf("New score :");
				scanf("%s",score);//바꾸고싶은 점수 입력
				for(j=0;j<i;j++)
				{
					sprintf(tmp,"%s,%.2f\n",score_table[j].qname,score_table[j].score);
					//입력받은 문제번호 이전까지 오프셋이동
					fseek(fp,strlen(tmp),SEEK_CUR);
				}
				sprintf(tmp,"%s,%.2f\n",score_table[i].qname,atof(score));//문제번호와 변경된 점수 저장
				fwrite(tmp,strlen(tmp),1,fp);//파일에 쓰기
				
				fclose(fp);
				break;
			}
			if(i==size-1&&strcmp(fname,num)!=0)
			{
				fprintf(stderr,"Enter another number.\n");//없는 번호를 입력했을때
				break;
			}

		}
	}
	fclose(fp);
}

void do_iOption(char (*ids)[FILELEN])//-i 옵션 처리하는 함수
{
	FILE *fp;
	char tmp1[BUFLEN];
	char tmp2[BUFLEN];
	char *qArr[BUFLEN];
	char *p;
	int i=0;
	if((fp=fopen("score.csv","r"))==NULL){//score.csv를 읽기 모드로 오픈
		fprintf(stderr,"file open error for score.csv\n");
		return;
	}

	fscanf(fp,"%s\n",tmp1);//문제번호써있는 행 tmp1에 저장
	char *ptr=strtok(tmp1,",");//tmp1을 ","기준으로 자르기

	while(ptr!=NULL)
	{
		qArr[i]=ptr;//문제번호 qArr배열저장
		i++;
		ptr=strtok(NULL,",");
	}

	while(fscanf(fp,"%s\n",tmp2)!=EOF)
	{//그 다음 행부터 tmp2에 저장
		int j=0;
		i=0;
		p=strtok(tmp2,",");//","를 기준으로 잘라서 저장
		if(!is_exist(ids,tmp2))//ids에 존재하는지 확인
			continue;

		printf("%s's wrong answer :\n",tmp2);
	
		while((p=strtok(NULL,","))!=NULL)//해당 학번행의 score.csv 정보를 ","기준으로 잘라서 저장
		{
			i++;
			if(!strcmp(p,"0"))//그 값이 "0"이면 
			{
				j++;

				if(j==1)//처음 나온 0점 문제 출력형식
					printf("%s",qArr[i-1]);

				else//그 후에 나온 0점 문제 출력형식
					printf(", %s",qArr[i-1]);
			}
		}
		printf("\n");
	}
	fclose(fp);
}

int is_exist(char (*src)[FILELEN], char *target)//존재여부 판별 함수
{
	int i = 0;

	while(1)
	{
		if(i >= ARGNUM)//5개를 넘어가면 안된다
			return false;
		else if(!strcmp(src[i], ""))
			return false;
		else if(!strcmp(src[i++], target))//i번째 id가 target이랑 같아야 true
			return true;
	}
	return false;
}

void set_scoreTable(char *ansDir)//정답과 점수테이블을 저장하는 함수
{
	char filename[FILELEN];

	sprintf(filename, "%s", "score_table.csv");//정답하고 점수테이블을 filename에 출력

	if(access(filename, F_OK) == 0)//filename이 존재하는지 확인
		read_scoreTable(filename);//파일 읽어서 scoreTable에 최종 저장
	else{//filename이 없는 경우
		make_scoreTable(ansDir);//score_table 생성
		write_scoreTable(filename);//score_table읽어서 파일에 쓰기
	}
}

void read_scoreTable(char *path)//정답점수테이블 읽어서 최종 저장
{
	FILE *fp;
	char qname[FILELEN];
	char score[BUFLEN];
	int idx = 0;

	if((fp = fopen(path, "r")) == NULL){//파일을 읽기모드로 오픈
		fprintf(stderr, "file open error for %s\n", path);
		return ;
	}

	while(fscanf(fp, "%[^,],%s\n", qname, score) != EOF){//파일에서 정답과 그에대한 점수를 각각 스캔함
		strcpy(score_table[idx].qname, qname);//정답을 score_table에 최종 저장
		score_table[idx++].score = atof(score);//점수를 score_table에 최종저장
	}

	fclose(fp);
}

void make_scoreTable(char *ansDir)//score_table을 만드는 함수
{
	int type, num;
	double score, bscore, pscore;
	struct dirent *dirp, *c_dirp; //디렉토리가 가지고 있는 정보 구조체
	DIR *dp, *c_dp;
	char tmp[BUFLEN];
	int idx = 0;
	int i;

	num = get_create_type();//점수 테이블 파일 생성시 입력 타입 설정

	if(num == 1)//1번일때
	{
		printf("Input value of blank question : ");
		scanf("%lf", &bscore);//빈칸 문제의 점수 입력
		printf("Input value of program question : ");
		scanf("%lf", &pscore);//프로그램 문제의 점수 입력
	}

	if((dp = opendir(ansDir)) == NULL){//정답디렉토리 오픈
		fprintf(stderr, "open dir error for %s\n", ansDir);
		return;
	}	

	while((dirp = readdir(dp)) != NULL)//정답디렉토리 읽기
	{
		if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, ".."))//디렉토리 이름 확인
			continue;

			if((type = get_file_type(dirp->d_name)) < 0)//디렉터리 파일 타입 얻기.
				continue;

		strcpy(score_table[idx++].qname,dirp->d_name);
	}

	closedir(dp);//ansDir(정답디렉터리)닫기
	sort_scoreTable(idx);//파일위치 정리

	for(i = 0; i < idx; i++)//파일의 개수만큼 반복
	{
		type = get_file_type(score_table[i].qname);//파일타입얻기
		//입력값 num
		if(num == 1)//1번일때.모두 동일하게 입력할때
		{
			if (type == TEXTFILE)
				score = bscore;//빈칸문제 점수 저장
			else if(type == CFILE)
				score = pscore;//프로그램문제 점수 저장
		}
		else if(num == 2)//2번일때.하나씩 입력하고싶을때
		{
			printf("Input of %s: ", score_table[i].qname);
			scanf("%lf", &score);
		}//각 문제에 대해서 점수 입력받기

		score_table[i].score = score;//점수판에 입력
	}
}

void write_scoreTable(char *filename)//score_table을 파일에 쓰는 함수
{
	int fd;
	char tmp[BUFLEN];
	int i;
	int num = sizeof(score_table) / sizeof(score_table[0]);

	if((fd = creat(filename, 0666)) < 0){//filename 생성
		fprintf(stderr, "creat error for %s\n", filename);
		return;
	}

	for(i = 0; i < num; i++)
	{
		if(score_table[i].score == 0)//0점일때 멈춤.정답 점수표이므로 0점일리는 없음
			break;

		sprintf(tmp, "%s,%.2f\n", score_table[i].qname, score_table[i].score);//score_table에서 이름,점수 불러와서 tmp에 합침
		write(fd, tmp, strlen(tmp));//파일에 문제 이름과 점수 내용입력
	}

	close(fd);
}


void set_idTable(char *stuDir)
{
	struct stat statbuf;
	struct dirent *dirp;//디렉터리 정보가 있는 구조체
	DIR *dp;
	char tmp[BUFLEN];
	int num = 0;

	if((dp = opendir(stuDir)) == NULL){//stuDir 오픈
		fprintf(stderr, "opendir error for %s\n", stuDir);
		exit(1);
	}

	while((dirp = readdir(dp)) != NULL){//stuDir 읽기
		if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, ".."))//stuDir의 이름
			continue;

		sprintf(tmp, "%s/%s", stuDir, dirp->d_name);//stuDir 경로 만들기
		stat(tmp, &statbuf);

		if(S_ISDIR(statbuf.st_mode))//tmp가 디렉터리 파일인지 확인
			strcpy(id_table[num++], dirp->d_name);//id_table에 학생학번 저장
		else
			continue;
	}

	sort_idTable(num);//학생학번 정렬
}

void sort_idTable(int size)//id_table을 오름차순으로 정리해주는 함수
{
	int i, j;
	char tmp[10];

	for(i = 0; i < size - 1; i++){
		for(j = 0; j < size - 1 -i; j++){
			if(strcmp(id_table[j], id_table[j+1]) > 0){//id_table이 오름차순으로 정리가 안된경우 정렬
				strcpy(tmp, id_table[j]);
				strcpy(id_table[j], id_table[j+1]);
				strcpy(id_table[j+1], tmp);
			}
		}
	}
}

void sort_scoreTable(int size)//파일이름의 숫자를 비교해 파일위치를 정리해주는 함수
{
	int i, j;
	struct ssu_scoreTable tmp;
	int num1_1, num1_2;
	int num2_1, num2_2;

	for(i = 0; i < size - 1; i++){
		for(j = 0; j < size - 1 - i; j++){

			get_qname_number(score_table[j].qname, &num1_1, &num1_2);//파일이름에서 숫자를 뽑아옴
			get_qname_number(score_table[j+1].qname, &num2_1, &num2_2);//파일이름에서 숫자를 뽑아옴


			if((num1_1 > num2_1) || ((num1_1 == num2_1) && (num1_2 > num2_2))){//숫자크기비교로 순서결정
				
				memcpy(&tmp, &score_table[j], sizeof(score_table[0]));
				memcpy(&score_table[j], &score_table[j+1], sizeof(score_table[0]));
				memcpy(&score_table[j+1], &tmp, sizeof(score_table[0]));
			}//앞에 있던 파일이름의 숫자가 클경우 뒷파일과 자리변경
		}
	}
}

void get_qname_number(char *qname, int *num1, int *num2)//파일의 이름에서 숫자를 뽑아내는 함수
{
	char *p;
	char dup[FILELEN];

	strncpy(dup, qname, strlen(qname));//문제이름을 dup에 복사
	*num1 = atoi(strtok(dup, "-."));//- . 문자를 기준으로 쪼개서 정수형으로 저장
	
	p = strtok(NULL, "-.");//그 다음 - . 기준으로 쪼갬
	if(p == NULL)
		*num2 = 0;
	else
		*num2 = atoi(p);//숫자로 변환
}

int get_create_type()//점수 테이블 파일 생성시 입력타입 설정
{
	int num;

	while(1)//1번:빈칸문제,프로그램 문제 점수입력. 2. 모든 문제의 점수입력
	{
		printf("score_table.csv file doesn't exist in TREUDIR!\n");
		printf("1. input blank question and program question's score. ex) 0.5 1\n");
		printf("2. input all question's score. ex) Input value of 1-1: 0.1\n");
		printf("select type >> ");
		scanf("%d", &num);

		if(num != 1 && num != 2)
			printf("not correct number!\n");
		else
			break;
	}

	return num;
}

void score_students()//학생들 점수 정리
{
	double score = 0;
	int num;
	int fd;
	char tmp[BUFLEN];
	int size = sizeof(id_table) / sizeof(id_table[0]);//학생수

	if((fd = creat("score.csv", 0666)) < 0){//채점 결과 테이블 생성
		fprintf(stderr, "creat error for score.csv");
		return;
	}
	write_first_row(fd);//테이블에 첫번째 행 쓰기

	for(num = 0; num < size; num++)
	{
		if(!strcmp(id_table[num], ""))//학번이 없으면 그만
			break;

		sprintf(tmp, "%s,", id_table[num]);//학번을 tmp에 출력
		write(fd, tmp, strlen(tmp));//tmp를 파일에 쓰기

		score += score_student(fd, id_table[num]);//학생 점수를 입력
	}

	printf("Total average : %.2f\n", score / num);//평균을 구한다

	close(fd);
}

double score_student(int fd, char *id)//해당 학생에 대한 점수 계산
{
	int type;
	double result;
	double score = 0;
	int i;
	char tmp[BUFLEN];
	int size = sizeof(score_table) / sizeof(score_table[0]);

	for(i = 0; i < size ; i++)
	{
		if(score_table[i].score == 0)//해당문제번호의 점수가 0일때 그만
			break;

		sprintf(tmp, "%s/%s/%s", stuDir, id, score_table[i].qname);//학생답안,학번,문제번호를 tmp에 쓰기

		if(access(tmp, F_OK) < 0)//tmp파일있는지 확인
			result = false;
		else
		{
			if((type = get_file_type(score_table[i].qname)) < 0)//문제의 타입알기
				continue;
			
			if(type == TEXTFILE)//텍스트파일이면
				result = score_blank(id, score_table[i].qname);//빈칸문제 점수 리턴.최종적으로 ans_root와 std_root가 동일할때 true
			else if(type == CFILE)//c파일이면
				result = score_program(id, score_table[i].qname);//프로그래밍 문제 매길수있는지 여부
		}

		if(result == false)
			write(fd, "0,", 2);//파일에 "0," 적기
		else{
			if(result == true){
				score += score_table[i].score;//score값 입력
				sprintf(tmp, "%.2f,", score_table[i].score);//학생 문제점수에 입력
			}
			else if(result < 0){//result가 음수일때 (언제지??)
				score = score + score_table[i].score + result;//result까지 더하기
				sprintf(tmp, "%.2f,", score_table[i].score + result);
			}
			write(fd, tmp, strlen(tmp));//tmp를 파일에 쓰기 (채점결과테이블에 쓰기)
		}
	}

		printf("%s is finished.. score : %.2f\n", id, score); 

	sprintf(tmp, "%.2f\n", score);//점수 입력
	write(fd, tmp, strlen(tmp));//fd에 점수입력

	return score;
}

void write_first_row(int fd)//첫번째 행 쓰기 함수
{
	int i;
	char tmp[BUFLEN];
	int size = sizeof(score_table) / sizeof(score_table[0]);

	write(fd, ",", 1);//파일에 ","을 쓰기

	for(i = 0; i < size; i++){
		if(score_table[i].score == 0)//정답테이블 점수값이 0이면 그만
			break;
		
		sprintf(tmp, "%s,", score_table[i].qname);//문제번호를 tmp에 적기
		write(fd, tmp, strlen(tmp));//tmp내용 파일에 쓰기
	}
	write(fd, "sum\n", 4);
}

char *get_answer(int fd, char *result)//답안지의 내용을 얻는 함수
{
	char c;
	int idx = 0;

	memset(result, 0, BUFLEN);//result를 0으로 채움
	while(read(fd, &c, 1) > 0)//fd를 1바이트 문자씩 읽음
	{
		if(c == ':')//다수의 답이 정답일 경우 정답파일에 ':'으로 표시
			break;
		
		result[idx++] = c;//result에 입력
	}
	if(result[strlen(result) - 1] == '\n')//개행문자는 널문자로 교체
		result[strlen(result) - 1] = '\0';

	return result;
}

int score_blank(char *id, char *filename)//빈칸문제 점수
{
	char tokens[TOKEN_CNT][MINLEN];
	node *std_root = NULL, *ans_root = NULL;
	int idx, start;
	char tmp[BUFLEN];
	char s_answer[BUFLEN], a_answer[BUFLEN];
	char qname[FILELEN];
	int fd_std, fd_ans;
	int result = true;
	int has_semicolon = false;

	memset(qname, 0, sizeof(qname));//qname을 0으로 채우겠다
	memcpy(qname, filename, strlen(filename) - strlen(strrchr(filename, '.')));//문제번호를 qname에 저장

	sprintf(tmp, "%s/%s/%s", stuDir, id, filename);//stuDir/id/filename의 경로로 tmp에 저장
	fd_std = open(tmp, O_RDONLY);//tmp를 읽기모드로 오픈
	strcpy(s_answer, get_answer(fd_std, s_answer));//파일에 있는 학생 답안지를 배열로 복사

	if(!strcmp(s_answer, "")){//학생의 답안이 공백일때 파일 닫음
		close(fd_std);
		return false;
	}

	if(!check_brackets(s_answer)){//괄호 쌍이 맞지 않을때 학생경로파일 닫음
		close(fd_std);
		return false;
	}

	strcpy(s_answer, ltrim(rtrim(s_answer)));//학생 답안의 오른쪽 공백, 왼쪽공백 삭제

	if(s_answer[strlen(s_answer) - 1] == ';'){//답안의 맨끝에 ';'가 있을경우
		has_semicolon = true;//세미콜론 있다고 표시
		s_answer[strlen(s_answer) - 1] = '\0';//답안 맨끝에 세미콜론 대신 널문자 넣음
	}

	if(!make_tokens(s_answer, tokens)){//학생답안을 이용해서 토큰을 만드는데 false일경우 fd_std 닫음
		close(fd_std);
		return false;
	}

	idx = 0;
	std_root = make_tree(std_root, tokens, &idx, 0);//위에서 만든 tokens을 이용해서 std_root를 만든다

	sprintf(tmp, "%s/%s", ansDir, filename);//<ANS_DIR>의 경로 표현(수정)
	fd_ans = open(tmp, O_RDONLY);//tmp를 읽기모드로 오픈

	while(1)
	{
		ans_root = NULL;
		result = true;

		for(idx = 0; idx < TOKEN_CNT; idx++)
			memset(tokens[idx], 0, sizeof(tokens[idx]));//tokens의 모든 칸을 0으로 채움

		strcpy(a_answer, get_answer(fd_ans, a_answer));//fd_ans의 정답을 a_answer배열로 옮긴다

		if(!strcmp(a_answer, ""))//a_answer가 공백이면 멈춤
			break;

		strcpy(a_answer, ltrim(rtrim(a_answer)));//a_answer의 좌우 공백을 제거하고 a_answer에 복사

		if(has_semicolon == false){//학생 답안에 세미콜론이 없다면
			if(a_answer[strlen(a_answer) -1] == ';')//만약 정답a_answer끝에 세미콜론이 있다면 계속(맞춤)
				continue;
		}

		else if(has_semicolon == true)//학생 답안에 세미콜론이 있다면
		{
			if(a_answer[strlen(a_answer) - 1] != ';')//만약 정답 a_answer 끝에 세미콜론이 없다면 계속(맞춤)
				continue;
			else//정답에 세미콜론이 있다면
				a_answer[strlen(a_answer) - 1] = '\0';//정답끝에 널문자를 넣어준다
		}

		if(!make_tokens(a_answer, tokens))//정답지를 토큰으로 만드는데 false일경우 while문 처음부터 실행
			continue;//토큰 만드는걸 성공할때까지 진행한다

		idx = 0;
		ans_root = make_tree(ans_root, tokens, &idx, 0);//ans_root로 트리를 만든다

		compare_tree(std_root, ans_root, &result);//std와 ans의 tree를 비교

		if(result == true){//result가 true이면 둘의 루트가 같다는거니깐 두 파일을 닫음
			close(fd_std);
			close(fd_ans);

			if(std_root != NULL)
				free_node(std_root);//std_root 비워주기
			if(ans_root != NULL)
				free_node(ans_root);//ans_root 비워주기
			return true;

		}
	}
	//std_root와 ans_root의 트리가 같아야만 true
	close(fd_std);
	close(fd_ans);

	if(std_root != NULL)
		free_node(std_root);
	if(ans_root != NULL)
		free_node(ans_root);

	return false;
}

double score_program(char *id, char *filename)//프로그램 문제 점수를 매길수있는지 여부 함수
{
	double compile;
	int result;

	compile = compile_program(id, filename);//학생학번과 파일에 대한 컴파일

	if(compile == ERROR || compile == false)//compile이 ERROR이거나 false일때 false
		return false;
	
	result = execute_program(id, filename);//프로그램 실행해서 학생답안과 정답지가 같은지까지 확인

	if(!result)
		return false;

	if(compile < 0)//컴파일여부
		return compile;

	return true;
}

int is_thread(char *qname)//qname이 속하는 스레드파일이 있는지 확인
{
	int i;
	int size = sizeof(threadFiles) / sizeof(threadFiles[0]);

	for(i = 0; i < size; i++){
		if(!strcmp(threadFiles[i], qname))//qname이 속하는 threadFiles 찾기
			return true;
	}
	return false;//없으면 false
}

double compile_program(char *id, char *filename)//학번과 파일에 대한 컴파일함수
{
	int fd;
	char tmp_f[BUFLEN], tmp_e[BUFLEN];
	char command[BUFLEN];
	char qname[FILELEN];
	int isthread;
	off_t size;
	double result;

	memset(qname, 0, sizeof(qname));//qname을 0으로 채워줌
	memcpy(qname, filename, strlen(filename) - strlen(strrchr(filename, '.')));
	//filename을 "."이전까지 qname에 복사 (1-1.c면 1-1까지)
	
	isthread = is_thread(qname);//qname이 속하는 스레드가 있는지

	sprintf(tmp_f, "%s/%s", ansDir, filename);//(ansDir/filename)으로 tmp_f에 저장
	sprintf(tmp_e, "%s/%s.exe", ansDir, qname);//(ansDir/qname)으로 tmp_e에 저장

	if(tOption && isthread)//-t옵션이고 isthread가 true이면
		sprintf(command, "gcc -o %s %s -lpthread", tmp_e, tmp_f);//-lpthread모드로 설정
	else//아니면 그냥 모드로 gcc
		sprintf(command, "gcc -o %s %s", tmp_e, tmp_f);

	sprintf(tmp_e, "%s/%s_error.txt", ansDir, qname);//에러메세지 만드는 파일
	fd = creat(tmp_e, 0666);//파일 생성

	redirection(command, fd, STDERR);//fd를 STDERR로 바꾸고 command 실행
	size = lseek(fd, 0, SEEK_END);//fd 파일크기 측정
	close(fd);//파일닫기
	unlink(tmp_e);//tmp_e 파일 제거

	if(size > 0)//fd 파일크기가 0보다 크면 false
		return false;

	sprintf(tmp_f, "%s/%s/%s", stuDir, id, filename);//stuDir/id/filename으로 tmp_f 파일 설정
	sprintf(tmp_e, "%s/%s/%s.stdexe", stuDir, id, qname);//stuDir/id/qname으로 tmp_e 파일 설정

	if(tOption && isthread)//-t 옵션이고 스레드에 존재하면 -lpthread로 설정
		sprintf(command, "gcc -o %s %s -lpthread", tmp_e, tmp_f);
	else//아니면 기본 gcc 설정
		sprintf(command, "gcc -o %s %s", tmp_e, tmp_f);

	sprintf(tmp_f, "%s/%s/%s_error.txt", stuDir, id, qname);//에러텍스트파일 만들기
	fd = creat(tmp_f, 0666);//tmp_f 0666모드로 생성

	redirection(command, fd, STDERR);//fd를 STDERR로 바꾸고 command 실행
	size = lseek(fd, 0, SEEK_END); //fd 파일크기 측정
	close(fd);//파일닫기

	if(size > 0){//fd 파일크기가 0보다 크면
		if(eOption)//-e 옵션
		{
			sprintf(tmp_e, "%s/%s", errorDir, id);//errorDir/id로 tmp_e 파일 설정
			if(access(tmp_e, F_OK) < 0)//tmp_e디렉토리 있는지 확인
				mkdir(tmp_e, 0755);//없으면 tmp_e 디렉토리 생성

			sprintf(tmp_e, "%s/%s/%s_error.txt", errorDir, id, qname);//errorDir/id/qname_error.txt로 tmp_e 설정
			rename(tmp_f, tmp_e);//tmp_f를 tmp_e로 변경

			result = check_error_warning(tmp_e);//rmp_e에 대한 에러 체크
		}
		else{ 
			result = check_error_warning(tmp_f);//warning 체크
			unlink(tmp_f);//tmp_f 파일 제거
		}

		return result;//warning 수 리턴
	}

	unlink(tmp_f);//tmp_f 파일제거
	return true;//true리턴
}

double check_error_warning(char *filename)//filename에 대한 에러체크
{
	FILE *fp;
	char tmp[BUFLEN];
	double warning = 0;

	if((fp = fopen(filename, "r")) == NULL){//filename을 읽기모드로 오픈
		fprintf(stderr, "fopen error for %s\n", filename);
		return false;
	}

	while(fscanf(fp, "%s", tmp) > 0){//fp 문자열 tmp로 scan.더이상 내용없을때까지 반복
		if(!strcmp(tmp, "error:"))//tmp와 "error:"가 일치하면 ERROR
			return ERROR;
		else if(!strcmp(tmp, "warning:"))//tmp와 "warning:"가 일치하면 warning 1증가
			warning += WARNING;
	}

	return warning;//warning 리턴
}

int execute_program(char *id, char *filename)//실행 프로그램
{
	char std_fname[BUFLEN], ans_fname[BUFLEN];
	char tmp[BUFLEN];
	char qname[FILELEN];
	time_t start, end;
	pid_t pid;
	int fd;

	memset(qname, 0, sizeof(qname));//qname을 0으로 채우기
	memcpy(qname, filename, strlen(filename) - strlen(strrchr(filename, '.')));
	//filename을 .이전까지 qname에 복사
	sprintf(ans_fname, "%s/%s.stdout", ansDir, qname);//정답 프로그램 실행결과를 ansDir/qname.stdout으로 저장(수정)
	fd = creat(ans_fname, 0666);//ans_fname을 0666모드로 저장

	sprintf(tmp, "%s/%s.exe", ansDir, qname);//정답 프로그램 실행파일을 ansDir/qname.exe로 저장(수정)
	redirection(tmp, fd, STDOUT);//fd를 STDOUT(표준출력)으로 복사해서 tmp를 실행한다.->출력할때 fd가 나온다
	close(fd);

	sprintf(std_fname, "%s/%s/%s.stdout", stuDir, id, qname);//학생답안 실행결과를 stuDir/id/qname.stdout으로 저장
	fd = creat(std_fname, 0666);//std_fname을 0666 모드로 저장

	sprintf(tmp, "%s/%s/%s.stdexe &", stuDir, id, qname);//학생답안 실행파일을 stuDir/id/qname.stdexe & 로 저장

	start = time(NULL);//1970년 1월 1일 0시부터 현재까지 흐른 초 수
	redirection(tmp, fd, STDOUT);//fd를 표준출력에 복사하고 tmp 실행
	
	sprintf(tmp, "%s.stdexe", qname);//qname.stdexe로 tmp에 저장
	while((pid = inBackground(tmp)) > 0){//pid 조회하는 함수.
		end = time(NULL);//프로세스 실행시키고 난 후 흐른 초수

		if(difftime(end, start) > OVER){//end와 start 차이를 구함.5초를 넘어가면 kill
			kill(pid, SIGKILL);//쉘명령의 kill과 같은 역할.지정한 프로세스를 죽임
			close(fd);//파일닫기
			return false;//false리턴
		}
	}

	close(fd);//fd 닫기

	return compare_resultfile(std_fname, ans_fname);//학생답안 실행결과와 정답 실행결과가 같은지 확인
}

pid_t inBackground(char *name)//프로세스 실행
{
	pid_t pid;//프로세스 번호 저장하는 타입 pid_t
	char command[64];
	char tmp[64];
	int fd;
	off_t size;
	
	memset(tmp, 0, sizeof(tmp));//tmp룰 0으로 채운다
	fd = open("background.txt", O_RDWR | O_CREAT | O_TRUNC, 0666);//"background.txt"

	sprintf(command, "ps | grep %s", name);//command에 ps | grep name으로 저장. 프로세스에서 name이 들어간 라인을 출력
	redirection(command, fd, STDOUT);//fd를 표준출력에 복사해서 command 수행

	lseek(fd, 0, SEEK_SET);//fd 오프셋을 현재 위치시킴
	read(fd, tmp, sizeof(tmp));//fd를 tmp에 읽음

	if(!strcmp(tmp, "")){//tmp가 공백일때
		unlink("background.txt");//background.txt 파일 제거
		close(fd);//fd닫음
		return 0;
	}

	pid = atoi(strtok(tmp, " "));//tmp를 띄어쓰기를 중심으로 쪼갬.문자열을 정수로 표현
	close(fd);//fd 닫기

	unlink("background.txt");//background.txt 파일 제거
	return pid;//pid리턴. background.txt관련
}

int compare_resultfile(char *file1, char *file2)//
{
	int fd1, fd2;
	char c1, c2;
	int len1, len2;

	fd1 = open(file1, O_RDONLY); //file1을 읽기모드로 오픈
	fd2 = open(file2, O_RDONLY); //file2를 읽기모드로 오픈

	while(1)
	{
		while((len1 = read(fd1, &c1, 1)) > 0){//fd1을 &c1에 읽어서 읽은 바이트 수만큼 len1에 저장
			if(c1 == ' ') //c1이 공백이면 계속
				continue;
			else //무언갈 읽으면 그만
				break;
		}//fd1의 내용을 읽을때까지 반복
		while((len2 = read(fd2, &c2, 1)) > 0){//fd2을 &c2에 읽어서 읽은 바이트 수만큼 len2에 저장
			if(c2 == ' ') //c2가 공백이면 계속
				continue;
			else //무언갈 읽으면 그만
				break;
		}
		
		if(len1 == 0 && len2 == 0)//아무것도 안읽었으면 그만
			break;

		to_lower_case(&c1);//c1을 소문자로 변경
		to_lower_case(&c2);//c2를 소문자로 변경

		if(c1 != c2){//c1,c2가 다르면 파일닫고 false리턴
			close(fd1);
			close(fd2);
			return false;
		}
	}
	close(fd1);
	close(fd2);
	return true;//fd1,fd2의 모든 문자를 비교해서 반복문 벗어나면 true
}

void redirection(char *command, int new, int old)//dup을 이용한 리디렉션
{
	int saved;

	saved = dup(old);//새로운 파일디스크립터에 복사
	dup2(new, old);//new가 old 파일디스크립터로 복사됨

	system(command);//command를 실행한다

	dup2(saved, old);//saved를 old에 복사. 되돌아옴
	close(saved);
}

int get_file_type(char *filename)//파일의 타입을 판별하는 함수
{
	char *extension = strrchr(filename, '.');

	if(!strcmp(extension, ".txt"))
		return TEXTFILE;
	else if (!strcmp(extension, ".c"))
		return CFILE;
	else
		return -1;
}

void rmdirs(const char *path)//디렉터리 지우는 함수
{
	struct dirent *dirp;
	struct stat statbuf;
	DIR *dp;
	char tmp[260];
	
	if((dp = opendir(path)) == NULL)//디렉터리오픈해서 없으면 끝
		return;

	while((dirp = readdir(dp)) != NULL)//디렉터리 읽어서 읽기
	{
		if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, ".."))
			continue;//디렉터리 이름에 "."".." 나올때까지 반복

		sprintf(tmp, "%s/%s", path, dirp->d_name);// path/dirp->d_name으로 경로 설정

		if(lstat(tmp, &statbuf) == -1)//tmp의 stat구조체에 대한 정보 얻기
			continue;

		if(S_ISDIR(statbuf.st_mode))//tmp가 디렉터리이면 디렉터리 삭제
			rmdirs(tmp);
		else
			unlink(tmp);//아니면 파일이라고 생각해 파일 제거
	}

	closedir(dp);//디렉터리 닫기
	rmdir(path);
}

void to_lower_case(char *c)//대문자를 소문자로 변경하는 함수
{
	if(*c >= 'A' && *c <= 'Z')
		*c = *c + 32;
}

void print_usage()//-h옵션일경우. 사용법 출력
{
	printf("Usage : ssu_score <STD_DIR> <ANS_DIR> [OPTION]\n");
	printf("Option : \n");
	printf(" -m                modify question's score\n");
	printf(" -e <DIRNAME>      print error on 'DIRNAME/ID/qname_error.txt' file \n");
	printf(" -t <QNAMES>       compile QNAME.c with -lpthread option\n");
	printf(" -i <IDS>          print ID's wrong questions\n");
	printf(" -h                print usage\n");
	
}

