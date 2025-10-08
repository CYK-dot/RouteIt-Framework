. = ALIGN(8);
__start_rti_vlan = .;
KEEP(*(.rti_vlan))
KEEP(*(.rti_vlan.*))
__end_rti_vlan = .;
. = ALIGN(8);