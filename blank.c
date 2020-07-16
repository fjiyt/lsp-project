#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include "blank.h"

char datatype[DATATYPE_SIZE][MINLEN] = {"int", "char", "double", "float", "long"
			, "short", "ushort", "FILE", "DIR","pid"
			,"key_t", "ssize_t", "mode_t", "ino_t", "dev_t"
			, "nlink_t", "uid_t", "gid_t", "time_t", "blksize_t"
			, "blkcnt_t", "pid_t", "pthread_mutex_t", "pthread_cond_t", "pthread_t"
			, "void", "size_t", "unsigned", "sigset_t", "sigjmp_buf"
			, "rlim_t", "jmp_buf", "sig_atomic_t", "clock_t", "struct"};


operator_precedence operators[OPERATOR_CNT] = {//연산자 우선순위 배열
	{"(", 0}, {")", 0}
	,{"->", 1}	
	,{"*", 4}	,{"/", 3}	,{"%", 2}	
	,{"+", 6}	,{"-", 5}	
	,{"<", 7}	,{"<=", 7}	,{">", 7}	,{">=", 7}
	,{"==", 8}	,{"!=", 8}
	,{"&", 9}
	,{"^", 10}
	,{"|", 11}
	,{"&&", 12}
	,{"||", 13}
	,{"=", 14}	,{"+=", 14}	,{"-=", 14}	,{"&=", 14}	,{"|=", 14}
};

void compare_tree(node *root1,  node *root2, int *result)//트리를 비교하는 함수
{
	node *tmp;
	int cnt1, cnt2;

	if(root1 == NULL || root2 == NULL){//root1,root2가 공백일때 result는 false
		*result = false;
		return;
	}

	if(!strcmp(root1->name, "<") || !strcmp(root1->name, ">") || !strcmp(root1->name, "<=") || !strcmp(root1->name, ">=")){
		//root1이 <,>,<=,>= 일때
		if(strcmp(root1->name, root2->name) != 0){
			//root1과 root2가 같지 않으면
			if(!strncmp(root2->name, "<", 1))//root2가 <이면 >를 복사한다
				strncpy(root2->name, ">", 1);

			else if(!strncmp(root2->name, ">", 1))//root2가 >이면 <를 복사한다
				strncpy(root2->name, "<", 1);

			else if(!strncmp(root2->name, "<=", 2))//root2가 <=이면 >=를 복사한다
				strncpy(root2->name, ">=", 2);

			else if(!strncmp(root2->name, ">=", 2))//root2 >=이면 <=를 복사한다
				strncpy(root2->name, "<=", 2);

			root2 = change_sibling(root2);//root2의 childhead를 childhead->next로 변경
		}
	}

	if(strcmp(root1->name, root2->name) != 0){//root1과 root2과 같지않으면 false
		*result = false;
		return;
	}

	if((root1->child_head != NULL && root2->child_head == NULL)
		|| (root1->child_head == NULL && root2->child_head != NULL)){
		//root1과 root2 중 하나만 childhead가 있다면 false
		*result = false; 
		return;
	}

	else if(root1->child_head != NULL){//root1->childhead가 존재, root2->childhead 존재
		if(get_sibling_cnt(root1->child_head) != get_sibling_cnt(root2->child_head)){
			//root1,root2의 sibling수가 다르면 false
			*result = false;
			return;
		}

		if(!strcmp(root1->name, "==") || !strcmp(root1->name, "!="))//root1이 ==,!=일때
		{
			compare_tree(root1->child_head, root2->child_head, result);
			//root1,root2 tree를 다시 비교
			if(*result == false)//위의 result가 false일경우
			{
				*result = true;//true로 바꿔주고 
				root2 = change_sibling(root2);//childhead를 childhead->next로 바꿔줌
				compare_tree(root1->child_head, root2->child_head, result);//다시 root1->childhead, root2->childhead비교
			}
		}
		else if(!strcmp(root1->name, "+") || !strcmp(root1->name, "*")
				|| !strcmp(root1->name, "|") || !strcmp(root1->name, "&")
				|| !strcmp(root1->name, "||") || !strcmp(root1->name, "&&"))
		{//root1이 다음과 같은 연산자 중에 있을때
			if(get_sibling_cnt(root1->child_head) != get_sibling_cnt(root2->child_head)){
				//root1,root2의 childhead의 형제자매수가 다르면 false
				*result = false;
				return;
			}

			tmp = root2->child_head;//root2->childhood를 tmp로 지정

			while(tmp->prev != NULL)//tmp->prev가 있을때 tmp->prev를 tmp로 지정.
				tmp = tmp->prev;//tmp->prev가 존재하지 않을때까지 반복

			while(tmp != NULL)//tmp가 공백이 아니면
			{
				compare_tree(root1->child_head, tmp, result);//root1->child_head와 tmp 트리를 비교
			
				if(*result == true)//true가 나오면 그만
					break;
				else{//flase라면
					if(tmp->next != NULL)//tmp->next가 존재하면 true로 변경하고 tmp->next를 tmp로 지정
						*result = true;
					tmp = tmp->next;
				}
			}
		}
		else{//이외에
			compare_tree(root1->child_head, root2->child_head, result);//root1->childhead,root2->childhead 트리 비교
		}
	}	


	if(root1->next != NULL){//root1->next가 공백이 아니면

		if(get_sibling_cnt(root1) != get_sibling_cnt(root2)){//root1,root2의 형제자매수가 다르면 false
			*result = false;
			return;
		}

		if(*result == true)//result가 true이면
		{
			tmp = get_operator(root1);//root1의 연산자를 tmp에 저장
	
			if(!strcmp(tmp->name, "+") || !strcmp(tmp->name, "*")
					|| !strcmp(tmp->name, "|") || !strcmp(tmp->name, "&")
					|| !strcmp(tmp->name, "||") || !strcmp(tmp->name, "&&"))
			{	//tmp가 이런 연산자라면
				tmp = root2;//root2를 tmp에 넣기
	
				while(tmp->prev != NULL)
					tmp = tmp->prev;//tmp->prev를 tmp에 저장. prev가 더이상 없을때까지 저장

				while(tmp != NULL)//tmp가 공백이 아닐때까지
				{
					compare_tree(root1->next, tmp, result);//root1->next를 tmp트리와 비교

					if(*result == true)//result가 true이면 그만
						break;
					else{
						if(tmp->next != NULL)//result가 false여도 tmp->next가 존재하면 true로 바꿔줌
							*result = true;
						tmp = tmp->next;//tmp->next를 tmp에 넣어줌
					}
				}
			}

			else//위의 연산자가 아니라면 
				compare_tree(root1->next, root2->next, result);//root1->next,root2->next 트리를 비교
		}
	}
}

