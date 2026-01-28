import openho_galaxy as og
import matplotlib.pyplot as plt

# Generate coordinates
coords = og.generate_coordinates(
    n_planets=200,
    n_players=4,
    density=0.5,
    shape='CIRCLE',
    seed=42
)

# Plot them
plt.scatter(coords[:, 0], coords[:, 1], s=10)
plt.axis('equal')
plt.title('Circle Galaxy')
plt.show()