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


operator_precedence operators[OPERATOR_CNT] = {//������ �켱���� �迭
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

void compare_tree(node *root1,  node *root2, int *result)//Ʈ���� ���ϴ� �Լ�
{
	node *tmp;
	int cnt1, cnt2;

	if(root1 == NULL || root2 == NULL){//root1,root2�� �����϶� result�� false
		*result = false;
		return;
	}

	if(!strcmp(root1->name, "<") || !strcmp(root1->name, ">") || !strcmp(root1->name, "<=") || !strcmp(root1->name, ">=")){
		//root1�� <,>,<=,>= �϶�
		if(strcmp(root1->name, root2->name) != 0){
			//root1�� root2�� ���� ������
			if(!strncmp(root2->name, "<", 1))//root2�� <�̸� >�� �����Ѵ�
				strncpy(root2->name, ">", 1);

			else if(!strncmp(root2->name, ">", 1))//root2�� >�̸� <�� �����Ѵ�
				strncpy(root2->name, "<", 1);

			else if(!strncmp(root2->name, "<=", 2))//root2�� <=�̸� >=�� �����Ѵ�
				strncpy(root2->name, ">=", 2);

			else if(!strncmp(root2->name, ">=", 2))//root2 >=�̸� <=�� �����Ѵ�
				strncpy(root2->name, "<=", 2);

			root2 = change_sibling(root2);//root2�� childhead�� childhead->next�� ����
		}
	}

	if(strcmp(root1->name, root2->name) != 0){//root1�� root2�� ���������� false
		*result = false;
		return;
	}

	if((root1->child_head != NULL && root2->child_head == NULL)
		|| (root1->child_head == NULL && root2->child_head != NULL)){
		//root1�� root2 �� �ϳ��� childhead�� �ִٸ� false
		*result = false; 
		return;
	}

	else if(root1->child_head != NULL){//root1->childhead�� ����, root2->childhead ����
		if(get_sibling_cnt(root1->child_head) != get_sibling_cnt(root2->child_head)){
			//root1,root2�� sibling���� �ٸ��� false
			*result = false;
			return;
		}

		if(!strcmp(root1->name, "==") || !strcmp(root1->name, "!="))//root1�� ==,!=�϶�
		{
			compare_tree(root1->child_head, root2->child_head, result);
			//root1,root2 tree�� �ٽ� ��
			if(*result == false)//���� result�� false�ϰ��
			{
				*result = true;//true�� �ٲ��ְ� 
				root2 = change_sibling(root2);//childhead�� childhead->next�� �ٲ���
				compare_tree(root1->child_head, root2->child_head, result);//�ٽ� root1->childhead, root2->childhead��
			}
		}
		else if(!strcmp(root1->name, "+") || !strcmp(root1->name, "*")
				|| !strcmp(root1->name, "|") || !strcmp(root1->name, "&")
				|| !strcmp(root1->name, "||") || !strcmp(root1->name, "&&"))
		{//root1�� ������ ���� ������ �߿� ������
			if(get_sibling_cnt(root1->child_head) != get_sibling_cnt(root2->child_head)){
				//root1,root2�� childhead�� �����ڸż��� �ٸ��� false
				*result = false;
				return;
			}

			tmp = root2->child_head;//root2->childhood�� tmp�� ����

			while(tmp->prev != NULL)//tmp->prev�� ������ tmp->prev�� tmp�� ����.
				tmp = tmp->prev;//tmp->prev�� �������� ���������� �ݺ�

			while(tmp != NULL)//tmp�� ������ �ƴϸ�
			{
				compare_tree(root1->child_head, tmp, result);//root1->child_head�� tmp Ʈ���� ��
			
				if(*result == true)//true�� ������ �׸�
					break;
				else{//flase���
					if(tmp->next != NULL)//tmp->next�� �����ϸ� true�� �����ϰ� tmp->next�� tmp�� ����
						*result = true;
					tmp = tmp->next;
				}
			}
		}
		else{//�̿ܿ�
			compare_tree(root1->child_head, root2->child_head, result);//root1->childhead,root2->childhead Ʈ�� ��
		}
	}	


	if(root1->next != NULL){//root1->next�� ������ �ƴϸ�

		if(get_sibling_cnt(root1) != get_sibling_cnt(root2)){//root1,root2�� �����ڸż��� �ٸ��� false
			*result = false;
			return;
		}

		if(*result == true)//result�� true�̸�
		{
			tmp = get_operator(root1);//root1�� �����ڸ� tmp�� ����
	
			if(!strcmp(tmp->name, "+") || !strcmp(tmp->name, "*")
					|| !strcmp(tmp->name, "|") || !strcmp(tmp->name, "&")
					|| !strcmp(tmp->name, "||") || !strcmp(tmp->name, "&&"))
			{	//tmp�� �̷� �����ڶ��
				tmp = root2;//root2�� tmp�� �ֱ�
	
				while(tmp->prev != NULL)
					tmp = tmp->prev;//tmp->prev�� tmp�� ����. prev�� ���̻� ���������� ����

				while(tmp != NULL)//tmp�� ������ �ƴҶ�����
				{
					compare_tree(root1->next, tmp, result);//root1->next�� tmpƮ���� ��

					if(*result == true)//result�� true�̸� �׸�
						break;
					else{
						if(tmp->next != NULL)//result�� false���� tmp->next�� �����ϸ� true�� �ٲ���
							*result = true;
						tmp = tmp->next;//tmp->next�� tmp�� �־���
					}
				}
			}

			else//���� �����ڰ� �ƴ϶�� 
				compare_tree(root1->next, root2->next, result);//root1->next,root2->next Ʈ���� ��
		}
	}
}

int make_tokens(char *str, char tokens[TOKEN_CNT][MINLEN])//��ū ����� �Լ�
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
	
	clear_tokens(tokens);//��ū�� ���

	start = str;
	
	if(is_typeStatement(str) == 0) //�л� ����� Ÿ�Կ� ���� ���� �ƴ϶�� ��ū �ȸ���
		return false;	
	
	while(1)
	{
		if((end = strpbrk(start, op)) == NULL)//�л���Ȱ� ��ū���� �߿� ��ġ�ϴ� ���ڰ� ���ٸ� ��ū �ȸ���
			break;

		if(start == end){//ó������ �����ڰ� �ִ°��

			if(!strncmp(start, "--", 2) || !strncmp(start, "++", 2)){//ó���� "--"�� "++"�� �������
				if(!strncmp(start, "++++", 4)||!strncmp(start,"----",4))//ó���� "----"�� "++++"�� �������
					return false;

				if(is_character(*ltrim(start + 2))){//"--"�� "++" 2����Ʈ �ڷΰ��� ���������� �Ǻ�
					if(row > 0 && is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1]))
						//���� ����̰� ��ū�� �����϶�
						return false; 

					end = strpbrk(start + 2, op);//2����Ʈ �ڷΰ� ���ڿ��� op ���ڿ� ù��°�� ��ġ�� ��
					if(end == NULL)//��ū�� ������
						end = &str[strlen(str)];//end�� ����� ������ ���� ����Ŵ
					while(start < end) {//start���� ������ū/������ ������ �ݺ�
						if(*(start - 1) == ' ' && is_character(tokens[row][strlen(tokens[row]) - 1]))
							//start-1�� ��ĭ�� �����ϰ� ��ū�� ����, ���ĺ��� �����Ұ��
							return false;
						else if(*start != ' ')//start�� ��ĭ�� �ƴѰ��
							strncat(tokens[row], start, 1);//��ū�� �л���������� ù��°�� ���� ��ū�� ����ִ´�
						start++;//��ĭ �ڷΰ���	
					}
				}//ó��("++","--")���� ���� ��ū�� ������ ������ tokens�� ����ִ´�
				
				else if(row>0 && is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])){
					//tokens�� ���� ����̰� ���ڰ� ��������
					if(strstr(tokens[row - 1], "++") != NULL || strstr(tokens[row - 1], "--") != NULL)	
						//"++"/"--"�� ������ false
						return false;

					memset(tmp, 0, sizeof(tmp));//tmp�� 0�� ä��
					strncpy(tmp, start, 2);//tmp�� start�� �ִ� 2����Ʈ ä�� ("++"."--")
					strcat(tokens[row - 1], tmp);//tokens�� tmp�̾��
					start += 2;//2ĭ �ڷ�
					row--;
				}
				else{
					memset(tmp, 0, sizeof(tmp));//tmp�� 0���� ä��
					strncpy(tmp, start, 2);//tmp�� start 2����Ʈ ä��
					strcat(tokens[row], tmp);//tokens�� tmp��ġ��
					start += 2;//2ĭ �ڷ�
				}
			}

			else if(!strncmp(start, "==", 2) || !strncmp(start, "!=", 2) || !strncmp(start, "<=", 2)
				|| !strncmp(start, ">=", 2) || !strncmp(start, "||", 2) || !strncmp(start, "&&", 2) 
				|| !strncmp(start, "&=", 2) || !strncmp(start, "^=", 2) || !strncmp(start, "!=", 2) 
				|| !strncmp(start, "|=", 2) || !strncmp(start, "+=", 2)	|| !strncmp(start, "-=", 2) 
				|| !strncmp(start, "*=", 2) || !strncmp(start, "/=", 2)){//�񱳿����ڰ� ������

				strncpy(tokens[row], start, 2);//start���� 2 ����Ʈ tokens�� ����
				start += 2;//start 2ĭ �ڷ�
			}
			else if(!strncmp(start, "->", 2))//"->"�� ������
			{
				end = strpbrk(start + 2, op);//2ĭ�ڷΰ��� op�� ��ġ�ϴ� ����Ȯ��

				if(end == NULL)//��ġ�ϴ� �� ������
					end = &str[strlen(str)];//�� ����� ���ڿ��� ���� ����Ŵ

				while(start < end){//���ڿ��� ��/ ���� ��ū����
					if(*start != ' ')
						strncat(tokens[row - 1], start, 1);//tokens�� ���ڸ� ����
					start++;
				}
				row--;
			}
			else if(*end == '&')//�����ڰ� '&'�� ���
			{
				
				if(row == 0 || (strpbrk(tokens[row - 1], op) != NULL)){//row�� 0�̰ų� ������ op�� ��ġ�ϴ°� �ִٸ�
					end = strpbrk(start + 1, op);//'&'�� �����ϰ� ���� op��ġ�� �ִ��� Ȯ��
					if(end == NULL)//op��ġ�� ������
						end = &str[strlen(str)];//end�� ���ڿ��� ���� ����Ŵ
					
					strncat(tokens[row], start, 1);//tokens�� '&'�� ����
					start++;

					while(start < end){//���� ��ū/���ڿ� ������� �ݺ�
						if(*(start - 1) == ' ' && tokens[row][strlen(tokens[row]) - 1] != '&')//��ĭ���� ��ĭ�̰� tokens�� '&'�� ���ٸ�
							return false;
						else if(*start != ' ')//��ĭ�ƴ� ���
							strncat(tokens[row], start, 1);//tokens�� ���� �Է�
						start++;
					}
				}
				
				else{//�׿ܿ�
					strncpy(tokens[row], start, 1);//tokens�� '&'�Է�
					start += 1;
				}
				
			}
		  	else if(*end == '*')//'*'�� ���
			{
				isPointer=0;

				if(row > 0)//���� ����϶�
				{
					
					for(i = 0; i < DATATYPE_SIZE; i++) {
						if(strstr(tokens[row - 1], datatype[i]) != NULL){//������ �ڿ� ������Ÿ�� ã��
							strcat(tokens[row - 1], "*");//�����͸� tokens�� �ֱ�
							start += 1;	
							isPointer = 1;//���������� ǥ��
							break;
						}
					}
					if(isPointer == 1)
						continue;
					if(*(start+1) !=0)
						end = start + 1;//������ �ڿ� ���ڸ� ����Ű���� end����

					
					if(row>1 && !strcmp(tokens[row - 2], "*") && (all_star(tokens[row - 1]) == 1)){
						//row�� 1���� ũ�� tokens[row-2]�� "*"�̰� tokens[row-1]�� "*"�θ� �̷����������
						strncat(tokens[row - 1], start, end - start);//tokens�� start���ڸ� �ִ´�
						row--;
					}
					
					
					else if(is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1]) == 1){ 
						//�ٷγ� tokens�� ����/���ĺ��� �����Ѵٸ�
						strncat(tokens[row], start, end - start);  //���� �� tokens�� ���ڿ� �ֱ�
					}

					
					else if(strpbrk(tokens[row - 1], op) != NULL){	//tokens���� op�� ������ ���ڵ� ã��	
						strncat(tokens[row] , start, end - start);//���� �� tokens�� ���ڿ� �ֱ� 
							
					}
					else
						strncat(tokens[row], start, end - start);//������ tokens�� ���ڿ� �ֱ�

					start += (end - start);//���� ��ū�� ����Ŵ
				}

			 	else if(row == 0)//tokens�� �ƹ��͵� �ȵ��������
				{
					if((end = strpbrk(start + 1, op)) == NULL){//���� ��ġ op�� ������
						strncat(tokens[row], start, 1);//�����͸� tokens�� ���ֱ�
						start += 1;
					}
					else{//���� ��ġ op�� ������
						while(start < end){//���� ��ū����
							if(*(start - 1) == ' ' && is_character(tokens[row][strlen(tokens[row]) - 1]))
								//��ĭ�̰� ���ڰ� �������� false
								return false;
							else if(*start != ' ')//start�� ��ĭ�� �ƴϸ�
								strncat(tokens[row], start, 1);//tokens�� �Է�
							start++;	
						}
						if (all_star(tokens[row]))//row�� ��� ������"*"�϶�
							row--;//�տ� ��ū ����Ŵ
						
					}
				}
			}
			else if(*end == '(') //'('�� ����
			{
				lcount = 0;
				rcount = 0;
				if(row>0 && (strcmp(tokens[row - 1],"&") == 0 || strcmp(tokens[row - 1], "*") == 0)){
					//������ "&"�� "*" �ִٸ�
					while(*(end + lcount + 1) == '(')//'('�� ���������� �ݺ�
						lcount++;
					start += lcount;//start�� '('�� ������ ����Ŵ

					end = strpbrk(start + 1, ")");//")"�� �ִ��� Ž��

					if(end == NULL)//")"�� ���ٸ� false
						return false;
					else{//")"�� �ִٸ�
						while(*(end + rcount +1) == ')')//")"�� ���������� �ݺ�
							rcount++;
						end += rcount;//end�� ')'�� ������ ����Ŵ

						if(lcount != rcount)//���� ���������� false
							return false;

						if( (row > 1 && !is_character(tokens[row - 2][strlen(tokens[row - 2]) - 1])) || row == 1){ 
							//row�� 1�̻�, ��ĭ��tokens�� �������� ���ڰ� �ƴ϶�� (�ٸ� ��ȣ�� �ִٸ�)
							strncat(tokens[row - 1], start + 1, end - start - rcount - 1);//��ȣ�� ������ �����Է�
							row--;
							start = end + 1;//���ȣ �� �������� �ٽ� ����Ŵ
						}
						else{
							strncat(tokens[row], start, 1);//���� �Է�(��ȣ ����)
							start += 1;
						}
					}
						
				}
				else{//������ "&","*"�� ���ٸ�
					strncat(tokens[row], start, 1);//�����Է�(��ȣ ����)
					start += 1;
				}

			}
			else if(*end == '\"')//'\"'�� ���� 
			{
				end = strpbrk(start + 1, "\"");//'\"'�� �� �ִ��� Ž��
				
				if(end == NULL)//�� ������ ����
					return false;

				else{//�� �ִٸ�
					strncat(tokens[row], start, end - start + 1);//start���� \"������������ �̾� ����
					start = end + 1;
				}

			}

			else{//�׿ܿ�
				
				if(row > 0 && !strcmp(tokens[row - 1], "++"))//row�� ����ε� "++"�� ������ false
					return false;

				
				if(row > 0 && !strcmp(tokens[row - 1], "--"))//row�� ����ε� "--"�� ������ false
					return false;
	
				strncat(tokens[row], start, 1);//tokens�� start�̾���̱�
				start += 1;
				
			
				if(!strcmp(tokens[row], "-") || !strcmp(tokens[row], "+") || !strcmp(tokens[row], "--") || !strcmp(tokens[row], "++")){
					//���࿡ ��ū�� -/+/--/++ �� ����ִٸ�

				
					if(row == 0)//row�� 0�̸�
						row--;

					
					else if(!is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])){
					    //��ū �������� ���ڰ� �ƴ϶��
						if(strstr(tokens[row - 1], "++") == NULL && strstr(tokens[row - 1], "--") == NULL)
							//��ū�� "++"/"--"�� ���ٸ�
							row--;//row-1
					}
				}
			}
		}
		else{ //ó������ ��ū�� ���� �������
			if(all_star(tokens[row - 1]) && row > 1 && !is_character(tokens[row - 2][strlen(tokens[row - 2]) - 1]))   
				//row-1�� ��� �����ͷ� �̷�����ְ� row�� 1���� ũ�� row-2�κ��� ��ū�� ���ڷ� �̷���������ʴٸ�
				row--;				

			if(all_star(tokens[row - 1]) && row == 1)   
				//row-1�� ��� �������ε� row�� 1. {***}{}
				row--;	

			for(i = 0; i < end - start; i++){//ó������ ���� ��ū���� �ݺ�
				if(i > 0 && *(start + i) == '.'){
					strncat(tokens[row], start + i, 1);//tokens�� .�ֱ�

					while( *(start + i +1) == ' ' && i< end - start )//��������
						i++; 
				}
				else if(start[i] == ' '){//�����϶�
					while(start[i] == ' ')//��������
						i++;
					break;
				}
				else//�׿ܿ�
					strncat(tokens[row], start + i, 1);//�����߰�
			}

			if(start[0] == ' '){//ó���� �����϶�
				start += i;//���� ����ŭ ���ؼ� ��������
				continue;
			}
			start += i;//���� ��ū���� �Ѿ����
		}
			
		strcpy(tokens[row], ltrim(rtrim(tokens[row])));//��ū�� ������ �����ؼ� tokens[row]�� ä���

		 if(row > 0 && is_character(tokens[row][strlen(tokens[row]) - 1]) 
				&& (is_typeStatement(tokens[row - 1]) == 2 
					|| is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])
					|| tokens[row - 1][strlen(tokens[row - 1]) - 1] == '.' ) ){
			 //{...}{...}{gcc/������Ÿ��{����/"."}}{����}
			if(row > 1 && strcmp(tokens[row - 2],"(") == 0)//{"("}{...}{...}
			{//row>1
				if(strcmp(tokens[row - 1], "struct") != 0 && strcmp(tokens[row - 1],"unsigned") != 0)
					//{"("}{"struct"/"unsigned"}{...} �̰� �ƴϸ� false
					return false;
			}
			else if(row == 1 && is_character(tokens[row][strlen(tokens[row]) - 1])) {
				//{{����}}{...}
				//row==1
				if(strcmp(tokens[0], "extern") != 0 && strcmp(tokens[0], "unsigned") != 0 && is_typeStatement(tokens[0]) != 2)	
					return false;//{"extern"/"unsigned",gcc/������Ÿ��{����}}{...}�� �ƴϸ� false
			}
			else if(row > 1 && is_typeStatement(tokens[row - 1]) == 2){
				//{....}{...}{gcc/������Ÿ��}{...}
				if(strcmp(tokens[row - 2], "unsigned") != 0 && strcmp(tokens[row - 2], "extern") != 0)
					return false;//{...}[unsigned/extern}{gcc/������Ÿ��}{...}�� �ƴϸ� false
			}
			
		}

		if((row == 0 && !strcmp(tokens[row], "gcc")) ){//{gcc}
			clear_tokens(tokens);//��ū Ŭ����
			strcpy(tokens[0], str);//�л������ �ϴ� tokens[0]�� ����
			return 1;
		} 

		row++;
	}

	if(all_star(tokens[row - 1]) && row > 1 && !is_character(tokens[row - 2][strlen(tokens[row - 2]) - 1]))  
		row--;	//{..}{{����}}{***}{}�̷��� �����͸� �ִ¹��� ����Ŵ			
	if(all_star(tokens[row - 1]) && row == 1)   //{***}{...} �̷��� �����͸� �ִ� �� ����Ŵ
		row--;	

	for(i = 0; i < strlen(start); i++)   
	{
		if(start[i] == ' ')  //�����̶��
		{
			while(start[i] == ' ')//��������
				i++;
			if(start[0]==' ') {//ó������ ������ �ִٸ�
				start += i;//�ƿ� �տ� ���� ������
				i = 0;
			}
			else
				row++;//�ƴϸ� �׳� ���� �߰�����
			
			i--;//�տ��� i++�ߴ��Ÿ� ��ȿ
		} 
		else//������ �ƴ϶��
		{
			strncat(tokens[row], start + i, 1);//tokens�� ���� �̾����
			if( start[i] == '.' && i<strlen(start)){//i��°�� .�̸�
				while(start[i + 1] == ' ' && i < strlen(start))//�� ������ �����̸�
					i++;//��������

			}
		}
		strcpy(tokens[row], ltrim(rtrim(tokens[row])));//tokens�� �¿���� ����

		if(!strcmp(tokens[row], "lpthread") && row > 0 && !strcmp(tokens[row - 1], "-")){
			//{...}{-}{lpthread} ��� (�����ϰ��� �ɼ�)
			strcat(tokens[row - 1], tokens[row]);//tokens[row-1]�� �̾���δ�
			memset(tokens[row], 0, sizeof(tokens[row]));//tokens[row]�� ����
			row--;
		}
		else if(row > 0 && is_character(tokens[row][strlen(tokens[row]) - 1]) 
				&& (is_typeStatement(tokens[row - 1]) == 2 
					|| is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])
					|| tokens[row - 1][strlen(tokens[row - 1]) - 1] == '.') ){
			//{gcc/������Ÿ��/{����/'.'}}{{����}}
			
			if(row > 1 && strcmp(tokens[row-2],"(") == 0)
			{//{"("}{}{}
				if(strcmp(tokens[row-1], "struct") != 0 && strcmp(tokens[row-1], "unsigned") != 0)
					return false;//{"("}{"struct","unsigned"}�� �ƴϸ� false
			}
			else if(row == 1 && is_character(tokens[row][strlen(tokens[row]) - 1])) {
				if(strcmp(tokens[0], "extern") != 0 && strcmp(tokens[0], "unsigned") != 0 && is_typeStatement(tokens[0]) != 2)	
					return false;//{extern,unsigned,gcc/������Ÿ��}{{����}} �ƴϸ� false
			}
			else if(row > 1 && is_typeStatement(tokens[row - 1]) == 2){
				if(strcmp(tokens[row - 2], "unsigned") != 0 && strcmp(tokens[row - 2], "extern") != 0)
					return false; //{unsigned,extern}{gcc/������Ÿ��}{...}�̸� false
			}
		} 
	}


	if(row > 0)//���� ����̸�
	{
		if(strcmp(tokens[0], "#include") == 0 || strcmp(tokens[0], "include") == 0 || strcmp(tokens[0], "struct") == 0){ 
			//{"#include"/"include"/"struct"}{}
			clear_tokens(tokens); //��ū ����
			strcpy(tokens[0], remove_extraspace(str));//������ ������ �� tokens�� ����
		}
	}

	if(is_typeStatement(tokens[0]) == 2 || strstr(tokens[0], "extern") != NULL){
		//{gcc/datatype/extern}
		for(i = 1; i < TOKEN_CNT; i++){
			if(strcmp(tokens[i],"") == 0)  //��������� break
				break;		       

			if(i != TOKEN_CNT -1 )
				strcat(tokens[0], " ");//������ �߰�
			strcat(tokens[0], tokens[i]);//tokens[0]�� tokens[i] �̾����
			memset(tokens[i], 0, sizeof(tokens[i]));//tokens[i]�� 0���� ä��
		}
	}
	
	
	while((p_str = find_typeSpecifier(tokens)) != -1){ //���������� ã�� �Լ�
		if(!reset_tokens(p_str, tokens))//��ū ���ġ
			return false;
	}

	
	while((p_str = find_typeSpecifier2(tokens)) != -1){  //����ü ���������� ã�� �Լ�
		if(!reset_tokens(p_str, tokens))//��ū ���ġ
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
		if(strcmp(tokens[*idx], "") == 0)//�����̸� break
			break;
	
		if(!strcmp(tokens[*idx], ")"))//")"�� ������
			return get_root(cur);//root���ϱ�

		else if(!strcmp(tokens[*idx], ","))//","�� ������
			return get_root(cur);//root���ϱ�

		else if(!strcmp(tokens[*idx], "("))//"("�� ������
		{
			
			if(*idx > 0 && !is_operator(tokens[*idx - 1]) && strcmp(tokens[*idx - 1], ",") != 0){
				//tokens[*idx-1]�� �����ڰ� �ƴϰ� ","�� ������ fstart=true
				fstart = true;

				while(1)
				{
					*idx += 1;

					if(!strcmp(tokens[*idx], ")"))//tokens[*idx]�� )�̸� break
						break;
					
					new = make_tree(NULL, tokens, idx, parentheses + 1);//������ �ҷ��ö����� �Լ��� �ҷ��´�
					
					if(new != NULL){//������ �ƴϸ�
						if(fstart == true){//���⼭���� ����.�θ𰡵�
							cur->child_head = new;//cur�� �ڽ��� new
							new->parent = cur;//new�� �θ�� cur
	
							fstart = false;
						}
						else{//�θ��ڽ� ���谡 �ƴϰԵ�
							cur->next = new;
							new->prev = cur;
						}

						cur = new;//�����͸� Ŀ������ �κ����� �Ű���
					}

					if(!strcmp(tokens[*idx], ")"))//tokens�� ")"�� ��Ÿ���� �ݺ��� ����
						break;
				}
			}
			else{//tokens[*idx]�� "("�� �������� �̿��� ��Ȳ
				*idx += 1;
	
				new = make_tree(NULL, tokens, idx, parentheses + 1);//���� tokens�� ���� make_tree�Լ��� �ҷ��´�

				if(cur == NULL)
					cur = new;//cur�����Ͱ� new�� ����Ų��

				else if(!strcmp(new->name, cur->name)){//new->name�� cur->name�� ������(���� ������)
					if(!strcmp(new->name, "|") || !strcmp(new->name, "||") 
						|| !strcmp(new->name, "&") || !strcmp(new->name, "&&"))//new->name�� �̿� ������
					{
						cur = get_last_child(cur);//cur�� ������ �ڽ��� cur�� ����Ű����

						if(new->child_head != NULL){//new�� �ڽ��� �ִٸ� �ڽ��� ���ֹ���
							new = new->child_head;//new�� �ڽ��� new ��

							new->parent->child_head = NULL;
							new->parent = NULL;
							new->prev = cur;
							cur->next = new;
						}
					}
					else if(!strcmp(new->name, "+") || !strcmp(new->name, "*"))//new->name�� "+","*"�� �ִٸ�
					{
						i = 0;

						while(1)
						{
							if(!strcmp(tokens[*idx + i], ""))//�����̸� ����
								break;

							if(is_operator(tokens[*idx + i]) && strcmp(tokens[*idx + i], ")") != 0)//tokens[*idx+i]�� 1���� �������̰� ")"�� ������ ����
								break;

							i++;
						}
						
						if(get_precedence(tokens[*idx + i]) < get_precedence(new->name))//tokens[*idx+i]�� ������ �켱���� ������
						{
							cur = get_last_child(cur);//cur�� ������ �ڽ��� cur�� ����Ŵ
							cur->next = new;
							new->prev = cur;
							cur = new;//�߰��� new�� cur�� ����Ű������
						}
						else//tokens[*idx+i]�� �켱������ �� ������
						{
							cur = get_last_child(cur);//cur�� �������ڽ��� cur�� ����Ŵ

							if(new->child_head != NULL){//new�� �ڽ��� �ִٸ�
								new = new->child_head;//�ڽ� ���� �̵�

								new->parent->child_head = NULL;//new�� �θ��� �ڽ��� ����
								new->parent = NULL;//new�� �θ� ����
								new->prev = cur;//new�� ������带 cur
								cur->next = new;//cur�� ������带 new
							}
						}
					}
					else{
						cur = get_last_child(cur);//cur�� �������ڽ��� ����Ŵ
						cur->next = new;//cur->next�� new
						new->prev = cur;//new->prev�� cur
						cur = new;//cur�� ���ܳ�� new�� ����Ŵ
					}
				}
	
				else//�̿ܿ�
				{
					cur = get_last_child(cur);//cur�� �������ڽ��� ����Ŵ

					cur->next = new;//cur->next�� new
					new->prev = cur;//new->prev�� cur
	
					cur = new;//cur�� ���ܳ�� new�� ����Ŵ
				}
			}
		}
		else if(is_operator(tokens[*idx]))//tokens[*idx]�� �������ϰ��
		{
			if(!strcmp(tokens[*idx], "||") || !strcmp(tokens[*idx], "&&")
					|| !strcmp(tokens[*idx], "|") || !strcmp(tokens[*idx], "&") 
					|| !strcmp(tokens[*idx], "+") || !strcmp(tokens[*idx], "*"))
			{//���� �̷� �������ϰ�쿡��
				if(is_operator(cur->name) == true && !strcmp(cur->name, tokens[*idx]))
					operator = cur;//cur->name�� �������̰� cur->name�� tokens[*idx]�� ������ operator�� cur�� ����Ŵ
		
				else
				{//�̿ܿ�
					new = create_node(tokens[*idx], parentheses);//���ο� ��带 ����
					operator = get_most_high_precedence_node(cur, new);//cur�� new �߿� �켱���� ���ϳ��� �����ڸ� operator�� ����

					if(operator->parent == NULL && operator->prev == NULL){//operator �θ��尡 ���� ������嵵 ������

						if(get_precedence(operator->name) < get_precedence(new->name)){
							cur = insert_node(operator, new);//operator�켱������ �� Ŭ�� new�� ����
						}

						else if(get_precedence(operator->name) > get_precedence(new->name))
						{//operator �켱������ �� ������
							if(operator->child_head != NULL){//operator �ڽĳ�尡 ������
								operator = get_last_child(operator);//operator�� �ǳ� �ڽĳ�� ����Ų
								cur = insert_node(operator, new);//new�� operator�� �����ϰ� new�� ����Ŵ
							}
						}
						else//operator�� new�� �켱������ ������
						{
							operator = cur;//operator�� cur�� ī��Ŵ
	
							while(1)
							{
								if(is_operator(operator->name) == true && !strcmp(operator->name, tokens[*idx]))
									break;//operator�� �������̰� tokens[*idx]�� operator�� ������ ����
						
								if(operator->prev != NULL)//operator������尡 ����
									operator = operator->prev;//operator�� operator->prev�� ����Ŵ
								else//������尡 �������� ������ ����
									break;
							}

							if(strcmp(operator->name, tokens[*idx]) != 0)//operator�� tokens[*idx]�� �ٸ���
								operator = operator->parent;//operator->parent�� ����Ŵ

							if(operator != NULL){//operator�� ������ �ƴϸ�
								if(!strcmp(operator->name, tokens[*idx]))
									//operator->name�� tokens[*idx]�� ������ cur�� operator�� ��
									cur = operator;
							}
						}
					}

					else//operator�� �θ��尡 �����ϰų� operator�� ������ �����ϸ�
						cur = insert_node(operator, new);//new ��带 ������
				}

			}
			else//���� �����ڰ� �ƴѰ�쿡
			{
				new = create_node(tokens[*idx], parentheses);//���ο� ��� �����Լ�

				if(cur == NULL)
					cur = new;//cur�� ���ο� ��尡 ��

				else//cur�� ���� ���� ��
				{
					operator = get_most_high_precedence_node(cur, new);//���� ���� �켱�������� ������

					if(operator->parentheses > new->parentheses)//operator�� ��ȣ�� new���� ������ ��带 �߰�
						cur = insert_node(operator, new);//operator ���� new�� ����

					else if(operator->parent == NULL && operator->prev ==  NULL){//operator�� �θ�, ������尡 ������
					
						if(get_precedence(operator->name) > get_precedence(new->name))//operator�� �켱������ �� ������
						{
							if(operator->child_head != NULL){//operator �ڽĳ�尡 ������
	
								operator = get_last_child(operator);//operator�� ������ �ڽ��� operator�� ����(�ǳ�)
								cur = insert_node(operator, new);//operator ���� new ����
							}
						}
					
						else	//operator �켱������ �� ������
							cur = insert_node(operator, new);//new�� operator������ ����
					}
	
					else
						cur = insert_node(operator, new);//new�� operator ������ ����
				}
			}
		}
		else //�׿ܿ�
		{
			new = create_node(tokens[*idx], parentheses);//���ο� ��� ����

			if(cur == NULL)
				cur = new;//cur�� ���ο� ����

			else if(cur->child_head == NULL){//cur�� �ڽ��� ���ٸ� cur�� �ڽ��� new�� �ȴ�.
				cur->child_head = new;
				new->parent = cur;

				cur = new;//�����Ͱ� ���ο� ���ܳ�带 ����Ű������
			}
			else{//�׿ܿ�

				cur = get_last_child(cur);//cur�� ���� ������ �ڽ��� cur�� ��

				cur->next = new;//cur������ new�� ��
				new->prev = cur;//new������ cur

				cur = new;//�����Ͱ� ���ο� ���ܳ�带 ����Ű������
			}
		}

		*idx += 1;//token�� ������ �ɶ����� �ݺ�
	}

	return get_root(cur);
}

node *change_sibling(node *parent)//�����ڸ� ��带 �ٲ�
{
	node *tmp;
	
	tmp = parent->child_head;//childhead �� ����Ŵ

	parent->child_head = parent->child_head->next;//parent->childhead�� parent->childhead->next�� �ٲ�
	parent->child_head->parent = parent;//�� �ٲ� childhead->parent�� parent�� ����
	parent->child_head->prev = NULL;//childhead->prev�� ���

	parent->child_head->next = tmp;//childhead->next�� ������ childhead�� tmp�� ����
	parent->child_head->next->prev = parent->child_head;
	parent->child_head->next->next = NULL;//tmp->next�� ����
	parent->child_head->next->parent = NULL;//tmp->parent�� ����

	return parent;//parent�� ����
}

node *create_node(char *name, int parentheses)//���ο�������Լ�
{
	node *new;

	new = (node *)malloc(sizeof(node));//���ο� ��� ����
	new->name = (char *)malloc(sizeof(char) * (strlen(name) + 1));//node new�� �̸� ����
	strcpy(new->name, name);//node new�� �̸��� name�� ����

	new->parentheses = parentheses;//��ȣ ��
	new->parent = NULL;
	new->child_head = NULL;
	new->prev = NULL;
	new->next = NULL;

	return new;
}