int make_tokens(char *str, char tokens[TOKEN_CNT][MINLEN])//토큰 만드는 함수
{
	char *start, *end;
	char tmp[BUFLEN];
	char str2[BUFLEN];
	char *op = "(),;><=!|&^/+-*\""; 
	int row = 0;
	int i;
 	int isPointer;
	int lcount, rcount;
	int p_str;
	
	clear_tokens(tokens);//토큰을 비움

	start = str;
	
	if(is_typeStatement(str) == 0) //학생 답안이 타입에 대한 답이 아니라면 토큰 안만듬
		return false;	
	
	while(1)
	{
		if((end = strpbrk(start, op)) == NULL)//학생답안과 토큰모음 중에 일치하는 문자가 없다면 토큰 안만듬
			break;

		if(start == end){//처음부터 연산자가 있는경우

			if(!strncmp(start, "--", 2) || !strncmp(start, "++", 2)){//처음에 "--"나 "++"가 있을경우
				if(!strncmp(start, "++++", 4)||!strncmp(start,"----",4))//처음에 "----"나 "++++"가 있을경우
					return false;

				if(is_character(*ltrim(start + 2))){//"--"나 "++" 2바이트 뒤로가서 문자인지를 판별
					if(row > 0 && is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1]))
						//행이 양수이고 토큰이 문자일때
						return false; 

					end = strpbrk(start + 2, op);//2바이트 뒤로간 문자열에 op 문자와 첫번째로 일치한 곳
					if(end == NULL)//토큰이 없을때
						end = &str[strlen(str)];//end는 답안지 파일의 끝을 가리킴
					while(start < end) {//start에서 다음토큰/파일의 끝까지 반복
						if(*(start - 1) == ' ' && is_character(tokens[row][strlen(tokens[row]) - 1]))
							//start-1에 빈칸이 존재하고 토큰에 숫자, 알파벳이 존재할경우
							return false;
						else if(*start != ' ')//start가 빈칸이 아닌경우
							strncat(tokens[row], start, 1);//토큰에 학생답안지에서 첫번째로 나온 토큰을 집어넣는다
						start++;//한칸 뒤로가기	
					}
				}//처음("++","--")부터 다음 토큰이 나오기 전까지 tokens에 집어넣는다
				
				else if(row>0 && is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])){
					//tokens의 행이 양수이고 문자가 들어가있으면
					if(strstr(tokens[row - 1], "++") != NULL || strstr(tokens[row - 1], "--") != NULL)	
						//"++"/"--"가 있으면 false
						return false;

					memset(tmp, 0, sizeof(tmp));//tmp를 0로 채움
					strncpy(tmp, start, 2);//tmp에 start에 있는 2바이트 채움 ("++"."--")
					strcat(tokens[row - 1], tmp);//tokens에 tmp이어쓰기
					start += 2;//2칸 뒤로
					row--;
				}
				else{
					memset(tmp, 0, sizeof(tmp));//tmp를 0으로 채움
					strncpy(tmp, start, 2);//tmp에 start 2바이트 채움
					strcat(tokens[row], tmp);//tokens에 tmp합치기
					start += 2;//2칸 뒤로
				}
			}

			else if(!strncmp(start, "==", 2) || !strncmp(start, "!=", 2) || !strncmp(start, "<=", 2)
				|| !strncmp(start, ">=", 2) || !strncmp(start, "||", 2) || !strncmp(start, "&&", 2) 
				|| !strncmp(start, "&=", 2) || !strncmp(start, "^=", 2) || !strncmp(start, "!=", 2) 
				|| !strncmp(start, "|=", 2) || !strncmp(start, "+=", 2)	|| !strncmp(start, "-=", 2) 
				|| !strncmp(start, "*=", 2) || !strncmp(start, "/=", 2)){//비교연산자가 있으면

				strncpy(tokens[row], start, 2);//start에서 2 바이트 tokens에 복사
				start += 2;//start 2칸 뒤로
			}
			else if(!strncmp(start, "->", 2))//"->"가 있으면
			{
				end = strpbrk(start + 2, op);//2칸뒤로가서 op와 일치하는 문자확인

				if(end == NULL)//일치하는 게 없으면
					end = &str[strlen(str)];//그 답안지 문자열의 끝을 가리킴

				while(start < end){//문자열의 끝/ 다음 토큰문자
					if(*start != ' ')
						strncat(tokens[row - 1], start, 1);//tokens에 문자를 넣음
					start++;
				}
				row--;
			}
			else if(*end == '&')//연산자가 '&'일 경우
			{
				
				if(row == 0 || (strpbrk(tokens[row - 1], op) != NULL)){//row가 0이거나 이전에 op와 일치하는게 있다면
					end = strpbrk(start + 1, op);//'&'을 제외하고 다음 op일치가 있는지 확인
					if(end == NULL)//op일치가 없으면
						end = &str[strlen(str)];//end는 문자열의 끝을 가리킴
					
					strncat(tokens[row], start, 1);//tokens에 '&'를 넣음
					start++;

					while(start < end){//다음 토큰/문자열 종료까지 반복
						if(*(start - 1) == ' ' && tokens[row][strlen(tokens[row]) - 1] != '&')//한칸전이 빈칸이고 tokens에 '&'가 없다면
							return false;
						else if(*start != ' ')//빈칸아닐 경우
							strncat(tokens[row], start, 1);//tokens에 문자 입력
						start++;
					}
				}
				
				else{//그외에
					strncpy(tokens[row], start, 1);//tokens에 '&'입력
					start += 1;
				}
				
			}
		  	else if(*end == '*')//'*'일 경우
			{
				isPointer=0;

				if(row > 0)//행이 양수일때
				{
					
					for(i = 0; i < DATATYPE_SIZE; i++) {
						if(strstr(tokens[row - 1], datatype[i]) != NULL){//포인터 뒤에 데이터타입 찾기
							strcat(tokens[row - 1], "*");//포인터를 tokens에 넣기
							start += 1;	
							isPointer = 1;//포인터임을 표시
							break;
						}
					}
					if(isPointer == 1)
						continue;
					if(*(start+1) !=0)
						end = start + 1;//포인터 뒤에 문자를 가리키도록 end설정

					
					if(row>1 && !strcmp(tokens[row - 2], "*") && (all_star(tokens[row - 1]) == 1)){
						//row가 1보다 크고 tokens[row-2]가 "*"이고 tokens[row-1]이 "*"로만 이루어져있으면
						strncat(tokens[row - 1], start, end - start);//tokens에 start문자를 넣는다
						row--;
					}
					
					
					else if(is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1]) == 1){ 
						//바로끝 tokens에 숫자/알파벳이 존재한다면
						strncat(tokens[row], start, end - start);  //다음 행 tokens에 문자열 넣기
					}

					
					else if(strpbrk(tokens[row - 1], op) != NULL){	//tokens에서 op와 동일한 문자들 찾기	
						strncat(tokens[row] , start, end - start);//다음 행 tokens에 문자열 넣기 
							
					}
					else
						strncat(tokens[row], start, end - start);//다음행 tokens에 문자열 넣기

					start += (end - start);//다음 토큰을 가리킴
				}

			 	else if(row == 0)//tokens에 아무것도 안들어있을때
				{
					if((end = strpbrk(start + 1, op)) == NULL){//다음 일치 op가 없을때
						strncat(tokens[row], start, 1);//포인터만 tokens에 써주기
						start += 1;
					}
					else{//다음 일치 op가 있을때
						while(start < end){//다음 토큰까지
							if(*(start - 1) == ' ' && is_character(tokens[row][strlen(tokens[row]) - 1]))
								//빈칸이고 문자가 들어가있을때 false
								return false;
							else if(*start != ' ')//start가 빈칸이 아니면
								strncat(tokens[row], start, 1);//tokens에 입력
							start++;	
						}
						if (all_star(tokens[row]))//row가 모두 포인터"*"일때
							row--;//앞에 토큰 가리킴
						
					}
				}
			}
			else if(*end == '(') //'('로 시작
			{
				lcount = 0;
				rcount = 0;
				if(row>0 && (strcmp(tokens[row - 1],"&") == 0 || strcmp(tokens[row - 1], "*") == 0)){
					//이전에 "&"나 "*" 있다면
					while(*(end + lcount + 1) == '(')//'('가 끝날때까지 반복
						lcount++;
					start += lcount;//start는 '('가 끝난후 가리킴

					end = strpbrk(start + 1, ")");//")"가 있는지 탐색

					if(end == NULL)//")"가 없다면 false
						return false;
					else{//")"가 있다면
						while(*(end + rcount +1) == ')')//")"가 끝날때까지 반복
							rcount++;
						end += rcount;//end는 ')'가 끝난후 가리킴

						if(lcount != rcount)//쌍이 맞지않으면 false
							return false;

						if( (row > 1 && !is_character(tokens[row - 2][strlen(tokens[row - 2]) - 1])) || row == 1){ 
							//row가 1이상, 두칸앞tokens의 마지막이 문자가 아니라면 (다른 부호가 있다면)
							strncat(tokens[row - 1], start + 1, end - start - rcount - 1);//괄호를 제거한 문자입력
							row--;
							start = end + 1;//우괄호 끝 다음부터 다시 가리킴
						}
						else{
							strncat(tokens[row], start, 1);//문자 입력(괄호 없이)
							start += 1;
						}
					}
						
				}
				else{//이전에 "&","*"가 없다면
					strncat(tokens[row], start, 1);//문자입력(괄호 포함)
					start += 1;
				}

			}
			else if(*end == '\"')//'\"'로 시작 
			{
				end = strpbrk(start + 1, "\"");//'\"'가 또 있는지 탐색
				
				if(end == NULL)//또 없으면 실패
					return false;

				else{//또 있다면
					strncat(tokens[row], start, end - start + 1);//start부터 \"나오기전까지 이어 붙임
					start = end + 1;
				}

			}

			else{//그외에
				
				if(row > 0 && !strcmp(tokens[row - 1], "++"))//row가 양수인데 "++"가 있으면 false
					return false;

				
				if(row > 0 && !strcmp(tokens[row - 1], "--"))//row가 양수인데 "--"가 있으면 false
					return false;
	
				strncat(tokens[row], start, 1);//tokens에 start이어붙이기
				start += 1;
				
			
				if(!strcmp(tokens[row], "-") || !strcmp(tokens[row], "+") || !strcmp(tokens[row], "--") || !strcmp(tokens[row], "++")){
					//만약에 토큰에 -/+/--/++ 이 들어있다면

				
					if(row == 0)//row가 0이면
						row--;

					
					else if(!is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])){
					    //토큰 마지막이 문자가 아니라면
						if(strstr(tokens[row - 1], "++") == NULL && strstr(tokens[row - 1], "--") == NULL)
							//토큰에 "++"/"--"가 없다면
							row--;//row-1
					}
				}
			}
		}
		else{ //처음부터 토큰이 있지 않은경우
			if(all_star(tokens[row - 1]) && row > 1 && !is_character(tokens[row - 2][strlen(tokens[row - 2]) - 1]))   
				//row-1이 모두 포인터로 이루어져있고 row가 1보다 크며 row-2부분의 토큰이 문자로 이루어져있지않다면
				row--;				

			if(all_star(tokens[row - 1]) && row == 1)   
				//row-1이 모두 포인터인데 row가 1. {***}{}
				row--;	

			for(i = 0; i < end - start; i++){//처음에서 다음 토큰까지 반복
				if(i > 0 && *(start + i) == '.'){
					strncat(tokens[row], start + i, 1);//tokens에 .넣기

					while( *(start + i +1) == ' ' && i< end - start )//공백제거
						i++; 
				}
				else if(start[i] == ' '){//공백일때
					while(start[i] == ' ')//공백제거
						i++;
					break;
				}
				else//그외에
					strncat(tokens[row], start + i, 1);//문자추가
			}

			if(start[0] == ' '){//처음이 공백일때
				start += i;//공백 수만큼 더해서 공백제거
				continue;
			}
			start += i;//다음 토큰으로 넘어가려고
		}
			
		strcpy(tokens[row], ltrim(rtrim(tokens[row])));//토큰의 공백을 제거해서 tokens[row]에 채우기

		 if(row > 0 && is_character(tokens[row][strlen(tokens[row]) - 1]) 
				&& (is_typeStatement(tokens[row - 1]) == 2 
					|| is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])
					|| tokens[row - 1][strlen(tokens[row - 1]) - 1] == '.' ) ){
			 //{...}{...}{gcc/데이터타입{문자/"."}}{문자}
			if(row > 1 && strcmp(tokens[row - 2],"(") == 0)//{"("}{...}{...}
			{//row>1
				if(strcmp(tokens[row - 1], "struct") != 0 && strcmp(tokens[row - 1],"unsigned") != 0)
					//{"("}{"struct"/"unsigned"}{...} 이게 아니면 false
					return false;
			}
			else if(row == 1 && is_character(tokens[row][strlen(tokens[row]) - 1])) {
				//{{문자}}{...}
				//row==1
				if(strcmp(tokens[0], "extern") != 0 && strcmp(tokens[0], "unsigned") != 0 && is_typeStatement(tokens[0]) != 2)	
					return false;//{"extern"/"unsigned",gcc/데이터타입{문자}}{...}이 아니면 false
			}
			else if(row > 1 && is_typeStatement(tokens[row - 1]) == 2){
				//{....}{...}{gcc/데이터타입}{...}
				if(strcmp(tokens[row - 2], "unsigned") != 0 && strcmp(tokens[row - 2], "extern") != 0)
					return false;//{...}[unsigned/extern}{gcc/데이터타입}{...}이 아니면 false
			}
			
		}

		if((row == 0 && !strcmp(tokens[row], "gcc")) ){//{gcc}
			clear_tokens(tokens);//토큰 클리어
			strcpy(tokens[0], str);//학생답안을 일단 tokens[0]에 복사
			return 1;
		} 

		row++;
	}

	if(all_star(tokens[row - 1]) && row > 1 && !is_character(tokens[row - 2][strlen(tokens[row - 2]) - 1]))  
		row--;	//{..}{{문자}}{***}{}이러면 포인터만 있는방을 가리킴			
	if(all_star(tokens[row - 1]) && row == 1)   //{***}{...} 이러면 포인터만 있는 방 가리킴
		row--;	

	for(i = 0; i < strlen(start); i++)   
	{
		if(start[i] == ' ')  //공백이라면
		{
			while(start[i] == ' ')//공백제거
				i++;
			if(start[0]==' ') {//처음부터 공백이 있다면
				start += i;//아예 앞에 공백 없애줌
				i = 0;
			}
			else
				row++;//아니면 그냥 행을 추가해줌
			
			i--;//앞에서 i++했던거를 무효
		} 
		else//공백이 아니라면
		{
			strncat(tokens[row], start + i, 1);//tokens에 문자 이어붙임
			if( start[i] == '.' && i<strlen(start)){//i번째가 .이면
				while(start[i + 1] == ' ' && i < strlen(start))//그 다음이 공백이면
					i++;//공백제거

			}
		}
		strcpy(tokens[row], ltrim(rtrim(tokens[row])));//tokens의 좌우공백 제거

		if(!strcmp(tokens[row], "lpthread") && row > 0 && !strcmp(tokens[row - 1], "-")){
			//{...}{-}{lpthread} 라면 (컴파일관련 옵션)
			strcat(tokens[row - 1], tokens[row]);//tokens[row-1]에 이어붙인다
			memset(tokens[row], 0, sizeof(tokens[row]));//tokens[row]를 비운다
			row--;
		}
		else if(row > 0 && is_character(tokens[row][strlen(tokens[row]) - 1]) 
				&& (is_typeStatement(tokens[row - 1]) == 2 
					|| is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])
					|| tokens[row - 1][strlen(tokens[row - 1]) - 1] == '.') ){
			//{gcc/데이터타입/{문자/'.'}}{{문자}}
			
			if(row > 1 && strcmp(tokens[row-2],"(") == 0)
			{//{"("}{}{}
				if(strcmp(tokens[row-1], "struct") != 0 && strcmp(tokens[row-1], "unsigned") != 0)
					return false;//{"("}{"struct","unsigned"}가 아니면 false
			}
			else if(row == 1 && is_character(tokens[row][strlen(tokens[row]) - 1])) {
				if(strcmp(tokens[0], "extern") != 0 && strcmp(tokens[0], "unsigned") != 0 && is_typeStatement(tokens[0]) != 2)	
					return false;//{extern,unsigned,gcc/데이터타입}{{문자}} 아니면 false
			}
			else if(row > 1 && is_typeStatement(tokens[row - 1]) == 2){
				if(strcmp(tokens[row - 2], "unsigned") != 0 && strcmp(tokens[row - 2], "extern") != 0)
					return false; //{unsigned,extern}{gcc/데이터타입}{...}이면 false
			}
		} 
	}


	if(row > 0)//행이 양수이면
	{
		if(strcmp(tokens[0], "#include") == 0 || strcmp(tokens[0], "include") == 0 || strcmp(tokens[0], "struct") == 0){ 
			//{"#include"/"include"/"struct"}{}
			clear_tokens(tokens); //토큰 비우기
			strcpy(tokens[0], remove_extraspace(str));//여분의 공백비운 후 tokens에 복사
		}
	}

	if(is_typeStatement(tokens[0]) == 2 || strstr(tokens[0], "extern") != NULL){
		//{gcc/datatype/extern}
		for(i = 1; i < TOKEN_CNT; i++){
			if(strcmp(tokens[i],"") == 0)  //비어있으면 break
				break;		       

			if(i != TOKEN_CNT -1 )
				strcat(tokens[0], " ");//공백을 추가
			strcat(tokens[0], tokens[i]);//tokens[0]에 tokens[i] 이어붙임
			memset(tokens[i], 0, sizeof(tokens[i]));//tokens[i]를 0으로 채움
		}
	}
	
	
	while((p_str = find_typeSpecifier(tokens)) != -1){ //형식지정자 찾는 함수
		if(!reset_tokens(p_str, tokens))//토큰 재배치
			return false;
	}

	
	while((p_str = find_typeSpecifier2(tokens)) != -1){  //구조체 형식지정자 찾는 함수
		if(!reset_tokens(p_str, tokens))//토큰 재배치
			return false;
	}
	
	return true;
}

