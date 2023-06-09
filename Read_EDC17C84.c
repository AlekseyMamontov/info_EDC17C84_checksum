#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
char* none = "unknown model";
char* renault = "Renault";
char* fiat = "Fiat";
char* opel = "Opel";

uint8_t* buffer_file;
long       file_size;

char 
    *red_block = "\033[48;5;1m",
    *green_block= "\033[48;5;22m",
    *def_block = "\e[0m",
    *blue_block = "\x1b[48;5;20m",
    *orange_block ="\x1b[48;5;208m";


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

struct VIN_code{
char* code;
char* automaker;
char* model;
char* engine;
char* year;
};

struct VIN_code info_vincode(struct VIN_code* vin){

    uint8_t* vin_text = vin->code;
    uint16_t maker = *((uint16_t*)vin_text);
    switch (maker){

        case 0x465A: 
        vin->automaker = fiat;
        vin->model = "";
        vin->engine = "";
        vin->year = "";
        break; //ZF

        case 0x4656: 
        
        vin->automaker = renault;
        vin->model = "";
        vin->engine = "";
        vin->year = "";
        
        break; //VF
        case 0x3057: 
        vin->automaker = opel;
        vin->model = "";
        vin->engine = "";
        vin->year = "";
        
        
        break; //W0

        default:
        vin->automaker = none;
        vin->model = "";
        vin->engine = "";
        vin->year = "";
        break;

    };








};





/* -------------------- EEPROM DFLASH TC1782 ----------------------*/

void info_block_crc32(uint8_t* buf){

    uint8_t * bCS1,*bCS2;
    char *text1 ="",*text2 ="";

    uint32_t 
    ID_block,*block = (uint32_t*)buf,
    CS1,CS1_count, CS2,CS2_count;

    ID_block = *block & 0x0000FFFF;
    CS1 = (*block)>>16;
    CS2 = *(block+31);
    CS1_count = crc_buffer(ID_block,(block+1),31)&0x0000ffff;
    CS2_count = crc_buffer(*(block+30),(block+2), 28); 

    bCS1 = (uint8_t*)&CS1_count;
    bCS2 = (uint8_t*)&CS2_count; 
    text1 = CS1 == CS1_count?green_block:red_block;
    text2 = CS2 == CS2_count?green_block:red_block;

    printf("\n  0x%04X     0x%05lX   ",ID_block,(buf-buffer_file));
    printf("     %02X %02X  %s %02X %02X \033[0m  ",*(buf+2),*(buf+3),text1,*bCS1,*(bCS1+1));
    printf("     %02X %02X %02X %02X  %s %02X %02X %02X %02X \e[0m  ",
                        *(buf+0x7c),*(buf+0x7d),*(buf+0x7e),*(buf+0x7f),text2,*bCS2,*(bCS2+1),*(bCS2+2),*(bCS2+3));

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

    printf("\nEDC17 TC1782 Checksums of all blocks in eeprom(dflash) \033[32m OK / \033[31m Error\033[0m \n");
    printf("\x1b[48;5;22m ID block    address       CS1 memory(Calc)       CS2 memory (Calculated)     \033[0m \n ");

    uint16_t* id_addr;
    uint8_t* max_buf = (buf+file_size);

    while(buf < max_buf){

        id_addr = (uint16_t*)buf;
        if(*id_addr) info_block_crc32(buf);
        buf +=0x80;

    };

};


void info_block_Date(uint8_t* buf){

uint16_t* id = (uint16_t*)buf;
printf("\n%s Block Date  id 0x%04X   addres 0x%06lX         \x1b[0m \n\n",green_block,*id,buf-buffer_file);
printf(" Date 1 :  %.8s \n",(buf+0x0F));
printf(" Date 2 :  %.8s \n",(buf+0x1A));
printf(" Software :%.10s \n\n",(buf+0x2E));

};

void info_block_ECU(uint8_t* buf){

uint16_t* id = (uint16_t*)buf;
printf("\n%s Block ECU info  id 0x%04X   addres 0x%06lX         \x1b[0m \n\n",blue_block,*id,buf-buffer_file);
printf(" OE:  %.10s \n",(buf+0x08));
printf(" OE:  %.11s \n",(buf+0x1c));
printf(" Type: %.11s \n\n",(buf+0x3c));

};

