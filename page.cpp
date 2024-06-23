#include "page.hpp"
#include <iostream> 
#include <cstring> 

void put2byte(void *dest, uint16_t data){
	*(uint16_t*)dest = data;
}

uint16_t get2byte(void *dest){
	return *(uint16_t*)dest;
}

page::page(uint16_t type){											//생성자
	hdr.set_num_data(0);											//현재 page 안에 있는 record는 0개
	hdr.set_data_region_off(PAGE_SIZE-1-sizeof(page*));				//record가 저장될 시작 offset --> 작아지는 방향으로 자라날 예정 (근데 왜 뒤에 page 사이즈만큼을 빼지?) --> 해당 부분은 b+tree 할 때 사용할 예정
	hdr.set_offset_array((void*)((uint64_t)this+sizeof(slot_header)));  //offset_array의 시작 주소 --> page 객체의 시작 주소에서 header 의 주소 공간 다음의 주소 번지를 지정 
	hdr.set_page_type(type);										//page의 type은 인자로 들어온 type --> LEAF 값이 들어옴.
}

uint16_t page::get_type(){											//header에서 type을 반환한다. 
	return hdr.get_page_type();
}

uint16_t page::get_record_size(void *record){						//record 시작 주소 (=offset의 값)을 인자로 받아서 해당 record 의 size 정보 (2Byte)를 반환 
	uint16_t size = *(uint16_t *)record;
	return size;
}

char *page::get_key(void *record){									//record 시작 주소 (=offset의 값)을 인자로 받아서 해당 record 의 key 의 시작 주소를 반환, c언어에서 string은 char의 시작 주소에서 \0 값까지 여기기 때문에 그냥 시작 주소값만 줘도 어디까지 key인지 알 수 있다. 
	char *key = (char *)((uint64_t)record+sizeof(uint16_t));
	return key;
}

uint64_t page::get_val(void *key){									// record에서 key의 시작 주소를 받아서 val의 값을 반환 (key 시작 주소 + key의 크기(\0까지의 크기) 한 주소가 가르키는 값을 반환)
	uint64_t val= *(uint64_t*)((uint64_t)key+(uint64_t)strlen((char*)key)+1);
	return val;
}

void page::set_leftmost_ptr(page *p){
	leftmost_ptr = p;
}

page *page::get_leftmost_ptr(){
	return leftmost_ptr;	
}

uint64_t page::find(char *key){
	// Please implement this function in project 2.

	uint64_t val = 0;

	uint16_t off;
	void *offset_array=nullptr;
	void *data_region=nullptr;
	char *stored_key=nullptr;
	void *record_address;

	offset_array = hdr.get_offset_array();

	for(uint32_t i = 0; i < hdr.get_num_data(); i++){
		off = *((uint16_t *)((uint64_t)offset_array+i*2));
		data_region = (void *)((uint64_t)this+(uint64_t)off);
		stored_key = get_key(data_region);

		if(get_type() == LEAF){					//현재 노드가 leaf node일 때 
			if(strcmp(stored_key, key) == 0){
				val = get_val(stored_key);
				return val;
			}
		}
		else{											//현재 노드가 internal node일 때 
			if(strcmp(stored_key, key) > 0){
				return (val == 0) ? (uint64_t)get_leftmost_ptr() : val;	//internal node의 key가 비교값 보다 같거나 크다면 
			}
			else{
				val = (uint64_t)get_val(stored_key);
			}
		}
	}
	return val;
}

