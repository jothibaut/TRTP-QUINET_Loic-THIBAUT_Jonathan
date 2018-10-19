#include "packet_interface.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <zlib.h>


/* Extra #includes */
/* Your code will be inserted here */

struct __attribute__((__packed__)) pkt {
uint8_t  window:5;
uint8_t  tr:1;
ptypes_t type:2;
uint8_t  seqnum;
uint16_t length;
uint32_t timestamp;
uint32_t crc1;
uint32_t crc2;
char* payload;
};

/* Extra code */
/* Your code will be inserted here */

pkt_t* pkt_new()
{
    struct pkt* r=(struct pkt*) calloc (1 , sizeof ( pkt_t ));
    if(r==NULL){
        return NULL;
    }
    return r;
}

void pkt_del(pkt_t *pkt)
{
    if(pkt->payload!=NULL){
        free(pkt->payload);
    }
    free(pkt);
}

pkt_status_code pkt_decode(const char *data, const size_t len, pkt_t *pkt)
{
    size_t offset=0;
    memcpy(pkt, data, sizeof(uint8_t));
    ptypes_t ty=pkt_get_type  (pkt);
    pkt_status_code staty=pkt_set_type(pkt, ty);
    if(staty!=0){
        return E_TYPE;
    }
    uint8_t tr= pkt_get_tr(pkt);
    pkt_status_code statr=pkt_set_tr(pkt, tr);
    if(statr!=0){
        return E_TR;
    }
    uint8_t window= pkt_get_window(pkt);
    pkt_status_code statwin=pkt_set_window(pkt, window);
    if(statwin!=0){
        return E_WINDOW;
    }
    offset=offset+sizeof(uint8_t);
    uint8_t buf1;
    memcpy(&buf1, data+offset, sizeof(uint8_t));
    pkt_set_seqnum(pkt, buf1);
    offset=offset+sizeof(uint8_t);
    
    uint16_t buf2;
    memcpy(&buf2, data+offset, sizeof(uint16_t));
    uint16_t length=ntohs(buf2);
    pkt_set_length(pkt, length);
    offset=offset+sizeof(uint16_t);
    
    uint32_t buf3;
    memcpy(&buf3, data+offset, sizeof(uint32_t));
    pkt_set_timestamp(pkt, buf3);
    offset=offset+sizeof(uint32_t);
    uint32_t buf4;
    memcpy(&buf4, data+offset, sizeof(uint32_t));
    buf4=ntohl(buf4);
    uint64_t controle;
    uint8_t b=0b11011111;
    memcpy(&controle, data, offset);
    uint8_t modif;
    memcpy(&modif,&controle,sizeof(uint8_t));
    modif=modif & b;
    memcpy(&controle,&modif,sizeof(uint8_t));
    offset=offset+sizeof(uint32_t);
    uint32_t cr1=crc32(0,(Bytef *) &controle,sizeof(uint64_t));
    if(cr1==buf4){
        pkt_set_crc1(pkt, cr1);
    }
    else{
        return E_CRC;
    }
    
    if(pkt_get_length(pkt)!=0){
        pkt_status_code retourpay;
        char buf[pkt_get_length(pkt)];
    	memcpy(buf, data+offset, pkt_get_length(pkt)*sizeof(char));
        retourpay=pkt_set_payload(pkt, buf, pkt_get_length(pkt));
        if(retourpay!=0){
            return E_NOMEM;
        }
        offset=offset+(pkt_get_length(pkt))*sizeof(char);
        if(offset!=len){
            uint32_t buf5;
            memcpy(&buf5, data+offset, sizeof(uint32_t));
            buf5=ntohl(buf5);
            uint32_t cr2=crc32(0,(Bytef *)pkt_get_payload(pkt),pkt_get_length(pkt)*sizeof(char));
            if( buf5==cr2){
                pkt_set_crc2(pkt, cr2);
                offset=offset+sizeof(uint32_t);
            }
            else{
                return E_CRC;
            }
        }
    }
    if(len==offset){
        return 0;
    }
    else{
        return E_UNCONSISTENT;
    }
   return E_UNCONSISTENT;
}