node *make_tree(node *root, char (*tokens)[MINLEN], int *idx, int parentheses)
{
	node *cur = root;
	node *new;
	node *saved_operator;
	node *operator;
	int fstart;
	int i;

	while(1)	
	{
		if(strcmp(tokens[*idx], "") == 0)//공백이면 break
			break;
	
		if(!strcmp(tokens[*idx], ")"))//")"를 만나면
			return get_root(cur);//root구하기

		else if(!strcmp(tokens[*idx], ","))//","를 만나면
			return get_root(cur);//root구하기

		else if(!strcmp(tokens[*idx], "("))//"("를 만나면
		{
			
			if(*idx > 0 && !is_operator(tokens[*idx - 1]) && strcmp(tokens[*idx - 1], ",") != 0){
				//tokens[*idx-1]에 연산자가 아니고 ","이 있으면 fstart=true
				fstart = true;

				while(1)
				{
					*idx += 1;

					if(!strcmp(tokens[*idx], ")"))//tokens[*idx]가 )이면 break
						break;
					
					new = make_tree(NULL, tokens, idx, parentheses + 1);//공백을 불러올때까지 함수를 불러온다
					
					if(new != NULL){//공백이 아니면
						if(fstart == true){//여기서부터 시작.부모가됨
							cur->child_head = new;//cur의 자식은 new
							new->parent = cur;//new의 부모는 cur
	
							fstart = false;
						}
						else{//부모자식 관계가 아니게됨
							cur->next = new;
							new->prev = cur;
						}

						cur = new;//포인터를 커서말단 부분으로 옮겨줌
					}

					if(!strcmp(tokens[*idx], ")"))//tokens에 ")"가 나타나면 반복문 멈춤
						break;
				}
			}
			else{//tokens[*idx]가 "("를 만났지만 이외의 상황
				*idx += 1;
	
				new = make_tree(NULL, tokens, idx, parentheses + 1);//다음 tokens에 대한 make_tree함수를 불러온다

				if(cur == NULL)
					cur = new;//cur포인터가 new를 가리킨다

				else if(!strcmp(new->name, cur->name)){//new->name과 cur->name이 같을때(같은 연산자)
					if(!strcmp(new->name, "|") || !strcmp(new->name, "||") 
						|| !strcmp(new->name, "&") || !strcmp(new->name, "&&"))//new->name이 이와 같을때
					{
						cur = get_last_child(cur);//cur의 마지막 자식을 cur이 가리키도록

						if(new->child_head != NULL){//new의 자식이 있다면 자식을 없애버림
							new = new->child_head;//new의 자식이 new 됨

							new->parent->child_head = NULL;
							new->parent = NULL;
							new->prev = cur;
							cur->next = new;
						}
					}
					else if(!strcmp(new->name, "+") || !strcmp(new->name, "*"))//new->name에 "+","*"가 있다면
					{
						i = 0;

						while(1)
						{
							if(!strcmp(tokens[*idx + i], ""))//공백이면 멈춤
								break;

							if(is_operator(tokens[*idx + i]) && strcmp(tokens[*idx + i], ")") != 0)//tokens[*idx+i]가 1순위 연산자이고 ")"가 없으면 멈춤
								break;

							i++;
						}
						
						if(get_precedence(tokens[*idx + i]) < get_precedence(new->name))//tokens[*idx+i]의 연산자 우선순위 높을때
						{
							cur = get_last_child(cur);//cur의 마지막 자식을 cur이 가리킴
							cur->next = new;
							new->prev = cur;
							cur = new;//추가된 new를 cur이 가리키도록함
						}
						else//tokens[*idx+i]의 우선순위가 더 낮을때
						{
							cur = get_last_child(cur);//cur의 마지막자식을 cur이 가리킴

							if(new->child_head != NULL){//new의 자식이 있다면
								new = new->child_head;//자식 노드로 이동

								new->parent->child_head = NULL;//new의 부모의 자식을 없앰
								new->parent = NULL;//new의 부모도 없앰
								new->prev = cur;//new의 이전노드를 cur
								cur->next = new;//cur의 다음노드를 new
							}
						}
					}
					else{
						cur = get_last_child(cur);//cur의 마지마자식을 가리킴
						cur->next = new;//cur->next는 new
						new->prev = cur;//new->prev는 cur
						cur = new;//cur이 말단노드 new를 가리킴
					}
				}
	
				else//이외에
				{
					cur = get_last_child(cur);//cur의 마지마자식을 가리킴

					cur->next = new;//cur->next는 new
					new->prev = cur;//new->prev는 cur
	
					cur = new;//cur이 말단노드 new를 가리킴
				}
			}
		}
		else if(is_operator(tokens[*idx]))//tokens[*idx]가 연산자일경우
		{
			if(!strcmp(tokens[*idx], "||") || !strcmp(tokens[*idx], "&&")
					|| !strcmp(tokens[*idx], "|") || !strcmp(tokens[*idx], "&") 
					|| !strcmp(tokens[*idx], "+") || !strcmp(tokens[*idx], "*"))
			{//만약 이런 연산자일경우에는
				if(is_operator(cur->name) == true && !strcmp(cur->name, tokens[*idx]))
					operator = cur;//cur->name이 연산자이고 cur->name과 tokens[*idx]가 같으면 operator는 cur를 가리킴
		
				else
				{//이외에
					new = create_node(tokens[*idx], parentheses);//새로운 노드를 만듬
					operator = get_most_high_precedence_node(cur, new);//cur과 new 중에 우선순위 제일높은 연산자를 operator에 넣음

					if(operator->parent == NULL && operator->prev == NULL){//operator 부모노드가 없고 이전노드도 없으면

						if(get_precedence(operator->name) < get_precedence(new->name)){
							cur = insert_node(operator, new);//operator우선순위가 더 클때 new를 삽입
						}

						else if(get_precedence(operator->name) > get_precedence(new->name))
						{//operator 우선순위가 더 낮을때
							if(operator->child_head != NULL){//operator 자식노드가 있을때
								operator = get_last_child(operator);//operator의 맨끝 자식노드 가리킨
								cur = insert_node(operator, new);//new를 operator에 삽입하고 new를 가리킴
							}
						}
						else//operator와 new의 우선순위가 같을때
						{
							operator = cur;//operator는 cur를 카리킴
	
							while(1)
							{
								if(is_operator(operator->name) == true && !strcmp(operator->name, tokens[*idx]))
									break;//operator가 연산자이고 tokens[*idx]가 operator와 같으면 멈춤
						
								if(operator->prev != NULL)//operator이전노드가 존재
									operator = operator->prev;//operator는 operator->prev를 가리킴
								else//이전노드가 존재하지 않으면 멈춤
									break;
							}

							if(strcmp(operator->name, tokens[*idx]) != 0)//operator와 tokens[*idx]가 다르면
								operator = operator->parent;//operator->parent를 가리킴

							if(operator != NULL){//operator가 공백이 아니면
								if(!strcmp(operator->name, tokens[*idx]))
									//operator->name과 tokens[*idx]가 같으면 cur는 operator가 됨
									cur = operator;
							}
						}
					}

					else//operator의 부모노드가 존재하거나 operator의 이전이 존재하면
						cur = insert_node(operator, new);//new 노드를 삽입함
				}

			}
			else//위에 연산자가 아닌경우에
			{
				new = create_node(tokens[*idx], parentheses);//새로운 노드 생성함수

				if(cur == NULL)
					cur = new;//cur이 새로운 노드가 됨

				else//cur에 뭐라도 있을 때
				{
					operator = get_most_high_precedence_node(cur, new);//가장 높은 우선순위가진 연산자

					if(operator->parentheses > new->parentheses)//operator의 괄호가 new보다 많으면 노드를 추가
						cur = insert_node(operator, new);//operator 전에 new를 삽입

					else if(operator->parent == NULL && operator->prev ==  NULL){//operator의 부모, 이전노드가 없으면
					
						if(get_precedence(operator->name) > get_precedence(new->name))//operator의 우선순위가 더 낮을때
						{
							if(operator->child_head != NULL){//operator 자식노드가 있을때
	
								operator = get_last_child(operator);//operator의 마지막 자식을 operator로 지정(맨끝)
								cur = insert_node(operator, new);//operator 전에 new 삽입
							}
						}
					
						else	//operator 우선순위가 더 높을때
							cur = insert_node(operator, new);//new를 operator이전에 삽입
					}
	
					else
						cur = insert_node(operator, new);//new를 operator 이전에 삽입
				}
			}
		}
		else //그외에
		{
			new = create_node(tokens[*idx], parentheses);//새로운 노드 생성

			if(cur == NULL)
				cur = new;//cur이 새로운 노드됨

			else if(cur->child_head == NULL){//cur의 자식이 없다면 cur의 자식이 new가 된다.
				cur->child_head = new;
				new->parent = cur;

				cur = new;//포인터가 새로운 말단노드를 가리키도록함
			}
			else{//그외에

				cur = get_last_child(cur);//cur의 가장 마지막 자식이 cur가 됨

				cur->next = new;//cur다음은 new가 됨
				new->prev = cur;//new이전은 cur

				cur = new;//포인터가 새로운 말단노드를 가리키도록함
			}
		}

		*idx += 1;//token이 공백이 될때까지 반복
	}

	return get_root(cur);
}