bool page::insert(char *key,uint64_t val){
	// Please implement this function in project 2.

	//offset_array 시작주소 구해놓기
	void * offset_array = hdr.get_offset_array();
	//insert 할 record의 크기 구해놓기
	uint64_t size_of_insert_record = (uint64_t)strlen(key)+11; //+2+1+8 (2는 record size를 위한 2Byte, 1은 \0 때문이고, 8은 val이 8byte라서)

	//만약 isFull() == true 이면 false 반환하기
	if(is_full(size_of_insert_record)) {
		return false;
	}

	uint16_t off;
	void *data_region=nullptr;
	char *stored_key=nullptr;
	uint16_t *index_of_insert;
	int is_insert_check = 0;
	void *record_address;

	for(uint32_t i = 0; i < hdr.get_num_data(); i++){
		
		//offset_array에서 순차적으로 offset 값 가져오기 --> index_of_insert에는 현재 offset의 주소, off에는 offset 값 (해당 record의 주소)이 들어간다. 
		index_of_insert = (uint16_t *)((uint64_t)offset_array+i*2);
		off = *index_of_insert;

		//offset 값 기반으로 record 위치 주소 가져오기
		data_region = (void *)((uint64_t)this+(uint64_t)off);

		//record 주소 기반으로 key값 추출하기 
		stored_key = get_key(data_region);

		//삽입할 key보다 크면 한 칸씩 미루고 빈 공간에 insert
		if(strcmp(stored_key, key) > 0) {
			//미루기
			uint16_t post_data, temp_data;
			uint16_t *iterator = index_of_insert;
			uint32_t limit = hdr.get_num_data();
			for(int j=0;j<limit - i+1; j++){
				// printf("im working!!!\n");
				temp_data = post_data;
				post_data = *(iterator);

				if(j!=0){	//한 칸씩 미루기
					*(iterator) = temp_data;
				}
				else{ // 삽입 대상 넣기
					*(iterator) = hdr.get_data_region_off() - size_of_insert_record + 1;
					hdr.set_num_data(hdr.get_num_data() + 1);
					record_address = (void *)((uint64_t)this+(uint64_t)*(iterator));				//record 시작 주소를 void* type으로 계산
				}
				iterator += 1;
			}
			is_insert_check =1;
			break;
		}
	}

	if(!is_insert_check){			
		if(hdr.get_num_data() == 0){ 		//첫 offset인 경우
			*((uint16_t *)offset_array) = hdr.get_data_region_off() - size_of_insert_record + 1;
			record_address = (void *)((uint64_t)this+(uint64_t)*((uint16_t *)offset_array));		//record 시작 주소를 void* type으로 계산
		}
		else {							//offset_array 가장 끝에 insert하는 경우
			*(index_of_insert + 1) = hdr.get_data_region_off() - size_of_insert_record + 1;
			record_address = (void *)((uint64_t)this+(uint64_t)*(index_of_insert + 1));				//record 시작 주소를 void* type으로 계산
		}
		hdr.set_num_data(hdr.get_num_data() + 1);
	}

	//record 값 삽입 =====================================================================================================================

	//record size(2Byte)를 넣기 위해 record_address 를 2byte 포인터(uint16_t*)로 변환해서 값 삽입
	*((uint16_t*)record_address) = (uint16_t)size_of_insert_record;
	//key 값을 넣기 위해 record_address(주소 단위 -> Byte 단위) + 2 를 한 다음, (그냥 주소 값에 + 2를 하고 싶은거지 주소가 가르키는 크기만큼 2칸 이동하라는 뜻이 아니므로 record_address를 그냥 숫자 타입으로 변환하고 + 2를 함) 
	//1Byte 포인터(char *)로 변환해서 값 삽입
	for(int i =0; i < strlen(key); i++){
		*((char *)((uint64_t)record_address + 2 + i)) = *(key+i);
	}
	//value 값을 넣기 위해 record_address(주소 단위 -> Byte 단위) + record_size(Byte 단위) - 8
	//8Byte 포인터(uint64_t*)로 변환해서 값 삽입
	*((uint64_t*)((uint64_t)record_address + size_of_insert_record - 8)) = val;
	
	hdr.set_data_region_off(hdr.get_data_region_off() - size_of_insert_record);

	// print();
	return true;
}

typedef struct record{
	char* key;
	uint64_t value;
} Record;

page* page::split(char *key, uint64_t val, char** parent_key){
	// Please implement this function in project 3.
	Record* arr[PAGE_SIZE];
	int size=0;

	uint64_t stored_val = 0;
	uint16_t off;
	void *offset_array=nullptr;
	void *data_region=nullptr;
	char *stored_key=nullptr;
	void *record_address;

	offset_array = hdr.get_offset_array();
	
	//기존 slotted_page에 있던 record들을 배열에 복사 --> 가득 차있는 상태에서 새로운 데이터를 넣을 수 없기 때문 --> 정렬해서 split하기 위해서
	for(uint32_t i = 0; i < hdr.get_num_data(); i++){
		off = *((uint16_t *)((uint64_t)offset_array+i*2));
		data_region = (void *)((uint64_t)this+(uint64_t)off);
		stored_key = get_key(data_region);
		stored_val = get_val(stored_key);

		arr[i] = (Record *)malloc(sizeof(Record));
		arr[i]->key = stored_key; 
		arr[i]->value = stored_val;
		size++;
	}
	
	//새로 추가될 record를 배열에 넣기
	Record* post_data;
	Record* temp;
	for(uint32_t i = 0; i < size; i++){
		if(strcmp(arr[i]->key, key) >= 0){
			post_data = arr[i];
			arr[i] = (Record *)malloc(sizeof(Record));
			arr[i]->key = key; 
			arr[i]->value = val;

			//한 칸씩 미루기
			for(int j=i+1;j<size;j++){
				temp = post_data;
				post_data = arr[j];
				arr[j] = temp;
			}

			break;
		}
		if(i == size-1){
			arr[i+1] = (Record *)malloc(sizeof(Record));
			arr[i+1]->key = key; 
			arr[i+1]->value = val;
		}
	}
	size++;

	// for(int i=0; i<size; i++){
	// 	printf("arr[i] : %s and %d\n", arr[i]->key, arr[i]->value);
	// }

	int medium = size/2;

	//앞의 절반은 this page에 저장 
	page *new_page = new page(get_type());
	for(int i=0; i<medium; i++){
		new_page->insert(arr[i]->key, arr[i]->value);				//새로 만든 page에 그 값 저장
	}	
	new_page->set_leftmost_ptr(get_leftmost_ptr());					//leftmost_ptr 은 값 그대로 복사


	//뒤의 절반은 새로운 page에 저장
	page *new_page2 = new page(get_type());
	for(int i=medium; i<size; i++){
		if(i == medium && get_type() == INTERNAL){				//새로 생긴 page의 key값을 부모 page에 추가하기 위해서 인자로 넘어온 parent_key 변수에 저장
			strcpy(*parent_key, arr[i]->key);					//split 하는 노드가 internal이면 첫 번째 record는 저장 x, parent node에게 넘겨야 한다.
			new_page2->set_leftmost_ptr((page*)arr[i]->value);
			continue;
		}
		else if(i == medium){
			strcpy(*parent_key, arr[i]->key);
			new_page2->insert(arr[i]->key, arr[i]->value);
			continue;	
		}
		new_page2->insert(arr[i]->key, arr[i]->value);				//새로 만든 page에 그 값 저장
	}

	char* temp_parent_key = new char[strlen(*parent_key) + 1];
    strcpy(temp_parent_key, *parent_key);

	memcpy(this, new_page, sizeof(page));							//이 page(this)에 새로 구성한 page (new_page)를 복사
	hdr.set_offset_array((void*)((uint64_t)this+sizeof(slot_header)));
	delete new_page;

	strcpy(*parent_key, temp_parent_key);

	for(int i=0;i<size;i++){
		free(arr[i]);
	}
	free(temp_parent_key);

	return new_page2;											//parent node에 추가할 value 값
}

