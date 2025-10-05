/**
 * @file rti_vlan.h
 * @author CYK-Dot
 * @brief Brief description
 * @version 0.1
 * @date 2025-10-04
 *
 * @copyright Copyright (c) 2025 CYK-Dot, all rights reserved.
 */
#pragma once

/* Header import ------------------------------------------------------------------*/
#include "rti_internal.h"

/* Config macros -----------------------------------------------------------------*/

/* Export macros -----------------------------------------------------------------*/

#define RTI_VLAN_REGISTER_STATIC(VLAN_IFX_ADDRESS, VLAN_NAME) \
    RTI_TYPE_SECTION_VLAN_USED const RTI_VLAN_DESC RTI_VLAN_##VLAN_NAME = { \
        .ifx = VLAN_IFX_ADDRESS, \
        .name = #VLAN_NAME, \
        .id = RTI_VLANID_##VLAN_NAME, \
    }

/* Exported typedef --------------------------------------------------------------*/

typedef void* (*RTI_VlanCreateFptr)(void);
typedef void (*RTI_VlanDeleteFptr)(void* vlan);
typedef void* (*RTI_VlanCreateProducerFptr)(void);
typedef void (*RTI_VlanDeleteProducerFptr)(void* producer);
typedef void* (*RTI_VlanCreateConsumerFptr)(void);
typedef void (*RTI_VlanDeleteConsumerFptr)(void* consumer);
typedef uint16_t RTI_VlanId;

typedef struct {
    RTI_VlanCreateFptr createF;
    RTI_VlanDeleteFptr deleteF;
    RTI_VlanCreateProducerFptr createProducerF;
    RTI_VlanDeleteProducerFptr deleteProducerF;
    RTI_VlanCreateConsumerFptr createConsumerF;
    RTI_VlanDeleteConsumerFptr deleteConsumerF;
} RTI_VLAN_IFX;

typedef struct {
    RTI_VLAN_IFX ifx;
    char *name;
    RTI_VlanId id;
} RTI_VLAN_DESC;

/* C++ ---------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

/* Exported function -------------------------------------------------------------*/

RTI_ERR RTIPriv_VlanSelect(RTI_VlanId id, RTI_VLAN_DESC *vlan);

/* C++ ---------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif