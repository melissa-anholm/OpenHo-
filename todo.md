# OpenHo Project TODO

## Session 7 - Distance Matrix Implementation

### Distance Matrix Caching (Session 7 - Current)
- [x] Implement distance matrix in Galaxy class
  - [x] Add `distance_matrix` member (2D vector)
  - [x] Add `compute_distance_matrix()` method
  - [x] Add `get_distance(from_id, to_id)` method
  - [x] Call `compute_distance_matrix()` in Galaxy constructor
  - [x] Use Euclidean distance formula with rounding to integer
  
- [x] Implement distance matrix in KnowledgeGalaxy
  - [x] Add `distance_matrix` member (local copy)
  - [x] Copy distance matrix from Galaxy in constructor
  - [x] Add `get_distance(from_id, to_id)` method
  - [x] Provide O(1) local access with no network latency
  
- [x] Verify compilation
  - [x] Project compiles successfully
  - [x] No new errors introduced

### Future Work

#### Phase 2d - KnowledgePlanet Integration
- [ ] Integrate KnowledgeGalaxy initialization into GameState constructor
- [ ] Initialize homeworld with full knowledge
- [ ] Implement initial knowledge updates for all players
- [ ] Add helper methods to Player for common KnowledgeGalaxy queries
- [ ] Update game logic to use KnowledgeGalaxy for planet queries
- [ ] Implement fog of war rules for knowledge updates
- [ ] Add observation mechanics (exploration, colonization, etc.)
- [ ] Implement fleet visibility tracking in KnowledgePlanet

#### Phase 2e - Future Enhancements
- [ ] Implement fog of war based on fleet positions
- [ ] Add sensor range calculations for fleet observation
- [ ] Implement knowledge decay/staleness tracking
- [ ] Add knowledge sharing between allied players
- [ ] Implement espionage/scouting mechanics

#### Phase 3 - UI Integration
- [ ] Create distance query UI components
- [ ] Implement route planning using distance matrix
- [ ] Add distance display in planet information panels
- [ ] Create fleet movement UI using distance calculations

#### Phase 4 - Testing & Optimization
- [ ] Write unit tests for distance calculations
- [ ] Verify distance matrix accuracy
- [ ] Profile memory usage for various galaxy sizes
- [ ] Optimize distance matrix if needed

## Architecture Notes

### Distance Matrix Design
- **Storage**: 2D vector `std::vector<std::vector<double>>` in both Galaxy and KnowledgeGalaxy
- **Calculation**: Euclidean distance `sqrt((x2-x1)² + (y2-y1)²)` rounded to nearest integer
- **Access Pattern**: O(1) lookup via `get_distance(from_id, to_id)`
- **Initialization**: Computed once in Galaxy constructor, copied to each player's KnowledgeGalaxy
- **Network Impact**: Zero network latency for distance queries (all local lookups)

### Key Benefits
✅ O(1) distance lookups for UI responsiveness  
✅ No network latency for common queries  
✅ Single source of truth in Galaxy  
✅ Each player has independent copy for self-sufficiency  
✅ Euclidean distance with integer rounding for consistency  

