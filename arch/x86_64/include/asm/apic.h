#ifndef _ASM_ACPI_H
#define _ASM_ACPI_H
#include <asm/cpu.h>
#include <asm/apicdef.h>
#include <asm/msrdef.h>
#include <yaos/types.h>


static inline u32 x2apic_read(unsigned reg){
    return rdmsr(0x800+reg/0x10);
}
static inline void x2apic_write(unsigned reg,u32 value){
    wrmsr(0x800+reg/0x10,value);
}
static inline u32 x2apic_id(){
    return x2apic_read((unsigned)(APIC_ID));
}
static inline void x2apic_enable(){
    u64 msr;
    msr=rdmsr(MSR_IA32_APICBASE);
    if(msr & X2APIC_ENABLE) return;
   
    wrmsr(MSR_IA32_APICBASE,msr|X2APIC_ENABLE);
}
static inline void x2apic_disable(){
    u64 msr=rdmsr(MSR_IA32_APICBASE);
    if(!(msr&X2APIC_ENABLE))return;
    wrmsr(MSR_IA32_APICBASE,msr& ~(X2APIC_ENABLE | XAPIC_ENABLE));
    wrmsr(MSR_IA32_APICBASE,msr& ~(X2APIC_ENABLE));


}
extern u32 *lapic_base;
static inline u32 xapic_read(unsigned reg){
    return lapic_base[reg/4];
}
static inline void xapic_write(unsigned reg,u32 value){
    lapic_base[reg/4]=value;
}
static inline u32 xapic_id(){
    return xapic_read((unsigned)(APIC_ID));
}

extern bool is_x2apic;
static inline u32 lapic_read(unsigned reg){
    if(is_x2apic)return x2apic_read(reg);
    return xapic_read(reg);
}
static inline void lapic_write(unsigned reg,u32 value){
    if(is_x2apic)x2apic_write(reg,value);
    else xapic_write(reg,value);
}
static inline u32 lapic_id(){
    if(is_x2apic){
        return x2apic_id();
    }
    return lapic_read((unsigned)(APIC_ID))>>24;
}
    
#endif
