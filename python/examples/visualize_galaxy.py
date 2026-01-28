#!/usr/bin/env python3
"""
OpenHo Galaxy Visualization Example

This script demonstrates how to use the openho_galaxy module to generate
and visualize different galaxy shapes.
"""

import openho_galaxy as og
import matplotlib.pyplot as plt
import numpy as np

def visualize_single_galaxy(n_planets=200, n_players=4, density=0.5, shape='RANDOM', seed=42):
	"""
	Generate and visualize a single galaxy.
	
	Parameters
	----------
	n_planets : int
		Number of planets to generate
	n_players : int
		Number of players
	density : float
		Planet distribution density (0.0-1.0)
	shape : str
		Galaxy shape (RANDOM, SPIRAL, CIRCLE, RING, CLUSTER, GRID)
	seed : int
		Random seed for reproducibility
	"""
	print(f"Generating {shape} galaxy with {n_planets} planets...")
	
	# Generate coordinates
	coords = og.generate_coordinates(n_planets, n_players, density, shape, seed)
	
	print(f"Generated {len(coords)} planet coordinates")
	
	# Create visualization
	plt.figure(figsize=(10, 10))
	plt.scatter(coords[:, 0], coords[:, 1], s=20, alpha=0.6, c='blue', edgecolors='black', linewidth=0.5)
	plt.title(f'{shape} Galaxy ({len(coords)} planets, density={density})', fontsize=16)
	plt.xlabel('X Coordinate')
	plt.ylabel('Y Coordinate')
	plt.axis('equal')
	plt.grid(True, alpha=0.3)
	plt.tight_layout()
	plt.show()

def compare_galaxy_shapes(n_planets=150, n_players=4, density=0.5, seed=42):
	"""
	Generate and compare all available galaxy shapes.
	
	Parameters
	----------
	n_planets : int
		Number of planets to generate for each shape
	n_players : int
		Number of players
	density : float
		Planet distribution density (0.0-1.0)
	seed : int
		Random seed for reproducibility
	"""
	shapes = ['RANDOM', 'CIRCLE', 'RING', 'GRID']
	
	fig, axes = plt.subplots(2, 2, figsize=(14, 14))
	axes = axes.flatten()
	
	for idx, shape in enumerate(shapes):
		print(f"Generating {shape} galaxy...")
		
		try:
			coords = og.generate_coordinates(n_planets, n_players, density, shape, seed)
			
			ax = axes[idx]
			ax.scatter(coords[:, 0], coords[:, 1], s=15, alpha=0.6, c='blue', edgecolors='black', linewidth=0.5)
			ax.set_title(f'{shape} ({len(coords)} planets)', fontsize=14, fontweight='bold')
			ax.set_xlabel('X Coordinate')
			ax.set_ylabel('Y Coordinate')
			ax.axis('equal')
			ax.grid(True, alpha=0.3)
			
			print(f"  Generated {len(coords)} coordinates")
		except Exception as e:
			print(f"  Error: {e}")
			axes[idx].text(0.5, 0.5, f'Error:\n{shape}\n{str(e)}', 
			              ha='center', va='center', transform=axes[idx].transAxes)
	
	plt.tight_layout()
	plt.savefig('galaxy_shapes_comparison.png', dpi=150, bbox_inches='tight')
	print("\nSaved comparison to 'galaxy_shapes_comparison.png'")
	plt.show()

def explore_density_effects(n_planets=150, n_players=4, shape='RANDOM', seed=42):
	"""
	Explore how density parameter affects galaxy generation.
	
	Parameters
	----------
	n_planets : int
		Number of planets to generate
	n_players : int
		Number of players
	shape : str
		Galaxy shape to test
	seed : int
		Random seed for reproducibility
	"""
	densities = [0.2, 0.4, 0.6, 0.8]
	
	fig, axes = plt.subplots(2, 2, figsize=(14, 14))
	axes = axes.flatten()
	
	for idx, density in enumerate(densities):
		print(f"Generating {shape} galaxy with density={density}...")
		
		coords = og.generate_coordinates(n_planets, n_players, density, shape, seed)
		
		ax = axes[idx]
		ax.scatter(coords[:, 0], coords[:, 1], s=15, alpha=0.6, c='blue', edgecolors='black', linewidth=0.5)
		ax.set_title(f'{shape} (density={density}, {len(coords)} planets)', fontsize=14, fontweight='bold')
		ax.set_xlabel('X Coordinate')
		ax.set_ylabel('Y Coordinate')
		ax.axis('equal')
		ax.grid(True, alpha=0.3)
		
		print(f"  Generated {len(coords)} coordinates")
	
	plt.tight_layout()
	plt.savefig('density_comparison.png', dpi=150, bbox_inches='tight')
	print("\nSaved comparison to 'density_comparison.png'")
	plt.show()

def main():
	"""Main function with example usage."""
	print("=" * 70)
	print("OpenHo Galaxy Visualization Examples")
	print("=" * 70)
	print()
	
	# Example 1: Single galaxy visualization
	print("Example 1: Visualizing a CIRCLE galaxy")
	print("-" * 70)
	visualize_single_galaxy(n_planets=200, shape='CIRCLE', seed=42)
	print()
	
	# Example 2: Compare all shapes
	print("Example 2: Comparing different galaxy shapes")
	print("-" * 70)
	compare_galaxy_shapes(n_planets=150, seed=42)
	print()
	
	# Example 3: Explore density effects
	print("Example 3: Exploring density parameter effects")
	print("-" * 70)
	explore_density_effects(n_planets=150, shape='RANDOM', seed=42)
	print()
	
	print("=" * 70)
	print("All examples completed!")
	print("=" * 70)

if __name__ == '__main__':
	main()
