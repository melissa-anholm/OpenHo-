# Poisson Disk Sampling Overview

**Purpose:** Reference documentation for implementing Poisson disk sampling in OpenHo galaxy generation algorithms.

**Current Status:** Planned for future implementation in Circle, Ring, and other galaxy shapes.

---

## **What is Poisson Disk Sampling?**

Poisson disk sampling is a method for generating a random set of points in a space such that:
1. Points are randomly distributed (not regular/grid-like)
2. No two points are closer than a minimum distance **r**
3. The distribution is relatively uniform (no large empty regions)

This creates a "blue noise" distribution - random but well-spaced.

---

## **Mathematical Foundation**

### **Poisson Point Process (Baseline)**
A Poisson point process with intensity λ generates random points where:
- Expected number of points in region A: **N = λ × Area(A)**
- Points are completely random (no spacing constraint)
- Nearest-neighbor distances follow exponential distribution

### **Minimum Distance Constraint**
For Poisson disk sampling with minimum distance **r**:
- No two points can be closer than distance **r**
- This is the "hard constraint" that distinguishes it from pure Poisson

### **Key Formula: Disk Radius and Point Density**

For a region with n points and minimum spacing r:

**Maximum achievable density:**
```
ρ_max = 1 / (π × r²)
```
This is the theoretical maximum packing density (hexagonal close packing).

**Expected number of neighbors within radius R:**
```
E[neighbors(R)] = λ × π × R²
```
where λ is the point intensity (points per unit area).

**Relationship between intensity and minimum distance:**
```
λ ≈ 1 / (π × r²)  (for optimal packing)
```

---

## **Algorithm: Bridson's Fast Poisson Disk Sampling**

The most practical algorithm (Bridson, 2007):

### **Setup:**
```
r = minimum_distance
cell_size = r / √2  (approximately r/1.414)
grid_width = domain_width / cell_size
grid_height = domain_height / cell_size
```

### **Algorithm Steps:**

1. **Initialize:**
   - Create a 2D grid of cells (size = r/√2)
   - Create an empty list of points
   - Create an "active list" of candidate points

2. **Generate first point:**
   - Pick random position (x₀, y₀) in domain
   - Add to points list and active list
   - Mark grid cell as occupied

3. **Main loop (while active list not empty):**
   ```
   while active_list is not empty:
       idx = random index in active_list
       found = false
       
       for attempt = 0 to k (typically k=30):
           angle = random(0, 2π)
           distance = random(r, 2r)
           
           new_x = active_point.x + distance × cos(angle)
           new_y = active_point.y + distance × sin(angle)
           
           if is_valid(new_x, new_y):
               add (new_x, new_y) to points and active_list
               mark grid cell as occupied
               found = true
               break
       
       if not found:
           remove active_point from active_list
   ```

4. **Validity Check:**
   ```
   is_valid(x, y):
       if (x, y) outside domain:
           return false
       
       cell_x = floor(x / cell_size)
       cell_y = floor(y / cell_size)
       
       // Check 3×3 neighborhood of cells
       for dx = -2 to 2:
           for dy = -2 to 2:
               check_cell_x = cell_x + dx
               check_cell_y = cell_y + dy
               
               if grid[check_cell_x][check_cell_y] occupied:
                   other_point = grid[check_cell_x][check_cell_y]
                   distance = sqrt((x - other_point.x)² + (y - other_point.y)²)
                   
                   if distance < r:
                       return false
       
       return true
   ```

---

## **Key Parameters**

| Parameter | Meaning | Typical Value |
|-----------|---------|---------------|
| **r** | Minimum distance between points | Problem-dependent |
| **cell_size** | Grid cell size | r / √2 ≈ 0.707r |
| **k** | Max attempts per active point | 30 (higher = better quality, slower) |
| **search_radius** | How far to search for neighbors | 2r (covers 3×3 grid cells) |

---

## **Complexity Analysis**

- **Time Complexity:** O(n) where n = number of points
  - Each point is added once to active list
  - Each point attempts k placements
  - Each placement checks O(1) grid cells
  
- **Space Complexity:** O(n) for grid and point storage

---

## **Why It's Better Than Naive Random**

**Naive Random (current approach):**
- Generate random position
- Check distance to all existing points: O(n)
- Rejection rate increases as galaxy fills: O(n) rejections
- Total time: O(n²) in worst case

**Poisson Disk (Bridson's algorithm):**
- Grid-accelerated neighbor search: O(1) per check
- Active list prevents checking far-away points
- Rejection rate stays bounded: max k attempts per point
- Total time: O(n) guaranteed

---

## **For Galaxy Generation in OpenHo**

### **Mapping to our parameters:**
```
r = 4.0  (minimum planet distance)
cell_size = 4.0 / √2 ≈ 2.83
k = 30 (or adjust based on desired quality)
domain = [0, gal_size] × [0, gal_size]
```

### **Expected behavior:**
- Density=2.0: Tighter packing, more neighbors within radius 6
- Density=0.5: Looser packing, fewer neighbors within radius 12
- No placement failures (or very few)
- Uniform distribution (no empty clusters)

---

## **Visual Intuition**

```
Naive Random:          Poisson Disk:
●    ●                 ●   ●
  ●  ●  ●              ●     ●
●       ●    ●         ●   ●   ●
  ●  ●                   ●   ●
●                      ●       ●
(clustered, sparse)    (uniform, well-spaced)
```

---

## **Implementation Notes for OpenHo**

### **Reusable Components:**
- `CheckDistanceSpatialGrid` can be adapted for Poisson disk sampling
- `DeterministicRNG` already provides reproducible randomness
- Existing planet property generation (gravity, temperature, metal) can be reused

### **Integration Points:**
- `Galaxy::initialize_planets_circle()` - Primary candidate
- `Galaxy::initialize_planets_ring()` - Secondary candidate
- Potentially `initialize_planets_spiral()` and `initialize_planets_cluster()`

### **Future Enhancements:**
- Adaptive k based on placement difficulty
- Constrained regions (e.g., planets preferring certain areas)
- Multi-scale Poisson disk sampling for hierarchical structures

---

## **References**

- **Bridson, R.** (2007). "Fast Poisson Disk Sampling in Arbitrary Dimensions." SIGGRAPH Sketches.
- **Lagae, A., & Dutré, P.** (2008). "A Comparison of Methods for Generating Poisson Disk Distributions."
- **Dunbar, D., & Humphreys, G.** (2006). "A Spatial Data Structure for Fast Poisson-Disk Sample Generation."

---

## **Status in OpenHo Project**

**Current Phase:** 2c (Galaxy Initialization)
**Planned Implementation:** Phase 2c-Extended or Phase 2d
**Target Galaxy Shapes:** Circle (primary), Ring, Spiral, Cluster (secondary)
**Priority:** Medium (after random and grid shapes are complete)
