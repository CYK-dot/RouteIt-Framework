/**
 * @file rti_internal.h
 * @author CYK-Dot
 * @brief Brief description
 * @version 0.1
 * @date 2025-10-04
 *
 * @copyright Copyright (c) 2025 CYK-Dot, all rights reserved.
 */
#pragma once

/* Header import ------------------------------------------------------------------*/
#include <stdint.h>

/* Config macros -----------------------------------------------------------------*/

#define RTI_TYPE_SECTION_VLAN __attribute__((section(".rti_vlan")))
#define RTI_TYPE_SECTION_VLAN_USED __attribute__((section(".rti_vlan"), used))

#define RTI_SYMBOL_SECTION_VLAN_START _rti_vlan_start
#define RTI_SYMBOL_SECTION_VLAN_STOP _rti_vlan_stop

/* Export macros -----------------------------------------------------------------*/

/* Exported typedef --------------------------------------------------------------*/

typedef enum rti_internal_errs {
    RTI_OK = 0,
    RTI_ERR_INVALID_PARAM = 1,
    RTI_ERR_NOT_SUPPORTED,
    RTI_ERR_FAILED,
    RTI_ERR_OBJECT_EMPTY,
} RTI_ERR;

/* C++ ---------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

/* Exported function -------------------------------------------------------------*/

/* C++ ---------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif