#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
char* none = "unknown model";
char* renault = "Renault Traffic";
char* fiat = "Fiat Tolento";
char* opel = "Opel Vivaro";

uint8_t* buffer_file;
long       file_size;

// crc 32bit CS1 CS2

unsigned long crc_buffer(uint32_t crc, const uint32_t *p, unsigned long sz)
{
   const uint32_t poly = 0xEDB88320;//poly
   uint32_t tmp1, tmp2;

   while(sz--)
   {
      tmp1 = 0;
      tmp2 = crc & poly;
      for(int i = 0; i <=31; i++ ) tmp1 ^= ((tmp2 >> i) & 1);
      crc = *p++ ^ ((crc << 1) | tmp1) ;
   }
   return crc;
}


void info_block_crc32(uint8_t* buf){

    uint8_t * bCS1,*bCS2;

    uint32_t 
    ID_block,*block = (uint32_t*)buf,
    CS1,CS1_count,
    CS2,CS2_count;

    ID_block = *block & 0x0000FFFF;
    CS1 = (*block)>>16;
    CS2 = *(block+31);
    CS1_count = crc_buffer(ID_block,(block+1),31)&0x0000ffff;
    CS2_count = crc_buffer(*(block+30),(block+2), 28); 

    bCS1 = (uint8_t*)&CS1_count;
    bCS2 = (uint8_t*)&CS2_count; 

    printf("\n  0x%04X     0x%05lX   ",ID_block,(buf-buffer_file));
    if(CS1 == CS1_count){printf("     %02X%02X (\033[32m %02X%02X \033[0m)  ",*(buf+2),*(buf+3),*bCS1,*(bCS1+1));
        }else{printf("     %04X  (\e[31m %04X \e[0m)  ",CS1,CS1_count);}

    if(CS2 == CS2_count){printf("    %08X (\e[32m %08X \e[0m)  ",CS2,CS2_count);
        }else{printf("    %08X (\e[31m %08X \e[0m)",CS2,CS2_count);}
    //printf("\n");

};

uint8_t *search_id_block(uint8_t* buf,uint16_t id){

    uint16_t* id_addr;
    uint8_t* max_buf = (buf+file_size);

    while(buf < max_buf){

        id_addr = (uint16_t*)buf;
        if( *id_addr == id) break;
        buf +=0x80;

    }
    
    return buf;
};