node *change_sibling(node *parent)//형제자매 노드를 바꿈
{
	node *tmp;
	
	tmp = parent->child_head;//childhead 를 가리킴

	parent->child_head = parent->child_head->next;//parent->childhead를 parent->childhead->next로 바꿈
	parent->child_head->parent = parent;//그 바꾼 childhead->parent를 parent로 정함
	parent->child_head->prev = NULL;//childhead->prev를 비움

	parent->child_head->next = tmp;//childhead->next를 이전의 childhead인 tmp로 변경
	parent->child_head->next->prev = parent->child_head;
	parent->child_head->next->next = NULL;//tmp->next는 공백
	parent->child_head->next->parent = NULL;//tmp->parent는 공백

	return parent;//parent를 리턴
}

node *create_node(char *name, int parentheses)//새로운노드생성함수
{
	node *new;

	new = (node *)malloc(sizeof(node));//새로운 노드 생성
	new->name = (char *)malloc(sizeof(char) * (strlen(name) + 1));//node new의 이름 생성
	strcpy(new->name, name);//node new의 이름에 name을 복사

	new->parentheses = parentheses;//괄호 쌍
	new->parent = NULL;
	new->child_head = NULL;
	new->prev = NULL;
	new->next = NULL;

	return new;
}

int get_precedence(char *op)//우선순위 얻는 함수
{
	int i;

	for(i = 2; i < OPERATOR_CNT; i++){
		if(!strcmp(operators[i].operator, op))//operators[i].operator와 입력받은 연산자가 같다면
			return operators[i].precedence;//우선순위 리턴
	}
	return false;
}

