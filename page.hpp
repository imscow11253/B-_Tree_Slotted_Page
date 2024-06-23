#include "slot_header.hpp"

// #define PAGE_SIZE 96	// 한 page에 2개만 들어가는 경우
#define PAGE_SIZE 256 
//#define PAGE_SIZE 4096
class page{      // slotted page 클래스
	private:
		slot_header hdr;   //header 
		char data[PAGE_SIZE-sizeof(slot_header)-sizeof(page*)];    // data라는 변수는 offset array와 record가 저장되는 공간이다. 마지막으로 빼는 sizeof(page*) 의 의미는 leftmose_ptr의 크기를 의미한다. 
		page *leftmost_ptr;    // project3 에 사용될 변수
	public:
		page(uint16_t);									// 생성자

		uint64_t find(char *);							// key값이 들어오면 val 반환,
		bool insert(char *, uint64_t);					// key와 val 을 page에 저장 --> 하나의 record는 2+key_size+8 Byte 이다. (record 크기 + key 크기 + value 크기)
		void print();									// page 내부의 모든 offset 과 record를 반환
		bool is_full(uint64_t);
		uint16_t get_record_size(void*);
		char *get_key(void *);
		uint64_t get_val(void *);
		uint16_t get_type();
		page* split(char *, uint64_t, char**);
		void set_leftmost_ptr(page *);
		page *get_leftmost_ptr();
		void defrag();
};