void info_all_crc_block(uint8_t* buf){
    printf("EDC17 TC1782 hecksums of all blocks in eeprom(dflash) \033[32m OK / \033[31m Error\033[0m \n");
    printf("\x1b[48;5;22m ID block    address     CS1 memory(Calc)    CS2 memory (Calculated) \x1b[0m \n");

    uint16_t* id_addr;
    uint8_t* max_buf = (buf+file_size);

    while(buf < max_buf){

        id_addr = (uint16_t*)buf;
        if(*id_addr) info_block_crc32(buf);
        buf +=0x80;

    };

};


 
void search_and_print_blok(uint8_t* addr, long max_buffer, uint16_t id){

char* model;
uint16_t id_block;
uint32_t data32,max_data32 = 0;
uint8_t * begin_addr = addr;
uint8_t * max_addr = addr + max_buffer;
uint8_t status = 0,col = 0;

printf("------------- block № 0x%x ------------ \n \n", id);

  while(addr < max_addr){

    id_block = *(addr + 1);
    id_block <<=8;
    id_block |= *addr;

    if(id_block == id){
        
        switch (id_block){

            case 0x3F:
            case 0x40:
            
                switch(*(addr+8)){
                    case 0x56: model = renault;break;
                    case 0x57: model = opel;break;
                    case 0x5A: model = fiat;break;
                    default: model = none;break;};
            printf("Model auto: %s \n",model);   
            printf("Vin_code: %s  \n",(addr+8));
            printf("eeprom addr: 0x%lx \n\n",(addr-begin_addr));
            break;

            case 0x43:

                for(uint8_t i=0;i<20;i++){
                    data32 = *(uint32_t*)(addr+(8+(i*4)));
                    if(max_data32 < data32) max_data32 = data32;
                }
                if(max_data32){printf("Mileage: %d km \n",max_data32);
                               printf("eeprom addr: 0x%lx \n\n",(addr-begin_addr));}
 
            break;

            case 0x48:
            case 0x4D:
                status = 0;
                data32 = *(uint32_t*)(addr+8);
                if(data32){printf("Mileage(0): %d km \n",data32);status=1;}
                data32 = *(uint32_t*)(addr+12);
                if(data32){printf("Mileage(1): %d km \n \n",data32);status=1;}
                if(status)printf("eeprom addr: 0x%lx \n\n",(addr-begin_addr));
            break;

            case 0x4E:
                data32 = *(uint32_t*)(addr+0x6c);     
                if(data32){
                        printf("eeprom addr: 0x%lx \n\n",(addr-begin_addr)); 
                        printf("Mileage: %d km \n",data32);}
            break;
            
            case 0x53:
                status = 0;
                data32 = *(uint32_t*)(addr+0x20);
                if(max_data32 < data32){
                     printf("Mileage(0): %d km \n \n",data32);
                     max_data32 = data32;status=1;
                };
                data32 = *(uint32_t*)(addr+0x30);
                if(max_data32 < data32){
                     printf("Mileage(1): %d km \n \n",data32);
                     max_data32 = data32;status=1;
                };
                if(status)printf("eeprom addr: 0x%lx \n\n",(addr-begin_addr));
            break;

            case 0x5B:
                data32 = *(uint32_t*)(addr+0x28);     
                if(data32){
                        printf("eeprom addr: 0x%lx \n\n",(addr-begin_addr)); 
                        printf("Mileage: %d km \n",data32);}
            break;

            case 0x85:

                printf("eeprom addr check (%d): 0x%lx \n",col,(addr-begin_addr)); 
                col++;

            break;

            case 0x11:
            case 0x12:
            case 0x13:

            printf("ISK: %x %x %x %x %x %x ",*(addr+0x0d),*(addr+0x0c),*(addr+0x0b),*(addr+0x0a),*(addr+0x09),*(addr+0x08));
            printf("%x %x %x %x %x %x ",*(addr+0x13),*(addr+0x12),*(addr+0x11),*(addr+0x10),*(addr+0x0f),*(addr+0x0e));
            printf("%x %x %x %x %x %x \n\n",*(addr+0x19),*(addr+0x18),*(addr+0x17),*(addr+0x16),*(addr+0x15),*(addr+0x14));

            break;

            case 0x01:
            printf("Year: %s \n", (addr+0x0f));
            printf("Software ECU: %s \n ",(addr+0x1A));
            break;

            case 0x14:
            case 0x15:
            case 0x16:
            printf("ECU serial: %s \n", (addr+0x08));
            printf("ECU serial: %s \n", (addr+0x1C));
            printf("ECU name: %s \n\n", (addr+0x29));
            break;

            case 0x17:
            case 0x18:
            case 0x19:
            printf("Bosch: %s \n", (addr+0x08));
            printf("ECU name: %s \n\n", (addr+0x23));
            break;







    }}
    addr +=0x80;
   };

};



int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s filename\n", argv[0]);
        return 1;
    }

    uint8_t* begin_addr;

    char *filename = argv[1];
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        printf("Error opening file %s\n", filename);
        return 1;
    }
 
    // size
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    rewind(file);

    // memory
    buffer_file = (uint8_t*) malloc(file_size);
    uint8_t* buffer = buffer_file;

    if (buffer == NULL) {
        printf("Error allocating memory\n");
        fclose(file);
        return 1;
    }

    // read file -> buffer
    fread(buffer, file_size, 1, file);
    search_and_print_blok(buffer,file_size,0x01);
   // vincode 
    search_and_print_blok(buffer,file_size,0x3f);
    search_and_print_blok(buffer,file_size,0x40);
   // mileage
    search_and_print_blok(buffer,file_size,0x48);
    search_and_print_blok(buffer,file_size,0x43);
    search_and_print_blok(buffer,file_size,0x4E);
    search_and_print_blok(buffer,file_size,0x4D);
    search_and_print_blok(buffer,file_size,0x53);
    search_and_print_blok(buffer,file_size,0x5b);

    // error
    search_and_print_blok(buffer,file_size,0x85);

    // 
    search_and_print_blok(buffer,file_size,0x11);
    search_and_print_blok(buffer,file_size,0x15);
    search_and_print_blok(buffer,file_size,0x17);


    //info_block_crc32(buffer + 0x10100);
    info_all_crc_block(buffer);
    printf("\n\n");


    // освобождаем память и закрываем файл
    free(buffer);
    fclose(file);
    return 0;
}