pkt_status_code pkt_encode(const pkt_t* pkt, char *buf, size_t *len)
{
	if(sizeof(pkt)>*len){
        return E_NOMEM;
    }
    size_t offset=0;
    uint8_t  modiftr;
    char  sauvegardebuf[sizeof(uint8_t)];
    memcpy(buf, pkt, sizeof(uint8_t));
    offset=offset+sizeof(uint8_t);
    memcpy(buf+offset, &(pkt->seqnum), sizeof(uint8_t));
    offset=offset+sizeof(uint8_t);
    uint16_t length = htons(pkt->length);
    memcpy(buf+offset, &length, sizeof(uint16_t));
    offset=offset+sizeof(uint16_t);
    memcpy(buf+offset, &(pkt->timestamp), sizeof(uint32_t));
    offset=offset+sizeof(uint32_t);
    
    memcpy(sauvegardebuf, buf , sizeof(uint8_t));
    memcpy(&modiftr, buf , sizeof(uint8_t));
    uint8_t b=0b11011111;
    uint8_t modif=modiftr & b;
    memcpy(buf, &modif , sizeof(uint8_t));
    uint32_t cr1=crc32(0,(Bytef *)buf,sizeof(uint64_t));
    uint32_t crc1=htonl(cr1);
    memcpy(buf, sauvegardebuf , sizeof(uint8_t));
    memcpy(buf+offset, &crc1 , sizeof(uint32_t));
    offset=offset+sizeof(uint32_t);
    if(pkt_get_length(pkt)!=0){
        memcpy(buf+offset, pkt_get_payload(pkt), sizeof(char)*pkt_get_length(pkt));
        offset=offset+pkt_get_length(pkt)*sizeof(char);
        uLong cr2=crc32(0,(Bytef *)pkt_get_payload(pkt),sizeof(char)*pkt_get_length(pkt));
        uint32_t crc2=htonl(cr2);
    	memcpy(buf+offset,&crc2, sizeof(uint32_t));
        offset=offset+sizeof(uint32_t);
    }
    *len=offset;
    return 0;
}

ptypes_t pkt_get_type  (const pkt_t* pkt)
{
	return pkt->type;
}

uint8_t  pkt_get_tr(const pkt_t* pkt)
{
	return pkt->tr;
}

uint8_t  pkt_get_window(const pkt_t* pkt)
{
	return pkt->window;
}

uint8_t  pkt_get_seqnum(const pkt_t* pkt)
{
	return pkt->seqnum;
}

uint16_t pkt_get_length(const pkt_t* pkt)
{
	return pkt->length;
}

uint32_t pkt_get_timestamp   (const pkt_t* pkt)
{
	return pkt->timestamp;
}

uint32_t pkt_get_crc1   (const pkt_t* pkt)
{
	return pkt->crc1;
}

uint32_t pkt_get_crc2   (const pkt_t* pkt)
{
    return pkt->crc2;
}

const char* pkt_get_payload(const pkt_t* pkt)
{
    if(pkt->length==0){
        return NULL;
    }
    return pkt->payload;
}


pkt_status_code pkt_set_type(pkt_t *pkt, const ptypes_t type)
{
	if(type>4 ||type<0){
        return E_TYPE;
    }
    pkt->type=type;
    return PKT_OK;
}

pkt_status_code pkt_set_tr(pkt_t *pkt, const uint8_t tr)
{
    if(tr>1){
        return E_TR;
    }
	pkt->tr=tr;
    return PKT_OK;
}

pkt_status_code pkt_set_window(pkt_t *pkt, const uint8_t window)
{
	if(window>MAX_WINDOW_SIZE){
        return E_WINDOW;
    }
    pkt->window=window;
    return PKT_OK;
}

pkt_status_code pkt_set_seqnum(pkt_t *pkt, const uint8_t seqnum)
{
    pkt->seqnum=seqnum;
    return PKT_OK;
}

pkt_status_code pkt_set_length(pkt_t *pkt, const uint16_t length)
{
	if(length<0 || length>512){
		return E_length;
	}
    pkt->length=length;
    return PKT_OK;
}

pkt_status_code pkt_set_timestamp(pkt_t *pkt, const uint32_t timestamp)
{
	pkt->timestamp=timestamp;
    return PKT_OK;
}

pkt_status_code pkt_set_crc1(pkt_t *pkt, const uint32_t crc1)
{
	pkt->crc1=crc1;
    return PKT_OK;
}

pkt_status_code pkt_set_crc2(pkt_t *pkt, const uint32_t crc2)
{
	pkt->crc2=crc2;
    return PKT_OK;
}

pkt_status_code pkt_set_payload(pkt_t *pkt,
							    const char *data,
								const uint16_t length)
{
	if(data==NULL || length==0){
        free(pkt->payload);
        pkt->payload=NULL;
        pkt->length=0;
        return 0;
    }
    if(sizeof(data)>512){
        return E_NOMEM;
    }
    pkt->payload=(char*)malloc(length*sizeof(char));
    if(pkt->payload==NULL){
        return E_NOMEM;
    }
    pkt->length=length;
    pkt->payload=(char*)memcpy((void*) pkt->payload,(void*) data, length);
    return 0;
}
