import matplotlib.pyplot as plt

# Corrected data with proper execution times
data = {}
data["floyd-warshal"] = [0.00, 0.14, 1.12, 3.72, 8.76]  # Times in seconds

# Fox execution times in seconds. The failed test for input size 6 is not included.
data["fox-16"] = [None, 1.97, 2.92, 5.46, 10.86]
data["fox-9"] = [1.82, 2.05, 3.60, 7.81, 17.34]
data["fox-4"] = [1.73, 2.15, 5.25, 14.56, 34.39]
data["fox-1"] = [1.70, 3.25, 15.59, 49.63, 125.14]

input_sizes = [6, 300, 600, 900, 1200]

# Plotting the graph with a logarithmic scale and specified colors
plt.figure(figsize=(14, 8))

# Floyd-Warshal plot with red color
plt.plot(input_sizes, data["floyd-warshal"], marker='o', color='red', label='Floyd-Warshal')

# Fox plots with specified colors
# We will use a lambda function to filter out 'None' values for plotting
plt.plot([size for size, time in zip(input_sizes, data["fox-16"]) if time is not None], 
         [time for time in data["fox-16"] if time is not None], 
         marker='s', color='blue', label='Fox 16 procs')

plt.plot(input_sizes, data["fox-9"], marker='^', color='yellow', label='Fox 9 procs')
plt.plot(input_sizes, data["fox-4"], marker='x', color='orange', label='Fox 4 procs')
plt.plot(input_sizes, data["fox-1"], marker='d', color='green', label='Fox 1 proc')  # Added green for contrast

# Setting logarithmic scale for the y-axis
plt.yscale('log')

# Titles and labels
plt.title('Execution Time Comparison on a Logarithmic Scale')
plt.xlabel('Input Size')
plt.ylabel('Execution Time (seconds, log scale)')
plt.legend()
plt.grid(True, which="both", ls="--")

# Show plot
# plt.savefig('performance-table-pic.png')
plt.show()

