import re
import sys

# read log file from argument
if len(sys.argv) != 4:
  print(
      'Usage: python collect.py <symbol_log_file> <bmcm23_poster_log_file>'
      ' <bmcm23_log_file>'
  )
  sys.exit(1)

# has the form index : [value]
noise_index_to_value = {}


def analyse_log(log_file_name):
  try:
    with open(log_file_name, 'r') as file:
      log_data = file.read()
  except FileNotFoundError:
    print(f"Error: File '{log_file_name}' not found.")
    sys.exit(1)

  noise_bounds = []

  prev_noise = None
  for line in log_data.splitlines():
    noise_match = re.search(r'noise:\s*([\d.]+)', line)
    noise_bound_match = re.search(r'noise bound:\s*([\d.]+)', line)
    if noise_match:
      prev_noise = float(noise_match.group(1))
    if noise_bound_match:
      noise_bound = float(noise_bound_match.group(1))
      if prev_noise is not None:
        if noise_bound not in noise_bounds:
          noise_bounds.append(noise_bound)
        index = noise_bounds.index(noise_bound)
        if index not in noise_index_to_value:
          noise_index_to_value[index] = []
        noise_index_to_value[index].append(prev_noise)

  return noise_bounds


symbol_noise_bounds = analyse_log(sys.argv[1])
bmcm23_poster_noise_bounds = analyse_log(sys.argv[2])
bmcm23_noise_bounds = analyse_log(sys.argv[3])

x_axis = range(1, len(noise_index_to_value) + 1)

# for each bound, calculate the max and min noise
max_noise = {}
min_noise = {}
average_noise = {}

for index, noises in noise_index_to_value.items():
  max_noise[index] = max(noises)
  min_noise[index] = min(noises)
  average_noise[index] = sum(noises) / len(noises)

# Plot the noise and noise bound
import matplotlib.pyplot as plt

# Prepare data for plotting
max_noises = [max_noise[index] for index in max_noise]
min_noises = [min_noise[index] for index in min_noise]
average_noises = [average_noise[index] for index in average_noise]

# minus average for max and min and other bounds
max_noises = [
    max_noise[index] - average_noise[index] for index in average_noise
]
min_noises = [
    min_noise[index] - average_noise[index] for index in average_noise
]
symbol_noise_bounds = [
    symbol_noise_bounds[index] - average_noise[index] for index in average_noise
]
bmcm23_poster_noise_bounds = [
    bmcm23_poster_noise_bounds[index] - average_noise[index]
    for index in average_noise
]
bmcm23_noise_bounds = [
    bmcm23_noise_bounds[index] - average_noise[index] for index in average_noise
]

# Plot max and min noise for each bound
plt.figure(figsize=(10, 6))
plt.plot(x_axis, max_noises, label='Max Noise', marker='v')
plt.plot(x_axis, min_noises, label='Min Noise', marker='^')
plt.plot(x_axis, symbol_noise_bounds, label='Symbol', marker='1')
plt.plot(x_axis, bmcm23_poster_noise_bounds, label='BMCM23 Poster', marker='2')
plt.plot(x_axis, bmcm23_noise_bounds, label='BMCM23', marker='3')

plt.legend()
plt.xlabel('Index')
plt.ylabel('Noise Diff')
plt.grid(True)
plt.savefig('noise.pdf')