int is_operator(char *op)//연산자인지 아닌지
{
	int i;

	for(i = 0; i < OPERATOR_CNT; i++)//연산자 개수만큼 반복
	{
		if(operators[i].operator == NULL)
			break;
		if(!strcmp(operators[i].operator, op)){//operators[i]의 연산자와 입력받은 연산자가 같으면 true
			return true;
		}
	}

	return false;
}

void print(node *cur)
{
	if(cur->child_head != NULL){
		print(cur->child_head);
		printf("\n");
	}

	if(cur->next != NULL){
		print(cur->next);
		printf("\t");
	}
	printf("%s", cur->name);
}

node *get_operator(node *cur)
{
	if(cur == NULL)
		return cur;

	if(cur->prev != NULL)
		while(cur->prev != NULL)
			cur = cur->prev;

	return cur->parent;
}

node *get_root(node *cur)//root룰 구하는 함수
{
	if(cur == NULL)//현재 아무것도 없을때
		return cur;

	while(cur->prev != NULL)//현재에서 처음으로 이동
		cur = cur->prev;

	if(cur->parent != NULL)//부모 노드로 이동
		cur = get_root(cur->parent);//root까지 가기위해 함수를 불러옴

	return cur;
}

node *get_high_precedence_node(node *cur, node *new)//높은 우선순위 노드를 얻음
{
	if(is_operator(cur->name))//연산자이면 
		if(get_precedence(cur->name) < get_precedence(new->name))//숫자가 작을수록 우선순위 높음
			return cur;

	if(cur->prev != NULL){
		while(cur->prev != NULL){
			cur = cur->prev;//이전 노드가 없을때까지 반복
			
			return get_high_precedence_node(cur, new);//다시 new와 우선순위를 측정
		}


		if(cur->parent != NULL)
			return get_high_precedence_node(cur->parent, new);//cur->parent하고 new의 우선순위 측정
	}

	if(cur->parent == NULL)//부모가 없을경우 cur리턴
		return cur;
}

