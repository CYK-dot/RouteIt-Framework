SECTIONS {
    .rti_vlan BLOCK(__section_alignment__) : {
        INCLUDE "rti_mbed_linker_rtivlan.ld"
    }
}
INSERT AFTER .text