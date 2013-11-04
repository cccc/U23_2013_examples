#ifndef BITBANGING_H_WGO01OSL
#define BITBANGING_H_WGO01OSL


// To view what happens while bitbanging exchange these two lines
#define wait_data 0
//#define wait_data 1000000

void init_gpio(void);
void gpiospi_write_buffer(uint32_t* data, int data_count);
void gpiospi_write_int32(uint32_t data);


#endif /* end of include guard: BITBANGING_H_WGO01OSL */

