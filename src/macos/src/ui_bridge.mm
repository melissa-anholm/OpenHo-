#include "openho_core.h"
#include "ui_adapter.h"

#include <cstddef>

// Compile-time/use-site check that the native adapter can see the portable,
// value-oriented UI boundary without retaining C++ domain-object pointers.
extern "C" std::size_t openho_macos_ui_adapter_boundary_size()
{
    return sizeof(UiGameSnapshot) + sizeof(UiEconomicSubmission) + sizeof(GameUiAdapter);
}

// A small core symbol reference ensures the native target actually links the
// OpenHoCore archive rather than merely compiling AppKit code in isolation.
extern "C" uint32_t openho_macos_core_boundary_probe()
{
    return game_get_current_turn(nullptr);
}
