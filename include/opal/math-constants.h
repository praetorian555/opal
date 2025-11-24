#pragma once

#include <float.h>

#include "opal/defines.h"
#include "opal/types.h"

namespace Opal
{

static constexpr f32 k_pi_float = 3.14159265358979323846f;
static constexpr f32 k_inv_pi_float = 0.31830988618379067154f;
static constexpr f32 k_inv_2pi_float = 0.15915494309189533577f;
static constexpr f32 k_inv_4pi_float = 0.07957747154594766788f;
static constexpr f32 k_pi_over_2_float = 1.57079632679489661923f;
static constexpr f32 k_pi_over_4_float = 0.78539816339744830961f;
static constexpr f32 k_sqrt_2_float = 1.41421356237309504880f;
static constexpr f32 k_machine_epsilon_float = FLT_EPSILON;
static constexpr f64 k_pi_double =
    3.1415926535897932384626433832795028841971693993751058209749445923;
static constexpr f64 k_inv_pi_double =
    0.3183098861837906715377675267450287240689192914809128974953346881;
static constexpr f64 k_inv_2pi_double =
    0.1591549430918953357688837633725143620344596457404564487476673440;
static constexpr f64 k_inv_4pi_double =
    0.0795774715459476678844418816862571810172298228702282243738336720;
static constexpr f64 k_pi_over_2_double =
    1.5707963267948966192313216916397514420985846996875529104874722961;
static constexpr f64 k_pi_over_4_double =
    0.7853981633974483096156608458198757210492923498437764552437361480;
static constexpr f64 k_sqrt_2_double =
    1.4142135623730950488016887242096980785696718753769480731766797379;
static constexpr f64 k_machine_epsilon_double = DBL_EPSILON;

#if defined(OPAL_COMPILER_MSVC)
static constexpr f32 k_inf_float = static_cast<f32>(1e300 * 1e300);
static constexpr f32 k_neg_inf_float = -k_inf_float;
static constexpr f32 k_nan_float = __builtin_nanf("");
static constexpr f64 k_inf_double = 1e300 * 1e300;
static constexpr f64 k_neg_inf_double = -k_inf_double;
static constexpr f64 k_nan_double = __builtin_nan("");
#elif defined(OPAL_COMPILER_GCC) || defined(OPAL_COMPILER_CLANG)
static constexpr f32 k_inf_float = __builtin_inff();
static constexpr f32 k_neg_inf_float = -k_inf_float;
static constexpr f32 k_nan_float = __builtin_nanf("");
static constexpr f64 k_inf_double = __builtin_inf();
static constexpr f64 k_neg_inf_double = -k_inf_double;
static constexpr f64 k_nan_double = __builtin_nan("");
#else
#error No definition for infinity constants!
#endif

static constexpr f32 k_largest_float = 3.4028234664e38f;
static constexpr f32 k_smallest_float = -3.4028234664e38f;
static constexpr f64 k_largest_double = 1.7976931348623158e308;
static constexpr f64 k_smallest_double = -1.7976931348623158e308;

}  // namespace Opal
