#include "economic_state.h"

#include <cmath>
#include <limits>

namespace EconomicAllocation
{

bool adjust_fraction_proportionally(
    double value,
    double& selected,
    const std::vector<double*>& peer_fractions,
    std::string& error)
{
    constexpr double epsilon = std::numeric_limits<double>::epsilon();
    if (!std::isfinite(value) || value < 0.0 || value > 1.0)
    {
        error = "fraction must be finite and between 0 and 1";
        return false;
    }

    double peer_total = 0.0;
    for (const double* fraction : peer_fractions)
    {
        if (fraction == nullptr || !std::isfinite(*fraction) || *fraction < 0.0)
        {
            error = "existing allocation is invalid";
            return false;
        }
        peer_total += *fraction;
    }

    const double current_total = selected + peer_total;
    if (!std::isfinite(selected) || selected < 0.0 ||
        std::abs(current_total - 1.0) > epsilon)
    {
        error = "existing allocation is not normalized";
        return false;
    }

    const double new_peer_total = 1.0 - value;
    if (peer_total <= epsilon)
    {
        if (new_peer_total > epsilon)
        {
            error = "cannot redistribute from zero-valued allocations";
            return false;
        }
        selected = value;
        for (double* fraction : peer_fractions)
            *fraction = 0.0;
        return true;
    }

    const double scale = new_peer_total / peer_total;
    selected = value;
    for (double* fraction : peer_fractions)
        *fraction *= scale;
    return true;
}

} // namespace EconomicAllocation