void info_block_Bosch(uint8_t* buf){

uint16_t* id = (uint16_t*)buf;
printf("\n%s Block Bosch info  id 0x%04X   addres 0x%06lX         \x1b[0m \n\n",blue_block,*id,buf-buffer_file);
printf(" Name:  %s \n",(buf+0x18));
printf(" Type:  %s \n",(buf+0x23));

};

void info_block_ISK(uint8_t* buf){

uint16_t* id = (uint16_t*)buf;
uint8_t* addr = buf;  

printf("\n%s Block ISK id 0x%04X   addres 0x%06lX                     \x1b[0m \n\n",orange_block,*id,buf-buffer_file);
printf("ISK: %x %x %x %x %x %x ",*(addr+0x0d),*(addr+0x0c),*(addr+0x0b),*(addr+0x0a),*(addr+0x09),*(addr+0x08));
printf("%x %x %x %x %x %x ",*(addr+0x13),*(addr+0x12),*(addr+0x11),*(addr+0x10),*(addr+0x0f),*(addr+0x0e));
printf("%x %x %x %x %x %x \n\n",*(addr+0x19),*(addr+0x18),*(addr+0x17),*(addr+0x16),*(addr+0x15),*(addr+0x14));

};



void info_block_VINcode(uint8_t* buf){

uint16_t*id = (uint16_t*)buf;
struct VIN_code vin;
vin.code = buf+0x08;
info_vincode(&vin);

printf("\n%s Block VIN code id 0x%04X  addres 0x%06lX        \x1b[0m \n\n",blue_block,*id,buf-buffer_file);
printf(" VIN code:  %.18s \n",vin.code);
printf(" Automaker :  %s \n",vin.automaker);
printf(" Model :%s \n",vin.model);
printf(" Engine :%s \n",vin.engine);
printf(" Year :%s \n\n",vin.year);

};

void info_block_Mileage(uint8_t* buf){

uint16_t* id = (uint16_t*)buf;
printf("\n%s Block Mileage (km)  id 0x%04X  addres 0x%06lX        \x1b[0m \n\n",green_block,*id,buf-buffer_file);
printf(" Data 1:  %d km \n",*(uint32_t*)(buf+8));
printf(" Data 2:  %d km \n",*(uint32_t*)(buf+12));

};









 
void search_and_print_blok(uint8_t* addr, long max_buffer, uint16_t id){

char* model;
uint16_t id_block;
uint32_t data32,max_data32 = 0;
uint8_t * begin_addr = addr;
uint8_t * max_addr = addr + max_buffer;
uint8_t status = 0,col = 0;

//printf("------------- block № 0x%x ------------ \n \n", id);

  while(addr < max_addr){

    id_block = *(addr + 1);
    id_block <<=8;
    id_block |= *addr;

    if(id_block == id){
        
        switch (id_block){

            case 0x3F:
            case 0x40:
            info_block_VINcode(addr);
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
            info_block_Mileage(addr);
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
            info_block_ISK(addr);
            break;

            case 0x01:info_block_Date(addr);break;

            case 0x14:
            case 0x15:
            case 0x16:
            info_block_ECU(addr);
            break;

            case 0x17:
            case 0x18:
            case 0x19:
            info_block_Bosch(addr);
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

    //info_block_crc32
    info_all_crc_block(buffer);
    printf("\n\n");

    //Date 
    search_and_print_blok(buffer,file_size,0x01);
    search_and_print_blok(buffer,file_size,0x17);
    search_and_print_blok(buffer,file_size,0x15);

    // vincode 
    search_and_print_blok(buffer,file_size,0x3f);
    search_and_print_blok(buffer,file_size,0x40);

   // mileage
    search_and_print_blok(buffer,file_size,0x48);
    search_and_print_blok(buffer,file_size,0x11);

    search_and_print_blok(buffer,file_size,0x85);

   // search_and_print_blok(buffer,file_size,0x43);
   // search_and_print_blok(buffer,file_size,0x4E);
   // search_and_print_blok(buffer,file_size,0x4D);
   // search_and_print_blok(buffer,file_size,0x53);
   // search_and_print_blok(buffer,file_size,0x5b);

    // error






    // освобождаем память и закрываем файл
    free(buffer);
    fclose(file);
    return 0;
}