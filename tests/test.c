#include <CUnit/Cunit.h>
#include <CUnit/Basic.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <zlib.h>
#include "../src/packet_interface.h"
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>

int setup(void){
	return 0;
}

int teardown(void) {
	return 0;
}

void test_get(void){
	struct pkt* test=pkt_new();
	b=0b01010100;
	memcpy(test,&b,sizeof(uint8_t));
	test->seqnum=0b01011011;
	test->length=0b0000010110101101;
	
	CU_ASSERT_EQUAL(pkt_get_tr(test),0);
	CU_ASSERT_EQUAL(pkt_get_type(test),01);
	CU_ASSERT_EQUAL(pkt_get_window(test),10100);
	CU_ASSERT_EQUAL(pkt_get_seqnum(test),0b01011011);
	CU_ASSERT_EQUAL(pkt_get_length(test),0b0000010110101101);	
}

int main(){
	CU_pSuite pSuite = NULL;
	/* initialise le CPU du répertoire de test*/
	if(CUE_SUCCESS!= CU_initialize_registry()){
		return CU_get_error();
	}
	/* ajoute une suite au repertoire*/
	pSuite = CU_add_suite("test",setup,teardown);
	if(pSuite==NULL){
		CU_cleanup_registry();
		return CU_get_error();
	}
	/* ajoute des teste à la suite*/
	if( (NULL == CU_add_test(pSuite,"Test création fractale et retourne ces élements",test_get){
		CU_cleanup_registry();
		return CU_get_error();
	}
	/*fait tourner les tests sur l'interface de base*/
	CU_basic_set_mode(CU_BRM_VERBOSE); 
	CU_basic_run_tests();
	CU_basic_show_failures(CU_get_failure_list());
	/* fait tourner les tests en utilisant l'interface automatisée*/
 	CU_automated_run_tests();
 	CU_list_tests_to_file();
 	/*fait tourner les tests en utilisant l'interface de controle*/
	/*clean le répertoire et le retourne*/
	CU_cleanup_registry();
	return CU_get_error();
}

