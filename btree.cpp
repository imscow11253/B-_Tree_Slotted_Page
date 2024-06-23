#include "btree.hpp"
#include <iostream> 

//slotted page 가 B+tree 에서의 노드 하나가 된다. leaf node는 구성이 key-value 그대로 이다.
//internal node에서는 구성이 조금 다르다. key-value에서 key는 그대로 key, value는 key보다 큰 쪽에 해당하는 child node의 포인터이다.
//그럼 첫 번째 key보다도 작은 key들이 있는 child node의 포인터는 어디에 있나? 그건 slotted page의 마지막 leftmost_ptr이 나타낸다.

btree::btree(){						//btree 생성자
	root = new page(LEAF);
	height = 1;
};

// void btree::print(){
// 	uint16_t off;
// 	void *offset_array=nullptr;
// 	void *data_region=nullptr;
// 	char *stored_key=nullptr;
// 	void *record_address;

// 	offset_array = root->hdr.get_offset_array();
// 	printf("root : \n");
// 	for(uint32_t i = 0; i < root->hdr.get_num_data(); i++){
// 		off = *((uint16_t *)((uint64_t)offset_array+i*2));
// 		data_region = (void *)((uint64_t)this+(uint64_t)off);
// 		stored_key = root->get_key(data_region);
// 		printf("%d 번째 : %s\n", i, stored_key);
// 	}
// }

void btree::insert(char *key, uint64_t val){
	// Please implement this function in project 3.
	
	page* parent_pages_address[height];
	page* current_node = root;
	
	for(int i =0;i<height-1;i++){						//leaf node에 다다를 때까지 타고 내려가기
		parent_pages_address[i] = current_node;
		current_node = (page*)(current_node -> find(key));
	}

	//이 시점부터는 current_node는 leaf node이고 parent_pages_address에는 root node부터 leaf node를 제외한 internal node들이 저장.

	int index = height-1;
	while(current_node != root){
	
		uint64_t size_of_insert_record = (uint64_t)strlen(key)+11;  //+2+1+8 (2는 record size를 위한 2Byte, 1은 \0 때문이고, 8은 val이 8byte라서)
		
		if(!current_node -> is_full(size_of_insert_record)){
			current_node -> insert(key, val);
			// root -> print();
			return;
		}

		char * parent_key = (char*)malloc(10000000);
		page* new_page_ptr = current_node -> split(key, val, &parent_key);

		key = parent_key;
		val = (uint64_t) new_page_ptr;

		current_node = parent_pages_address[index-1];
		index--;
	}

	//root node에 key를 추가해야하는 상황
	uint64_t size_of_insert_record = (uint64_t)strlen(key)+11;  //+2+1+8 (2는 record size를 위한 2Byte, 1은 \0 때문이고, 8은 val이 8byte라서)
	if(!current_node -> is_full(size_of_insert_record)){
		current_node -> insert(key, val);
		// root -> print();
		return;
	}
	char * parent_key = (char*)malloc(10000000);
	page* new_page_ptr = current_node -> split(key, val, &parent_key);

	page* new_root = new page(INTERNAL);
	new_root -> insert(parent_key,(uint64_t)new_page_ptr);
	new_root->set_leftmost_ptr(root);
	root = new_root;
	height++;

	//root -> print();
}

uint64_t btree::lookup(char *key){
	// Please implement this function in project 3.

	uint64_t val = 0;
	page* current_node = root;

	while(current_node->get_type() != LEAF){				//leaf node에 다다를 때까지 타고 내려가기
		current_node = (page*)(current_node -> find(key));
	}
	//leaf node일 때
	val = current_node->find(key);

	return val;			//없으면 0 return
}

