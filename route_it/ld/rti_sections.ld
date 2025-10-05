/**
 * @brief RouteIt框架主链接脚本
 * @note 使用方式
 * 在主链接脚本的SECTIONS中INCLUDE本文件
 * 
 * 内存需求: 
 * - 需要提供RTI_VLAN_REGION内存区域
 * - 或者默认放置在.data段之后
 */
SECTIONS {
    .rti_vlan : {
        . = ALIGN(8);
        PROVIDE(__start_rti_vlan = .);
        KEEP(*(SORT(.rti_vlan.*)))
        PROVIDE(__end_rti_vlan = .);
        . = ALIGN(8);
    }
}