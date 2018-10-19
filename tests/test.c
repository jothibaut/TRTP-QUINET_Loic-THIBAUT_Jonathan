#include <CUnit/Cunit.h>
#include <CUnit/Basic.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <zlib.h>
#include "packet_interface.h"
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>

int setup(void){
	return 0;
}

int teardown(void) {
	return 0;
}

void test_get_width(void){
	struct fractal* test=new_fractal("name",800,800,0.8,-0.4);
	CU_ASSERT_EQUAL(fractal_get_width(test),800);
	CU_ASSERT_EQUAL(fractal_get_name(test),"name");
	CU_ASSERT_EQUAL(fractal_get_height(test),800);
	CU_ASSERT_EQUAL(fractal_get_a(test),0.8);
	CU_ASSERT_EQUAL(fractal_get_b(test),-0.4);
}

int main(){
	CU_pSuite pSuite = NULL;
	if(CUE_SUCCESS!= CU_initialize_registry()){
		return CU_get_error();
	}
	pSuite = CU_add_suite("test",setup,teardown);
	if(pSuite==NULL){
		CU_cleanup_registry();
		return CU_get_error();
	}
	if( (NULL == CU_add_test(pSuite,"Test création fractale et retourne ces élements",test_get_width){
		CU_cleanup_registry();
		return CU_get_error();
	}
	CU_basic_run_tests();
	CU_basic_show_failures(CU_get_failure_list());
	CU_cleanup_registry();
	return CU_get_error();
}

