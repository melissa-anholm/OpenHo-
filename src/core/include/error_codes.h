#ifndef OPENHO_ERROR_CODES_H
#define OPENHO_ERROR_CODES_H

#include <cstdint>

/**
 * Error codes for C API operations
 * 0 = Success, non-zero = specific error
 */
enum class ErrorCode : uint32_t
{
	// Success
	SUCCESS = 0,
	
	// Generic errors
	UNKNOWN_ERROR = 1,
	INVALID_PLAYER_ID = 2,
	INVALID_PLANET_ID = 3,
	INVALID_FLEET_ID = 4,
	INVALID_DESIGN_ID = 5,
	
	// Player state errors
	INSUFFICIENT_MONEY = 10,
	INSUFFICIENT_METAL = 11,
	INSUFFICIENT_FUNDS = 12,  // Either money or metal
	INVALID_TECH_LEVEL = 13,
	TECH_LEVEL_NOT_AVAILABLE = 14,
	TECH_LEVEL_OUT_OF_RANGE = 15,
	
	// Ship design errors
	INVALID_SHIP_TYPE = 20,
	SHIP_DESIGN_LIMIT_REACHED = 21,  // Max 100 designs per player
	SHIP_DESIGN_NOT_FOUND = 22,
	INVALID_DESIGN_NAME = 23,
	DESIGN_NAME_DUPLICATE = 24,
	
	// Fleet errors
	FLEET_NOT_FOUND = 30,
	INVALID_FLEET_SIZE = 31,
	FLEET_LIMIT_REACHED = 32,
	FLEET_IN_TRANSIT = 33,  // Can't perform action on fleet in transit
	
	// Planet errors
	PLANET_NOT_OWNED = 40,
	PLANET_NOT_FOUND = 41,
	PLANET_ALREADY_OWNED = 42,
	PLANET_NOT_COLONIZED = 43,
	
	// Turn/game state errors
	GAME_NOT_READY = 50,
	PLAYER_ALREADY_READY = 51,
	INVALID_TURN_STATE = 52,
	
	// Validation errors
	VALIDATION_FAILED = 60,
	INVALID_ALLOCATION = 61,  // Spending allocation doesn't sum to 1.0
	INVALID_PARAMETER = 62,
};

/**
 * Helper function to get human-readable error message
 * (For debugging; UI should map error codes to localized strings)
 */
inline const char* error_code_to_string(ErrorCode code)
{
	switch (code)
	{
		case ErrorCode::SUCCESS:
			return "Success";
		case ErrorCode::UNKNOWN_ERROR:
			return "Unknown error";
		case ErrorCode::INVALID_PLAYER_ID:
			return "Invalid player ID";
		case ErrorCode::INVALID_PLANET_ID:
			return "Invalid planet ID";
		case ErrorCode::INVALID_FLEET_ID:
			return "Invalid fleet ID";
		case ErrorCode::INVALID_DESIGN_ID:
			return "Invalid design ID";
		case ErrorCode::INSUFFICIENT_MONEY:
			return "Insufficient money";
		case ErrorCode::INSUFFICIENT_METAL:
			return "Insufficient metal";
		case ErrorCode::INSUFFICIENT_FUNDS:
			return "Insufficient funds";
		case ErrorCode::INVALID_TECH_LEVEL:
			return "Invalid technology level";
		case ErrorCode::TECH_LEVEL_NOT_AVAILABLE:
			return "Technology level not available";
		case ErrorCode::TECH_LEVEL_OUT_OF_RANGE:
			return "Technology level out of range";
		case ErrorCode::INVALID_SHIP_TYPE:
			return "Invalid ship type";
		case ErrorCode::SHIP_DESIGN_LIMIT_REACHED:
			return "Ship design limit reached";
		case ErrorCode::SHIP_DESIGN_NOT_FOUND:
			return "Ship design not found";
		case ErrorCode::INVALID_DESIGN_NAME:
			return "Invalid design name";
		case ErrorCode::DESIGN_NAME_DUPLICATE:
			return "Design name already exists";
		case ErrorCode::FLEET_NOT_FOUND:
			return "Fleet not found";
		case ErrorCode::INVALID_FLEET_SIZE:
			return "Invalid fleet size";
		case ErrorCode::FLEET_LIMIT_REACHED:
			return "Fleet limit reached";
		case ErrorCode::FLEET_IN_TRANSIT:
			return "Fleet is in transit";
		case ErrorCode::PLANET_NOT_OWNED:
			return "Planet not owned by player";
		case ErrorCode::PLANET_NOT_FOUND:
			return "Planet not found";
		case ErrorCode::PLANET_ALREADY_OWNED:
			return "Planet already owned";
		case ErrorCode::PLANET_NOT_COLONIZED:
			return "Planet not colonized";
		case ErrorCode::GAME_NOT_READY:
			return "Game not ready";
		case ErrorCode::PLAYER_ALREADY_READY:
			return "Player already ready";
		case ErrorCode::INVALID_TURN_STATE:
			return "Invalid turn state";
		case ErrorCode::VALIDATION_FAILED:
			return "Validation failed";
		case ErrorCode::INVALID_ALLOCATION:
			return "Invalid allocation";
		case ErrorCode::INVALID_PARAMETER:
			return "Invalid parameter";
		default:
			return "Unknown error code";
	}
}

#endif // OPENHO_ERROR_CODES_H