int get_precedence(char *op)//�켱���� ��� �Լ�
{
	int i;

	for(i = 2; i < OPERATOR_CNT; i++){
		if(!strcmp(operators[i].operator, op))//operators[i].operator�� �Է¹��� �����ڰ� ���ٸ�
			return operators[i].precedence;//�켱���� ����
	}
	return false;
}

int is_operator(char *op)//���������� �ƴ���
{
	int i;

	for(i = 0; i < OPERATOR_CNT; i++)//������ ������ŭ �ݺ�
	{
		if(operators[i].operator == NULL)
			break;
		if(!strcmp(operators[i].operator, op)){//operators[i]�� �����ڿ� �Է¹��� �����ڰ� ������ true
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

node *get_root(node *cur)//root�� ���ϴ� �Լ�
{
	if(cur == NULL)//���� �ƹ��͵� ������
		return cur;

	while(cur->prev != NULL)//���翡�� ó������ �̵�
		cur = cur->prev;

	if(cur->parent != NULL)//�θ� ���� �̵�
		cur = get_root(cur->parent);//root���� �������� �Լ��� �ҷ���

	return cur;
}

node *get_high_precedence_node(node *cur, node *new)//���� �켱���� ��带 ����
{
	if(is_operator(cur->name))//�������̸� 
		if(get_precedence(cur->name) < get_precedence(new->name))//���ڰ� �������� �켱���� ����
			return cur;

	if(cur->prev != NULL){
		while(cur->prev != NULL){
			cur = cur->prev;//���� ��尡 ���������� �ݺ�
			
			return get_high_precedence_node(cur, new);//�ٽ� new�� �켱������ ����
		}


		if(cur->parent != NULL)
			return get_high_precedence_node(cur->parent, new);//cur->parent�ϰ� new�� �켱���� ����
	}

	if(cur->parent == NULL)//�θ� ������� cur����
		return cur;
}

node *get_most_high_precedence_node(node *cur, node *new)//������� �켱���� ��带 ��� �Լ�
{
	node *operator = get_high_precedence_node(cur, new);//���� �켱������ ���� ������ ����
	node *saved_operator = operator;//�� �����ڸ� saved_operator�� ����

	while(1)
	{
		if(saved_operator->parent == NULL)//�θ� ��尡 ������� ����
			break;

		if(saved_operator->prev != NULL)//���� ��尡 ������ �� ���� ���� �ٽ� �켱������
			operator = get_high_precedence_node(saved_operator->prev, new);

		else if(saved_operator->parent != NULL)//�θ��尡 �ִٸ� �ٽ� �켱������
			operator = get_high_precedence_node(saved_operator->parent, new);

		saved_operator = operator;//���� ���� �켱������ ���� �����ڸ� �˼��ִ�
	}
	
	return saved_operator;
}

node *insert_node(node *old, node *new)//���ο� ��� �����Լ�(new�� prev�� old ���̿� ����)
{
	if(old->prev != NULL){//old->prev�� �����Ѵٸ� new�� old�� ���
		new->prev = old->prev;
		old->prev->next = new;
		old->prev = NULL;
	}

	new->child_head = old;//new�� �ڽĳ�� old
	old->parent = new;//old�� �θ��� new

	return new;
}

node *get_last_child(node *cur)
{
	if(cur->child_head != NULL)//cur�� �ڽĳ�带 cur�� ����
		cur = cur->child_head;

	while(cur->next != NULL)//cur�� ������尡 ���������� �ݺ�
		cur = cur->next;

	return cur;//�� ������ �ڽ��� ����
}

int get_sibling_cnt(node *cur)//�����ڸ� ��带 ����Լ�
{
	int i = 0;

	while(cur->prev != NULL)//������尡 ������
		cur = cur->prev;//cur�� ������带 ����Ŵ. ������尡 �� �̻� �������� ���������� �ݺ�

	while(cur->next != NULL){//���� ��尡 ������
		cur = cur->next;//cur�� ������带 ����Ŵ
		i++;//���� prev������ ���� next����ŭ�� node ����. siblings ���� �˼�����
	}

	return i;//sibling ����
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


int is_character(char c)//����,���ĺ������� �Ǻ��ϴ� �Լ�
{
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int is_typeStatement(char *str)//Ÿ�Ի���ǥ��
{ 
	char *start;
	char str2[BUFLEN] = {0}; 
	char tmp[BUFLEN] = {0}; 
	char tmp2[BUFLEN] = {0}; 
	int i;	 
	
	start = str;
	strncpy(str2,str,strlen(str));//str���� ���̸�ŭ str2 ����
	remove_space(str2);//������ ����

	while(start[0] == ' ')//str ���ۺ��� �����̸� �������ڷ� �Ѿ
		start += 1;

	if(strstr(str2, "gcc") != NULL)//str2���� gcc�� ã�´�
	{
		strncpy(tmp2, start, strlen("gcc"));//start���� "gcc"ũ�⸸ŭ tmp2�� ����
		if(strcmp(tmp2,"gcc") != 0)//tmp2�� "gcc"�� �ٸ��� 0����
			return 0;
		else
			return 2;//������ 2����
	}
	
	for(i = 0; i < DATATYPE_SIZE; i++)
	{
		if(strstr(str2,datatype[i]) != NULL)//str2�� ������Ÿ���� �������� ������
		{	
			strncpy(tmp, str2, strlen(datatype[i]));//tmp�� str2�� ����
			strncpy(tmp2, start, strlen(datatype[i]));//tmp2�� start�� ����
			
			if(strcmp(tmp, datatype[i]) == 0)//tmp�� ������Ÿ���� ���ڿ��� ���ٸ�
				if(strcmp(tmp, tmp2) != 0)//tmp�� tmp2�� �ٸ��� -> 0
					return 0;  
				else//������ -> 2
					return 2;
		}

	}
	return 1;

}

int find_typeSpecifier(char tokens[TOKEN_CNT][MINLEN]) //���������� ã�� �Լ�
{
	int i, j;

	for(i = 0; i < TOKEN_CNT; i++)
	{
		for(j = 0; j < DATATYPE_SIZE; j++)
		{
			if(strstr(tokens[i], datatype[j]) != NULL && i > 0)//tokens���� datatype ã��
			{
				if(!strcmp(tokens[i - 1], "(") && !strcmp(tokens[i + 1], ")") 
						&& (tokens[i + 2][0] == '&' || tokens[i + 2][0] == '*' 
							|| tokens[i + 2][0] == ')' || tokens[i + 2][0] == '(' 
							|| tokens[i + 2][0] == '-' || tokens[i + 2][0] == '+' 
							|| is_character(tokens[i + 2][0])))  
					return i;//�̷� ������ �����ϴ� i�� ����
			}
		}
	}
	return -1;
}

int find_typeSpecifier2(char tokens[TOKEN_CNT][MINLEN]) //����ü ���������� ã�� �Լ�
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

int all_star(char *str)//�����Ͱ� ������ 1 ����
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

int reset_tokens(int start, char tokens[TOKEN_CNT][MINLEN]) //��ū ���ġ�ϴ� �Լ�
{
	int i;
	int j = start - 1;
	int lcount = 0, rcount = 0;
	int sub_lcount = 0, sub_rcount = 0;

	if(start > -1){
		if(!strcmp(tokens[start], "struct")) {		//find_typesSpecifier2�� ����
			strcat(tokens[start], " ");//tokens�� �����߰�
			strcat(tokens[start], tokens[start+1]);	 //�� ������ �̾���̱�    

			for(i = start + 1; i < TOKEN_CNT - 1; i++){
				strcpy(tokens[i], tokens[i + 1]);//�ϳ��� �մ������
				memset(tokens[i + 1], 0, sizeof(tokens[0]));//���� ��ū�� 0���� ä������
			}
		}

		else if(!strcmp(tokens[start], "unsigned") && strcmp(tokens[start+1], ")") != 0) {
			//unsigned�� )�� �̷�����ִ°��
			strcat(tokens[start], " ");
			strcat(tokens[start], tokens[start + 1]);	     
			strcat(tokens[start], tokens[start + 2]);
			//�����߰�. +2ĭ���� start�� �̾���̱�
			for(i = start + 1; i < TOKEN_CNT - 1; i++){
				strcpy(tokens[i], tokens[i + 1]);//�ϳ��� �մ����
				memset(tokens[i + 1], 0, sizeof(tokens[0]));//�״����� 0���� ä��
			}
		}

     		j = start + 1;           
        	while(!strcmp(tokens[j], ")")){//)�� ������
                	rcount ++;//������ ��ȣ ���� ����
                	if(j==TOKEN_CNT)
                        	break;
                	j++;
        	}
	
		j = start - 1;
		while(!strcmp(tokens[j], "(")){//(�� ������
        	        lcount ++;//�°�ȣ ���� ����
                	if(j == 0)
                        	break;
               		j--;
		}
		if( (j!=0 && is_character(tokens[j][strlen(tokens[j])-1]) ) || j==0)
			lcount = rcount;//��ȣ���� �¾ƾߵ�

		if(lcount != rcount )
			return false;

		if( (start - lcount) >0 && !strcmp(tokens[start - lcount - 1], "sizeof")){
			return true; //ó������ ��ȣ���۾��ϴ°Ŵ� sizeof�� ��������
		}
		
		else if((!strcmp(tokens[start], "unsigned") || !strcmp(tokens[start], "struct")) && strcmp(tokens[start+1], ")")) {		
			strcat(tokens[start - lcount], tokens[start]);//���ʰ�ȣ����ŭ ���� start ��ū �̾���̱�
			strcat(tokens[start - lcount], tokens[start + 1]);//start+1��ū���� �̾���̱�
			strcpy(tokens[start - lcount + 1], tokens[start + rcount]);//start-lcount+1���ٰ� ������ ��ȣ�� �Ѿ ��ū ����ֱ�
		 
			for(int i = start - lcount + 1; i < TOKEN_CNT - lcount -rcount; i++) {
				//��ȣ�� ����ŭ�� �ݺ�
				strcpy(tokens[i], tokens[i + lcount + rcount]);//���� ũ���� ��ū�� �پ�� i��° ��ū�� ����
				memset(tokens[i + lcount + rcount], 0, sizeof(tokens[0]));//������ū�� 0���� ä����
			}


		}
 		else{//�׿ܿ�
			if(tokens[start + 2][0] == '('){
				j = start + 2;
				while(!strcmp(tokens[j], "(")){
					sub_lcount++;//���� ��ȣ �� ����
					j++;//���� ��ū���� �̵�
				} 	
				if(!strcmp(tokens[j + 1],")")){
					j = j + 1;
					while(!strcmp(tokens[j], ")")){
						sub_rcount++;//������ ��ȣ �� ����
						j++;//���� ��ū���� �̵�
					}
				}
				else 
					return false;

				if(sub_lcount != sub_rcount)//��ȣ ¦�� �ȸ����� false
					return false;
				
				strcpy(tokens[start + 2], tokens[start + 2 + sub_lcount]);	//�°�ȣ ������ ������ ����
				for(int i = start + 3; i<TOKEN_CNT; i++)
					memset(tokens[i], 0, sizeof(tokens[0]));//0���� ä���

			}
			strcat(tokens[start - lcount], tokens[start]);//�°�ȣ������ŭ ��ū�� start��ū �̾���̱�
			strcat(tokens[start - lcount], tokens[start + 1]);//start+1 ��ū�� �̾���̱�
			strcat(tokens[start - lcount], tokens[start + rcount + 1]);//���ȣ�A����ŭ ��ū�� �̾���̱�
		 
			for(int i = start - lcount + 1; i < TOKEN_CNT - lcount -rcount -1; i++) {
				strcpy(tokens[i], tokens[i + lcount + rcount +1]);//������ ��ū���� ���ȣ ������ ������ ���δ�
				memset(tokens[i + lcount + rcount + 1], 0, sizeof(tokens[0]));//0���� ä��

			}
		}
	}
	return true;
}

void clear_tokens(char tokens[TOKEN_CNT][MINLEN])//��ū �����Լ�
{
	int i;

	for(i = 0; i < TOKEN_CNT; i++)
		memset(tokens[i], 0, sizeof(tokens[i]));//i��° ��ū�� 0���� ä��
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

char* remove_extraspace(char *str)//������ ������ ���ִ� �Լ�
{
	int i;
	char *str2 = (char*)malloc(sizeof(char) * BUFLEN);
	char *start, *end;
	char temp[BUFLEN] = "";
	int position;

	if(strstr(str,"include<")!=NULL){//"include<"�� ������
		start = str;
		end = strpbrk(str, "<");//"<"���۵Ǵ� �κ�
		position = end - start;//"include"�κ�
	
		strncat(temp, str, position);//temp�� ����
		strncat(temp, " ", 1);//�����߰�
		strncat(temp, str + position, strlen(str) - position + 1);//include�� �� �������� �̾����.

		str = temp;		//�ٽ� str�� ����
	}
	
	for(i = 0; i < strlen(str); i++)
	{
		if(str[i] ==' ')//������ ������
		{
			if(i == 0 && str[0] ==' ')//ó������ �̾����� ��������
				while(str[i + 1] == ' ')
					i++;	
			else{//ó������ ������ �ƴ�
				if(i > 0 && str[i - 1] != ' ')//��ĭ ���� ������ �ƴϸ�
					str2[strlen(str2)] = str[i];//str2�� �����߰�
				while(str[i + 1] == ' ')
					i++;//������ ������ŭ ++
			} 
		}
		else//������ ������
			str2[strlen(str2)] = str[i];//iĭ���� �̹� ������ ����. str[i]�� str2�� ����
	}

	return str2;
}



void remove_space(char *str)//���� ���ִ� �Լ�
{
	char* i = str;
	char* j = str;
	
	while(*j != 0)//���ڿ��� ����������
	{
		*i = *j++;
		if(*i != ' ')//������ ������ i�� �̵����� �ʴ´�
			i++;
	}
	*i = 0;
}

int check_brackets(char *str)//��ȣ�˻��ϴ� �Լ�
{
	char *start = str;
	int lcount = 0, rcount = 0;
	
	while(1){
		if((start = strpbrk(start, "()")) != NULL){//start�� ���ڵ� �� "()"�� ��ġ�ϴ� ���� ����Ŵ.
			//ù��°�� ��ġ�ϴ� ���ڸ� ����Ų��
			if(*(start) == '(')
				lcount++;//"(" ��
			else
				rcount++;//")" ��

			start += 1;//����Ű�� ���ڸ� ��ĭ�� �̵� 		
		}
		else
			break;
	}

	if(lcount != rcount)//��ȣ ���� ���� ������
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
