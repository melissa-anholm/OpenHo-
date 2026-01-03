#ifndef OPENHO_TEMPERATURE_UTILS_H
#define OPENHO_TEMPERATURE_UTILS_H

// ============================================================================
// Temperature Conversion Utilities
// ============================================================================
// All internal game temperatures are stored in Kelvin.
// These utilities convert between Kelvin and other temperature scales
// for display and user interaction.

namespace TemperatureUtils
{
	// ========================================================================
	// Conversion Constants
	// ========================================================================
	
	/// Absolute zero in Kelvin (0 K = -273.15°C = -459.67°F)
	constexpr double ABSOLUTE_ZERO_K = 0.0;
	
	/// Celsius to Kelvin offset
	constexpr double CELSIUS_OFFSET = 273.15;
	
	/// Fahrenheit conversion factor
	constexpr double FAHRENHEIT_FACTOR = 9.0 / 5.0;
	
	// ========================================================================
	// Conversion Functions
	// ========================================================================
	
	/// Convert temperature from Kelvin to Celsius
	/// Formula: °C = K - 273.15
	inline double celsius_from_kelvin(double temp_k)
	{
		return temp_k - CELSIUS_OFFSET;
	}
	
	/// Convert temperature from Kelvin to Fahrenheit
	/// Formula: °F = (K - 273.15) × 9/5 + 32
	inline double fahrenheit_from_kelvin(double temp_k)
	{
		double celsius = celsius_from_kelvin(temp_k);
		return celsius * FAHRENHEIT_FACTOR + 32.0;
	}
	
	/// Convert temperature from Celsius to Kelvin
	/// Formula: K = °C + 273.15
	inline double kelvin_from_celsius(double temp_c)
	{
		return temp_c + CELSIUS_OFFSET;
	}
	
	/// Convert temperature from Fahrenheit to Kelvin
	/// Formula: K = (°F - 32) × 5/9 + 273.15
	inline double kelvin_from_fahrenheit(double temp_f)
	{
		double celsius = (temp_f - 32.0) / FAHRENHEIT_FACTOR;
		return kelvin_from_celsius(celsius);
	}
	
	/// Convert temperature from Celsius to Fahrenheit
	/// Formula: °F = °C × 9/5 + 32
	inline double fahrenheit_from_celsius(double temp_c)
	{
		return temp_c * FAHRENHEIT_FACTOR + 32.0;
	}
	
	/// Convert temperature from Fahrenheit to Celsius
	/// Formula: °C = (°F - 32) × 5/9
	inline double celsius_from_fahrenheit(double temp_f)
	{
		return (temp_f - 32.0) / FAHRENHEIT_FACTOR;
	}
	
}  // namespace TemperatureUtils

#endif  // OPENHO_TEMPERATURE_UTILS_H
