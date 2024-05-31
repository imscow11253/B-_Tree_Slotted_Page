
#include <stdint.h>

#define LEAF 1
#define INTERNAL 2


class slot_header{
	public:
		uint16_t page_type;					// main 예시에서는 LEAF 값이 들어갔다. 아마 project3 에서 쓰일 듯?
		uint16_t data_region_off;			// record가 저장될 시작 주소 --> 작아지는 방향으로 자라날 예정
		uint32_t num_data;					// 저장된 record의 개수
		void *offset_array;					// offset array가 시작하는 주소, offset은 2Byte로 정해져 있다. 
		void *sibling_ptr;
		
	public:
		void set_page_type(uint16_t);
		uint16_t get_page_type();
		void set_num_data(uint32_t);
		uint32_t get_num_data();
		uint16_t get_data_region_off();
		void set_data_region_off(uint16_t);
		void set_offset_array(void *);
		void *get_offset_array();
};


