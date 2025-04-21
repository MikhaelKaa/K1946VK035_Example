// K1921VK035  example
// 15.04.2025
// Михаил Каа

#include <stdio.h>

#include "K1921VK035.h"

#include "retarget.h"
#include "microrl.h"
#include "ucmd.h"
#include "memory_man.h"
#include "term_gxf.h"

#ifndef VERSION
#define VERSION "Dev build 0.00"
const unsigned char build_version[] = VERSION " " __DATE__ " "__TIME__;
#endif /* VERSION */ 

void adc_init() {
  //Инициализация ADC
  RCU->ADCCFG_bit.CLKSEL  = RCU_ADCCFG_CLKSEL_OSECLK;
  RCU->ADCCFG_bit.DIVN    = 0x1;
  RCU->ADCCFG_bit.DIVEN   = 0x1;
  RCU->ADCCFG_bit.CLKEN   = 0x1;
  RCU->ADCCFG_bit.RSTDIS  = 0x1;

  //Настройка модуля АЦП
  ADC->ACTL_bit.ADCEN     = 0x1;
  //Настройка секвенсора 0
  // CH0, CH1, CH2, CH3.
  ADC->EMUX_bit.EM0               = ADC_EMUX_EM0_SwReq;
  ADC->SEQSYNC                    = ADC_SEQSYNC_SYNC0_Msk;
  ADC->SEQ[0].SCCTL_bit.ICNT      = 3;
  ADC->SEQ[0].SRQCTL_bit.RQMAX    = 0x3;
  ADC->SEQ[0].SRQCTL_bit.QAVGVAL  = ADC_SEQ_SRQCTL_QAVGVAL_Average64;
  ADC->SEQ[0].SRQCTL_bit.QAVGEN   = 1;
  ADC->SEQ[0].SRQSEL_bit.RQ0      = 0x0;
  ADC->SEQ[0].SRQSEL_bit.RQ1      = 0x1;
  ADC->SEQ[0].SRQSEL_bit.RQ2      = 0x2;
  ADC->SEQ[0].SRQSEL_bit.RQ3      = 0x3;
  ADC->SEQEN_bit.SEQEN0           = 1;
  while (!ADC->ACTL_bit.ADCRDY) {
  };

  // Прерывание
  ADC->IM_bit.SEQIM0 = 1;
  NVIC_EnableIRQ(ADC_SEQ0_IRQn);
}


uint8_t adc_show_in_console = 0;
void adc_log_sigint(void) {
  printf("adc_log_sigint\r\n");
  adc_show_in_console = 0;
  ucmd_set_sigint(default_sigint);
}


// ucmd handler for mem_dump.
int ucmd_adc(int argc, char *argv[])
{
  switch (argc) {
    case 1:
    printf("adc usage: ---\r\n");
    printf("\tcpy dst src len\r\n");
      break;

    case 3:
      // adc log on
      if((strcmp(&argv[1][0], "log") == 0) &\
         (strcmp(&argv[2][0], "on") == 0)) {
        printf("adc log on\r\n");
        adc_show_in_console = 1;
        ucmd_set_sigint(adc_log_sigint);
      }
      // adc log off
      if((strcmp(&argv[1][0], "log") == 0) &\
         (strcmp(&argv[2][0], "off")) == 0) {
        printf("adc log off\r\n");
        adc_show_in_console = 0;
      }
      break;

    default:
      break;
    }
  return 0;
}

// define command list
command_t cmd_list[] = {
  {
    .cmd  = "help",
    .help = "print available commands with their help text",
    .fn   = print_help_cb,
  },

  {
    .cmd  = "mem",
    .help = "memory man, use mem help",
    .fn   = ucmd_mem,
  },

  {
    .cmd  = "adc",
    .help = "adc log on or ctrl+c",
    .fn   = ucmd_adc,
  },

  {}, // null list terminator DON'T FORGET THIS!
};



void delay(uint32_t val) {
    for(uint32_t i = 0; i < val; i++) { __NOP(); }
}

void gpio_a15_init(void) {
    RCU->HCLKCFG_bit.GPIOAEN    = 1;
    RCU->HRSTCFG_bit.GPIOAEN    = 1;
    GPIOA->DATAOUTSET_bit.PIN15 = 1;
    GPIOA->DENSET_bit.PIN15     = 1;
    GPIOA->OUTENSET_bit.PIN15   = 1;
}

void gpio_a15_toggle(void) {
    GPIOA->DATAOUTTGL_bit.PIN15 = 1;
}

void show_version(void) {

  char l[] = "████████████\000\045\033[H\033[2J\r\n\000\033[0m\r\n";
  printf("%s", &(12*3+1)[l]);
  for(uint8_t i = 0; i < 3; i++, (12*3+1)[l] -= 3) {
    printf("\033[%dm\t\t%s\r\n", (12*3+1)[l], &0[l]);
  }
  printf("%s", &48[l]);

  printf("%s\r\n", build_version);
  set_display_atrib(F_GREEN);
  printf ("MCU ID:  0x%08x\r\n", SIU->CHIPID_bit.ID);
  printf ("MCU REV: 0x%08x\r\n", SIU->CHIPID_bit.REV);
  resetcolor();

  printf("System core clock %ld MHz\r\n", SystemCoreClock/1000000);
}

uint32_t ch_res[] = { 0x777, 0x777, 0x777, 0x777 };


int main() {
    // TODO: код для PLL не работает, нужно починить.
    // SystemInit();
    SystemCoreClockUpdate();
    printf_init();
    show_version();
    printf_flush();
    gpio_a15_init();
    ucmd_default_init();
    int n = 0;

    adc_init();

    while (1) {
      ucmd_default_proc();
      printf_flush();
      
      if(n++%1638==0) {
        gpio_a15_toggle();
        ADC->SEQSYNC |= ADC_SEQSYNC_GSYNC_Msk;
      }
    }
}

static volatile float avdd = 3.264f;
void ADC_SEQ0_IRQHandler()
{
    uint32_t i = 0;

    ADC->IC_bit.SEQIC0 = 1;

    while (ADC->SEQ[0].SFLOAD) {
        ch_res[i++] = ADC->SEQ[0].SFIFO;
    }
    if(adc_show_in_console) {
      clrscr();
      gotoxy(0, 0);
      printf("0x%03x, 0x%03x, 0x%03x, 0x%03x\r\n", ch_res[0], ch_res[1], ch_res[2], ch_res[3]);
      printf("%04d, %04d, %04d, %04d\r\n", ch_res[0], ch_res[1], ch_res[2], ch_res[3]);
      printf("%.03f, %.03f, %.03f, %.03f\r\n",
             (ch_res[0] * avdd) / 4096,
             (ch_res[1] * avdd) / 4096,
             (ch_res[2] * avdd) / 4096,
             (ch_res[3] * avdd) / 4096);
    }
}


void SysTick_Handler() {
    __NOP();
}

void WDT_IRQHandler() {
  // TODO: 
  while (1) __NOP();
}

void UsageFault_Handler() {
  // TODO: 
  while (1) __NOP();
}
