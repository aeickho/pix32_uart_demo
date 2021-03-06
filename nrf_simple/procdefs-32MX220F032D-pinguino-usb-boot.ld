/*************************************************************************
 * Processor-specific object file.  Contains SFR definitions.
 *************************************************************************/
INPUT("processor.o")

/*************************************************************************
 * Processor-specific peripheral libraries are optional
 *************************************************************************/
OPTIONAL("libmchp_peripheral.a")
OPTIONAL("libmchp_peripheral_32MX220F032D.a")

/*************************************************************************
 * For interrupt vector handling
 *************************************************************************/
PROVIDE(_vector_spacing = 0x00000001);
_ebase_address  = 0x9D003000;

/*************************************************************************
 * Memory Address Equates
 * _RESET_ADDR      -- Reset Vector
 * _BEV_EXCPT_ADDR  -- Boot exception Vector
 * _DBG_EXCPT_ADDR  -- In-circuit Debugging Exception Vector
 * _DBG_CODE_ADDR   -- In-circuit Debug Executive address
 * _DBG_CODE_SIZE   -- In-circuit Debug Executive size
 * _GEN_EXCPT_ADDR  -- General Exception Vector
 *************************************************************************/
_RESET_ADDR              = 0x9D004000;
_BEV_EXCPT_ADDR          = 0x9D004380;
_DBG_EXCPT_ADDR          = 0x9D004480;
_DBG_CODE_ADDR           = 0xBFC02000;
_GEN_EXCPT_ADDR          = _ebase_address + 0x180;

/*************************************************************************
 * Memory Regions
 *
 * Memory regions without attributes cannot be used for orphaned sections.
 * Only sections specifically assigned to these regions can be allocated
 * into these regions.
 *************************************************************************/
MEMORY
{
  kseg0_program_mem    (rx)  : ORIGIN = 0x9D004A00, LENGTH = 0x3600
  kseg0_boot_mem             : ORIGIN = 0x9D004490, LENGTH = 0x570
  exception_mem              : ORIGIN = 0x9D003000, LENGTH = 0x1000
  kseg1_user_mem             : ORIGIN = 0xBFC00400, LENGTH = 0x800
  kseg1_boot_mem             : ORIGIN = 0x9D004000, LENGTH = 0x490
  debug_exec_mem             : ORIGIN = 0xBFC02000, LENGTH = 0xFF0
  config3                    : ORIGIN = 0xBFC02FF0, LENGTH = 0x4
  config2                    : ORIGIN = 0xBFC02FF4, LENGTH = 0x4
  config1                    : ORIGIN = 0xBFC02FF8, LENGTH = 0x4
  config0                    : ORIGIN = 0xBFC02FFC, LENGTH = 0x4
  kseg1_data_mem       (w!x) : ORIGIN = 0xA0000000, LENGTH = 0x2000
  sfrs                       : ORIGIN = 0xBF800000, LENGTH = 0x100000
}
SECTIONS
{
  .config_BFC02FF0 : {
    KEEP(*(.config_BFC02FF0))
  } > config3
  .config_BFC02FF4 : {
    KEEP(*(.config_BFC02FF4))
  } > config2
  .config_BFC02FF8 : {
    KEEP(*(.config_BFC02FF8))
  } > config1
  .config_BFC02FFC : {
    KEEP(*(.config_BFC02FFC))
  } > config0

 .extra_prg_mem :
  {
	* (extra_prg_mem)
  } >kseg1_user_mem  
 }
