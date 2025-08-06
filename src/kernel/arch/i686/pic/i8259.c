#include <stdint.h>
#include <stdbool.h>

#include <arch/i686/pic/i8259.h>
#include <arch/i686/io.h>

#define PIC1_COMMAND_PORT          0x20
#define PIC1_DATA_PORT             0x21
#define PIC2_COMMAND_PORT          0xA0
#define PIC2_DATA_PORT             0xA1

enum {
    PIC_ICW1_ICW4                = 0X01,
    PIC_ICW1_SINGLE              = 0X02,
    PIC_ICW1_INTERVAL4           = 0X04,
    PIC_ICW1_LEVEL               = 0X08,
    PIC_ICW1_INITIALIZE          = 0X10,
} PIC_ICW1;

enum {
    PIC_ICW4_8086                = 0X01,
    PIC_ICW4_AUTO_EOI            = 0X02,
    PIC_ICW4_BUFFER_MASTER       = 0X04,
    PIC_ICW4_BUFFER_SLAVE        = 0X08,
    PIC_ICW4_BUFFERRED           = 0X10,
} PIC_ICW2;

enum {
    PIC_CMD_END_OF_INTERRUPT     = 0X20,
    PIC_CMD_READ_IRR             = 0X0A,
    PIC_CMD_READ_ISR             = 0X0B,
} PIC_CMD;

#define PIC_RESET 0
#define PIC_DISABLE 0xFFFF

static uint16_t g_picmask = 0xFF;
static bool g_AutoEOI = false;

void i8259_SetMask(uint16_t newMask){
    g_picmask = newMask;
    i686_outb(PIC1_DATA_PORT, g_picmask & 0xFF);
    i686_iowait();
    i686_outb(PIC2_DATA_PORT, g_picmask >> 8);
    i686_iowait();
}

uint16_t i8259_GetMask(){
    return i686_inb(PIC1_DATA_PORT) | (i686_inb(PIC2_DATA_PORT) << 8);
}

uint16_t i8259_ReadIRQRequestRegister(){
    i686_outb(PIC1_COMMAND_PORT, PIC_CMD_READ_IRR);
    i686_outb(PIC2_COMMAND_PORT, PIC_CMD_READ_IRR);
    return i686_inb(PIC1_COMMAND_PORT) | (i686_inb(PIC2_COMMAND_PORT) << 8);
}

uint16_t i8259_ReadInServiceRegister(){
    i686_outb(PIC1_COMMAND_PORT, PIC_CMD_READ_ISR);
    i686_outb(PIC2_COMMAND_PORT, PIC_CMD_READ_ISR);
    return i686_inb(PIC1_COMMAND_PORT) | (i686_inb(PIC2_COMMAND_PORT) << 8);
}

void i8259_SendEOI(int irq){
    if (irq >= 8){
        i686_outb(PIC2_COMMAND_PORT, PIC_CMD_END_OF_INTERRUPT);
    }
    i686_outb(PIC1_COMMAND_PORT, PIC_CMD_END_OF_INTERRUPT);

}

void i8259_Configure(uint8_t offsetPic1, uint8_t offsetPic2, bool autoEOI){
    i8259_SetMask(0xFFFF);

    i686_outb(PIC1_COMMAND_PORT, PIC_ICW1_ICW4 | PIC_ICW1_INITIALIZE);
    i686_iowait();
    i686_outb(PIC2_COMMAND_PORT, PIC_ICW1_ICW4 | PIC_ICW1_INITIALIZE);
    i686_iowait();

    i686_outb(PIC1_DATA_PORT, offsetPic1);
    i686_iowait();
    i686_outb(PIC2_DATA_PORT, offsetPic2);
    i686_iowait();

    i686_outb(PIC1_DATA_PORT, 0x4);
    i686_iowait();
    i686_outb(PIC2_DATA_PORT, 0x2);
    i686_iowait();

    uint8_t icw4 = PIC_ICW4_8086;

    if(autoEOI){
        g_AutoEOI = autoEOI;
        icw4 |= PIC_ICW4_AUTO_EOI;
    }

    i686_outb(PIC1_DATA_PORT, icw4);
    i686_iowait();
    i686_outb(PIC2_DATA_PORT, icw4);
    i686_iowait();

    i8259_SetMask(0xFFFF);

}

void i8259_Disable(){
    i8259_SetMask(0xFFFF);
}

void i8259_Mask(int irq){
    i8259_SetMask(g_picmask | 1 << irq);
}

void i8259_Unmask(int irq){
    i8259_SetMask(g_picmask & ~(1 << irq));
}

bool i8259_Probe(){
    i8259_Disable();
    i8259_SetMask(0x1337);
    return i8259_GetMask() == 0x1337;
}

static const PICDriver g_PicDriver = {
    .Name = "i8259_PIC",
    .Probe = &i8259_Probe,
    .Initialize = &i8259_Configure,
    .Disable = &i8259_Disable,
    .SendEOI = &i8259_SendEOI,
    .Mask = &i8259_Mask,
    .Unmask = &i8259_Unmask
};

const PICDriver* i8259_GetDriver(){
    return &g_PicDriver;
}