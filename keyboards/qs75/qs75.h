#pragma once

#include "quantum.h"

#define LAYOUT( \
    k00, k01, k02, k03, k04, k05, k06, k07, k08, k09, k0a, k0b, k0c, k0d, k0e,\
    k10, k11, k12, k13, k14, k15, k16, k17, k18, k19, k1a, k1b, k1c, k1d, k1e,\
    k20, k21, k22, k23, k24, k25, k26, k27, k28, k29, k2a, k2b, k2c, k2d, k2e,\
    k30, k31, k32, k33, k34, k35, k36, k37, k38, k39, k3a, k3b, k3c, k3d, k3e, \
    k40, k41, k42, k43, k44, k45, k46, k47, k48, k49, k4a, k4b, k4c, k4d, k4e \
) \
{ \
    { k00, k10, k20, k30, k40 }, \
    { k01, k11, k21, k31, k41 }, \
    { k02, k12, k22, k32, k42 }, \
    { k03, k13, k23, k33, k43 }, \
    { k04, k14, k24, k34, k44 }, \
    { k05, k15, k25, k35, k45 }, \
    { k06, k16, k26, k36, k46 }, \
    { k07, k17, k27, k37, k47 }, \
    { k08, k18, k28, k38, k48 }, \
    { k09, k19, k29, k39, k49 }, \
    { k0a, k1a, k2a, k3a, k4a }, \
    { k0b, k1b, k2b, k3b, k4b }, \
    { k0c, k1c, k2c, k3c, k4c }, \
    { k0d, k1d, k2d, k3d, k4d }, \
    { k0e, k1e, k2e, k3e, k4e } \
}

#define LAYOUT_ortho_5x15 LAYOUT
