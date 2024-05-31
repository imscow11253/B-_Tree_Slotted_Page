#include "page.hpp"
#include <iostream> 
#define STRING_LEN 20

int main(){
	page *p = new page(LEAF);

	char key[STRING_LEN];
	char i;
	uint64_t val = 100;
	uint64_t cnt = 0;

	for(i='a'; i<='j'; i++){
		for(int j=0; j<STRING_LEN-1; j++){
			key[j] = i;
		}
		key[STRING_LEN-1]='\0';
		cnt++;
		val*=cnt;
		p->insert(key, val);
		p->print();
	}

	val = 100;
	cnt = 0;
	for(i='a'; i<='j'; i++){
		for(int j=0; j<STRING_LEN-1; j++){
			key[j] = i;
		}
		key[STRING_LEN-1]='\0';
		cnt++;
		val*=cnt;
		if(val== p->find(key)){
			printf("key :%s founds\n",key);		
		}
		else{
			printf("key :%s Something wrong\n",key);		

		}
	}

	// << test for offset sorting >>
	// for(int j=0; j<STRING_LEN-1; j++) key[j] = 'a';
	// val = 'a';
	// p->insert(key, val);
	// p->print();
	// for(int j=0; j<STRING_LEN-1; j++) key[j] = 'c';
	// val = 'c';
	// p->insert(key, val);
	// p->print();
	// for(int j=0; j<STRING_LEN-1; j++) key[j] = 'd';
	// val = 'd';
	// p->insert(key, val);
	// p->print();
	// for(int j=0; j<STRING_LEN-1; j++) key[j] = 'e';
	// val = 'e';
	// p->insert(key, val);
	// p->print();
	// for(int j=0; j<STRING_LEN-1; j++) key[j] = 'b';
	// val = 'b';
	// p->insert(key, val);
	// p->print();

	// << test for address check >>
	// printf("page 객체의 시작 주소 : %d\n", p);
	// printf("header의 크기 : %d\n", sizeof(slot_header));
	// printf("offset_aaray의 시작 주소(hdr의 offset_array) :  %d\n", p->hdr.offset_array);
	// printf("offset_aaray의 시작 주소(data) : %d\n", p-> data);
	// printf("offset_aaray의 시작 주소(p의 시작 주소 + header 크기) : %d\n", (uint64_t)p + sizeof(p->hdr));
	// printf("record의 시작 주소(hdr의 data_region_off) : %d\n", (uint64_t)p + p->hdr.data_region_off);
	// printf("record의 시작 주소(p의 끝에서 page pointer 크기 뺀 만큼) : %d\n", (uint64_t)p + (PAGE_SIZE-1) - sizeof(page*));
	// printf("page의 끝 주소 : %d\n", (uint64_t)p + (PAGE_SIZE-1));

	printf("header size : %d page* size %d", sizeof(slot_header), sizeof(page*));

	return 0;

}


//offset_array는 key값에 따라 오름차순으로 정렬할 것 