node *get_most_high_precedence_node(node *cur, node *new)//가장높은 우선순위 노드를 얻는 함수
{
	node *operator = get_high_precedence_node(cur, new);//높은 우선순위를 가진 연산자 저장
	node *saved_operator = operator;//그 연산자를 saved_operator에 저장

	while(1)
	{
		if(saved_operator->parent == NULL)//부모 노드가 없을경우 멈춤
			break;

		if(saved_operator->prev != NULL)//전에 노드가 있으면 그 전에 노드와 다시 우선순위비교
			operator = get_high_precedence_node(saved_operator->prev, new);

		else if(saved_operator->parent != NULL)//부모노드가 있다면 다시 우선순위비교
			operator = get_high_precedence_node(saved_operator->parent, new);

		saved_operator = operator;//제일 높은 우선순위를 가진 연산자를 알수있다
	}
	
	return saved_operator;
}

node *insert_node(node *old, node *new)//새로운 노드 삽입함수(new가 prev와 old 사이에 삽입)
{
	if(old->prev != NULL){//old->prev가 존재한다면 new가 old를 대신
		new->prev = old->prev;
		old->prev->next = new;
		old->prev = NULL;
	}

	new->child_head = old;//new의 자식노드 old
	old->parent = new;//old의 부모노드 new

	return new;
}