bool page::is_full(uint64_t inserted_record_size){ //인자는 현재 삽입할 records의 size	
	// Please implement this function in project 2.

	if(hdr.get_data_region_off() - (sizeof(slot_header) - 1)  - (2 * hdr.get_num_data()) >= inserted_record_size + 2){  //offset을 위한 2Byte도 고려해야 함. inserted_record_size + 2
		return false;
	}
	return true;
}

//제공되는 함수이지만 난 안썼다.
void page::defrag(){												//아마 project3에서 쓰일 듯
	page *new_page = new page(get_type());							//이 노드에서 0 ~ num_data/2-1 까지의 데이터를 남기고 삭제한다.
	int num_data = hdr.get_num_data();
	void *offset_array=hdr.get_offset_array();
	void *stored_key=nullptr;
	uint16_t off=0;
	uint64_t stored_val=0;
	void *data_region=nullptr;

	for(int i=0; i<num_data/2; i++){
		off= *(uint16_t *)((uint64_t)offset_array+i*2);	        	//offset의 값 (= record의 주소값)
		data_region = (void *)((uint64_t)this+(uint64_t)off);		//해당 record의 포인터
		stored_key = get_key(data_region);							//key 추출
		stored_val= get_val((void *)stored_key);					//value 추출
		new_page->insert((char*)stored_key,stored_val);				//새로 만든 page에 그 값 저장
	}	
	new_page->set_leftmost_ptr(get_leftmost_ptr());					//leftmost_ptr 은 값 그대로 복사

	memcpy(this, new_page, sizeof(page));							//이 page(this)에 새로 구성한 page (new_page)를 복사
	hdr.set_offset_array((void*)((uint64_t)this+sizeof(slot_header)));
	delete new_page;

}

void page::print(){													//page 에 존재하는 offset_array값과 record의 값을 차례로 출력한다.
	uint32_t num_data = hdr.get_num_data();
	uint16_t off=0;
	uint16_t record_size= 0;
	void *offset_array=hdr.get_offset_array();
	void *stored_key=nullptr;
	uint64_t stored_val=0;

	printf("## slot header\n");
	printf("Number of data :%d\n",num_data);
	printf("offset_array : |");
	for(int i=0; i<num_data; i++){
		off= *(uint16_t *)((uint64_t)offset_array+i*2);				//offset_array에 저장되는 offset의 크기는 2Byte이다. void* 타입은 포인터 변수가 가르키는 주소의 크기를 모를 때 사용하는 변수이다. i가 1씩 증가함에 따라 off 변수는 2(Byte)씩 증가한다. 
		printf(" %d |",off);
	}
	printf("\n");

	void *data_region=nullptr;
	for(int i=0; i<num_data; i++){
		off= *(uint16_t *)((uint64_t)offset_array+i*2);	
		data_region = (void *)((uint64_t)this+(uint64_t)off);
		record_size = get_record_size(data_region);
		stored_key = get_key(data_region);
		stored_val= get_val((void *)stored_key);
		printf("==========================================================\n");
		printf("| data_sz:%u | key: %s | val :%lu | key_len:%lu\n",record_size,(char*)stored_key, stored_val,strlen((char*)stored_key));

	}
}




