#import <AppKit/AppKit.h>

#include <cstddef>
#include <cstdint>

extern "C" std::size_t openho_macos_ui_adapter_boundary_size();
extern "C" uint32_t openho_macos_core_boundary_probe();

int main()
{
    @autoreleasepool
    {
        NSApplication *application = [NSApplication sharedApplication];
        [application setActivationPolicy:NSApplicationActivationPolicyProhibited];

        // Keep this first target intentionally non-interactive.  It validates
        // the AppKit lifecycle, Objective-C++ bridge, and portable core link;
        // window/controller work belongs to the next UI milestone.
        const std::size_t boundary_size = openho_macos_ui_adapter_boundary_size();
        const uint32_t current_turn = openho_macos_core_boundary_probe();
        (void)application;
        (void)boundary_size;
        (void)current_turn;
    }
    return 0;
}