node *get_last_child(node *cur)
{
	if(cur->child_head != NULL)//cur의 자식노드를 cur로 지정
		cur = cur->child_head;

	while(cur->next != NULL)//cur의 다음노드가 없을때까지 반복
		cur = cur->next;

	return cur;//그 마지막 자식을 리턴
}

int get_sibling_cnt(node *cur)//형제자매 노드를 얻는함수
{
	int i = 0;

	while(cur->prev != NULL)//이전노드가 있으면
		cur = cur->prev;//cur은 이전노드를 가리킴. 이전노드가 더 이상 존재하지 않을때까지 반복

	while(cur->next != NULL){//다음 노드가 있으면
		cur = cur->next;//cur은 다음노드를 가리킴
		i++;//제일 prev끝에서 제일 next끝만큼의 node 저장. siblings 수를 알수있음
	}

	return i;//sibling 리턴
}

void free_node(node *cur)
{
	if(cur->child_head != NULL)
		free_node(cur->child_head);

	if(cur->next != NULL)
		free_node(cur->next);

	if(cur != NULL){
		cur->prev = NULL;
		cur->next = NULL;
		cur->parent = NULL;
		cur->child_head = NULL;
		free(cur);
	}
}


int is_character(char c)//숫자,알파벳인지를 판별하는 함수
{
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int is_typeStatement(char *str)//타입상태표시
{ 
	char *start;
	char str2[BUFLEN] = {0}; 
	char tmp[BUFLEN] = {0}; 
	char tmp2[BUFLEN] = {0}; 
	int i;	 
	
	start = str;
	strncpy(str2,str,strlen(str));//str에서 길이만큼 str2 복사
	remove_space(str2);//공백을 지움

	while(start[0] == ' ')//str 시작부터 공백이면 다음문자로 넘어감
		start += 1;

	if(strstr(str2, "gcc") != NULL)//str2에서 gcc를 찾는다
	{
		strncpy(tmp2, start, strlen("gcc"));//start에서 "gcc"크기만큼 tmp2에 복사
		if(strcmp(tmp2,"gcc") != 0)//tmp2가 "gcc"랑 다르면 0리턴
			return 0;
		else
			return 2;//같으면 2리턴
	}
	
	for(i = 0; i < DATATYPE_SIZE; i++)
	{
		if(strstr(str2,datatype[i]) != NULL)//str2와 데이터타입이 같은것이 있을때
		{	
			strncpy(tmp, str2, strlen(datatype[i]));//tmp에 str2를 복사
			strncpy(tmp2, start, strlen(datatype[i]));//tmp2에 start를 복사
			
			if(strcmp(tmp, datatype[i]) == 0)//tmp와 데이터타입이 문자열이 같다면
				if(strcmp(tmp, tmp2) != 0)//tmp와 tmp2가 다르면 -> 0
					return 0;  
				else//같으면 -> 2
					return 2;
		}

	}
	return 1;

}

int find_typeSpecifier(char tokens[TOKEN_CNT][MINLEN]) //형식지정자 찾는 함수
{
	int i, j;

	for(i = 0; i < TOKEN_CNT; i++)
	{
		for(j = 0; j < DATATYPE_SIZE; j++)
		{
			if(strstr(tokens[i], datatype[j]) != NULL && i > 0)//tokens에서 datatype 찾기
			{
				if(!strcmp(tokens[i - 1], "(") && !strcmp(tokens[i + 1], ")") 
						&& (tokens[i + 2][0] == '&' || tokens[i + 2][0] == '*' 
							|| tokens[i + 2][0] == ')' || tokens[i + 2][0] == '(' 
							|| tokens[i + 2][0] == '-' || tokens[i + 2][0] == '+' 
							|| is_character(tokens[i + 2][0])))  
					return i;//이런 조건을 만족하는 i를 리턴
			}
		}
	}
	return -1;
}

int find_typeSpecifier2(char tokens[TOKEN_CNT][MINLEN]) //구조체 형식지정자 찾는 함수
{
    int i, j;

   
    for(i = 0; i < TOKEN_CNT; i++)
    {
        for(j = 0; j < DATATYPE_SIZE; j++)
        {
            if(!strcmp(tokens[i], "struct") && (i+1) <= TOKEN_CNT && is_character(tokens[i + 1][strlen(tokens[i + 1]) - 1]))  
                    return i;
        }
    }
    return -1;
}

int all_star(char *str)//포인터가 있으면 1 리턴
{
	int i;
	int length= strlen(str);
	
 	if(length == 0)	
		return 0;
	
	for(i = 0; i < length; i++)
		if(str[i] != '*')
			return 0;
	return 1;

}

int all_character(char *str)
{
	int i;

	for(i = 0; i < strlen(str); i++)
		if(is_character(str[i]))
			return 1;
	return 0;
	
}

int reset_tokens(int start, char tokens[TOKEN_CNT][MINLEN]) //토큰 재배치하는 함수
{
	int i;
	int j = start - 1;
	int lcount = 0, rcount = 0;
	int sub_lcount = 0, sub_rcount = 0;

	if(start > -1){
		if(!strcmp(tokens[start], "struct")) {		//find_typesSpecifier2에 적합
			strcat(tokens[start], " ");//tokens에 공백추가
			strcat(tokens[start], tokens[start+1]);	 //그 다음꺼 이어붙이기    

			for(i = start + 1; i < TOKEN_CNT - 1; i++){
				strcpy(tokens[i], tokens[i + 1]);//하나씩 앞당겨진다
				memset(tokens[i + 1], 0, sizeof(tokens[0]));//다음 토큰은 0으로 채워진다
			}
		}

		else if(!strcmp(tokens[start], "unsigned") && strcmp(tokens[start+1], ")") != 0) {
			//unsigned와 )로 이루어져있는경우
			strcat(tokens[start], " ");
			strcat(tokens[start], tokens[start + 1]);	     
			strcat(tokens[start], tokens[start + 2]);
			//공백추가. +2칸까지 start에 이어붙이기
			for(i = start + 1; i < TOKEN_CNT - 1; i++){
				strcpy(tokens[i], tokens[i + 1]);//하나씩 앞당겨짐
				memset(tokens[i + 1], 0, sizeof(tokens[0]));//그다음꺼 0으로 채움
			}
		}

     		j = start + 1;           
        	while(!strcmp(tokens[j], ")")){//)가 있으면
                	rcount ++;//오른쪽 괄호 변수 증가
                	if(j==TOKEN_CNT)
                        	break;
                	j++;
        	}
	
		j = start - 1;
		while(!strcmp(tokens[j], "(")){//(가 있으면
        	        lcount ++;//좌괄호 변수 증가
                	if(j == 0)
                        	break;
               		j--;
		}
		if( (j!=0 && is_character(tokens[j][strlen(tokens[j])-1]) ) || j==0)
			lcount = rcount;//괄호쌍이 맞아야됨

		if(lcount != rcount )
			return false;

		if( (start - lcount) >0 && !strcmp(tokens[start - lcount - 1], "sizeof")){
			return true; //처음부터 괄호시작안하는거는 sizeof가 있을수도
		}
		
		else if((!strcmp(tokens[start], "unsigned") || !strcmp(tokens[start], "struct")) && strcmp(tokens[start+1], ")")) {		
			strcat(tokens[start - lcount], tokens[start]);//왼쪽괄호수만큼 빼서 start 토큰 이어붙이기
			strcat(tokens[start - lcount], tokens[start + 1]);//start+1토큰까지 이어붙이기
			strcpy(tokens[start - lcount + 1], tokens[start + rcount]);//start-lcount+1에다가 오른쪽 괄호수 넘어서 토큰 집어넣기
		 
			for(int i = start - lcount + 1; i < TOKEN_CNT - lcount -rcount; i++) {
				//괄호뺀 수만큼만 반복
				strcpy(tokens[i], tokens[i + lcount + rcount]);//원래 크기의 토큰을 줄어든 i번째 토큰에 복사
				memset(tokens[i + lcount + rcount], 0, sizeof(tokens[0]));//원래토큰은 0으로 채워짐
			}


		}
 		else{//그외에
			if(tokens[start + 2][0] == '('){
				j = start + 2;
				while(!strcmp(tokens[j], "(")){
					sub_lcount++;//왼쪽 괄호 수 증가
					j++;//다음 토큰으로 이동
				} 	
				if(!strcmp(tokens[j + 1],")")){
					j = j + 1;
					while(!strcmp(tokens[j], ")")){
						sub_rcount++;//오른쪽 괄호 수 증가
						j++;//다음 토큰으로 이동
					}
				}
				else 
					return false;

				if(sub_lcount != sub_rcount)//괄호 짝이 안맞으면 false
					return false;
				
				strcpy(tokens[start + 2], tokens[start + 2 + sub_lcount]);	//좌괄호 이후의 내용을 복사
				for(int i = start + 3; i<TOKEN_CNT; i++)
					memset(tokens[i], 0, sizeof(tokens[0]));//0으로 채우기

			}
			strcat(tokens[start - lcount], tokens[start]);//좌괄호뺀값만큼 토큰에 start토큰 이어붙이기
			strcat(tokens[start - lcount], tokens[start + 1]);//start+1 토큰도 이어붙이기
			strcat(tokens[start - lcount], tokens[start + rcount + 1]);//우괄호뺸값만큼 토큰에 이어붙이기
		 
			for(int i = start - lcount + 1; i < TOKEN_CNT - lcount -rcount -1; i++) {
				strcpy(tokens[i], tokens[i + lcount + rcount +1]);//이후의 토큰에는 우괄호 이후의 내용을 붙인다
				memset(tokens[i + lcount + rcount + 1], 0, sizeof(tokens[0]));//0으로 채움

			}
		}
	}
	return true;
}

void clear_tokens(char tokens[TOKEN_CNT][MINLEN])//토큰 삭제함수
{
	int i;

	for(i = 0; i < TOKEN_CNT; i++)
		memset(tokens[i], 0, sizeof(tokens[i]));//i번째 토큰을 0으로 채움
}

char *rtrim(char *_str)
{
	char tmp[BUFLEN];
	char *end;

	strcpy(tmp, _str);
	end = tmp + strlen(tmp) - 1;
	while(end != _str && isspace(*end))
		--end;

	*(end + 1) = '\0';
	_str = tmp;
	return _str;
}

char *ltrim(char *_str)
{
	char *start = _str;

	while(*start != '\0' && isspace(*start))
		++start;
	_str = start;
	return _str;
}

char* remove_extraspace(char *str)//여분의 공간을 없애는 함수
{
	int i;
	char *str2 = (char*)malloc(sizeof(char) * BUFLEN);
	char *start, *end;
	char temp[BUFLEN] = "";
	int position;

	if(strstr(str,"include<")!=NULL){//"include<"가 있으면
		start = str;
		end = strpbrk(str, "<");//"<"시작되는 부분
		position = end - start;//"include"부분
	
		strncat(temp, str, position);//temp에 복사
		strncat(temp, " ", 1);//공백추가
		strncat(temp, str + position, strlen(str) - position + 1);//include를 뺀 나머지를 이어붙임.

		str = temp;		//다시 str로 복사
	}
	
	for(i = 0; i < strlen(str); i++)
	{
		if(str[i] ==' ')//공백이 있으면
		{
			if(i == 0 && str[0] ==' ')//처음부터 이어지는 공백제거
				while(str[i + 1] == ' ')
					i++;	
			else{//처음부터 공백은 아님
				if(i > 0 && str[i - 1] != ' ')//한칸 앞이 공백이 아니면
					str2[strlen(str2)] = str[i];//str2에 공백추가
				while(str[i + 1] == ' ')
					i++;//공백의 갯수만큼 ++
			} 
		}
		else//공백이 없으면
			str2[strlen(str2)] = str[i];//i칸에는 이미 공백이 없다. str[i]를 str2에 복사
	}

	return str2;
}



void remove_space(char *str)//공간 없애는 함수
{
	char* i = str;
	char* j = str;
	
	while(*j != 0)//문자열이 끝날때까지
	{
		*i = *j++;
		if(*i != ' ')//공백을 만나면 i는 이동하지 않는다
			i++;
	}
	*i = 0;
}

int check_brackets(char *str)//괄호검사하는 함수
{
	char *start = str;
	int lcount = 0, rcount = 0;
	
	while(1){
		if((start = strpbrk(start, "()")) != NULL){//start의 문자들 중 "()"와 일치하는 문자 가리킴.
			//첫번째로 일치하는 문자를 가리킨다
			if(*(start) == '(')
				lcount++;//"(" 수
			else
				rcount++;//")" 수

			start += 1;//가리키는 문자를 한칸씩 이동 		
		}
		else
			break;
	}

	if(lcount != rcount)//괄호 쌍이 맞지 않을때
		return 0;
	else 
		return 1;
}

int get_token_cnt(char tokens[TOKEN_CNT][MINLEN])
{
	int i;
	
	for(i = 0; i < TOKEN_CNT; i++)
		if(!strcmp(tokens[i], ""))
			break;

	return i;
}
