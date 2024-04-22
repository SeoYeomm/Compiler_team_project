#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "STMain.h"
#include "HashFunc.h"

// 팀 10 (유서현, 김서현, 이서연)

/*유서현: 입력이 string pool 크기 초과하는 경우 에러 메시지 출력 & 현재까지 입력된 식별자에 대해 정상 출력 처리 (33.3%)
* 김서현: 식별자 길이 15자 넘어가는 경우 에러 메시지 출력 & 해시 버킷의 비어있는 부분은 출력하지 않으면서 해시 버킷과 심볼 테이블 출력, 중복 식별자 처리 (33.3%)
* 이서연: 식별자 첫 글자는 숫자가 오는 경우 에러 메시지 출력 & 식별자에 영어 대소문자, _ , 구분자, 숫자 제외한 다른 문자 입력 시 에러 메시지 출력 (33.3%)
*/

char separators[] = " ,;\t\n\r\n";
char str_pool[STR_POOL_SIZE];
int sym_table[SYM_TABLE_SIZE][2];
int hash_value = -1;

typedef struct HTentry* HTpointer;
typedef struct HTentry {
	int index;
	HTpointer next;
} HTentry;

HTpointer HT[HASH_TABLE_SIZE];

// 해시테이블에서 찾고자 하는 값 반환
HTpointer lookup_hash_table(int id_index, int hscode) {
	HTpointer entry = HT[hscode];

	while (entry != NULL) {
		// HT에서 찾고자 하는 값 비교
		if (strcmp(str_pool + sym_table[entry->index][0], str_pool + (id_index)) == 0) {
			return entry; // 찾은 항목 반환
		}
		entry = entry->next;
	}
	return NULL; // 항목을 찾지 못한 경우 
}

// 해시테이블에 추가
void add_hash_table(int id_index, int hscode) {
	// 새 항목 생성 및 초기화
	HTpointer newEntry = (HTpointer)malloc(sizeof(HTentry));
	if (newEntry == NULL) {
		printf("메모리 할당 실패\n");
		exit(1);
	}
	newEntry->index = id_index;
	newEntry->next = NULL;

	// HT에 추가
	if (HT[hscode] == NULL) {
		HT[hscode] = newEntry;
	}
	else {
		newEntry->next = HT[hscode];
		HT[hscode] = newEntry;
	}
}

// 해시테이블 출력
void print_hash_table() {
	printf("\nHash Table\n");
	for (int i = 0; i < HASH_TABLE_SIZE; i++) {
		HTpointer curr = HT[i];

		if (curr == NULL) { // 해시 버킷이 비어 있다면 출력하지 않음
			continue;
		}

		printf("[%d]: ", i);
		while (curr->next != NULL) {
			printf("%d -> ", curr->index);
			curr = curr->next;
		}
		printf("%d\n", curr->index);
	}
}

// 심볼 테이블 초기화
void init_sym_table() {
	for (int i = 0; i < SYM_TABLE_SIZE; i++) {
		sym_table[i][0] = -1;
		sym_table[i][1] = -1;
	}
}

// 심볼테이블에서 찾고자 하는 값 반환
int lookup_sym_table(int index_start) {
	for (int i = 0; i < SYM_TABLE_SIZE; i++) {
		if (sym_table[i][0] != -1) {
			if (!strcmp(str_pool + sym_table[i][0], str_pool + index_start)) {
				return i;
			}
		}
	}
	return -1;
}

// 심볼테이블 출력
void print_sym_table() {
	printf("\nSymbol Table\n");
	printf("index\tLength\tSymbol\n");
	for (int i = 0; i < SYM_TABLE_SIZE; i++) {
		if (sym_table[i][0] != -1) {
			printf("%d\t%d\t%s\n", sym_table[i][0], sym_table[i][1], str_pool + sym_table[i][0]);
		}
	}
}

int main() {
	FILE* fp;
	int result;
	int c; // 읽은 문자를 저장할 변수
	int index_start = 0;
	int index_next = 0;
	int index = 0; // 심볼테이블용 변수
	int hash_value = -1;

	init_sym_table();

	result = fopen_s(&fp, "example1_error.txt", "r"); // 파일을 읽기 모드로 열기
	if (result != 0) {
		printf("파일 열기 실패(%d)\n", result);
		return -1;
	}

	while ((c = fgetc(fp)) != EOF) { // 파일 끝까지 문자 읽기
		if (strchr(separators, c) == NULL) {
			str_pool[index_next++] = (char)c; // 버퍼에 문자 저장
			continue;
		}

		// 구분자를 만나거나 버퍼 크기 제한에 도달했을 때
		if (index_start < index_next) { // 버퍼에 내용이 있을 때만 출력

			// 입력이 string pool의 크기를 초과할 경우
			// 마지막 문자열은 제외하도록 처리하고 while문을 빠져나감.
			if (index_next >= STR_POOL_SIZE) {
				index_next = index_start; // 버퍼 인덱스 초기화
				printf("Error - OVERFLOW...\n\n");
				break;
			}

			str_pool[index_next] = '\0'; // 문자열 종료
			// 문자열의 시작 문자가 숫자인지 체크
			if (str_pool[index_start] >= '0' && str_pool[index_start] <= '9') {
				printf("Error - start with digit (%s)\n", str_pool + index_start); // 에러 출력
				index_next = index_start; // 버퍼 인덱스 초기화
			}
			// 식별자의 길이가 15자 이내인지 체크
			else if (index_next - index_start > MAX_IDENTIFIER_SIZE) {
				printf("Error - maximum length is 15 (%s)\n", str_pool + index_start);
				index_next = index_start;  // 버퍼 인덱스 초기화
			}
			else {
				// 영어 대소문자, 밑줄, 숫자, 구분자 이외의 문자가 있는지 체크
				int is_valid = 1;
				for (int i = index_start; i <= index_next; i++) {
					if (!isalnum(str_pool[i]) && str_pool[i] != '_' && !strchr(separators, str_pool[i])) { // isalnum: 알파벳이나 숫자면 1을 반환
						is_valid = 0;
					}
				}
				if (!is_valid) {
					printf("Error - invalid character in identifier (%s)\n", str_pool + index_start);
					index_next = index_start; // 버퍼 인덱스 초기화
				}
				else {
					// 문자열이 이미 심볼 테이블에 저장되어있는지 확인
					int already_exists = lookup_sym_table(index_start);
					if (already_exists != -1) {
						printf("%s (Already exists, Symbol Table Info: Index {%d} Length {%d})\n", str_pool + index_start, sym_table[already_exists][0], sym_table[already_exists][1]); // 버퍼 내용 화면에 출력
						index_start = ++index_next;
						continue;
					}

					sym_table[index][0] = index_start; // 현재 처리 중인 문자열의 시작 인덱스를 심볼 테이블에 기록
					sym_table[index++][1] = (int)strlen(str_pool + index_start); // str_pool의 index_start 위치부터 문자열의 끝까지 길이 계산

					hash_value = divisionMethod(str_pool + index_start, HASH_TABLE_SIZE);

					HTpointer htp = lookup_hash_table(index_start, hash_value);
					if (htp == NULL) {
						add_hash_table(index_start, hash_value);
						printf("%s (Hash: %d)\n", str_pool + index_start, hash_value); // 버퍼 내용 화면에 출력
					}
					index_start = ++index_next; // 다음 문자열의 시작 인덱스 설정
				}
			}
		}
		if (strchr(separators, c) == NULL) {
			str_pool[index_next++] = (char)c; // 구분자가 아닌 문자를 버퍼에 저장
		}
	}

	if (index_start < index_next) { // 마지막 문자열 출력
		str_pool[index_next] = "\0"; // 문자열 종료
		sym_table[index][0] = index_start;
		sym_table[index++][1] = strlen(str_pool + index_start);

		hash_value = divisionMethod(str_pool + index_start, HASH_TABLE_SIZE);

		HTpointer htp = lookup_hash_table(index_start, hash_value);
		if (htp == NULL) {
			add_hash_table(index_start, hash_value);
			printf("%s (Hash: %d)\n", str_pool + sym_table[index - 1][0], hash_value); // 버퍼 내용 화면에 출력
		}
		else {
			printf("%s (Already exists, Hash: %d)\n", str_pool + sym_table[index - 1][0], hash_value); // 버퍼 내용 화면에 출력
		}
	}

	print_sym_table();
	print_hash_table();

	fclose(fp); // 파일 닫기
	return 0;
}